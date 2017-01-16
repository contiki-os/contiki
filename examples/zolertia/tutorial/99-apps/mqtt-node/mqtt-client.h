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
 */
/*---------------------------------------------------------------------------*/
#ifndef MQTT_CLIENT_H_
#define MQTT_CLIENT_H_
/*---------------------------------------------------------------------------*/
#define __client_include(x) #x
#define client_include(x) __client_include(x.h)
/*---------------------------------------------------------------------------*/
#include client_include(cloud/MQTT_PLATFORM)
#include client_include(device/MQTT_SENSORS)
/*---------------------------------------------------------------------------*/
#define PLATFORM_NAME_EXPAND(x, y) x##y
#define PLATFORM_NAME(x, y) PLATFORM_NAME_EXPAND(x, y)
/*---------------------------------------------------------------------------*/
#define SENSORS_NAME_EXPAND(x, y) x##y
#define SENSORS_NAME(x, y) SENSORS_NAME_EXPAND(x, y)
/*---------------------------------------------------------------------------*/
/* Connection information, this is taken from the platform header file */
#define MQTT_DEMO_BROKER_IP_ADDR      MQTT_DEMO_CONF_BROKER_IP_ADDR
#define DEFAULT_BROKER_PORT           1883
#define DEFAULT_AUTH_TOKEN            DEFAULT_CONF_AUTH_TOKEN
#define DEFAULT_AUTH_USER             DEFAULT_CONF_AUTH_USER
#define DEFAULT_AUTH_USER_LEN         DEFAULT_CONF_AUTH_USER_LEN
#define DEFAULT_AUTH_TOKEN_LEN        DEFAULT_CONF_AUTH_TOKEN_LEN
#define DEFAULT_IP_ADDR_STR_LEN       DEFAULT_CONF_IP_ADDR_STR_LEN

/* Do we need to use AUTH_USER and AUTH_token? */
#define DEFAULT_AUTH_IS_REQUIRED      DEFAULT_CONF_AUTH_IS_REQUIRED

/* Required only for thethingsio and relayr */
#define DEFAULT_AUTH_USER_ONLY_REQUIRED DEFAULT_CONF_AUTH_USER_IS_REQUIRED

/* Number of seconds we keep the alarm flag high so we don't send too many
 * alarms in a short period
 */
#ifndef DEFAULT_CONF_ALARM_TIME
#define DEFAULT_ALARM_TIME            15
#else
#define DEFAULT_ALARM_TIME            DEFAULT_CONF_ALARM_TIME
#endif

/* Maximum number of connection attempts after a disconnection */
#define RECONNECT_ATTEMPTS_CONF       10

/* Maximum waiting period after attempting to connect to the Broken */
#define CONNECT_MAX_WAITING_PERIOD    50

/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration, saved
 * in flash memory
 */
typedef struct mqtt_client_config {
  uint16_t magic_word;
  char auth_user[DEFAULT_AUTH_USER_LEN];
  char auth_token[DEFAULT_AUTH_TOKEN_LEN];
  clock_time_t pub_interval;
  uint16_t pub_interval_check;
  char client_id[DEFAULT_IP_ADDR_STR_LEN];
  uint16_t crc;
} mqtt_client_config_t;

extern mqtt_client_config_t conf; 
/*---------------------------------------------------------------------------*/
/* MQTT client available functions */
void subscribe(char * topic);
void publish(uint8_t *app_buffer, char *pub_topic, uint16_t len);
int mqtt_write_config_to_flash(char *name, uint8_t *buf, uint16_t len);
int mqtt_read_config_from_flash(char *name, uint8_t *buf, uint16_t len);
/*---------------------------------------------------------------------------*/
/* MQTT client process events */
extern process_event_t mqtt_client_event_connected;
extern process_event_t mqtt_client_event_disconnected;
/*---------------------------------------------------------------------------*/
/* Macro to register the callback handler when a PUBLISH event is received */
#define MQTT_PUB_REGISTER_HANDLER(ptr) pub_handler = ptr;
extern void (*pub_handler)(const char *topic, uint16_t topic_len,
                           const uint8_t *chunk, uint16_t chunk_len);
/*---------------------------------------------------------------------------*/
#endif /* MQTT_CLIENT_H_ */
/** @} */

