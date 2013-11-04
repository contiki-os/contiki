/*
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
/**
 * \addtogroup cc2538dk-adc-sensor
 * @{
 *
 * \file
 *  Driver for the SmartRF06EB ADC
 */
#include "contiki.h"
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/adc-sensor.h"

#include <stdint.h>

#define ADC_ALS_PWR_PORT_BASE    GPIO_PORT_TO_BASE(ADC_ALS_PWR_PORT)
#define ADC_ALS_PWR_PIN_MASK     GPIO_PIN_MASK(ADC_ALS_PWR_PIN)
#define ADC_ALS_OUT_PIN_MASK     GPIO_PIN_MASK(ADC_ALS_OUT_PIN)
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint8_t channel;
  int16_t res;

  switch(type) {
  case ADC_SENSOR_VDD_3:
    channel = SOC_ADC_ADCCON_CH_VDD_3;
    break;
  case ADC_SENSOR_TEMP:
    channel = SOC_ADC_ADCCON_CH_TEMP;
    break;
  case ADC_SENSOR_ALS:
    channel = SOC_ADC_ADCCON_CH_AIN0 + ADC_ALS_OUT_PIN;
    GPIO_SET_PIN(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    clock_delay_usec(2000);
    break;
  default:
    return 0;
  }

  res = adc_get(channel, SOC_ADC_ADCCON_REF_INT, SOC_ADC_ADCCON_DIV_512);

  if(type == ADC_SENSOR_ALS) {
    GPIO_CLR_PIN(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
  }

  return res;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    GPIO_SOFTWARE_CONTROL(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    GPIO_SET_OUTPUT(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    GPIO_CLR_PIN(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    ioc_set_over(ADC_ALS_PWR_PORT, ADC_ALS_PWR_PIN, IOC_OVERRIDE_DIS);

    GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, ADC_ALS_OUT_PIN_MASK);
    GPIO_SET_INPUT(GPIO_A_BASE, ADC_ALS_OUT_PIN_MASK);
    ioc_set_over(GPIO_A_NUM, ADC_ALS_OUT_PIN, IOC_OVERRIDE_ANA);

    adc_init();
    break;
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
SENSORS_SENSOR(adc_sensor, ADC_SENSOR, value, configure, status);

/** @} */
