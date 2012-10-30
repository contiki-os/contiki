/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
#include "contiki.h"
#include "lib/sensors.h"
#include "dev/irq.h"
#include "dev/lpm.h"
#include "isr_compat.h"

#define ADC12MCTL_NO(adcno) ((unsigned char *) ADC12MCTL0_)[adcno]

static int (* adc12_irq[8])(void);
static int (* port1_irq[8])(void);
static unsigned char adcflags;

/*---------------------------------------------------------------------------*/
ISR(PORT1, irq_p1)
{
  int i;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  for(i = 0; i < 8; i++) {
    if((P1IFG & (1 << i)) && port1_irq[i] != NULL) {
      if((port1_irq[i])()) {
        LPM4_EXIT;
      }
    }
  }
  P1IFG = 0x00;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
ISR(ADC, irq_adc)
{
  int i;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  for(i = 0; i < 8; i++) {
    if(adc12_irq[i] != NULL) {
      if((adc12_irq[i])()) {
        LPM4_EXIT;
      }
    }
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
irq_init(void)
{
  int i;
  adcflags = 0;
  for(i = 0; i < 8; i++) {
    adc12_irq[i] = NULL;
    port1_irq[i] = NULL;
  }
  /* Setup ADC12, ref., sampling time */
  ADC12CTL0 = REF2_5V | SHT0_10 | SHT1_10 | MSC;

  /* Use sampling timer, repeat-sequence-of-channels */
/*   ADC12CTL1 = SHP | CONSEQ_3 | ADC12DIV_3; */
  ADC12CTL1 = SHP | CONSEQ_3 | ADC12DIV_7;
}
/*---------------------------------------------------------------------------*/
void
irq_port1_activate(unsigned char irqno, int (* irq)(void))
{
  if(irqno < 8) {
    port1_irq[irqno] = irq;
  }
}
/*---------------------------------------------------------------------------*/
void
irq_port1_deactivate(unsigned char irqno)
{
  if(irqno < 8) {
    port1_irq[irqno] = NULL;
  }
}
/*---------------------------------------------------------------------------*/
/* Set lowest ADC to be start in sequence and highest to be interrupt
   enabled and set end-of-sequence on the highest active ADC */
static void
sethilo(void)
{
  int c;

  /* Clear start of sequence */
  ADC12CTL1 &= ~(CSTARTADD_15);

  /* Set new start of sequence to lowest active memory holder */
  for(c = 0; c < 8; c++) {
    if(adcflags & (1 << c)) {
      ADC12CTL1 |= (c * CSTARTADD_1);
      break;
    }
  }

  /* Clear all interrupts and end-of-sequences */
  ADC12IE = 0;
  for(c = 0; c < 8; c++) {
    ADC12MCTL_NO(c) &= ~EOS;
  }

  /* Set highest interrupt and end-of-sequence. This will generate one
     interrupt for each sequence of conversions. */
  for(c = 0; c < 8; c++) {
    if(adcflags & (128 >> c)) {
      ADC12IE |= 128 >> c;
      ADC12MCTL_NO(7 - c) |= EOS;
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
irq_adc12_activate(unsigned char adcno, unsigned char config,
                   int (* irq)(void))
{
  if(adcno >= 8) {
    return;
  }
  /* stop converting */
  ADC12CTL0 &= ~ENC;
  /* wait for conversion to stop */
  while(ADC12CTL0 & ADC12BUSY);
  ADC12CTL0 &= ~(ADC12ON | REFON);
  ADC12IE = 0;

  /* clear any pending interrupts */
  ADC12IFG = 0;

  adcflags |= (1 << adcno);

  ADC12MCTL_NO(adcno) = config;

  sethilo();

  ADC12CTL0 |= ADC12ON | REFON;

  adc12_irq[adcno] = irq;

  /* Delay */
  clock_delay(20000);

  ADC12CTL0 |= ENC | ADC12SC;
}
/*---------------------------------------------------------------------------*/
void
irq_adc12_deactivate(unsigned char adcno)
{
  if(adcno >= 8) {
    return;
  }
  /* stop converting */
  ADC12CTL0 &= ~ENC;
  /* wait for conversion to stop */
  while(ADC12CTL0 & ADC12BUSY);
  ADC12CTL0 &= ~(ADC12ON | REFON);
  ADC12IE = 0;

  /* clear any pending interrupts */
  ADC12IFG = 0;

  adcflags &= ~(1 << adcno);

  ADC12MCTL_NO(adcno) = 0;

  sethilo();

  adc12_irq[adcno] = NULL;

  if(adcflags) {
    /* Turn on the ADC12 */
    ADC12CTL0 |= (ADC12ON | REFON);

    /* Delay */
    clock_delay(20000);

    /* Still active. Turn on the conversion. */
    ADC12CTL0 |= ENC | ADC12SC;
  }
}
/*---------------------------------------------------------------------------*/
int
irq_adc12_active(unsigned char adcno)
{
  return adcflags & (1 << adcno) ? 1 : 0;
}
/*---------------------------------------------------------------------------*/
