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
 * $Id: battery-sensor.c,v 1.6 2010/01/14 20:01:18 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2005-11-01
 * Updated : $Date: 2010/01/14 20:01:18 $
 *           $Revision: 1.6 $
 */

#include "dev/battery-sensor.h"
#include <io.h>

const struct sensors_sensor battery_sensor;
static uint8_t active;
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  /* This assumes that some other sensor system already did setup the ADC */
  /* (in the case of the sky platform it is sensors_light_init that does it) */

  P6SEL |= 0x80;
  P6DIR = 0xff;
  P6OUT = 0x00;

  /* stop converting immediately */
  ADC12CTL0 &= ~ENC;
  ADC12CTL1 &= ~CONSEQ_3;

  /* Configure ADC12_2 to sample channel 11 (voltage) and use */
  /* the Vref+ as reference (SREF_1) since it is a stable reference */
  ADC12MCTL2 = (INCH_11 + SREF_1);

  ADC12CTL1 |= CONSEQ_3;
  ADC12CTL0 |= ENC | ADC12SC;

  /*  Irq_adc12_activate(&battery_sensor, 6, (INCH_11 + SREF_1)); */

  active = 1;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  active = 0;
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  return ADC12MEM2/*battery_value*/;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
  case SENSORS_ACTIVE:
    if (c) {
      activate();
    } else {
      deactivate();
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return active;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR,
	       value, configure, status);
