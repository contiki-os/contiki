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
#ifndef UBIDOTS_H_
#define UBIDOTS_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-client.h"
/*---------------------------------------------------------------------------*/
/* Default configuration values */
/*
 * This are the values from Ubidots, note the Ubidots TOKEN is used as the user
 * name, and the password should be empty
 *
 * {
 *   "user": TOKEN
 *   "password": blank
 *   "clientId": TOKEN,
 *   "topic": "/v1.6/devices/"
 * }
 */
#define DEFAULT_CONF_AUTH_TOKEN       ""  /* Not used */
#define DEFAULT_CONF_AUTH_USER        ""
#define DEFAULT_TOPIC_STR             "/v1.6/devices"
#define DEFAULT_TOPIC_LONG            DEFAULT_TOPIC_STR

#define DEFAULT_CONF_AUTH_IS_REQUIRED       1
#define DEFAULT_CONF_AUTH_USER_IS_REQUIRED  1
/*---------------------------------------------------------------------------*/
/* Host "things.ubidots.com" with IP 50.23.124.68 and IPv6 address
 * 2607:f0d0:2101:39::2
 */
#define MQTT_DEMO_CONF_BROKER_IP_ADDR "::ffff:3217:7c44"

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

/* Define the maximum lenght of the topics and tokens6
 * The user ID string is normally 30 bytes long, the "/v1.6/" adds 6 bytes more
 */
#define CONFIG_TOPIC_LEN              42
#define CONFIG_PUB_TOPIC_LEN          42
#define CONFIG_SUB_CMD_TOPIC_LEN      42
#define DEFAULT_CONF_IP_ADDR_STR_LEN  64
#define DEFAULT_CONF_AUTH_USER_LEN    37
#define DEFAULT_CONF_AUTH_TOKEN_LEN   13
/*---------------------------------------------------------------------------*/
#endif /* UBIDOTS_H_ */
/** @} */

