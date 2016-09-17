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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne, Sumankumar Panchal
 * Created : 2005-11-01
 * Updated : $Date: 2016/09/17 11:30:00 $
 *           $Revision: 1.11 $
 */

#include "dev/battery-sensor.h"
#include "dev/sky-sensors.h"

const struct sensors_sensor battery_sensor;
static uint8_t active;
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  /* Setup ADC12, ref., sampling time */
  ADC12CTL0 = ADC12REF2_5V + ADC12SHT0_6 + ADC12SHT1_6 + ADC12MSC + ADC12REFON;
  /* Use sampling timer, repeat-sequence-of-channels */
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_3 + ADC12CSTARTADD_1;

  /* Configure ADC12_2 to sample channel 11 (voltage) and use */
  /* the Vref+ as reference (SREF_1) since it is a stable reference */
  ADC12MCTL2 = (ADC12INCH_11 + ADC12SREF_1);

  ADC12CTL0 |= ADC12ON;
  ADC12CTL0 |= ADC12ENC;        /* enable conversion */
  ADC12CTL0 |= ADC12SC;         /* sample & convert */

  active = 1;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  active = 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return ADC12MEM2; /*battery_value*/
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
  case SENSORS_ACTIVE:
    if(c) {
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
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return active;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR,
               value, configure, status);
