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
#define MQTT_DEMO_STATUS_LED      LEDS_GREEN
#define MQTT_DEMO_PUBLISH_TRIGGER &button_right_sensor

#define DEFAULT_ORG_ID            "mqtt-demo"
#define MQTT_DEMO_BROKER_IP_ADDR  "mqtt.relayr.io"
/*---------------------------------------------------------------------------*/
/* Default configuration values */
#define DEFAULT_EVENT_TYPE_ID       "2826729f-d677-4425-88d6-78a62ae7061f" /* DEVICE ID same as MQTT USER */
#define DEFAULT_SUBSCRIBE_CMD_TYPE  "leds"
#define DEFAULT_AUTH_USER           "TKCZyn9Z3RCWI1nimKucGHw" /* CLIENT ID */
#define DEFAULT_AUTH_TOKEN          "Ei316V-YqznM" /* PASSWORD */
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (45 * CLOCK_SECOND)
#define DEFAULT_KEEP_ALIVE_TIMER    60
/*---------------------------------------------------------------------------*/
/* Use either the cc1200_driver for sub-1GHz, or cc2538_rf_driver (default)
 * for 2.4GHz built-in radio interface
 */
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO              cc2538_rf_driver

/* Alternate between ANTENNA_SW_SELECT_SUBGHZ or ANTENNA_SW_SELECT_2_4GHZ */
#define ANTENNA_SW_SELECT_DEF_CONF       ANTENNA_SW_SELECT_2_4GHZ

#define NETSTACK_CONF_RDC                nullrdc_driver

#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/** @} */
