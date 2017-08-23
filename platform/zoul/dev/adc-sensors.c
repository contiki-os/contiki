/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 * Generic driver for the Zoul ADC wrapper for analogue sensors
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "adc-sensors.h"
#include "adc-zoul.h"
#include "zoul-sensors.h"
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
typedef struct {
  int type;
  uint8_t pin_mask;
  uint8_t vdd3;
} adc_info_t;

typedef struct {
  uint8_t sensors_num;
  uint8_t sensors_ports;
  adc_info_t sensor[ADC_SENSORS_MAX];
} adc_wrapper_t;

static adc_wrapper_t sensors;
/*---------------------------------------------------------------------------*/
static uint16_t
convert_to_value(uint8_t index)
{
  uint32_t value;
  value = adc_zoul.value(sensors.sensor[index].pin_mask);

  if(value == ZOUL_SENSORS_ERROR) {
    PRINTF("ADC sensors: failed retrieving data\n");
    return ADC_WRAPPER_ERROR;
  }

  /* Default voltage divisor relation is 5/3 aprox, change at adc_wrapper.h,
   * calculations below assume a decimation rate of 512 (12 bits ENOB) and
   * AVVD5 voltage reference of 3.3V
   */

  if(!sensors.sensor[index].vdd3) {
    value *= ADC_WRAPPER_EXTERNAL_VREF;
    value /= ADC_WRAPPER_EXTERNAL_VREF_CROSSVAL;
  }

  switch(sensors.sensor[index].type) {
  case ANALOG_GROVE_LIGHT:
    /* Light dependant resistor (LDR) resistance value*/
    value = (10230 - (value * 10)) / value;
    /* TODO: With the resistance we could calculate the lux as 63*R^(-0.7) */
    return (uint16_t)value;

  case ANALOG_GROVE_LOUDNESS:
    /* Based on the LM2904 amplifier (blue version with potentiometer) */
    return (uint16_t)value;

  case ANALOG_PHIDGET_ROTATION_1109:
    /* Linear sensor with 0-300º, 300/33000 = 0.00909 */
    value *= 909;
    value /= 100000;
    return (uint16_t)value;

   /* VDD+5 sensors */ 
  case ANALOG_VAC_SENSOR:
    /* Linear sensor from 0 to 5 V; 0.0088 resolution*/
    value *= 88;
    value /= 10000;
    return (uint16_t)value;

  case ANALOG_AAC_SENSOR:
    /* Linear sensor from 0 to 5 V;*/
    return (uint16_t)value;

  default:
    return ADC_WRAPPER_ERROR;
  }

  return ADC_WRAPPER_ERROR;
}
/*---------------------------------------------------------------------------*/
static uint8_t
is_sensor_in_list(int type)
{
  uint8_t i;

  for(i = 0; i <= sensors.sensors_num; i++) {
    if(sensors.sensor[i].type == type) {
      return i + 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint8_t index;
  uint16_t sensor_value;

  index = is_sensor_in_list(type);

  if(!index) {
    PRINTF("ADC sensors: sensor not registered\n");
    return ADC_WRAPPER_SUCCESS;
  }

  /* Restore index value after the check */
  index -= 1;
  sensor_value = convert_to_value(index);

  return sensor_value;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  uint8_t pin_mask = GPIO_PIN_MASK(value);

  if((type != ANALOG_GROVE_LIGHT) && (type != ANALOG_PHIDGET_ROTATION_1109) &&
     (type != ANALOG_GROVE_LOUDNESS) && (type != ANALOG_VAC_SENSOR) && 
     (type != ANALOG_AAC_SENSOR) ) {
    PRINTF("ADC sensors: sensor not supported, check adc_wrapper.h header\n");
    return ADC_WRAPPER_ERROR;
  }

  if(sensors.sensors_num >= ADC_SENSORS_MAX) {
    PRINTF("ADC sensors: all adc channels available have been assigned\n");
    return ADC_WRAPPER_ERROR;
  }

  if((value < 0x01) || (value > 0x07) || ((value == BUTTON_USER_PIN) && (ADC_SENSORS_ADC6_PIN < 0))) {
    PRINTF("ADC sensors: invalid pin value, (PA0-PA1, PA3) are reserved\n");
    return ADC_WRAPPER_ERROR;
  }

  if(sensors.sensors_ports & pin_mask) {
    PRINTF("ADC sensors: a sensor has been already assigned to this pin\n");
    return ADC_WRAPPER_ERROR;
  }

  switch(type) {
  /* V+3.3 sensors */
  case ANALOG_GROVE_LIGHT:
  case ANALOG_GROVE_LOUDNESS:
  case ANALOG_PHIDGET_ROTATION_1109:
    if(adc_zoul.configure(SENSORS_HW_INIT, pin_mask) == ZOUL_SENSORS_ERROR) {
      return ADC_WRAPPER_ERROR;
    }
    sensors.sensor[sensors.sensors_num].type = type;
    sensors.sensor[sensors.sensors_num].pin_mask = pin_mask;
    sensors.sensor[sensors.sensors_num].vdd3 = 1;
    break;

  /*V+5 sensors*/
  case ANALOG_VAC_SENSOR:
  case ANALOG_AAC_SENSOR:
    if(adc_zoul.configure(SENSORS_HW_INIT, pin_mask) == ZOUL_SENSORS_ERROR) {
      return ADC_WRAPPER_ERROR;
    }
    sensors.sensor[sensors.sensors_num].type = type;
    sensors.sensor[sensors.sensors_num].pin_mask = pin_mask;
    sensors.sensor[sensors.sensors_num].vdd3 = 0;
    break;


  default:
    return ADC_WRAPPER_ERROR;
  }

  PRINTF("ADC sensors: type %u mask 0x%02X vdd3 %u\n",
         sensors.sensor[sensors.sensors_num].type,
         sensors.sensor[sensors.sensors_num].pin_mask,
         sensors.sensor[sensors.sensors_num].vdd3);

  sensors.sensors_num++;
  sensors.sensors_ports |= pin_mask;

  return ADC_WRAPPER_SUCCESS;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(adc_sensors, ADC_SENSORS, value, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */

