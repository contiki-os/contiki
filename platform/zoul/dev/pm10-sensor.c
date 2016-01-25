	/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-pm10-sensor
 * @{
 *
 * \file
 *      GP2Y1010AU0F PM10 sensor example using the ADC sensors wrapper  
 * \author
 *      Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "adc-sensors.h"
#include "dev/pm10-sensor.h"
#include "dev/sys-ctrl.h"
#include "lib/sensors.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
#ifdef PM10_SENSOR_CONF_CTRL_PIN
#define PM10_SENSOR_CTRL_PIN PM10_SENSOR_CONF_CTRL_PIN
#else
#define PM10_SENSOR_CTRL_PIN 0x07
#endif

#define PM10_SENSOR_PORT GPIO_A_BASE

static int
configure(int type, int value)
{
  int error;
  if(type != SENSORS_ACTIVE) {
    return PM10_ERROR;
  }

  /*Set PA7 as output, used as pulse-driven wave*/
  ioc_set_over(PM10_SENSOR_PORT, PM10_SENSOR_CTRL_PIN, IOC_OVERRIDE_DIS); 
  GPIO_SOFTWARE_CONTROL(PM10_SENSOR_PORT, GPIO_PIN_MASK(PM10_SENSOR_CTRL_PIN));
  GPIO_SET_OUTPUT(PM10_SENSOR_PORT, GPIO_PIN_MASK(PM10_SENSOR_CTRL_PIN));

  GPIO_CLR_PIN(PM10_SENSOR_PORT, GPIO_PIN_MASK(PM10_SENSOR_CTRL_PIN));

  /* Use pin number not mask, for example if using the PA5 pin then use 5 */
  error = adc_sensors.configure(ANALOG_PM10_SENSOR, value);

  return error;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint16_t val;
  /*Set Pulse Wave pin before measure*/
  GPIO_SET_PIN(PM10_SENSOR_PORT, GPIO_PIN_MASK(PM10_SENSOR_CTRL_PIN));
  /*Pulse wave delay*/
  clock_delay_usec(280);
  /*Data acquisition*/  
  val = adc_sensors.value(ANALOG_PM10_SENSOR); 
  /*Clear pulse wave pin*/
  GPIO_CLR_PIN(PM10_SENSOR_PORT, GPIO_PIN_MASK(PM10_SENSOR_CTRL_PIN));
  
  /*Applied constant conversion from UAir project*/
  /*to obtain value in ppm (value in mV * 0.28)*/
  val *= 28;
  val /= 1000;

  return val;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(pm10, PM10_SENSOR, value, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
