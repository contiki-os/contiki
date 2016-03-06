/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Theo van Daele <theo.van.daele@nxp.com>
 *
 */
#include "contiki.h"
#include "sys/etimer.h"
#include "lib/sensors.h"
#include "ht-sensor.h"
#include <stdlib.h>
#include <HtsDriver.h>

/*---------------------------------------------------------------------------*/
/* LOCAL DEFINITIONS                                                         */
/*---------------------------------------------------------------------------*/
/* #define DEBUG */
#ifdef DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

typedef enum {
  HT_SENSOR_STATUS_NOT_INIT = 0,
  HT_SENSOR_STATUS_INIT,
  HT_SENSOR_STATUS_NOT_ACTIVE = HT_SENSOR_STATUS_INIT,
  HT_SENSOR_STATUS_ACTIVE
} ht_sensor_status_t;

/* Absolute delta in light or humidity level needed to generate event */
#define DELTA_TEMP_SENSOR_VALUE  1
#define DELTA_HUM_SENSOR_VALUE   1

/*---------------------------------------------------------------------------*/
/* LOCAL DATA DEFINITIONS                                                    */
/*---------------------------------------------------------------------------*/
const struct sensors_sensor ht_sensor;
volatile static ht_sensor_status_t ht_sensor_status = HT_SENSOR_STATUS_NOT_INIT;
static int prev_temp_event_val = 0;
static int prev_hum_event_val = 0;
static int temp_sensor_value = 0;
static int hum_sensor_value = 0;

/*---------------------------------------------------------------------------*/
/* LOCAL FUNCTION PROTOTYPES                                                 */
/*---------------------------------------------------------------------------*/
PROCESS(HTSensorSampling, "Humidity/Temperature sensor");

/*---------------------------------------------------------------------------*/
/* PUBLIC FUNCTIONS                                                          */
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == SENSORS_HW_INIT) {
    PRINTF("SENSORS_HW_INIT\n");
    ht_sensor_status = HT_SENSOR_STATUS_INIT;
    process_start(&HTSensorSampling, NULL);
    return 1;
  } else if(type == SENSORS_ACTIVE) {
    if(ht_sensor_status != HT_SENSOR_STATUS_NOT_INIT) {
      if(value) {
        /* ACTIVATE SENSOR */
        vHTSreset();
        prev_temp_event_val = 0;
        prev_hum_event_val = 0;
        /* Activate ht sensor. Start sampling */
        PRINTF("HT SENSOR ACTIVATED\n");
        ht_sensor_status = HT_SENSOR_STATUS_ACTIVE;
        process_post(&HTSensorSampling, PROCESS_EVENT_MSG, (void *)&ht_sensor_status);
      } else {
        /* DE-ACTIVATE SENSOR */
        PRINTF("HT SENSOR DE-ACTIVATED\n");
        ht_sensor_status = HT_SENSOR_STATUS_NOT_ACTIVE;
        process_post(&HTSensorSampling, PROCESS_EVENT_MSG, (void *)&ht_sensor_status);
      }
      return 1;
    } else {
      /* HT sensor must be intialised before being (de)-activated */
      PRINTF("ERROR: NO HW_INIT HT SENSOR\n");
      return 0;
    }
  } else {
    /* Non valid type */
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  if(type == SENSORS_ACTIVE) {
    return ht_sensor_status == HT_SENSOR_STATUS_ACTIVE;
  } else if(type == SENSORS_READY) {
    return ht_sensor_status != HT_SENSOR_STATUS_NOT_INIT;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* type: HT_SENSOR_TEMP is to return temperature
           !=HT_SENSOR_TEMP is to return humidity */
  if(type == HT_SENSOR_TEMP) {
    return temp_sensor_value;
  } else {
    return hum_sensor_value;
  }
}
/*---------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS                                                           */
/*---------------------------------------------------------------------------*/
/* Process to get ht sensor value.
   ht sensor is sampled. Sampling stopped when sensor is de-activated.
   Event is generated if temp and/or hum value changed at least the value DELTA_TEMP_SENSOR_VALUE
   or DELTA_HUM_SENSOR_VALUE since last event. */
PROCESS_THREAD(HTSensorSampling, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;

  etimer_set(&et, CLOCK_SECOND);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) || (ev == PROCESS_EVENT_MSG));
    if(ev == PROCESS_EVENT_TIMER) {
      /* Handle sensor reading. */
      vHTSstartReadTemp();
      temp_sensor_value = u16HTSreadTempResult();
      PRINTF("Temperature sample: %d\n", temp_sensor_value);
      vHTSstartReadHumidity();
      hum_sensor_value = u16HTSreadHumidityResult();
      PRINTF("Humidity sample: %d\n", hum_sensor_value);
      if((abs(temp_sensor_value - prev_temp_event_val) > DELTA_TEMP_SENSOR_VALUE) ||
         (abs(hum_sensor_value - prev_hum_event_val) > DELTA_HUM_SENSOR_VALUE)) {
        prev_temp_event_val = temp_sensor_value;
        prev_hum_event_val = hum_sensor_value;
        sensors_changed(&ht_sensor);
      }
      etimer_reset(&et);
    } else {
      /* ev == PROCESS_EVENT_MSG */
      if(*(int *)data == HT_SENSOR_STATUS_NOT_ACTIVE) {
        /* Stop sampling */
        etimer_stop(&et);
      } else if((*(int *)data == HT_SENSOR_STATUS_ACTIVE)) {
        /* restart sampling */
        etimer_restart(&et);
      }
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Sensor defintion for sensor module */
SENSORS_SENSOR(ht_sensor, HT_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
