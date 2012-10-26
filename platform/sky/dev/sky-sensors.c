/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Joakim Eriksson
 * Created : 2010-02-02
 * Updated : $Date: 2010/08/25 19:30:53 $
 *           $Revision: 1.3 $
 */
#include "contiki.h"
#include "lib/sensors.h"

#define ADC12MCTL_NO(adcno) ((unsigned char *) ADC12MCTL0_)[adcno]

static uint16_t adc_on;
static uint16_t ready;
/*---------------------------------------------------------------------------*/
static CC_INLINE void
start(void)
{
  uint16_t c, last;

  /* Set up the ADC. */
  P6DIR = 0xff;
  P6OUT = 0x00;

  /* Setup ADC12, ref., sampling time */
  /* XXX Note according to the specification a minimum of 17 ms should
     be allowed after turn on of the internal reference generator. */
  ADC12CTL0 = REF2_5V + SHT0_6 + SHT1_6 + MSC + REFON;
  /* Use sampling timer, repeat-sequence-of-channels */
  ADC12CTL1 = SHP + CONSEQ_3;

  last = 15;
  for(c = 0; c < 16; c++) {
    /* Clear all end-of-sequences */
    ADC12MCTL_NO(c) &= ~EOS;
    if(adc_on & (1 << c)) {
      if(last == 15) {
        /* Set new start of sequence to lowest active memory holder */
        ADC12CTL1 |= (c * CSTARTADD_1);
      }
      last = c;
    }
  }

  /* Set highest end-of-sequence. */
  ADC12MCTL_NO(last) |= EOS;

  ADC12CTL0 |= ADC12ON;
  ADC12CTL0 |= ENC;                   /* enable conversion */
  ADC12CTL0 |= ADC12SC;               /* sample & convert */
}
/*---------------------------------------------------------------------------*/
static CC_INLINE void
stop(void)
{
  /* stop converting immediately, turn off reference voltage, etc. */

  ADC12CTL0 &= ~ENC;
  /* need to remove CONSEQ_3 if not EOS is configured */
  ADC12CTL1 &= ~CONSEQ_3;

  /* wait for conversion to stop */
  while(ADC12CTL1 & ADC12BUSY);

  /* clear any pending interrupts */
  ADC12IFG = 0;
}
/*---------------------------------------------------------------------------*/
int
sky_sensors_status(uint16_t input, int type)
{
  if(type == SENSORS_ACTIVE) {
    return (adc_on & input) == input;
  }
  if(type == SENSORS_READY) {
    ready |= ADC12IFG & adc_on & input;
    return (ready & adc_on & input) == input;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
sky_sensors_configure(uint16_t input, uint8_t ref, int type, int value)
{
  uint16_t c;

  if(type == SENSORS_ACTIVE) {
    stop();

    if(value) {
      adc_on |= input;
      P6SEL |= input & 0xff;

      /* Set ADC config */
      for(c = 0; c < 16; c++) {
        if(input & (1 << c)) {
          ADC12MCTL_NO(c) = (c * INCH_1) | ref;
        }
      }

    } else {
      adc_on &= ~input;
      ready &= ~input;
      P6SEL &= ~(input & 0xff);
    }

    if(adc_on == 0) {
      P6DIR = 0x00;
      P6SEL = 0x00;

      /* Turn off ADC and internal reference generator */
      ADC12CTL0 = 0;
      ADC12CTL1 = 0;
    } else {
      start();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
