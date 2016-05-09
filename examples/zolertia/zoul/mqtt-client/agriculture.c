/*
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
#include "contiki.h"
#include "sys/etimer.h"
#include "dev/sht25.h"
#include "agriculture.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
sensor_values_t agriculture_sensors;
/*---------------------------------------------------------------------------*/
process_event_t agriculture_sensors_data_event;
process_event_t agriculture_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
PROCESS(agriculture_sensors_process, "Agriculture sensor process");
/*---------------------------------------------------------------------------*/
static void
poll_sensors(void)
{
  /* Poll the temperature and humidity sensor */
  SENSORS_ACTIVATE(sht25);
  fridge_sensors.sensor[AGRI_SENSOR_TEMP].value = sht25.value(SHT25_VAL_TEMP);
  fridge_sensors.sensor[AGRI_SENSOR_HUMD].value = sht25.value(SHT25_VAL_HUM);
  SENSORS_DEACTIVATE(sht25);

  PRINTF("Fridge sensors: Temperature %d.%02u Humidity %u.%02u\n",
         fridge_sensors.sensor[AGRI_SENSOR_TEMP].value / 100,
         fridge_sensors.sensor[AGRI_SENSOR_TEMP].value % 100,
         fridge_sensors.sensor[AGRI_SENSOR_HUMD].value / 100,
         fridge_sensors.sensor[AGRI_SENSOR_HUMD].value % 100);

  /* Check for valid values, if a mishap is found (i.e sensor not present but
   * enabled, then use the default unused-value and let is skip the checks
   * further below
   */
  if((fridge_sensors.sensor[AGRI_SENSOR_TEMP].value < DEFAULT_SHT25_TEMP_MIN)
    || (fridge_sensors.sensor[AGRI_SENSOR_TEMP].value > DEFAULT_SHT25_TEMP_MAX)) {
    PRINTF("Fridge sensors: temperature value invalid: should be between %d and %d\n",
           DEFAULT_SHT25_TEMP_MIN, DEFAULT_SHT25_TEMP_MAX);
    fridge_sensors.sensor[AGRI_SENSOR_TEMP].value = DEFAULT_TEMP_NOT_USED;
  }

  if((fridge_sensors.sensor[AGRI_SENSOR_HUMD].value < DEFAULT_SHT25_HUMD_MIN)
    || (fridge_sensors.sensor[AGRI_SENSOR_HUMD].value > DEFAULT_SHT25_HUMD_MAX)) {
    PRINTF("Fridge sensors: humidity value invalid: should be between %d and %d\n",
           DEFAULT_SHT25_HUMD_MIN, DEFAULT_SHT25_HUMD_MAX);
    fridge_sensors.sensor[AGRI_SENSOR_HUMD].value = DEFAULT_HUMD_NOT_USED;
  }

  /* Publish a temperature alarm
   * It has a higher priority than the humidity alarm, to avoid publishing
   * to two topics at the same time
   */

  if(fridge_sensors.sensor[AGRI_SENSOR_TEMP].value >=
     fridge_sensors.sensor[AGRI_SENSOR_TEMP].threshold) {
    PRINTF("Fridge sensors: temperature alarm! (over %d.%02u)\n",
           fridge_sensors.sensor[AGRI_SENSOR_TEMP].threshold / 100,
           fridge_sensors.sensor[AGRI_SENSOR_TEMP].threshold % 100);
    process_post(PROCESS_BROADCAST, fridge_sensors_alarm_event,
                 &fridge_sensors.sensor[AGRI_SENSOR_TEMP]);
    return;
  }

  if(fridge_sensors.sensor[AGRI_SENSOR_HUMD].value >=
     fridge_sensors.sensor[AGRI_SENSOR_HUMD].threshold) {
    PRINTF("Fridge sensors: humidity alarm! (over %d.%02u)\n", 
           fridge_sensors.sensor[AGRI_SENSOR_HUMD].threshold / 100,
           fridge_sensors.sensor[AGRI_SENSOR_HUMD].threshold % 100);
    process_post(PROCESS_BROADCAST, fridge_sensors_alarm_event,
                 &fridge_sensors.sensor[AGRI_SENSOR_HUMD]);
    return;
  }

  /* Post a process notifying there's new sensor data available */
  process_post(PROCESS_BROADCAST, fridge_sensors_data_event, &fridge_sensors);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(agriculture_sensors_process, ev, data)
{
  static struct etimer et;

  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load sensor defaults */
  agriculture_sensors.num = DEFAULT_CONF_SENSORS_NUM;

  agriculture_sensors.sensor[AGRI_SENSOR_TEMP].value = DEFAULT_TEMP_NOT_USED;
  agriculture_sensors.sensor[AGRI_SENSOR_HUMD].value = DEFAULT_HUMD_NOT_USED;

  agriculture_sensors.sensor[AGRI_SENSOR_TEMP].threshold = DEFAULT_TEMP_THRESH;
  agriculture_sensors.sensor[AGRI_SENSOR_HUMD].threshold = DEFAULT_HUMD_THRESH;

  agriculture_sensors.sensor[AGRI_SENSOR_TEMP].min = DEFAULT_SHT25_TEMP_MIN;
  agriculture_sensors.sensor[AGRI_SENSOR_HUMD].min = DEFAULT_SHT25_HUMD_MIN;
  agriculture_sensors.sensor[AGRI_SENSOR_TEMP].max = DEFAULT_SHT25_TEMP_MAX;
  agriculture_sensors.sensor[AGRI_SENSOR_HUMD].max = DEFAULT_SHT25_HUMD_MAX;

  memcpy(agriculture_sensors.sensor[AGRI_SENSOR_TEMP].sensor_name,
         DEFAULT_PUBLISH_EVENT_TEMP, strlen(DEFAULT_PUBLISH_EVENT_TEMP));
  memcpy(agriculture_sensors.sensor[AGRI_SENSOR_HUMD].sensor_name,
         DEFAULT_PUBLISH_EVENT_HUMD, strlen(DEFAULT_PUBLISH_EVENT_HUMD));

  memcpy(agriculture_sensors.sensor[AGRI_SENSOR_TEMP].alarm_name,
         DEFAULT_PUBLISH_ALARM_TEMP, strlen(DEFAULT_PUBLISH_ALARM_TEMP));
  memcpy(agriculture_sensors.sensor[AGRI_SENSOR_HUMD].alarm_name,
         DEFAULT_PUBLISH_ALARM_HUMD, strlen(DEFAULT_PUBLISH_ALARM_HUMD));

  memcpy(agriculture_sensors.sensor[AGRI_SENSOR_TEMP].sensor_config,
         DEFAULT_SUBSCRIBE_CFG_TEMPTHR, strlen(DEFAULT_SUBSCRIBE_CFG_TEMPTHR));
  memcpy(agriculture_sensors.sensor[AGRI_SENSOR_HUMD].sensor_config,
         DEFAULT_SUBSCRIBE_CFG_HUMDTHR, strlen(DEFAULT_SUBSCRIBE_CFG_HUMDTHR));

  /* Get an event ID for our events */
  agriculture_sensors_data_event = process_alloc_event();
  agriculture_sensors_alarm_event = process_alloc_event();

  /* Start the periodic process */
  etimer_set(&et, DEFAULT_SAMPLING_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      poll_sensors();
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
