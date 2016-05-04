/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc2538-mqtt-demo
 * @{
 *
 * \file
 * Project specific configuration defines for the MQTT demo
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
/* User configuration */
#define STATUS_LED                    LEDS_GREEN
#define CMD_LED                       LEDS_RED

/* Host "mqtt.relayr.io" with IP 54.171.127.130 */
#define MQTT_DEMO_BROKER_IP_ADDR      "::ffff:36ab:7f82"
#define DEFAULT_BROKER_PORT           1883

/* This is pretty harcoded by the v1 API version */
#define DEFAULT_PUB_STRING            "/data"
#define DEFAULT_CMD_STRING            "/cmd"
#define DEFAULT_CFG_STRING            "/config"
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
#define DEFAULT_USER_ID               ""
#define DEFAULT_AUTH_TOKEN            ""
#define DEFAULT_AUTH_USER             DEFAULT_USER_ID

#define DEFAULT_TOPIC_STR             "/v1/"
#define DEFAULT_TOPIC_LONG            DEFAULT_TOPIC_STR DEFAULT_USER_ID

#define RELAYR_CREATE_TOPIC(PATH, TOPIC) 

/* This is the base-time unit, if using a DEFAULT_SAMPLING_INTERVAL of 1 second
 * (given by the CLOCK_SECOND macro) the node will periodically publish every
 * DEFAULT_PUBLISH_INTERVAL seconds.
 */
#define DEFAULT_PUBLISH_INTERVAL      45
#define DEFAULT_SAMPLING_INTERVAL     CLOCK_SECOND

/* Specific SUB command topics */
#define DEFAULT_SUBSCRIBE_CMD         DEFAULT_TOPIC_LONG DEFAULT_CMD_STRING


#define DEFAULT_SUBSCRIBE_CMD_LEDS    "leds_toggle"
#define DEFAULT_SUBSCRIBE_CMD_REBOOT  "reboot"
#define DEFAULT_SUBSCRIBE_CMD_SENSOR  "enable_sensor"

/* Specific SUB config topics */
#define DEFAULT_SUBSCRIBE_CFG         DEFAULT_TOPIC_LONG DEFAULT_CFG_STRING
#define DEFAULT_SUBSCRIBE_CFG_EVENT   "update_period"
#define DEFAULT_SUBSCRIBE_CFG_TEMPTHR "temperature_thresh"
#define DEFAULT_SUBSCRIBE_CFG_HUMDTHR "humidity_thresh"

/* Specific PUB event topics */
#define DEFAULT_PUBLISH_EVENT         DEFAULT_TOPIC_LONG DEFAULT_PUB_STRING
#define DEFAULT_PUBLISH_EVENT_TEMP    "temperature"
#define DEFAULT_PUBLISH_EVENT_HUMD    "humidity"
#define DEFAULT_PUBLISH_ALARM_TEMP    "alarm_temperature"
#define DEFAULT_PUBLISH_ALARM_HUMD    "alarm_humidity"
#define DEFAULT_PUBLISH_EVENT_ID      "address"
#define DEFAULT_PUBLISH_EVENT_RSSI    "rssi"
#define DEFAULT_PUBLISH_EVENT_UPTIME  "uptime"
#define DEFAULT_PUBLISH_EVENT_PARENT  "parent"

/* Define the maximum lenght of the topics and tokens
 * The user ID string is normally 36 bytes long, the "/v1/" adds 4 bytes more
 */
#define CONFIG_TOPIC_LEN              40
#define CONFIG_PUB_TOPIC_LEN          45
#define CONFIG_SUB_CMD_TOPIC_LEN      44
#define CONFIG_SUB_CFG_TOPIC_LEN      47
#define CONFIG_IP_ADDR_STR_LEN        64
#define CONFIG_AUTH_USER_LEN          40
#define CONFIG_AUTH_TOKEN_LEN         16

/* Default sensor state and thresholds */
#define DEFAULT_SENSOR_STATE          1
#define DEFAULT_TEMP_THRESH           3000
#define DEFAULT_HUMD_THRESH           8000

/* Minimum and maximum update rate values */
#define DEFAULT_UPDATE_PERIOD_MIN     5
#define DEFAULT_UPDATE_PERIOD_MAX     600

/* Minimum and maximum values for the SHT25 sensor */
#define DEFAULT_SHT25_TEMP_MIN        (-2000)
#define DEFAULT_SHT25_TEMP_MAX        12000
#define DEFAULT_SHT25_HUMD_MIN        0
#define DEFAULT_SHT25_HUMD_MAX        10000

#define DEFAULT_TEMP_NOT_USED         DEFAULT_SHT25_TEMP_MIN
#define DEFAULT_HUMD_NOT_USED         DEFAULT_SHT25_HUMD_MIN
/*---------------------------------------------------------------------------*/
/* Select the minimum low power mode the node should drop to */
#define LPM_CONF_MAX_PM               1

/* Use either the cc1200_driver for sub-1GHz, or cc2538_rf_driver (default)
 * for 2.4GHz built-in radio interface
 */
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO           cc2538_rf_driver

/* Alternate between ANTENNA_SW_SELECT_SUBGHZ or ANTENNA_SW_SELECT_2_4GHZ */
#define ANTENNA_SW_SELECT_DEF_CONF    ANTENNA_SW_SELECT_2_4GHZ

#define NETSTACK_CONF_RDC             nullrdc_driver

#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/** @} */
