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
 * \addtogroup zoul-adc-sensors
 * @{
 *
 * \file
 * Generic driver for the Zoul ADC sensors
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "adc-sensors.h"
#include "zoul-sensors.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
static uint8_t decimation_rate;
static uint8_t enabled_channels;
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
    return ZOUL_SENSORS_ERROR;
  }

  return decimation_rate;
}
/*---------------------------------------------------------------------------*/
static int
get_channel_pin(int type)
{
  if((ZOUL_SENSORS_ADC1) && (type == ZOUL_SENSORS_ADC1)) {
    return SOC_ADC_ADCCON_CH_AIN0 + ADC_SENSORS_ADC1_PIN;
  }
  if((ZOUL_SENSORS_ADC2) && (type == ZOUL_SENSORS_ADC2)) {
    return SOC_ADC_ADCCON_CH_AIN0 + ADC_SENSORS_ADC2_PIN;
  }
  if((ZOUL_SENSORS_ADC3) && (type == ZOUL_SENSORS_ADC3)) {
    return SOC_ADC_ADCCON_CH_AIN0 + ADC_SENSORS_ADC3_PIN;
  }
  return ZOUL_SENSORS_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  int channel;
  int16_t res;

  if(!(type & enabled_channels)) {
    return ZOUL_SENSORS_ERROR;
  }

  channel = get_channel_pin(type);

  if(channel == ZOUL_SENSORS_ERROR) {
    return ZOUL_SENSORS_ERROR;
  }

  res = adc_get(channel, SOC_ADC_ADCCON_REF_AVDD5, decimation_rate);
  return res;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:

    /* This should filter out disabled sensors as its value should be zero */
    if((value < ZOUL_SENSORS_ADC_MIN) || (value > ZOUL_SENSORS_ADC_ALL)) {
      return ZOUL_SENSORS_ERROR;
    }

    if((value != ZOUL_SENSORS_ADC1) && (value != ZOUL_SENSORS_ADC2) &&
       (value != ZOUL_SENSORS_ADC3) && (value != ZOUL_SENSORS_ADC12) &&
       (value != ZOUL_SENSORS_ADC13) && (value != ZOUL_SENSORS_ADC23)) {
      return ZOUL_SENSORS_ERROR;
    }

    GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, value);
    GPIO_SET_INPUT(GPIO_A_BASE, value);

    if(value & ZOUL_SENSORS_ADC1) {
      ioc_set_over(GPIO_A_NUM, ADC_SENSORS_ADC1_PIN, IOC_OVERRIDE_ANA);
    }
    if(value & ZOUL_SENSORS_ADC2) {
      ioc_set_over(GPIO_A_NUM, ADC_SENSORS_ADC2_PIN, IOC_OVERRIDE_ANA);
    }
    if(value & ZOUL_SENSORS_ADC3) {
      ioc_set_over(GPIO_A_NUM, ADC_SENSORS_ADC3_PIN, IOC_OVERRIDE_ANA);
    }
    adc_init();
    set_decimation_rate(SOC_ADC_ADCCON_DIV_512);
    enabled_channels = value;
    break;

  case ZOUL_SENSORS_CONFIGURE_TYPE_DECIMATION_RATE:
    return set_decimation_rate((uint8_t)value);

  default:
    return ZOUL_SENSORS_ERROR;
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
SENSORS_SENSOR(adc_sensors, ADC_SENSORS, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */

