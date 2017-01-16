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
#include "lib/random.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include "dev/sys-ctrl.h"
#include "mqtt-client.h"
#include "mqtt-sensors.h"
#include "mqtt-res.h"
#include "bluemix.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_PLATFORM
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#if DEFAULT_SENSORS_NUM
static char *buf_ptr;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/* Topic placeholders */
static char data_topic[CONFIG_PUB_TOPIC_LEN];
#endif
/*---------------------------------------------------------------------------*/
PROCESS(bluemix_process, "IBM bluemix MQTT process");
/*---------------------------------------------------------------------------*/
/* Include there the processes to include */
PROCESS_NAME(mqtt_res_process);
PROCESS_NAME(SENSORS_NAME(MQTT_SENSORS, _sensors_process));
#if DEFAULT_SENSORS_NUM
PROCESS_NAME(mqtt_sensors_process);
/*---------------------------------------------------------------------------*/
static struct etimer alarm_expired;
/*---------------------------------------------------------------------------*/
static int
add_pub_topic(uint16_t length, char *meaning, char *value,
              uint8_t first, uint8_t more)
{
  int len = 0;
  int pos = 0;

  if((buf_ptr == NULL) || (length <= 0)){
    PRINTF("bluemix: null buffer or lenght less than zero\n");
    return -1;
  }

  if(first) {
    len = snprintf(buf_ptr, length, "%s", "{\"d\":{");
    pos = len;
    buf_ptr += len;
  }

  len = snprintf(buf_ptr, (length - pos),
                 "\"%s\":\"%s\"", meaning, value);
 
  if(len < 0 || pos >= length) {
    PRINTF("bluemix: Buffer too short. Have %d, need %d + \\0\n", length, len);
    return -1;
  }

  pos += len;
  buf_ptr += len;

  if(more) {
    len = snprintf(buf_ptr, (length - pos), "%s", ",");
  } else {
    len = snprintf(buf_ptr, (length - pos), "%s", "}}");
  }

  pos += len;
  buf_ptr += len;

  return pos;
}
/*---------------------------------------------------------------------------*/
void
publish_alarm(sensor_val_t *sensor)
{
  uint16_t aux_int, aux_res;

  if(etimer_expired(&alarm_expired)) {

    /* Clear buffer */
    memset(app_buffer, 0, APP_BUFFER_SIZE);

    PRINTF("bluemix: Alarm! %s --> %u\n", sensor->alarm_name, sensor->value);
    aux_int = sensor->value;
    aux_res = sensor->value;

    if(sensor->pres > 0) {
      aux_int /= sensor->pres;
      aux_res %= sensor->pres;
    } else {
      aux_res = 0;
    }

    snprintf(app_buffer, APP_BUFFER_SIZE, aux_res > 9 ?
             "{\"d\":{\"%s\":%d.%02u}}" : "{\"d\":{\"%s\":%d.%01u0}}",
             sensor->alarm_name, aux_int, aux_res);

    publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));

    /* Schedule the timer to prevent flooding the broker with the same event */
    etimer_set(&alarm_expired, (CLOCK_SECOND * DEFAULT_ALARM_TIME));
  }
}
/*---------------------------------------------------------------------------*/
void
publish_event(sensor_values_t *msg)
{
  char aux[64];
  int len = 0;
  uint8_t i;
  uint16_t aux_int, aux_res;
  int remain = APP_BUFFER_SIZE;

  /* Clear buffer */
  memset(app_buffer, 0, APP_BUFFER_SIZE);

  /* Use the buf_ptr as pointer to the actual application buffer */
  buf_ptr = app_buffer;

  /* Retrieve our own IPv6 address
   * This is the starting value to be sent, the `first` argument should be 1,
   * and the `more` argument 1 as well, as we want to add more values to our
   * list
   */
  mqtt_res_u16_addr(aux, sizeof(aux));
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_ID, aux, 1, 1);
  remain =- len;

  /* Include the sensor values, if `sensor_name` is NULL discard */
  for(i=0; i < msg->num; i++) {
    if(msg->sensor[i].sensor_name != NULL) {
      memset(aux, 0, sizeof(aux));

      aux_int = msg->sensor[i].value;
      aux_res = msg->sensor[i].value;

      if(msg->sensor[i].pres > 0) {
        aux_int /= msg->sensor[i].pres;
        aux_res %= msg->sensor[i].pres;
      } else {
        aux_res = 0;
      }

      snprintf(aux, sizeof(aux), aux_res > 9 ? "%d.%02u" : "%d.%01u0",
               aux_int, aux_res);

      len = add_pub_topic(remain, msg->sensor[i].sensor_name, aux, 0, 1);
      remain =- len;
    }
  }

  mqtt_res_uptime(aux, sizeof(aux));
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_UPTIME, aux, 0, 1);
  remain =- len;

  mqtt_res_parent_addr(aux, sizeof(aux));
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_PARENT, aux, 0, 1);
  remain =- len;

  /* The last value to be sent, the `more` argument should be zero */
  mqtt_res_parent_rssi(aux, sizeof(aux));
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_RSSI, aux, 0, 0);

  PRINTF("bluemix: publish %s (%u)\n", app_buffer, strlen(app_buffer));
  publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));
}
#endif /* DEFAULT_SENSORS_NUM*/
/*---------------------------------------------------------------------------*/
/* This function handler receives publications to which we are subscribed */
static void
bluemix_pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  PRINTF("bluemix: no subscription supported right now\n");
}
/*---------------------------------------------------------------------------*/
static void
init_platform(void)
{
  /* Register the publish callback handler */
  MQTT_PUB_REGISTER_HANDLER(bluemix_pub_handler);

  /* Create client id */
  mqtt_res_client_id(conf.client_id, DEFAULT_IP_ADDR_STR_LEN);
#if DEFAULT_SENSORS_NUM
  /* Create topics */
  snprintf(data_topic, CONFIG_PUB_TOPIC_LEN, "iot-2/evt/%s/fmt/json",
           DEFAULT_PUB_STRING);
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(bluemix_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize platform-specific */
  init_platform();

  printf("\nIBM bluemix process started\n");
  printf("  Client ID:    %s\n", conf.client_id);
#if DEFAULT_SENSORS_NUM
  printf("  Data topic:   %s\n", data_topic);
#endif

  while(1) {

    PROCESS_YIELD();

    if(ev == mqtt_client_event_connected) {

      /* Start the MQTT resource process */
      process_start(&mqtt_res_process, NULL);

      /* No subscription implemented at the moment, continue */

      /* Start the mqtt-sensors process */
      process_start(&mqtt_sensors_process, NULL);

      /* Enable the sensor */
      process_start(&SENSORS_NAME(MQTT_SENSORS, _sensors_process), NULL);
    }

    if(ev == mqtt_client_event_disconnected) {
      /* We are not connected, disable the sensors */
      process_exit(&SENSORS_NAME(MQTT_SENSORS, _sensors_process));
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
