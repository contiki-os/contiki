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
#include "dev/adc-zoul.h"
#include "dev/weather-meter.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
#include "sys/timer.h"
#include "sys/ctimer.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define DEBOUNCE_DURATION  (CLOCK_SECOND >> 6)
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
static struct ctimer ct;
static struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
typedef struct {
  uint16_t ticks;
  uint16_t value;
  uint8_t int_en;
  uint16_t int_thres;
} weather_meter_sensors_t;

typedef struct {
  uint16_t value_max;
  uint64_t ticks_avg;
  uint64_t value_avg;
  uint32_t value_buf_xm;
  uint16_t value_avg_xm;
} weather_meter_ext_t;

typedef struct {
  uint16_t wind_vane;
  weather_meter_sensors_t rain_gauge;
  weather_meter_sensors_t anemometer;
} weather_meter_sensors;

typedef struct {
  int32_t value_buf_xm;
  int16_t value_prev;
  int16_t value_avg_xm;
} weather_meter_wind_vane_ext_t;

static weather_meter_sensors weather_sensors;
static weather_meter_ext_t anemometer;
static weather_meter_wind_vane_ext_t wind_vane;
/*---------------------------------------------------------------------------*/
typedef struct {
  uint16_t mid_point;
  uint16_t degree;
} wind_vane_mid_point_t;

/* From the datasheet we adjusted the values for a 3V divider, using a 10K
 * resistor, the check values are the following:
 * --------------------+------------------+-------------------------------
 * Direction (Degrees)  Resistance (Ohms)  Voltage (mV)
 *     0                   33k                 2532.55  *
 *     22.5                6.57k               1308.44  *
 *     45                  8.2k                1486.81  *
 *     67.5                891                 269.97   *
 *     90                  1k                  300.00   *
 *     112.5               688                 212.42   *
 *     135                 2.2k                595.08   *
 *     157.5               1.41k               407.80   *
 *     180                 3.9k                925.89   *
 *     202.5               3.14k               788.58   *
 *     225                 16k                 2030.76  *
 *     247.5               14.12k              1930.84  *
 *     270                 120k                3046.15  *
 *     292.5               42.12k              2666.84  *
 *     315                 64.9k               2859.41  *
 *     337.5               21.88k              2264.86  *
 * --------------------+------------------+-------------------------------
 */
static const wind_vane_mid_point_t wind_vane_table[16] = {
  { 2124,   1125  },
  { 2699,   675   },
  { 3000,   900   },
  { 4078,   1575  },
  { 5950,   1350  },
  { 7885,   2025  },
  { 9258,   1800  },
  { 13084,  225   },
  { 14868,  450   },
  { 19308,  2475  },
  { 20307,  2250  },
  { 22648,  3375  },
  { 25325,  0     },
  { 26668,  2925  },
  { 28594,  3150  },
  { 30461,  2700  },
};
/*---------------------------------------------------------------------------*/
static int
weather_meter_wind_vane_degrees(uint16_t value)
{
  uint8_t i;
  for(i = 0; i < 16; i++) {
    if(value <= wind_vane_table[i].mid_point) {
      return (int)wind_vane_table[i].degree;
    } else {
      if(i == 15) {
        return (int)wind_vane_table[i].degree;
      }
    }
  }

  PRINTF("Weather: invalid wind vane value\n");
  return WEATHER_METER_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
weather_meter_get_wind_dir(void)
{
  weather_sensors.wind_vane = adc_zoul.value(WIND_VANE_ADC);
  if((int16_t)weather_sensors.wind_vane < 0) {
    weather_sensors.wind_vane = 0;
  }
  return weather_meter_wind_vane_degrees(weather_sensors.wind_vane);
}
/*---------------------------------------------------------------------------*/
static void
ct_callback(void *ptr)
{
  uint32_t wind_speed;
  int16_t wind_dir;
  int16_t wind_dir_delta;

  /* Disable to make the calculations in an interrupt-safe context */
  GPIO_DISABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE,
                         ANEMOMETER_SENSOR_PIN_MASK);
  wind_speed = weather_sensors.anemometer.ticks;
  wind_speed *= WEATHER_METER_ANEMOMETER_SPEED_1S;
  weather_sensors.anemometer.value = (uint16_t)wind_speed;
  anemometer.ticks_avg++;
  anemometer.value_avg += weather_sensors.anemometer.value;
  anemometer.value_buf_xm += weather_sensors.anemometer.value;

  /* Take maximum value */
  if(weather_sensors.anemometer.value > anemometer.value_max) {
    anemometer.value_max = weather_sensors.anemometer.value;
  }

  /* Mitsuta method to get the wind direction average */
  wind_dir = weather_meter_get_wind_dir();
  wind_dir_delta = wind_dir - wind_vane.value_prev;

  if(wind_dir_delta < -1800) {
    wind_vane.value_prev += wind_dir_delta + 3600;
  } else if(wind_dir_delta > 1800) {
    wind_vane.value_prev += wind_dir_delta - 3600;
  } else {
    wind_vane.value_prev += wind_dir_delta;
  }

  wind_vane.value_buf_xm += wind_vane.value_prev;

  /* Calculate the 2 minute average */
  if(!(anemometer.ticks_avg % WEATHER_METER_AVG_PERIOD)) {
    PRINTF("\nWeather: calculate the %u averages ***\n", WEATHER_METER_AVG_PERIOD);

    if(anemometer.value_buf_xm) {
      anemometer.value_avg_xm = anemometer.value_buf_xm / WEATHER_METER_AVG_PERIOD;
      anemometer.value_buf_xm = 0;
    } else {
      anemometer.value_avg_xm = 0;
    }

    if(wind_vane.value_buf_xm >= 0) {
      wind_vane.value_buf_xm = wind_vane.value_buf_xm / WEATHER_METER_AVG_PERIOD;
      wind_vane.value_avg_xm = wind_vane.value_buf_xm;
    } else {
      wind_vane.value_buf_xm = ABS(wind_vane.value_buf_xm) / WEATHER_METER_AVG_PERIOD;
      wind_vane.value_avg_xm = wind_vane.value_buf_xm;
      wind_vane.value_avg_xm = ~wind_vane.value_avg_xm + 1;
    }

    if(wind_vane.value_avg_xm >= 3600) {
      wind_vane.value_avg_xm -= 3600;
    } else if(wind_vane.value_avg_xm < 0) {
      wind_vane.value_avg_xm += 3600;
    }

    wind_vane.value_buf_xm = 0;
    wind_vane.value_prev = wind_dir;
  }

  /* Check for roll-over */
  if(!anemometer.ticks_avg) {
    anemometer.value_avg = 0;
  }

  weather_sensors.anemometer.ticks = 0;

  /* Enable the interrupt again */
  GPIO_ENABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE,
                        ANEMOMETER_SENSOR_PIN_MASK);

  ctimer_set(&ct, CLOCK_SECOND, ct_callback, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS(weather_meter_int_process, "Weather meter interrupt process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(weather_meter_int_process, ev, data)
{
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

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
  uint64_t aux;

  if((type != WEATHER_METER_ANEMOMETER) &&
     (type != WEATHER_METER_RAIN_GAUGE) &&
     (type != WEATHER_METER_WIND_VANE) &&
     (type != WEATHER_METER_WIND_VANE_AVG_X) &&
     (type != WEATHER_METER_ANEMOMETER_AVG) &&
     (type != WEATHER_METER_ANEMOMETER_AVG_X) &&
     (type != WEATHER_METER_ANEMOMETER_MAX)) {
    PRINTF("Weather: requested an invalid sensor value\n");
    return WEATHER_METER_ERROR;
  }

  if(!enabled) {
    PRINTF("Weather: module is not configured\n");
    return WEATHER_METER_ERROR;
  }

  switch(type) {
  case WEATHER_METER_WIND_VANE:
    return weather_meter_get_wind_dir();

  case WEATHER_METER_WIND_VANE_AVG_X:
    return wind_vane.value_avg_xm;

  case WEATHER_METER_ANEMOMETER:
    return weather_sensors.anemometer.value;

  case WEATHER_METER_ANEMOMETER_AVG:
    if(anemometer.value_avg <= 0) {
      return (uint16_t)anemometer.value_avg;
    }
    aux = anemometer.value_avg / anemometer.ticks_avg;
    return (uint16_t)aux;

  case WEATHER_METER_ANEMOMETER_AVG_X:
    return anemometer.value_avg_xm;

  case WEATHER_METER_ANEMOMETER_MAX:
    return anemometer.value_max;

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

    anemometer.value_avg = 0;
    anemometer.ticks_avg = 0;

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
    adc_zoul.configure(SENSORS_HW_INIT, WIND_VANE_ADC);

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

    /* Initialize here prior the first second tick */
    wind_vane.value_prev = weather_meter_get_wind_dir();

    ctimer_set(&ct, CLOCK_SECOND, ct_callback, NULL);

    GPIO_ENABLE_INTERRUPT(ANEMOMETER_SENSOR_PORT_BASE, ANEMOMETER_SENSOR_PIN_MASK);
    GPIO_ENABLE_INTERRUPT(RAIN_GAUGE_SENSOR_PORT_BASE, RAIN_GAUGE_SENSOR_PIN_MASK);
    NVIC_EnableIRQ(ANEMOMETER_SENSOR_VECTOR);
    NVIC_EnableIRQ(RAIN_GAUGE_SENSOR_VECTOR);

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

