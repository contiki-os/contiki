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
 * $Id: sky-sensors.c,v 1.2 2010/02/06 18:28:26 joxe Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Joakim Eriksson
 * Created : 2010-02-02
 * Updated : $Date: 2010/02/06 18:28:26 $
 *           $Revision: 1.2 $
 */

#include "contiki.h"

static uint8_t adc_on;
/*---------------------------------------------------------------------------*/
void
sky_sensors_activate(uint8_t type)
{
  uint8_t pre = adc_on;

  adc_on |= type;
  P6SEL |= type;

  if(pre == 0 && adc_on > 0) {
    P6DIR = 0xff;
    P6OUT = 0x00;
    /* if nothing was started before, start up the ADC system */
    /* Set up the ADC. */
    /* ADC12CTL0 = REF2_5V + SHT0_6 + SHT1_6 + MSC; /\* Setup ADC12, ref., sampling time *\/ */
    /* ADC12CTL1 = SHP + CONSEQ_3 + CSTARTADD_0;	/\* Use sampling timer, repeat-sequenc-of-channels  */
    /* /\* convert up to MEM4 *\/ */
    /* ADC12MCTL9 |= EOS; */

    /* ADC12CTL0 |= ADC12ON + REFON; */
    /* ADC12CTL0 |= ENC;		/\* enable conversion  *\/ */
    /* ADC12CTL0 |= ADC12SC;		/\* sample & convert *\/ */
  }
}
/*---------------------------------------------------------------------------*/
void
sky_sensors_deactivate(uint8_t type)
{
  adc_on &= ~type;

  if(adc_on == 0) {
    /* stop converting immediately, turn off reference voltage, etc. */
    /* wait for conversion to stop */

    /* ADC12CTL0 &= ~ENC; */
    /* /\* need to remove CONSEQ_3 if not EOS is configured *\/ */
    /* ADC12CTL1 &= ~CONSEQ_3; */

    /* while(ADC12CTL1 & ADC12BUSY); */

    /* ADC12CTL0 = 0; */
    /* ADC12CTL1 = 0; */

    /* P6DIR = 0x00; */
    /* P6OUT = 0x00; */
    /* P6SEL = 0x00; */
  }
}
/*---------------------------------------------------------------------------*/
