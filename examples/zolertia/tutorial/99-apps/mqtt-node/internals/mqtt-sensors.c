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
#include "mqtt-sensors.h"
#include "../mqtt-client.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_CLIENT
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#if DEFAULT_SENSORS_NUM
PROCESS(mqtt_sensors_process, "mqtt_sensors_process");
PROCESS_NAME(PLATFORM_NAME(MQTT_PLATFORM,_process));
/*---------------------------------------------------------------------------*/
static void
mqtt_sensor_strings(char *dest, char *topic)
{
  if(topic != NULL) {
    memcpy(dest, topic, strlen(topic));
  } else {
    /* Zero-length string */
    dest[0] = '\0';
  }
}
/*---------------------------------------------------------------------------*/
/* Auxiliary function to fill the sensor_values_t structure */
int
mqtt_sensor_register(sensor_values_t *reg, uint8_t i, uint16_t val, char *name,
                     char *alarm, char *config, int16_t min, int16_t max,
                     int16_t thresh, int16_t thresl, uint16_t pres)
{
  if((strlen(name) > SENSOR_NAME_STRING) || (strlen(alarm) > SENSOR_NAME_STRING)
     || (strlen(config) > SENSOR_NAME_STRING)) {
    return -1;
  }

  reg->num++;
  reg->sensor[i].fail_value = val;
  reg->sensor[i].over_threshold = thresh;
  reg->sensor[i].below_threshold = thresl;
  reg->sensor[i].min = min;
  reg->sensor[i].max = max;
  reg->sensor[i].pres = pres;

  mqtt_sensor_strings(reg->sensor[i].sensor_name, name);
  mqtt_sensor_strings(reg->sensor[i].alarm_name, alarm);
  mqtt_sensor_strings(reg->sensor[i].sensor_config, config);
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void
mqtt_sensor_check(sensor_values_t *reg, process_event_t alarm,
                  process_event_t data)
{
  uint8_t i;

  /* Check if the values are valid, else use minimum value as default, check the
   * alarms threshold as well
   */
  for(i=0; i < reg->num; i++) {
    if((reg->sensor[i].value < reg->sensor[i].min) ||
      (reg->sensor[i].value > reg->sensor[i].max)) {
      PRINTF("  > MQTT sensors: %s value %d invalid: should be between %d and %d\n",
             reg->sensor[i].sensor_name, reg->sensor[i].value,
             reg->sensor[i].min, reg->sensor[i].max);
      reg->sensor[i].value = reg->sensor[i].fail_value;
    } else {
      if(strlen(reg->sensor[i].sensor_name)) {
        PRINTF("  > MQTT sensors: %s value %d\n", reg->sensor[i].sensor_name,
                                                  reg->sensor[i].value);
      }
    }

    /* Currently we are limiting the alarms to be first-heard, first-served,
     * meaning the first alarm occurrence will be sent, and no other check will
     * be done for remaining variables.  One way to fix this would be to flag
     * the variable as "alarmed", and remove the flag after the alarm timeout,
     * allowing other variables to send an alarm during the timeout
     */
    if(((reg->sensor[i].value < reg->sensor[i].below_threshold) ||
      (reg->sensor[i].value > reg->sensor[i].over_threshold)) &&
      (strlen(reg->sensor[i].alarm_name))) {
      PRINTF("  > MQTT sensors: %s! (over %d, below %d)\n", reg->sensor[i].alarm_name,
                                                        reg->sensor[i].over_threshold,
                                                        reg->sensor[i].below_threshold);
      process_post(&mqtt_sensors_process, alarm, &reg->sensor[i]);
      return;
    }
  }

  /* Post a process notifying there's new sensor data available */
  process_post(&mqtt_sensors_process, data, reg);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_sensors_process, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("MQTT sensors process started\n");
  static uint16_t seq_nr_value = 0;

  while(1) {
    PROCESS_YIELD();

    /* Check for periodic publish events */
    if(ev == SENSORS_NAME(MQTT_SENSORS,_sensors_data_event)) {
      seq_nr_value++;

      /* The `pub_interval_check` is an external struct defined in mqtt-client */
      if(!(seq_nr_value % conf.pub_interval_check)) {
        sensor_values_t *msgPtr = (sensor_values_t *) data;
        publish_event(msgPtr);
      }
    }

    /* Check for alarms */
    if(ev == SENSORS_NAME(MQTT_SENSORS,_sensors_alarm_event)) {
      sensor_val_t *sensorPtr = (sensor_val_t *) data;
      publish_alarm(sensorPtr);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#endif /* DEFAULT_SENSORS_NUM */
/*---------------------------------------------------------------------------*/
/** @} */
