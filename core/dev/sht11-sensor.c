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
 * @(#)$Id: sht11-sensor.c,v 1.2 2010/01/15 08:51:21 adamdunkels Exp $
 */

#include <stdlib.h>

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/sht11.h"
#include "dev/sht11-sensor.h"

const struct sensors_sensor sht11_sensor;

enum {
  ON, OFF
};
static uint8_t state = OFF;

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  switch(type) {
    /* Photosynthetically Active Radiation. */
  case SHT11_SENSOR_TEMP:
    return sht11_temp();;

    /* Total Solar Radiation. */
  case SHT11_SENSOR_HUMIDITY:
    return sht11_humidity();

  case SHT11_SENSOR_BATTERY_INDICATOR:
    return sht11_sreg() & 0x40? 1: 0;
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
    return (state == ON);
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
        rtimer_clock_t t0;
	sht11_init();
        state = ON;

        /* For for about 11 ms before the SHT11 can be used. */
        t0 = RTIMER_NOW();
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + RTIMER_SECOND / 100));
      }
    } else {
      sht11_off();
      state = OFF;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sht11_sensor, "sht11",
	       value, configure, status);
