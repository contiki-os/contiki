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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-weather-meter-test Test the Sparkfun's weather meter
 *
 * The example application shows how to read data from the anemometer, wind vane
 * and rain gauge, on board the Sparkfun's weater meter
 *
 * @{
 *
 * \file
 *         Test application for the Sparkfun's weather meter
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/weather-meter.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define READ_SENSOR_PERIOD          CLOCK_SECOND
#define ANEMOMETER_THRESHOLD_TICK   13  /**< 16 Km/h */
#define RAIN_GAUGE_THRESHOLD_TICK   15  /**< each increment of 4.19 mm */
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(test_weather_meter_sensors, "Test Weather meter sensors");
AUTOSTART_PROCESSES(&test_weather_meter_sensors);
/*---------------------------------------------------------------------------*/
static void
rain_callback(uint16_t value)
{
  /* To calculate ticks from mm of rain, divide by 0.2794 mm */
  printf("*** Rain gauge over threshold (%u ticks)\n", value);
  weather_meter.configure(WEATHER_METER_RAIN_GAUGE_INT_OVER,
                          (value + RAIN_GAUGE_THRESHOLD_TICK));
}
/*---------------------------------------------------------------------------*/
static void
wind_speed_callback(uint16_t value)
{
  /* This checks for instant wind speed values (over a second), the minimum
   * value is 1.2 Km/h (one tick), as the reference is 2.4KM/h per rotation, and
   * the anemometer makes 2 ticks per rotation.  Instant speed is calculated as
   * multiples of this, so if you want to check for 16Km/h, then it would be 13
   * ticks
   */
  printf("*** Wind speed over threshold (%u ticks)\n", value);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_weather_meter_sensors, ev, data)
{
  PROCESS_BEGIN();

  static uint32_t rain;
  static uint16_t wind_speed;
  static uint16_t wind_dir;
  static uint16_t wind_dir_avg_2m;
  static uint16_t wind_speed_avg;
  static uint16_t wind_speed_avg_2m;
  static uint16_t wind_speed_max;

  printf("Weather meter test example, integration period %u\n",
         WEATHER_METER_AVG_PERIOD);

  /* Register the callback handler when thresholds are met */
  WEATHER_METER_REGISTER_ANEMOMETER_INT(wind_speed_callback);
  WEATHER_METER_REGISTER_RAIN_GAUGE_INT(rain_callback);

  /* Enable the sensors, this has to be called before any of the interrupt calls
   * like the ones below
   */
  SENSORS_ACTIVATE(weather_meter);

  /* And the upper threshold value to compare and generate an interrupt */
  weather_meter.configure(WEATHER_METER_ANEMOMETER_INT_OVER,
                          ANEMOMETER_THRESHOLD_TICK);
  weather_meter.configure(WEATHER_METER_RAIN_GAUGE_INT_OVER,
                          RAIN_GAUGE_THRESHOLD_TICK);

  etimer_set(&et, READ_SENSOR_PERIOD);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    rain = weather_meter.value(WEATHER_METER_RAIN_GAUGE);
    wind_speed = weather_meter.value(WEATHER_METER_ANEMOMETER);
    wind_dir = weather_meter.value(WEATHER_METER_WIND_VANE);
    wind_dir_avg_2m = weather_meter.value(WEATHER_METER_WIND_VANE_AVG_X);
    wind_speed_avg = weather_meter.value(WEATHER_METER_ANEMOMETER_AVG);
    wind_speed_avg_2m = weather_meter.value(WEATHER_METER_ANEMOMETER_AVG_X);
    wind_speed_max = weather_meter.value(WEATHER_METER_ANEMOMETER_MAX);

#if WEATHER_METER_RAIN_RETURN_TICKS
    rain *= WEATHER_METER_AUX_RAIN_MM;
    if(rain > (WEATHER_METER_AUX_RAIN_MM * 3)) {
      printf("Rain: %lu.%lu mm, ", (rain / 10000), (rain % 10000));
#else
    if(rain >= 10) {
      printf("Rain: %u.%u mm, ", (rain / 10), (rain % 10));
#endif
    } else {
      printf("Rain: 0.%lu mm, ", rain);
    }

    printf("Wind dir: %u.%01u deg, ", (wind_dir / 10), (wind_dir % 10));
    printf("(%u.%01u deg avg)\n", (wind_dir_avg_2m / 10), (wind_dir_avg_2m % 10));
    printf("Wind speed: %u m/h ", wind_speed);
    printf("(%u m/h avg, %u m/h 2m avg, %u m/h max)\n\n", wind_speed_avg,
                                                          wind_speed_avg_2m,
                                                          wind_speed_max);
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

