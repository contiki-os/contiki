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
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
/* Prototypes required to be implemented by platforms */
void publish_alarm(sensor_val_t *sensor);
void publish_event(sensor_values_t *msg);
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

#define DEFAULT_ORG_CONF_ID           "zolertia"

#define DEFAULT_TOPIC_STR             "/v1.6/devices"
#define DEFAULT_TOPIC_LONG            DEFAULT_TOPIC_STR

#define DEFAULT_CONF_AUTH_IS_REQUIRED       1
#define DEFAULT_CONF_AUTH_USER_IS_REQUIRED  1
/*---------------------------------------------------------------------------*/
/* Host "things.ubidots.com" with IP 50.23.124.68 and IPv6 address
 * 2607:f0d0:2101:39::2
 */
#define MQTT_DEMO_CONF_BROKER_IP_ADDR "::ffff:3217:7c44"
#define CMD_LED                       LEDS_RED

/* We are using the device Client ID to publish to a topic as:
 * /v1.6/devices/{CLIENT_ID}
 * That it why the DEFAULT_PUB_STRING is not used and removed
 *
 * Subscription topic is:
 * /v1.6/devices/{CLIENT_ID}/#
 * Thus why the DEFAULT_CMD_STRING was removed...
 *
 * The expected Ubidots subscription topic is:
 * /v1.6/devices/{LABEL_DATA_SOURCE}/{LABEL_VARIABLE}
 * As currently Ubidots doesn't support multilevel "#" wildcards, it is not
 * possible to subscribe to a topic, as currently done in other platforms, so we
 * need to choose a variable.  As default we are mapping the leds_toggle, change
 * to other value as required.  The topic would be then:
 * /v1.6/devices/{CLIENT_ID}/leds_toggle/lv
 *
 * We use "/lv" to retrieve the value only, if you need the full JSON response
 * it would be something like:
 * {"timestamp": 1475154149778, "context": {}, "value": 1.0,
 *  "id": "57ed10e576254270bd7fc455"}
 * You will need to change the ubidots_pub_handler() to parse accordingly 
 * The following default commands are available only if using the
 * DEFAULT_CMD_STRING as shown above
 */
#define DEFAULT_SUBSCRIBE_CMD_LEDS    "/leds_toggle/lv"
#define DEFAULT_SUBSCRIBE_CMD_REBOOT  "/reboot/lv"
#define DEFAULT_SUBSCRIBE_CMD_SENSOR  "/enable_sensor/lv"
#define DEFAULT_SUBSCRIBE_CMD_EVENT   "/update_period/lv"

#define DEFAULT_CMD_STRING            DEFAULT_SUBSCRIBE_CMD_LEDS

/* Specific PUB event topics */
#define DEFAULT_PUBLISH_EVENT_ID      "ID"
#define DEFAULT_PUBLISH_EVENT_RSSI    "rssi"
#define DEFAULT_PUBLISH_EVENT_UPTIME  "uptime"
#define DEFAULT_PUBLISH_EVENT_PARENT  "parent"

/* This is hardcoded to avoid using the full client ID as Source Data label,
 * and save some bytes: we use the last 12 bytes of the Client ID, that is, we
 * use the MAC-based string
 */
#define UBIDOTS_LABEL_LEN             12

/* Define the maximum lenght of the topics and tokens
 * The user ID string is normally 30 bytes long, the "/v1.6/" adds 6 bytes more
 */
#define CONFIG_PUB_TOPIC_LEN          50
#define CONFIG_SUB_CMD_TOPIC_LEN      50
#define DEFAULT_CONF_IP_ADDR_STR_LEN  64
#define DEFAULT_CONF_AUTH_USER_LEN    37
#define DEFAULT_CONF_AUTH_TOKEN_LEN   13
/*---------------------------------------------------------------------------*/
#endif /* UBIDOTS_H_ */
/** @} */

