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
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#include "dev/button-sensor.h"
#include "mqtt-res.h"
#include "remote.h"
#include "../mqtt-client.h"

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
/* We include here the platform process as we post alarms directly */
PROCESS_NAME(PLATFORM_NAME(MQTT_PLATFORM,_process));
/*---------------------------------------------------------------------------*/
sensor_values_t remote_sensors;
/*---------------------------------------------------------------------------*/
process_event_t remote_sensors_data_event;
process_event_t remote_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
PROCESS(remote_sensors_process, "RE-Mote sensor process");
/*---------------------------------------------------------------------------*/
static void
poll_sensors(void)
{
  /* Note: as we are using int16_t values, the alarms and thresholds have this
   * wide.  The cc2538 core, battery and ADC return values scaled down to 10^-3,
   * so we could end up with values beyond 32767 being treated as negative.
   * For this cases we discard the least significant digit
   */

  remote_sensors.sensor[REMOTE_SENSOR_TEMP].value =
             cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED) / 10;
  remote_sensors.sensor[REMOTE_SENSOR_BATT].value =
             vdd3_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED) / 10;
  remote_sensors.sensor[REMOTE_SENSOR_ADC1].value =
             adc_zoul.value(ZOUL_SENSORS_ADC1) / 10;
  remote_sensors.sensor[REMOTE_SENSOR_ADC3].value =
             adc_zoul.value(ZOUL_SENSORS_ADC3) / 10;

  /* Check the sensor values and publish alarms if required, else send the data
   * to any subscriber
   */
  mqtt_sensor_check(&remote_sensors, remote_sensors_alarm_event,
                    remote_sensors_data_event);

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_sensors_process, ev, data)
{
  static struct etimer et;

  /* This is where our process start */
  PROCESS_BEGIN();

  /* Configure the ADC ports */
  adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

  /* Load sensor defaults */
  remote_sensors.num = 0;

  mqtt_sensor_register(&remote_sensors, REMOTE_SENSOR_TEMP,
                       DEFAULT_CC2538_TEMP_MIN, DEFAULT_PUBLISH_EVENT_TEMP,
                       NULL, NULL, DEFAULT_CC2538_TEMP_MIN,
                       DEFAULT_CC2538_TEMP_MAX, DEFAULT_TEMP_THRESH,
                       DEFAULT_TEMP_THRESL, 100);

  mqtt_sensor_register(&remote_sensors, REMOTE_SENSOR_BATT,
                       DEFAULT_CC2538_BATT_MIN, DEFAULT_PUBLISH_EVENT_BATT,
                       NULL, NULL, DEFAULT_CC2538_BATT_MIN,
                       DEFAULT_CC2538_BATT_MAX, DEFAULT_BATT_THRESH,
                       DEFAULT_BATT_THRESL, 100);

  mqtt_sensor_register(&remote_sensors, REMOTE_SENSOR_ADC1,
                       DEFAULT_CC2538_ADC1_MIN, DEFAULT_PUBLISH_EVENT_ADC1,
                       NULL, NULL, DEFAULT_CC2538_ADC1_MIN,
                       DEFAULT_CC2538_ADC1_MAX, DEFAULT_ADC1_THRESH,
                       DEFAULT_ADC1_THRESL, 100);

  mqtt_sensor_register(&remote_sensors, REMOTE_SENSOR_ADC3,
                       DEFAULT_CC2538_ADC3_MIN, DEFAULT_PUBLISH_EVENT_ADC3,
                       NULL, NULL, DEFAULT_CC2538_ADC3_MIN,
                       DEFAULT_CC2538_ADC3_MAX, DEFAULT_ADC3_THRESH,
                       DEFAULT_ADC3_THRESL, 100);

  /* We post alarms directly to the platform, instead of having the mqtt-sensors
   * checking for occurences
   */
  mqtt_sensor_register(&remote_sensors, REMOTE_SENSOR_BUTN,
                       DEFAULT_CC2538_BUTN_MIN, NULL, DEFAULT_PUBLISH_ALARM_BUTN,
                       NULL, DEFAULT_CC2538_BUTN_MIN,
                       DEFAULT_CC2538_BUTN_MAX, DEFAULT_BUTN_THRESH,
                       DEFAULT_BUTN_THRESL, 0);

  /* Sanity check */
  if(remote_sensors.num != DEFAULT_SENSORS_NUM) {
    printf("remote sensors: error! number of sensors mismatch!\n");
    PROCESS_EXIT();
  }

  /* Get an event ID for our events */
  remote_sensors_data_event = process_alloc_event();
  remote_sensors_alarm_event = process_alloc_event();

  /* Start the periodic process */
  etimer_set(&et, DEFAULT_SAMPLING_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      poll_sensors();
      etimer_reset(&et);

    } else if(ev == sensors_event) {
      if(data == &button_sensor) {
        if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
           BUTTON_SENSOR_PRESSED_LEVEL) {
          PRINTF("RE-Mote: Button pressed\n");
        } else {
          PRINTF("RE-Mote: ...and button released!\n");
          remote_sensors.sensor[REMOTE_SENSOR_BUTN].value++;
          process_post(&PLATFORM_NAME(MQTT_PLATFORM, _process),
                       remote_sensors_alarm_event,
                       &remote_sensors.sensor[REMOTE_SENSOR_BUTN]);
        }
      }

    } else if(ev == sensors_stop_event) {
      PRINTF("RE-Mote: sensor readings paused\n");
      etimer_stop(&et);
    } else if(ev == sensors_restart_event) {
      PRINTF("RE-Mote: sensor readings enabled\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
