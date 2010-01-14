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
 *
 * @(#)$Id: light-sensor.c,v 1.3 2010/01/14 20:23:02 adamdunkels Exp $
 */

#include <stdlib.h>

#include <io.h>

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/light-sensor.h"

const struct sensors_sensor light_sensor;

/*
 * Initialize periodic readings from the 2 photo diodes. The most
 * recent readings will be stored in ADC internal registers/memory.
 */
static void
light_sensor_init(void)
{
  P6SEL |= 0x30;
  P6DIR = 0xff;
  P6OUT = 0x00;

  /* Set up the ADC. */
  ADC12CTL0 = REF2_5V + SHT0_6 + SHT1_6 + MSC; // Setup ADC12, ref., sampling time
  ADC12CTL1 = SHP + CONSEQ_3 + CSTARTADD_0;	// Use sampling timer, repeat-sequenc-of-channels

  ADC12MCTL0 = (INCH_4 + SREF_0); // photodiode 1 (P64)
  ADC12MCTL1 = (INCH_5 + SREF_0); // photodiode 2 (P65)

  ADC12CTL0 |= ADC12ON + REFON;

  ADC12CTL0 |= ENC;		// enable conversion
  ADC12CTL0 |= ADC12SC;		// sample & convert
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{

  switch(type) {
    /* Photosynthetically Active Radiation. */
  case LIGHT_SENSOR_PHOTOSYNTHETIC:
    return ADC12MEM0;

    /* Total Solar Radiation. */
  case LIGHT_SENSOR_TOTAL_SOLAR:
    return ADC12MEM1;
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
    return (ADC12CTL0 & (ADC12ON + REFON)) == (ADC12ON + REFON);
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
	light_sensor_init();
      }
    } else {
      /* shut down sensing */
      ADC12CTL0 = 0;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(light_sensor, "Light",
	       value, configure, status);
