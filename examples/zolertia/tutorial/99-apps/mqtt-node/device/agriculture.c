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
#include "dev/tsl256x.h"
#include "dev/bmpx8x.h"
#include "agriculture.h"
#include "mqtt-res.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_APP
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
  agriculture_sensors.sensor[AGRICULTURE_SENSOR_TEMP].value = sht25.value(SHT25_VAL_TEMP);
  agriculture_sensors.sensor[AGRICULTURE_SENSOR_HUMD].value = sht25.value(SHT25_VAL_HUM);
  SENSORS_DEACTIVATE(sht25);

  SENSORS_ACTIVATE(tsl256x);
  agriculture_sensors.sensor[AGRICULTURE_SENSOR_LIGHT].value = tsl256x.value(TSL256X_VAL_READ);
  SENSORS_DEACTIVATE(tsl256x);

  SENSORS_ACTIVATE(bmpx8x);
  agriculture_sensors.sensor[AGRICULTURE_SENSOR_PRES].value = bmpx8x.value(BMPx8x_READ_PRESSURE);
  SENSORS_DEACTIVATE(bmpx8x);

  /* Check the sensor values and publish alarms if required, else send the data
   * to any subscriber
   */
  mqtt_sensor_check(&agriculture_sensors, agriculture_sensors_alarm_event,
                    agriculture_sensors_data_event);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(agriculture_sensors_process, ev, data)
{
  static struct etimer et;

  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load sensor defaults */
  agriculture_sensors.num = 0;

  mqtt_sensor_register(&agriculture_sensors, AGRICULTURE_SENSOR_TEMP,
                       DEFAULT_SHT25_TEMP_MIN, DEFAULT_PUBLISH_EVENT_TEMP,
                       DEFAULT_PUBLISH_ALARM_TEMP, DEFAULT_SUBSCRIBE_CMD_TEMPTHR,
                       DEFAULT_SHT25_TEMP_MIN, DEFAULT_SHT25_TEMP_MAX,
                       DEFAULT_TEMP_THRESH, DEFAULT_TEMP_THRESL, 100);

  mqtt_sensor_register(&agriculture_sensors, AGRICULTURE_SENSOR_HUMD,
                       DEFAULT_SHT25_HUMD_MIN, DEFAULT_PUBLISH_EVENT_HUMD,
                       DEFAULT_PUBLISH_ALARM_HUMD, DEFAULT_SUBSCRIBE_CMD_HUMDTHR,
                       DEFAULT_SHT25_HUMD_MIN, DEFAULT_SHT25_HUMD_MAX,
                       DEFAULT_HUMD_THRESH, DEFAULT_HUMD_THRESL, 100);

  mqtt_sensor_register(&agriculture_sensors, AGRICULTURE_SENSOR_LIGHT,
                       DEFAULT_TSL2563_LIGHT_MIN, DEFAULT_PUBLISH_EVENT_LIGHT,
                       DEFAULT_PUBLISH_ALARM_LIGHT, DEFAULT_SUBSCRIBE_CMD_LIGHTHR,
                       DEFAULT_TSL2563_LIGHT_MIN, DEFAULT_TSL2563_LIGHT_MIN,
                       DEFAULT_LIGHT_THRESH, DEFAULT_LIGHT_THRESL, 0);

  mqtt_sensor_register(&agriculture_sensors, AGRICULTURE_SENSOR_PRES,
                       DEFAULT_BMP180_PRES_MIN, DEFAULT_PUBLISH_EVENT_PRES,
                       DEFAULT_PUBLISH_ALARM_PRES, DEFAULT_SUBSCRIBE_CMD_PRESTHR,
                       DEFAULT_BMP180_PRES_MIN, DEFAULT_BMP180_PRES_MAX,
                       DEFAULT_PRES_THRESH, DEFAULT_PRES_THRESL, 10);

  /* Sanity check */
  if(agriculture_sensors.num != DEFAULT_SENSORS_NUM) {
    printf("Agriculture sensors: error! number of sensors mismatch!\n");
    PROCESS_EXIT();
  }

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

    } else if(ev == sensors_stop_event) {
      PRINTF("Agriculture sensors: sensor readings paused\n");
      etimer_stop(&et);
    } else if(ev == sensors_restart_event) {
      PRINTF("Agriculture sensors: sensor readings enabled\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
