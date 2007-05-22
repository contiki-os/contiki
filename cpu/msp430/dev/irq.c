/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: irq.c,v 1.3 2007/05/22 21:01:32 adamdunkels Exp $
 */
#include "lib/sensors.h"
#include "dev/irq.h"
#include "dev/lpm.h"
/*#include "contiki-esb.h"*/
#include <signal.h>

#define ADC12MCTL_NO(adcno) ((unsigned char *) ADC12MCTL0_)[adcno]

static unsigned char adcflags;

/*---------------------------------------------------------------------------*/
#if IRQ_PORT1_VECTOR
interrupt(PORT1_VECTOR)
     irq_p1(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(sensors_handle_irq(IRQ_PORT1)) {
    LPM4_EXIT;
  }
  P1IFG = 0x00;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#endif
/*---------------------------------------------------------------------------*/
interrupt(PORT2_VECTOR)
     irq_p2(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(sensors_handle_irq(IRQ_PORT2)) {
    LPM4_EXIT;
  }
  P2IFG = 0x00;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
interrupt (ADC_VECTOR)
     irq_adc(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(sensors_handle_irq(IRQ_ADC)) {
    LPM4_EXIT;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
irq_init(void)
{
  adcflags = 0;

  /* Setup ADC12, ref., sampling time */
  ADC12CTL0 = REF2_5V | SHT0_10 | SHT1_10 | MSC;

  /* Use sampling timer, repeat-sequence-of-channels */
/*   ADC12CTL1 = SHP | CONSEQ_3 | ADC12DIV_3; */
  ADC12CTL1 = SHP | CONSEQ_3;
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
      /*ADC12IE |= 128 >> c;*/
	      /*	      ADC12MCTL_NO(7 - c) |= EOS;*/
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
irq_adc12_activate(const struct sensors_sensor *sensor,
		   unsigned char adcno, unsigned char config)
{
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

  sensors_add_irq(sensor, IRQ_ADC);

  /* Delay */
  clock_delay(20000);

  ADC12CTL0 |= ENC | ADC12SC;
}
/*---------------------------------------------------------------------------*/
void
irq_adc12_deactivate(const struct sensors_sensor *sensor, unsigned char adcno)
{
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

  sensors_remove_irq(sensor, IRQ_ADC);

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
