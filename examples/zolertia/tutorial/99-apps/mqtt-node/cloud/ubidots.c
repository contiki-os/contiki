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
#include "ubidots.h"

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
/* Pub topic types */
enum {
  PUB_TOPIC_RAW,
  PUB_TOPIC_STRING
};
/*---------------------------------------------------------------------------*/
#if DEFAULT_SENSORS_NUM
static char *buf_ptr;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/* Topic placeholders */
static char data_topic[CONFIG_PUB_TOPIC_LEN];
#endif
static char cmd_topic[CONFIG_SUB_CMD_TOPIC_LEN];
/*---------------------------------------------------------------------------*/
PROCESS(ubidots_process, "Ubidots MQTT process");
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
              uint8_t type, uint8_t first, uint8_t more)
{
  int len = 0;
  int pos = 0;
  char *topic = "\"%s\":%s";

  if((buf_ptr == NULL) || (length <= 0)){
    PRINTF("Ubidots: null buffer or lenght less than zero\n");
    return -1;
  }

  if(first) {
    len = snprintf(buf_ptr, length, "%s", "{");
    pos = len;
    buf_ptr += len;
  }

  if(type == PUB_TOPIC_STRING) {
    topic = "\"%s\":\"%s\"";
  }

  len = snprintf(buf_ptr, (length - pos), topic, meaning, value);

  if(len < 0 || pos >= length) {
    PRINTF("Ubidots: Buffer too short. Have %d, need %d + \\0\n", length, len);
    return -1;
  }

  pos += len;
  buf_ptr += len;

  if(more) {
    len = snprintf(buf_ptr, (length - pos), "%s", ",");
  } else {
    len = snprintf(buf_ptr, (length - pos), "%s", "}");
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

    PRINTF("Ubidots: Alarm! %s --> %u\n", sensor->alarm_name, sensor->value);
    aux_int = sensor->value;
    aux_res = sensor->value;

    if(sensor->pres > 0) {
      aux_int /= sensor->pres;
      aux_res %= sensor->pres;
    } else {
      aux_res = 0;
    }

    snprintf(app_buffer, APP_BUFFER_SIZE, aux_res > 9 ? "{\"%s\":%d.%02u}" :
             "{\"%s\":%d.%01u0}", sensor->alarm_name, aux_int, aux_res);

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

  /* Ubidots does not support string as payload content, default values such as
   * parent address, etc are not sent
   */
  mqtt_res_uptime(aux, sizeof(aux));
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_UPTIME,
                      aux, PUB_TOPIC_RAW, 1, 1);
  remain =- len;

  mqtt_res_u16_addr(aux, sizeof(aux));

  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_ID, aux,
                      PUB_TOPIC_RAW, 0, 1);
  remain =- len;

  /* Include the sensor values, if `sensor_name` lenght is zero discard */
  for(i=0; i < msg->num; i++) {
    if(strlen(msg->sensor[i].sensor_name)) {
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
      len = add_pub_topic(remain, msg->sensor[i].sensor_name,
                          aux, PUB_TOPIC_RAW, 0, 1);
      remain =- len;
    }
  }

  /* The last value to be sent, the `more` argument should be zero */
  mqtt_res_parent_rssi(aux, sizeof(aux));
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_RSSI,
                      aux, PUB_TOPIC_RAW, 0, 0);

  PRINTF("Ubidots: publish %s (%u)\n", app_buffer, strlen(app_buffer));
  publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));
}
#endif /* DEFAULT_SENSORS_NUM */
/*---------------------------------------------------------------------------*/
/* This function handler receives publications to which we are subscribed */
static void
ubidots_pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  uint8_t i;
  uint16_t aux;

  PRINTF("Ubidots: Pub Handler, topic='%s' (len=%u), chunk='%s', chunk_len=%u\n",
         topic, topic_len, chunk, chunk_len);

  /* Most of the commands follow a boolean-logic at least */
  if(chunk_len <= 0) {
    PRINTF("Ubidots: Chunk should be at least a single digit integer or string\n");
    return;
  }

  /* This is a command event, it uses "1" and "0" strings for true or false
   * As default we use the "/lv" request upon subscribing, we expect an numeric
   * string as chunk
   */
  if(strncmp(topic, cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN) == 0) {

    /* Add an extra byte for the level separator */
    aux = strlen(DEFAULT_TOPIC_LONG) + UBIDOTS_LABEL_LEN + 1;

    /* Toggle a given LED */
    if(strncmp((const char *)&topic[aux], DEFAULT_SUBSCRIBE_CMD_LEDS,
               strlen(DEFAULT_SUBSCRIBE_CMD_LEDS)) == 0) {
      PRINTF("Ubidots: Command received --> toggle LED\n");

      if(strncmp((const char *)chunk, "1", 1) == 0) {
        leds_on(CMD_LED);
      } else if(strncmp((const char *)chunk, "0", 1) == 0) {
        leds_off(CMD_LED);
      } else {
        PRINTF("Ubidots: invalid command argument (expected boolean)!\n");
      }

      return;

    /* Restart the device */
    } else if(strncmp((const char *)&topic[aux], DEFAULT_SUBSCRIBE_CMD_REBOOT,
               strlen(DEFAULT_SUBSCRIBE_CMD_REBOOT)) == 0) {
      PRINTF("Ubidots: Command received --> reboot\n");

      /* This is fixed to check only "true" arguments */
      if(strncmp((const char *)chunk, "1", 1) == 0) {
        sys_ctrl_reset();
      } else {
        PRINTF("Ubidots: invalid command argument (expected only 'true')!\n");
      }

      return;

    /* Enable or disable external sensors */
    } else if(strncmp((const char *)&topic[aux], DEFAULT_SUBSCRIBE_CMD_SENSOR,
               strlen(DEFAULT_SUBSCRIBE_CMD_SENSOR)) == 0) {
      PRINTF("Ubidots: Command received --> enable/disable sensor\n");

      if(strncmp((const char *)chunk, "1", 1) == 0) {
        activate_sensors(0x01);
      } else if(strncmp((const char *)chunk, "0", 1) == 0) {
        activate_sensors(0x00);
      } else {
        PRINTF("Ubidots: invalid command argument (expected boolean)!\n");
      }

      return;

    /* This is a configuration event
     * As currently Contiki's MQTT driver does not support more than one SUBSCRIBE
     * we are handling both commands and configurations in the same "cmd" topic
     * We expect the configuration payload to follow the next syntax:
     * {"name":"update_period","value":61}
     */

    /* Change the update period */
    } else if(strncmp((const char *)&topic[aux], DEFAULT_SUBSCRIBE_CMD_EVENT,
               strlen(DEFAULT_SUBSCRIBE_CMD_EVENT)) == 0) {

      /* Take integers as configuration value */
      aux = atoi((const char*) chunk);

      if(mqtt_check_int_chunk_len(aux, chunk_len)) {
        PRINTF("Ubidots: chunk lenght doesn't match integer\n");
        return;
      }

      /* Check for allowed values */
      if((aux < DEFAULT_UPDATE_PERIOD_MIN) || (aux > DEFAULT_UPDATE_PERIOD_MAX)) {
        PRINTF("Ubidots: update interval should be between %u and %u\n", 
                DEFAULT_UPDATE_PERIOD_MIN, DEFAULT_UPDATE_PERIOD_MAX);
        return;
      }

      conf.pub_interval_check = aux;
      PRINTF("Ubidots: New update interval --> %u secs\n", conf.pub_interval_check);
      // mqtt_write_config_to_flash();
      return;
    }
#if DEFAULT_SENSORS_NUM
    /* Change a sensor's threshold, skip is `sensor_config` is empty */
    for(i=0; i<SENSORS_NAME(MQTT_SENSORS, _sensors.num); i++) {

      if((strlen(SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_config))) &&
        (strncmp((const char *)&topic[aux], SENSORS_NAME(MQTT_SENSORS, _sensors.sensor[i].sensor_config),
                      strlen(SENSORS_NAME(MQTT_SENSORS, _sensors.sensor[i].sensor_config))) == 0)) {

        /* Take integers as configuration value */
        aux = atoi((const char*) chunk);

        if(mqtt_check_int_chunk_len(aux, chunk_len)) {
          PRINTF("Ubidots: chunk lenght doesn't match integer\n");
          return;
        }

        if((aux < SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].min)) || 
          (aux > SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].max))) {
          PRINTF("Ubidots: %s threshold should be between %d and %d\n",
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_name),
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].min),
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].max));
          return;
        }

        /* We have now a valid threshold value, the logic is simple: each
         * variable has a `thresh` to configure a limit over the given value,
         * and `thresl` which respectively checks for values below this limit.
         * As a convention we are expecting `sensor_config` strings ending in
         * `_thresh` or `_thresl`.  The check below "should" be "safe" as we are
         * sure it matches an expected string.
         */

        if(strstr((const char *)topic, "_thresh") != NULL) {
          SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].over_threshold) = aux;
          PRINTF("Ubidots: New %s over threshold --> %u\n",
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_name),
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].over_threshold));
        } else if(strstr((const char *)topic, "_thresl") != NULL) {
          SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].below_threshold) = aux;
          PRINTF("Ubidots: New %s below threshold --> %u\n",
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_name),
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].below_threshold));
        } else {
          PRINTF("Ubidots: Expected threshold configuration name to end ");
          PRINTF("either in thresh or thresl\n");
        }

        return;
      }
    }
#endif /* DEFAULT_SENSORS_NUM */

    /* We are now checking for any string command expected by the subscribed
     * sensor module
     */
#if DEFAULT_COMMANDS_NUM
    for(i=0; i<SENSORS_NAME(MQTT_SENSORS, _commands.num); i++) {

      if(strncmp((const char *)&topic[aux], 
          SENSORS_NAME(MQTT_SENSORS, _commands.command[i].command_name),
          strlen(SENSORS_NAME(MQTT_SENSORS, _commands.command[i].command_name))) == 0) {

        /* Take integers as argument value */
        aux = atoi((const char*) chunk);

        /* Invoke the command handler */
        SENSORS_NAME(MQTT_SENSORS,_commands.command[i].cmd(aux));
        return;
      }
    }
#endif /* DEFAULT_COMMANDS_NUM */

    /* Invalid configuration topic, we should have returned before */
    PRINTF("Ubidots: Configuration/Command parameter not recognized\n");

  } else {
    PRINTF("Ubidots: Incorrect topic or chunk len. Ignored\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
init_platform(void)
{
  /* Register the publish callback handler */
  MQTT_PUB_REGISTER_HANDLER(ubidots_pub_handler);

  /* Create client id */
  mqtt_res_client_id(conf.client_id, DEFAULT_IP_ADDR_STR_LEN);
#if DEFAULT_SENSORS_NUM
  /* Create topics, use only the last 12 bytes of the client ID */
  snprintf(data_topic, CONFIG_PUB_TOPIC_LEN, "%s/%s", DEFAULT_TOPIC_LONG,
           &conf.client_id[strlen(conf.client_id) - UBIDOTS_LABEL_LEN]);
#endif
  snprintf(cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN, "%s/%s%s", DEFAULT_TOPIC_LONG,
           &conf.client_id[strlen(conf.client_id) - UBIDOTS_LABEL_LEN],
           DEFAULT_CMD_STRING);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ubidots_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize platform-specific */
  init_platform();

  printf("\nUbidots process started\n");
  printf("  Client ID:    %s\n", conf.client_id);
#if DEFAULT_SENSORS_NUM
  printf("  Data topic:   %s\n", data_topic);
#endif
  printf("  Cmd topic:    %s\n\n", cmd_topic);

  while(1) {

    PROCESS_YIELD();

    if(ev == mqtt_client_event_connected) {

      /* Start the MQTT resource process */
      process_start(&mqtt_res_process, NULL);

      /* Subscribe to topics (MQTT driver only supports 1 topic at the moment */
      subscribe(cmd_topic);

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

