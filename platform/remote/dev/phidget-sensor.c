/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * Copyright (c) 2015, University of Bristol - http://www.bristol.ac.uk
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup remote-phidget-sensor
 * @{
 *
 * \file
 * Generic driver for the Re-Mote Phidget/ADC sensors
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/phidget-sensor.h"
#include "dev/remote-sensors.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define ADC_PHIDGET_PORT_BASE      GPIO_PORT_TO_BASE(ADC_PHIDGET_PORT)
#define ADC_PHIDGET_ADC2_PIN_MASK  GPIO_PIN_MASK(ADC_PHIDGET_ADC2_PIN)
#define ADC_PHIDGET_ADC3_PIN_MASK  GPIO_PIN_MASK(ADC_PHIDGET_ADC3_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t decimation_rate;
/*---------------------------------------------------------------------------*/
static int
set_decimation_rate(uint8_t rate)
{
  switch(rate) {
  case SOC_ADC_ADCCON_DIV_64:
  case SOC_ADC_ADCCON_DIV_128:
  case SOC_ADC_ADCCON_DIV_256:
  case SOC_ADC_ADCCON_DIV_512:
    decimation_rate = rate;
    break;
  default:
    return REMOTE_SENSORS_ERROR;
  }

  return decimation_rate;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint8_t channel;
  int16_t res;

  switch(type) {
  case PHIDGET_SENSORS_ADC2:
    channel = SOC_ADC_ADCCON_CH_AIN0 + ADC_PHIDGET_ADC2_PIN;
    break;
  case PHIDGET_SENSORS_ADC3:
    channel = SOC_ADC_ADCCON_CH_AIN0 + ADC_PHIDGET_ADC3_PIN;
    break;
  default:
    return REMOTE_SENSORS_ERROR;
  }

  res = adc_get(channel, SOC_ADC_ADCCON_REF_INT, decimation_rate);

  return res;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, ADC_PHIDGET_ADC2_PIN_MASK);
    GPIO_SET_INPUT(GPIO_A_BASE, ADC_PHIDGET_ADC2_PIN_MASK);
    ioc_set_over(GPIO_A_NUM, ADC_PHIDGET_ADC2_PIN, IOC_OVERRIDE_ANA);

    GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, ADC_PHIDGET_ADC3_PIN_MASK);
    GPIO_SET_INPUT(GPIO_A_BASE, ADC_PHIDGET_ADC3_PIN_MASK);
    ioc_set_over(GPIO_A_NUM, ADC_PHIDGET_ADC3_PIN, IOC_OVERRIDE_ANA);
    adc_init();
    set_decimation_rate(SOC_ADC_ADCCON_DIV_512);
    break;
  case REMOTE_SENSORS_CONFIGURE_TYPE_DECIMATION_RATE:
    return set_decimation_rate((uint8_t)value);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(phidget_sensor, PHIDGET_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
