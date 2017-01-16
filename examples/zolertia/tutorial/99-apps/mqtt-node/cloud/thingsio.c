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
#include "thingsio.h"

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
/* Lenght of the `{"key":"` substring in the publication handler */
#define MQTT_THINGSIO_KEYVAR_LEN      9
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
PROCESS(thingsio_process, "The Things.io MQTT process");
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
    PRINTF("Things.io: null buffer or lenght less than zero\n");
    return -1;
  }

  if(first) {
    len = snprintf(buf_ptr, length, "%s", "{\"values\":[");
    pos = len;
    buf_ptr += len;
  }

  len = snprintf(buf_ptr, (length - pos),
                 "{\"key\":\"%s\",\"value\":\"%s\"}",
                 meaning, value);
 
  if(len < 0 || pos >= length) {
    PRINTF("Things.io: Buffer too short. Have %d, need %d + \\0\n", length, len);
    return -1;
  }

  pos += len;
  buf_ptr += len;

  if(more) {
    len = snprintf(buf_ptr, (length - pos), "%s", ",");
  } else {
    len = snprintf(buf_ptr, (length - pos), "%s", "]}");
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

    PRINTF("Things.io: Alarm! %s --> %u\n", sensor->alarm_name, sensor->value);
    aux_int = sensor->value;
    aux_res = sensor->value;

    if(sensor->pres > 0) {
      aux_int /= sensor->pres;
      aux_res %= sensor->pres;
    } else {
      aux_res = 0;
    }

    snprintf(app_buffer, APP_BUFFER_SIZE, aux_res > 9 ?
             "{\"values\":[{\"key\":\"%s\",\"value\":%d.%02u}]}" :
             "{\"values\":[{\"key\":\"%s\",\"value\":%d.%01u0}]}",
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
  char aux[DEFAULT_CONF_IP_ADDR_STR_LEN];
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

  /* Include the sensor values, if `sensor_name` is empty */
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

  PRINTF("Things.io: publish %s (%u)\n", app_buffer, strlen(app_buffer));
  publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));
}
#endif /* DEFAULT_SENSORS_NUM */
/*---------------------------------------------------------------------------*/
/* This function handler receives publications to which we are subscribed */
static void
thingsio_pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  uint8_t i;
  uint8_t *j;
  uint16_t aux;

  PRINTF("Things.io: Pub Handler, topic='%s' (len=%u), chunk='%s', chunk_len=%u\n",
         topic, topic_len, chunk, chunk_len);

  /* Most of the commands follow a boolean-logic at least */
  if(chunk_len <= 0) {
    PRINTF("Relay: Chunk should be at least a single digit integer or string\n");
    return;
  }

  /* This is a command event, it uses "1" and "0" as boolean
   * We expect commands to have the following syntax:
   * [{"key":"enable_sensor","value":"1"}]
   */
  if(strncmp(topic, cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN) == 0) {

    /* Search for the string "value" and retrieve index */
    j = (uint8_t *)strstr((const char *)chunk, (const char *)"value");

    if(j == NULL) {
      return;
    }

    /* We use 8 as magic number to save processing, this is the lenght of the
     * `value":"` string, as shown before this is expected in the topic
     * publication
     */
    aux = atoi((const char*) &chunk[(j - chunk) + 8]);

    /* Yet another magic number to save processing, as above we take into
     * account the `value":` string plus the ending `"}]` character
     */
    if(mqtt_check_int_chunk_len(aux, chunk_len - (j - chunk) - 11)) {
      PRINTF("Things.io: chunk lenght doesn't match integer %u\n", aux);
      return;
    }

    /* Toggle a given LED */
    if(strncmp((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
               DEFAULT_SUBSCRIBE_CMD_LEDS,
               strlen(DEFAULT_SUBSCRIBE_CMD_LEDS)) == 0) {
      PRINTF("Things.io: Command received --> toggle LED\n");

      if(!aux) {
        leds_off(CMD_LED);
      } else if(aux == 1) {
        leds_on(CMD_LED);
      } else {
        PRINTF("Things.io: invalid command argument (expected boolean)!\n");
      }

      return;

    /* Restart the device */
    } else if(strncmp((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
               DEFAULT_SUBSCRIBE_CMD_REBOOT,
               strlen(DEFAULT_SUBSCRIBE_CMD_REBOOT)) == 0) {
      PRINTF("Things.io: Command received --> reboot\n");

      /* This is fixed to check only "true" arguments */
      if(aux) {
        sys_ctrl_reset();
      } else {
        PRINTF("Things.io: invalid command argument (expected only 'true')!\n");
      }

      return;

    /* Enable or disable external sensors */
    } else if(strncmp((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
               DEFAULT_SUBSCRIBE_CMD_SENSOR,
               strlen(DEFAULT_SUBSCRIBE_CMD_SENSOR)) == 0) {
      PRINTF("Things.io: Command received --> enable/disable sensor\n");

      if(!aux) {
        activate_sensors(0x00);
      } else if(aux == 1) {
        activate_sensors(0x01);
      } else {
        PRINTF("Things.io: invalid command argument (expected boolean)!\n");
      }

      return;

    /* This is a configuration event
     * As currently Contiki's MQTT driver does not support more than one SUBSCRIBE
     * we are handling both commands and configurations in the same "cmd" topic
     * We expect the configuration payload to follow the next syntax:
     * [{"key":"update_period","value":"61"}]
     */

    /* Change the update period */
   } else if(strncmp((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
             DEFAULT_SUBSCRIBE_CMD_EVENT,
             strlen(DEFAULT_SUBSCRIBE_CMD_EVENT)) == 0) {

      /* Check for allowed values */
      if((aux < DEFAULT_UPDATE_PERIOD_MIN) || (aux > DEFAULT_UPDATE_PERIOD_MAX)) {
        PRINTF("Things.io: update interval should be between %u and %u\n", 
                DEFAULT_UPDATE_PERIOD_MIN, DEFAULT_UPDATE_PERIOD_MAX);
        return;
      }

      conf.pub_interval_check = aux;
      PRINTF("Things.io: New update interval --> %u secs\n", conf.pub_interval_check);

      // FIXME: write_config_to_flash();
      return;
    }

#if DEFAULT_SENSORS_NUM
    /* Change a sensor's threshold, skip is `sensor_config` is empty */
    for(i=0; i<SENSORS_NAME(MQTT_SENSORS, _sensors.num); i++) {

      if((strlen(SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_config))) &&
        (strncmp((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
                  SENSORS_NAME(MQTT_SENSORS, _sensors.sensor[i].sensor_config),
                  strlen(SENSORS_NAME(MQTT_SENSORS, _sensors.sensor[i].sensor_config))) == 0)) {

        /* Take integers as configuration value */
        aux = atoi((const char*) &chunk[strlen(SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_config)) + 18]);

        if((aux < SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].min)) || 
          (aux > SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].max))) {
          PRINTF("Things.io: %s threshold should be between %d and %d\n",
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

        if(strstr((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
                  "_thresh") != NULL) {
          SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].over_threshold) = aux;
          PRINTF("Things.io: New %s over threshold --> %u\n",
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_name),
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].over_threshold));
        } else if(strstr((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
                  "_thresl") != NULL) {
          SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].below_threshold) = aux;
          PRINTF("Things.io: New %s below threshold --> %u\n",
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].sensor_name),
                 SENSORS_NAME(MQTT_SENSORS,_sensors.sensor[i].below_threshold));
        } else {
          PRINTF("Things.io: Expected threshold configuration name to end ");
          PRINTF("either in thresh or thresl\n");
          /* Exit earlier to avoid writting in flash */
          return;
        }

        // FIXME: write_config_to_flash();
        return;
      }
    }
#endif /* DEFAULT_SENSORS_NUM */

    /* We are now checking for any string command expected by the subscribed
     * sensor module
     */
#if DEFAULT_COMMANDS_NUM
    for(i=0; i<SENSORS_NAME(MQTT_SENSORS, _commands.num); i++) {

      if((strncmp((const char *)&chunk[MQTT_THINGSIO_KEYVAR_LEN],
          SENSORS_NAME(MQTT_SENSORS, _commands.command[i].command_name),
          strlen(SENSORS_NAME(MQTT_SENSORS, _commands.command[i].command_name))) == 0)) {

        /* Take integers as argument value */
        aux = atoi((const char*) &chunk[strlen(SENSORS_NAME(MQTT_SENSORS,_commands.command[i].command_name)) + 18]);

        /* Invoke the command handler */
        SENSORS_NAME(MQTT_SENSORS,_commands.command[i].cmd(aux));
        return;
      }
    }
#endif /* DEFAULT_COMMANDS_NUM */

    /* Invalid configuration topic, we should have returned before */
    PRINTF("Things.io: Configuration/Command parameter not recognized\n");

  } else {
    PRINTF("Things.io: Incorrect topic or chunk len. Ignored\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
init_platform(void)
{
  /* Register the publish callback handler */
  MQTT_PUB_REGISTER_HANDLER(thingsio_pub_handler);

  /* Create client id */
  mqtt_res_client_id(conf.client_id, DEFAULT_IP_ADDR_STR_LEN);

#if DEFAULT_SENSORS_NUM
  /* Create topics */
  if(strlen(DEFAULT_CONF_AUTH_USER)) {
    snprintf(data_topic, CONFIG_PUB_TOPIC_LEN, "%s%s", DEFAULT_TOPIC_LONG,
             DEFAULT_PUB_STRING);
  } else {
    /* If we are here it means the mqtt_client has already check credentials */
    snprintf(data_topic, CONFIG_PUB_TOPIC_LEN, "%s%s%s", DEFAULT_TOPIC_STR,
             conf.auth_user, DEFAULT_PUB_STRING);
  }
#endif
#if MQTT_THINGSIO_USE_PUB_TOPIC_AS_CMD
  if(strlen(DEFAULT_CONF_AUTH_USER)) {
    snprintf(cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN, "%s%s", DEFAULT_TOPIC_LONG,
             DEFAULT_CMD_STRING);
  } else {
    snprintf(cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN, "%s%s%s", DEFAULT_TOPIC_STR,
             conf.auth_user, DEFAULT_CMD_STRING);
  }
#else
  if(strlen(DEFAULT_CONF_AUTH_TOKEN)) {
    snprintf(cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN, "%s%s", DEFAULT_TOPIC_STR,
             DEFAULT_CONF_AUTH_TOKEN);
  } else {
    /* We assume it has been written before the Auth User information */
    snprintf(cmd_topic, CONFIG_SUB_CMD_TOPIC_LEN, "%s%s", DEFAULT_TOPIC_STR,
             conf.auth_token);
  }
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(thingsio_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize platform-specific */
  init_platform();

  printf("\nThe Things.io process started\n");
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
