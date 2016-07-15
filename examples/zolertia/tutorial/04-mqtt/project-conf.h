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
#define DEFAULT_ORG_ID            "mqtt-demo"

/*
 * If you have an IPv6 network or a NAT64-capable border-router:
 * test.mosquitto.org
 * IPv6 "2001:41d0:a:3a10::1"
 * NAT64 address "::ffff:5577:53c2" (85.119.83.194)
 *
 * To test locally you can use a mosquitto local broker in your host and use
 * i.e the fd00::1/64 address the Border router defaults to
 */
#define MQTT_DEMO_BROKER_IP_ADDR  "fd00::1"
/*---------------------------------------------------------------------------*/
/* Default configuration values */
#define DEFAULT_EVENT_TYPE_ID        "status"
#define DEFAULT_SUBSCRIBE_CMD_TYPE   "leds"
#define DEFAULT_BROKER_PORT          1883
#define DEFAULT_PUBLISH_INTERVAL     (45 * CLOCK_SECOND)
#define DEFAULT_KEEP_ALIVE_TIMER     60

#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID        0xABCD

/* The following are Zoul (RE-Mote, etc) specific */
#undef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL       26

/* Specific platform values */
#if CONTIKI_TARGET_ZOUL
#define BUFFER_SIZE                  64
#define APP_BUFFER_SIZE              512

#else /* Default is Z1 */
#define BUFFER_SIZE                  48
#define APP_BUFFER_SIZE              256
#define BOARD_STRING                 "Zolertia Z1 Node"
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 3
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES          3
#endif
/*---------------------------------------------------------------------------*/
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC          nullrdc_driver

/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE       32

#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/** @} */

