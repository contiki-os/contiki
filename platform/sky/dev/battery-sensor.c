/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: battery-sensor.c,v 1.1 2007/11/13 20:29:07 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2005-11-01
 * Updated : $Date: 2007/11/13 20:29:07 $
 *           $Revision: 1.1 $
 */

#include "dev/battery-sensor.h"
#include <io.h>
#include "dev/irq.h"

const struct sensors_sensor battery_sensor;
/*static unsigned int battery_value;*/

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /*  battery_value = 0;*/
}
/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  /*  battery_value = ADC12MEM6;*/
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  SVSCTL = 0xf0;
  P6SEL |= 0x80;
  P6DIR = 0xff;
  P6OUT = 0x00;

  /* Set up the ADC. */
  //  ADC12CTL2 = SHT0_6 + SHT1_6 + MSC; // Setup ADC12, ref., sampling time
  ADC12MCTL2 = (INCH_11 + SREF_0);

/*   ADC12CTL2 |= ADC12ON + REFON; */

/*   ADC12CTL2 |= ENC;		// enable conversion */
/*   ADC12CTL2 |= ADC12SC;		// sample & convert */

  //  Irq_adc12_activate(&battery_sensor, 6, (INCH_11 + SREF_0));
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  /*  irq_adc12_deactivate(&battery_sensor, 6);
      battery_value = 0;*/
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return 0; /* irq_adc12_active(6);*/
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  return ADC12MEM2/*battery_value*/;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, void *c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void *
status(int type)
{
  return NULL;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
