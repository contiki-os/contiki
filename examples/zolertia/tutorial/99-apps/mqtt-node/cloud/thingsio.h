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
#ifndef THINGSIO_H_
#define THINGSIO_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-client.h"
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
/* Prototypes required to be implemented by platforms */
void publish_alarm(sensor_val_t *sensor);
void publish_event(sensor_values_t *msg);
/*---------------------------------------------------------------------------*/
/* Default configuration values */
/*
 * The Things.io follows the next scheme:
 *
 * /v2/things/token_id  for publish and subscriptions
 *
 * And the content in the following format:
 * {"values":[{"key":"sht25","value":0xABCD}]}
 */
#define DEFAULT_CONF_AUTH_TOKEN       ""
#define DEFAULT_CONF_AUTH_USER        ""
#define DEFAULT_TOPIC_STR             "v2/things/"
#define DEFAULT_TOPIC_LONG            DEFAULT_TOPIC_STR DEFAULT_CONF_AUTH_USER

#define DEFAULT_ORG_CONF_ID           "zolertia"

/* For this application we use two Thing-Token, one to publish data and the
 * other to create the control variables to send commands and change the
 * configuration of the application.  This can be omitted if you prefer to
 * use a sub-level in the data topic, and push commands on your own using a
 * script or a different application, if this is the case just enable the
 * following MQTT_THINGSIO_USE_PUB_TOPIC_AS_CMD, then the command topic will be:
 *
 * v2/things/{Thing-Token}/cmd
 *
 * Note this won't let you create widgets in TheThings.io as variables inside
 * are not shown in the widget creator
 */
#ifndef MQTT_THINGSIO_USE_PUB_TOPIC_AS_CMD
#define MQTT_THINGSIO_USE_PUB_TOPIC_AS_CMD  0
#endif

#define DEFAULT_CONF_AUTH_IS_REQUIRED       0
#define DEFAULT_CONF_AUTH_USER_IS_REQUIRED  1
/*---------------------------------------------------------------------------*/
/* Host "mqtt.thethings.io" with IP 23.102.55.248 */
#define MQTT_DEMO_CONF_BROKER_IP_ADDR  "::ffff:1766:37f8"

/* This is pretty harcoded by the v2 API version
 * The v2 API only defines how to create the topic, we suggest using the
 * following substrings.  Note: to automatically visualize the reading values
 * on the dashboard, do not publish to another topic than v2/things/{token},
 * however if you want to publish to i.e v2/things/{topic}/data the readings
 * will be received by the broker as expected
 */
#define DEFAULT_PUB_STRING            ""
#define CMD_LED                       LEDS_RED

#if MQTT_THINGSIO_USE_PUB_TOPIC_AS_CMD
#define DEFAULT_CMD_STRING            "/cmd"
#else
/* The Auth-Token will be used as Thing-Token for the command topic */
#define DEFAULT_CMD_STRING            ""
#endif
/*---------------------------------------------------------------------------*/
/* Specific SUB command topics */
#define DEFAULT_SUBSCRIBE_CMD_LEDS    "leds_toggle"
#define DEFAULT_SUBSCRIBE_CMD_REBOOT  "reboot"
#define DEFAULT_SUBSCRIBE_CMD_SENSOR  "enable_sensor"
#define DEFAULT_SUBSCRIBE_CMD_EVENT   "update_period"

/* Specific PUB event topics */
#define DEFAULT_PUBLISH_EVENT_ID      "ID"
#define DEFAULT_PUBLISH_EVENT_RSSI    "rssi"
#define DEFAULT_PUBLISH_EVENT_UPTIME  "uptime"
#define DEFAULT_PUBLISH_EVENT_PARENT  "parent"

/* Define the maximum lenght of the topics and tokens
 * The user ID string is normally 43 bytes long, the "/v2/things" adds 10 bytes more
 */
#define CONFIG_TOPIC_LEN              54
#define CONFIG_PUB_TOPIC_LEN          54
#define CONFIG_SUB_CMD_TOPIC_LEN      58
#define DEFAULT_CONF_IP_ADDR_STR_LEN  64
#define DEFAULT_CONF_AUTH_USER_LEN    44
#if MQTT_THINGSIO_USE_PUB_TOPIC_AS_CMD
#define DEFAULT_CONF_AUTH_TOKEN_LEN   1
#else
#define DEFAULT_CONF_AUTH_TOKEN_LEN   44
#endif
/*---------------------------------------------------------------------------*/
#endif /* THINGSIO_H_ */
/** @} */

