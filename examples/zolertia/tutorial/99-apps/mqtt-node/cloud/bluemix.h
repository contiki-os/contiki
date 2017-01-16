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
#ifndef BLUEMIX_H_
#define BLUEMIX_H_
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
 * Bluemix follows the next scheme:
 *
 * iot-2/evt/{topic}/fmt/json to publish data to quickstart
 *
 * And the content in the following format:
 * {"d":{"var_name":var_value}}
 */
#define DEFAULT_CONF_AUTH_TOKEN       ""
#define DEFAULT_CONF_AUTH_USER        ""
#define DEFAULT_PUB_STRING            "data"

/* Using "quickstart" as DEFAULT_ORG_ID will enable to use IBM Bluemix
 * quickstart service, not requiring to register, but only to use the six bytes
 * of the device in the `client_id` to visualize the data.  Upon registering the
 * device to IBM Bluemix, change this accordingly
 */
#define DEFAULT_ORG_CONF_ID           "quickstart"

/* If not using the quickstart service, change below to 1 to enable */
#define DEFAULT_CONF_AUTH_IS_REQUIRED       0
#define DEFAULT_CONF_AUTH_USER_IS_REQUIRED  0
/*---------------------------------------------------------------------------*/
/* Host "quickstart.messaging.internetofthings.ibmcloud.com" with
 * IP 184.172.124.189
 */
#define MQTT_DEMO_CONF_BROKER_IP_ADDR  "::ffff:b8ac:7cbd"

#define CMD_LED                       LEDS_RED
/*---------------------------------------------------------------------------*/
/* Specific PUB event topics */
#define DEFAULT_PUBLISH_EVENT_ID      "ID"
#define DEFAULT_PUBLISH_EVENT_RSSI    "rssi"
#define DEFAULT_PUBLISH_EVENT_UPTIME  "uptime"
#define DEFAULT_PUBLISH_EVENT_PARENT  "parent"

/* Define the maximum lenght of the topics and tokens
 * The user ID string is normally 43 bytes long, the "/v2/things" adds 10 bytes more
 */
#define CONFIG_PUB_TOPIC_LEN          33
#define DEFAULT_CONF_IP_ADDR_STR_LEN  64
#define DEFAULT_CONF_AUTH_USER_LEN    1
#define DEFAULT_CONF_AUTH_TOKEN_LEN   1
/*---------------------------------------------------------------------------*/
#endif /* BLUEMIX_H_ */
/** @} */

