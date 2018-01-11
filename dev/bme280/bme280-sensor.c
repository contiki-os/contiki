/*
 * Copyright (c) 2015, Copyright Robert Olsson
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
 * Author  : Robert Olsson rolss@kth.se/robert@radio-sensors.com
 * Created : 2016-09-14
 */

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/bme280/bme280.h"
#include "dev/bme280/bme280-sensor.h"
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* Read all measurements with one burst read */
  bme280_read(BME280_MODE_WEATHER);

  /* Return a la Contiki API */
  switch(type) {

  case BME280_SENSOR_TEMP:
    return bme280_mea.t_overscale100 / 100;

  case BME280_SENSOR_HUMIDITY:
    return bme280_mea.h_overscale1024 >> 10;

  case BME280_SENSOR_PRESSURE:
    /* Scale down w. 10 not to overslow the signed int */
#ifdef BME280_64BIT
    return bme280_mea.p_overscale256 / (256 * 10);
#else
    return bme280_mea.p / 10;
#endif
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  return bme280_init(BME280_MODE_WEATHER);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(bme280_sensor, "bme280", value, configure, status);
