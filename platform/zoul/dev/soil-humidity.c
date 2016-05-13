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
 * \addtogroup soil-humidity sensor
 * @{
 *
 * \file
 *      Soil humidity sensor driver using the ADC sensors wrapper  
 * \author
 *      Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "adc-sensors.h"
#include "adc-zoul.h"
#include "zoul-sensors.h"
#include "dev/soil-humidity.h"
#include "dev/sys-ctrl.h"
#include "lib/sensors.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
/*---------------------------------------------------------------------------*/
#define SOIL_DEBUG 0
#if SOIL_DEBUG
#define PRINTF_SOIL(...) printf(__VA_ARGS__)
#else
#define PRINTF_SOIL(...)
#endif
/*---------------------------------------------------------------------------*/
#define SOIL_HUM_SENSOR_PORT_BASE   GPIO_PORT_TO_BASE(SOIL_HUM_SENSOR_CTRL_PORT)
#define SOIL_HUM_SENSOR_PIN_MASK    GPIO_PIN_MASK(SOIL_HUM_SENSOR_CTRL_PIN)
/*---------------------------------------------------------------------------*/
static int soil_hum_channel;
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return SOIL_HUM_ERROR;
  }

  if(value) {
    /* Set as output, used as supply-driven pulse */
    /*
    GPIO_SOFTWARE_CONTROL(SOIL_HUM_SENSOR_PORT_BASE, SOIL_HUM_SENSOR_PIN_MASK);
    ioc_set_over(SOIL_HUM_SENSOR_CTRL_PORT, SOIL_HUM_SENSOR_CTRL_PIN, IOC_OVERRIDE_DIS); 
    GPIO_SET_OUTPUT(SOIL_HUM_SENSOR_PORT_BASE, SOIL_HUM_SENSOR_PIN_MASK);
    GPIO_CLR_PIN(SOIL_HUM_SENSOR_PORT_BASE, SOIL_HUM_SENSOR_PIN_MASK);
    */
    soil_hum_channel = (1 << value);
    return adc_zoul.configure(SENSORS_HW_INIT, soil_hum_channel);
  }

  soil_hum_channel = 0;
  return SOIL_HUM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint32_t val;

  if(!soil_hum_channel) {
    return SOIL_HUM_ERROR;
  }

  /* Set Pulse Wave pin before measure */
  /*GPIO_SET_PIN(SOIL_HUM_SENSOR_PORT_BASE, SOIL_HUM_SENSOR_PIN_MASK);*/
  /* Pulse wave delay */
  /*clock_delay_usec(SOIL_HUM_SENSOR_PULSE_DELAY);*/
  /* Data acquisition */  
  val = (uint32_t)adc_zoul.value(soil_hum_channel); 
  PRINTF_SOIL("SOIL_HUM sensor: %u\n", val);
  
  if(val == ZOUL_SENSORS_ERROR) {
    PRINTF_SOIL("SOIL_HUM sensor: failed retrieving data\n");
    return SOIL_HUM_ERROR;
  }

  /* Clear pulse wave pin */
  GPIO_CLR_PIN(SOIL_HUM_SENSOR_PORT_BASE, SOIL_HUM_SENSOR_PIN_MASK);
  
  return (uint16_t)val;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(soil_hum, SOIL_HUM_SENSOR, value, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
