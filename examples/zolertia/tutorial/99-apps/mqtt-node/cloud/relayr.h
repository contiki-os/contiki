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
#ifndef RELAYR_H_
#define RELAYR_H_
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
 * This are the values from relayr.io, note we are not using the clientId,
 * but rather using the device's address
 *
 * {
 *   "user": "abcdefgh-ijkl-mnop-qrst-uvwxyz123456",
 *   "password": "1234-abcdefg",
 *   "clientId": "TyUu7wDjlT7q2a+2KxrWBEg",
 *   "topic": "/v1/abcdefgh-ijkl-mnop-qrst-uvwxyz123456/"
 * }
 */
#define DEFAULT_CONF_AUTH_TOKEN       ""
#define DEFAULT_CONF_AUTH_USER        ""
#define DEFAULT_TOPIC_STR             "/v1/"
#define DEFAULT_TOPIC_LONG            DEFAULT_TOPIC_STR DEFAULT_CONF_AUTH_USER

#define DEFAULT_ORG_CONF_ID           "zolertia"

#define DEFAULT_CONF_AUTH_IS_REQUIRED      1
#define DEFAULT_CONF_AUTH_USER_IS_REQUIRED 0
/*---------------------------------------------------------------------------*/
/* Host "mqtt.relayr.io" with IP 52.48.96.194 */
#define MQTT_DEMO_CONF_BROKER_IP_ADDR "::ffff:3430:60C2"

/* This is pretty harcoded by the v1 API version */
#define DEFAULT_PUB_STRING            "/data"
#define DEFAULT_CMD_STRING            "/cmd"
#define DEFAULT_CFG_STRING            "/config" /* Currently not used as Contiki
                                                 * allows only 1 subscription
                                                 * maximum */
#define CMD_LED                       LEDS_RED
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
 * The user ID string is normally 36 bytes long, the "/v1/" adds 4 bytes more
 */
#define CONFIG_TOPIC_LEN              41
#define CONFIG_PUB_TOPIC_LEN          46
#define CONFIG_SUB_CMD_TOPIC_LEN      45
#define DEFAULT_CONF_IP_ADDR_STR_LEN  64
#define DEFAULT_CONF_AUTH_USER_LEN    37
#define DEFAULT_CONF_AUTH_TOKEN_LEN   13
/*---------------------------------------------------------------------------*/
#endif /* RELAYR_H_ */
/** @} */

