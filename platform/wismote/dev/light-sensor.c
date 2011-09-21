/*
 * Copyright (c) 2005-2010, Swedish Institute of Computer Science
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
 */

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "dev/light-sensor.h"

const struct sensors_sensor light_sensor;

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* switch(type) { */
  /* case LIGHT_SENSOR_PHOTOSYNTHETIC: */
  /*    /\* Photosynthetically Active Radiation. *\/ */
  /*   return ADC12MEM0; */
  /* case LIGHT_SENSOR_TOTAL_SOLAR: */
  /*   /\* Total Solar Radiation. *\/ */
  /*   return ADC12MEM1; */
  /* } */
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
/*
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return (ADC12CTL0 & (ADC12ON + REFON)) == (ADC12ON + REFON);
  }
*/
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

//	ADC12MCTL0 = (INCH_4 + SREF_0); // photodiode 1 (P64)
//	ADC12MCTL1 = (INCH_5 + SREF_0); // photodiode 2 (P65)

	sky_sensors_activate(0x30);
      }
    } else {
      sky_sensors_deactivate(0x30);
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(light_sensor, "Light",
	       value, configure, status);
