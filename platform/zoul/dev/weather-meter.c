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
 * \addtogroup zoul-weather-meter-sensor
 * @{
 *
 * The Sparkfun's weather meter comprises an anemometer, wind vane and rain
 * gauge, see https://www.sparkfun.com/products/8942
 *
 * \file
 *         Weather meter sensor driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/adc-sensors.h"
#include "dev/weather-meter.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
#include "sys/timer.h"
#include "sys/etimer.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define DEBOUNCE_DURATION  (CLOCK_SECOND >> 4)
/*---------------------------------------------------------------------------*/
#define ANEMOMETER_SENSOR_PORT_BASE  GPIO_PORT_TO_BASE(ANEMOMETER_SENSOR_PORT)
#define ANEMOMETER_SENSOR_PIN_MASK   GPIO_PIN_MASK(ANEMOMETER_SENSOR_PIN)
#define RAIN_GAUGE_SENSOR_PORT_BASE  GPIO_PORT_TO_BASE(RAIN_GAUGE_SENSOR_PORT)
#define RAIN_GAUGE_SENSOR_PIN_MASK   GPIO_PIN_MASK(RAIN_GAUGE_SENSOR_PIN)
/*---------------------------------------------------------------------------*/
void (*rain_gauge_int_callback)(uint16_t value);
void (*anemometer_int_callback)(uint16_t value);
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
process_event_t anemometer_int_event;
process_event_t rain_gauge_int_event;
/*---------------------------------------------------------------------------*/
static struct etimer et;
static struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
typedef struct {
  uint16_t ticks;
  uint16_t value;
  uint8_t  int_en;
  uint16_t int_thres;
} weather_meter_sensors_t;

typedef struct {
  uint16_t wind_vane;
  weather_meter_sensors_t rain_gauge;
  weather_meter_sensors_t anemometer;
} weather_meter_sensors;
static weather_meter_sensors weather_sensors;
/*---------------------------------------------------------------------------*/
PROCESS(weather_meter_int_process, "Weather meter interrupt process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(weather_meter_int_process, ev, data)
{
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();
  static uint32_t mph;
  static uint16_t rpm;

  etimer_set(&et, CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();

    if((ev == anemometer_int_event) && (weather_sensors.anemometer.int_en)) {
      if(weather_sensors.anemometer.ticks >=
        weather_sensors.anemometer.int_thres) {
        anemometer_int_callback(weather_sensors.anemometer.ticks);
      }
    }

    if((ev == rain_gauge_int_event) && (weather_sensors.rain_gauge.int_en)) {
      if(weather_sensors.rain_gauge.ticks >=
        weather_sensors.rain_gauge.int_thres) {
        rain_gauge_int_callback(weather_sensors.rain_gauge.ticks);
      }
    }

    if(ev == PROCESS_EVENT_TIMER) {
      if(weather_sensors.anemometer.ticks) {

        /* Disable to make the calculations in an interrupt-safe context */
        GPIO_DISABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE,
                               ANEMOMETER_SENSOR_PIN_MASK);

        /* The anemometer ticks twice per rotation, and a wind speed of 2.4 km/h
         * makes the switch close every second, convert RPM to linear velocity
         */
        rpm = weather_sensors.anemometer.ticks * 30;
        mph = rpm * WEATHER_METER_AUX_ANGULAR;
        mph /= 1000;

        /* This will return values in metres per hour */
        weather_sensors.anemometer.value = (uint16_t)mph;

        /* Restart the counter */
        weather_sensors.anemometer.ticks = 0;

        /* Enable the interrupt again */
        GPIO_ENABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE,
                              ANEMOMETER_SENSOR_PIN_MASK);      
        etimer_restart(&et);
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
weather_meter_interrupt_handler(uint8_t port, uint8_t pin)
{
  uint32_t aux;

  /* Prevent bounce events */
  if(!timer_expired(&debouncetimer)) {
    return;
  }

  timer_set(&debouncetimer, DEBOUNCE_DURATION);

  /* We make a process_post() to check in the pollhandler any specific threshold
   * value
   */

  if((port == ANEMOMETER_SENSOR_PORT) && (pin == ANEMOMETER_SENSOR_PIN)) {
    weather_sensors.anemometer.ticks++;
    process_post(&weather_meter_int_process, anemometer_int_event, NULL);

  } else if((port == RAIN_GAUGE_SENSOR_PORT) && (pin == RAIN_GAUGE_SENSOR_PIN)) {
    weather_sensors.rain_gauge.ticks++;
    aux = weather_sensors.rain_gauge.ticks * WEATHER_METER_AUX_RAIN_MM;
    aux /= 1000;
    weather_sensors.rain_gauge.value = (uint16_t)aux;
    process_post(&weather_meter_int_process, rain_gauge_int_event, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if((type != WEATHER_METER_ANEMOMETER) && (type != WEATHER_METER_RAIN_GAUGE) &&
    (type != WEATHER_METER_WIND_VANE)) {
    PRINTF("Weather: requested an invalid sensor value\n");
    return WEATHER_METER_ERROR;
  }

  if(!enabled) {
    PRINTF("Weather: module is not configured\n");
    return WEATHER_METER_ERROR;
  }

  switch(type) {
  case WEATHER_METER_WIND_VANE:
    /* FIXME: return the values in degrees */
    weather_sensors.wind_vane = adc_sensors.value(WIND_VANE_ADC);
    return weather_sensors.wind_vane;

  case WEATHER_METER_ANEMOMETER:
    return weather_sensors.anemometer.value;

  /* as the default return type is int, we have a lower resolution if returning
   * the calculated value as it is truncated, an alternative is returning the
   * ticks and calculating on your own with WEATHER_METER_AUX_RAIN_MM
   */
  case WEATHER_METER_RAIN_GAUGE:
    #if WEATHER_METER_RAIN_RETURN_TICKS
      return weather_sensors.rain_gauge.ticks;
    #else
      return weather_sensors.rain_gauge.value;
    #endif

  default:
    return WEATHER_METER_ERROR;
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if((type != WEATHER_METER_ACTIVE) && 
    (type != WEATHER_METER_ANEMOMETER_INT_OVER) &&
    (type != WEATHER_METER_RAIN_GAUGE_INT_OVER) &&
    (type != WEATHER_METER_ANEMOMETER_INT_DIS) &&
    (type != WEATHER_METER_RAIN_GAUGE_INT_DIS)) {
    PRINTF("Weather: invalid configuration option\n");
    return WEATHER_METER_ERROR;
  }

  if(type == WEATHER_METER_ACTIVE) {

    weather_sensors.anemometer.int_en = 0;
    weather_sensors.rain_gauge.int_en = 0;
    weather_sensors.anemometer.ticks = 0;
    weather_sensors.rain_gauge.ticks = 0;
    weather_sensors.anemometer.value = 0;
    weather_sensors.rain_gauge.value = 0;

    if(!value) {
      anemometer_int_callback = NULL;
      rain_gauge_int_callback = NULL;
      GPIO_DISABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE,
                             ANEMOMETER_SENSOR_PIN_MASK);
      GPIO_DISABLE_INTERRUPT(RAIN_GAUGE_SENSOR_PORT_BASE,
                             RAIN_GAUGE_SENSOR_PIN_MASK);
      process_exit(&weather_meter_int_process);
      enabled = 0;
      PRINTF("Weather: disabled\n");
      return WEATHER_METER_SUCCESS;
    }

    /* Configure the wind vane */
    adc_sensors.configure(SENSORS_HW_INIT, WIND_VANE_ADC);

    /* Configure anemometer interruption */
    GPIO_SOFTWARE_CONTROL(ANEMOMETER_SENSOR_PORT_BASE, ANEMOMETER_SENSOR_PIN_MASK);
    GPIO_SET_INPUT(ANEMOMETER_SENSOR_PORT_BASE, ANEMOMETER_SENSOR_PIN_MASK);
    GPIO_DETECT_RISING(ANEMOMETER_SENSOR_PORT_BASE, ANEMOMETER_SENSOR_PIN_MASK);
    GPIO_TRIGGER_SINGLE_EDGE(ANEMOMETER_SENSOR_PORT_BASE,
                             ANEMOMETER_SENSOR_PIN_MASK);
    ioc_set_over(ANEMOMETER_SENSOR_PORT, ANEMOMETER_SENSOR_PIN, IOC_OVERRIDE_DIS);
    gpio_register_callback(weather_meter_interrupt_handler, ANEMOMETER_SENSOR_PORT,
                           ANEMOMETER_SENSOR_PIN);

    /* Configure rain gauge interruption */
    GPIO_SOFTWARE_CONTROL(RAIN_GAUGE_SENSOR_PORT_BASE, RAIN_GAUGE_SENSOR_PIN_MASK);
    GPIO_SET_INPUT(RAIN_GAUGE_SENSOR_PORT_BASE, RAIN_GAUGE_SENSOR_PIN_MASK);
    GPIO_DETECT_RISING(RAIN_GAUGE_SENSOR_PORT_BASE, RAIN_GAUGE_SENSOR_PIN_MASK);
    GPIO_TRIGGER_SINGLE_EDGE(RAIN_GAUGE_SENSOR_PORT_BASE,
                             RAIN_GAUGE_SENSOR_PIN_MASK);
    ioc_set_over(RAIN_GAUGE_SENSOR_PORT, RAIN_GAUGE_SENSOR_PIN, IOC_OVERRIDE_DIS);
    gpio_register_callback(weather_meter_interrupt_handler, RAIN_GAUGE_SENSOR_PORT,
                           RAIN_GAUGE_SENSOR_PIN);

    process_start(&weather_meter_int_process, NULL);

    GPIO_ENABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE, ANEMOMETER_SENSOR_PIN_MASK);
    GPIO_ENABLE_INTERRUPT(RAIN_GAUGE_SENSOR_PORT_BASE, RAIN_GAUGE_SENSOR_PIN_MASK);
    nvic_interrupt_enable(ANEMOMETER_SENSOR_VECTOR);
    nvic_interrupt_enable(RAIN_GAUGE_SENSOR_VECTOR);

    enabled = 1;
    PRINTF("Weather: started\n");
    return WEATHER_METER_SUCCESS;
  }

  switch(type) {
  case WEATHER_METER_ANEMOMETER_INT_OVER:
    weather_sensors.anemometer.int_en = 1;
    weather_sensors.anemometer.int_thres = value;
    PRINTF("Weather: anemometer threshold %u\n", value);
    break;
  case WEATHER_METER_RAIN_GAUGE_INT_OVER:
    weather_sensors.rain_gauge.int_en = 1;
    weather_sensors.rain_gauge.int_thres = value;
    PRINTF("Weather: rain gauge threshold %u\n", value);
    break;
  case WEATHER_METER_ANEMOMETER_INT_DIS:
    PRINTF("Weather: anemometer int disabled\n");
    weather_sensors.anemometer.int_en = 0;
    break;
  case WEATHER_METER_RAIN_GAUGE_INT_DIS:
    PRINTF("Weather: rain gauge int disabled\n");
    weather_sensors.rain_gauge.int_en = 0;
    break;
  default:
    return WEATHER_METER_ERROR;
  }

  return WEATHER_METER_SUCCESS;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(weather_meter, WEATHER_METER_SENSOR, value, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
