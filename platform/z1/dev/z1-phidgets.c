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
 * $Id: z1-phidgets.c,v 1.3 2010/11/05 10:31:57 joxe Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Joakim Eriksson
 * Created : 2010-02-02
 * Updated : $Date: 2010/11/05 10:31:57 $
 *           $Revision: 1.3 $
 */

#include "contiki.h"
#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif

#include "lib/sensors.h"
#include "dev/z1-phidgets.h"

static uint8_t adc_on;
static uint8_t active;
/*---------------------------------------------------------------------------*/
static void
sensors_activate(uint8_t type)
{
  uint8_t pre = adc_on;

  adc_on |= type;

  if(pre == 0 && adc_on > 0) {
    P6DIR = 0xff;
    P6OUT = 0x00;
    P6SEL |= 0x8b; /* bit 7 + 3 + 1 + 0 */

    /* if nothing was started before, start up the ADC system */
    /* Set up the ADC. */
    ADC12CTL0 = REF2_5V + SHT0_6 + SHT1_6 + MSC; /* Setup ADC12, ref., sampling time */
    ADC12CTL1 = SHP + CONSEQ_3 + CSTARTADD_0;	/* Use sampling timer, repeat-sequenc-of-channels */
    /* convert up to MEM4 */
    ADC12MCTL4 |= EOS;

    ADC12CTL0 |= ADC12ON + REFON;
    ADC12CTL0 |= ENC;		/* enable conversion */
    ADC12CTL0 |= ADC12SC;		/* sample & convert */
  }
}
/*---------------------------------------------------------------------------*/
static void
sensors_deactivate(uint8_t type)
{
  adc_on &= ~type;

  if(adc_on == 0) {
    /* stop converting immediately, turn off reference voltage, etc. */
    /* wait for conversion to stop */

    ADC12CTL0 &= ~ENC;
    /* need to remove CONSEQ_3 if not EOS is configured */
    ADC12CTL1 &= ~CONSEQ_3;

    while(ADC12CTL1 & ADC12BUSY);

    ADC12CTL0 = 0;
    ADC12CTL1 = 0;

    P6DIR = 0x00;
    P6OUT = 0x00;
    P6SEL = 0x00;
  }
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* ADC0 corresponds to the port under the logo, ADC1 to the port over the logo,
     ADC2 and ADC3 corresponds to port on the JCreate bottom expansion port) */
  switch(type) {
    case PHIDGET5V_1:
      return ADC12MEM0;
    case PHIDGET5V_2:
      return ADC12MEM1;
    case PHIDGET3V_1:
      return ADC12MEM2;
    case PHIDGET3V_2:
      return ADC12MEM3;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
    case SENSORS_ACTIVE:
    case SENSORS_READY:
      return active;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
    case SENSORS_ACTIVE:
      if(c) {
        if(!status(SENSORS_ACTIVE)) {
          /* SREF_1 is Vref+ */
          /* MemReg6 == P6.0/A0 == 5V 1  */
          ADC12MCTL0 = (INCH_0 + SREF_0);
          /* MemReg7 == P6.3/A3 == 5V 2 */
          ADC12MCTL1 = (INCH_3 + SREF_0);
          /* MemReg8 == P6.1/A1 == 3V 1 */
          ADC12MCTL2 = (INCH_1 + SREF_0);
          /* MemReg9 == P6.7/A7 == 3V_2 */
          ADC12MCTL3 = (INCH_7 + SREF_0);

          sensors_activate(0x0F);
          active = 1;
        }
      } else {
        sensors_deactivate(0x0F);
        active = 0;
      }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(phidgets, "Phidgets", value, configure, status);
