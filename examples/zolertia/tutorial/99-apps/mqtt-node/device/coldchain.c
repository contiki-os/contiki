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
#include "dev/dht22.h"
#include "dev/tsl256x.h"
#include "coldchain.h"
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
sensor_values_t coldchain_sensors;
/*---------------------------------------------------------------------------*/
process_event_t coldchain_sensors_data_event;
process_event_t coldchain_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
PROCESS(coldchain_sensors_process, "coldchain sensor process");
/*---------------------------------------------------------------------------*/
static void
poll_sensors(void)
{
  int temp, humd;

  /* Poll the temperature, humidity and light sensors */
  coldchain_sensors.sensor[COLDCHAIN_SENSOR_LGHT].value = tsl256x.value(TSL256X_VAL_READ);

  dht22_read_all(&temp, &humd);

  coldchain_sensors.sensor[COLDCHAIN_SENSOR_TEMP].value = (int16_t)temp;
  coldchain_sensors.sensor[COLDCHAIN_SENSOR_HUMD].value = (int16_t)humd;

  /* Check the sensor values and publish alarms if required, else send the data
   * to any subscriber
   */
  mqtt_sensor_check(&coldchain_sensors, coldchain_sensors_alarm_event,
                    coldchain_sensors_data_event);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coldchain_sensors_process, ev, data)
{
  static struct etimer et;

  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load sensor defaults */
  coldchain_sensors.num = 0;

  mqtt_sensor_register(&coldchain_sensors, COLDCHAIN_SENSOR_TEMP,
                       DEFAULT_TEMP_MIN, DEFAULT_PUBLISH_EVENT_TEMP,
                       DEFAULT_PUBLISH_ALARM_TEMP, DEFAULT_SUBSCRIBE_CFG_TEMPTHR,
                       DEFAULT_TEMP_MIN, DEFAULT_TEMP_MAX,
                       DEFAULT_TEMP_THRESH, DEFAULT_TEMP_THRESL, 10);

  mqtt_sensor_register(&coldchain_sensors, COLDCHAIN_SENSOR_HUMD,
                       DEFAULT_HUMD_MIN, DEFAULT_PUBLISH_EVENT_HUMD,
                       DEFAULT_PUBLISH_ALARM_HUMD, DEFAULT_SUBSCRIBE_CFG_HUMDTHR,
                       DEFAULT_HUMD_MIN, DEFAULT_HUMD_MAX,
                       DEFAULT_HUMD_THRESH, DEFAULT_HUMD_THRESL, 10);

  mqtt_sensor_register(&coldchain_sensors, COLDCHAIN_SENSOR_LGHT,
                       DEFAULT_LGTH_MIN, DEFAULT_PUBLISH_EVENT_LGTH,
                       DEFAULT_PUBLISH_ALARM_LGTH, DEFAULT_SUBSCRIBE_CFG_LGTHTHR,
                       DEFAULT_LGTH_MIN, DEFAULT_LGTH_MAX,
                       DEFAULT_LGTH_THRESH, DEFAULT_LGTH_THRESL, 1);

  /* Sanity check */
  if(coldchain_sensors.num != DEFAULT_SENSORS_NUM) {
    printf("coldchain sensors: error! number of sensors mismatch!\n");
    PROCESS_EXIT();
  }

  /* Get an event ID for our events */
  coldchain_sensors_data_event = process_alloc_event();
  coldchain_sensors_alarm_event = process_alloc_event();

  /* Enable the sensors */
  SENSORS_ACTIVATE(dht22);
  SENSORS_ACTIVATE(tsl256x);

  /* Start the periodic process */
  etimer_set(&et, DEFAULT_SAMPLING_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      poll_sensors();
      etimer_reset(&et);
    } else if(ev == sensors_stop_event) {
      PRINTF("coldchain sensors: sensor readings paused\n");
      etimer_stop(&et);
    } else if(ev == sensors_restart_event) {
      PRINTF("coldchain sensors: sensor readings enabled\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
