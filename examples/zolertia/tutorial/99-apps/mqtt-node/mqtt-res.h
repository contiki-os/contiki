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
#ifndef MQTT_RES_H_
#define MQTT_RES_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-client.h"
/*---------------------------------------------------------------------------*/
#ifdef MQTT_RES_CONF_PING_INTERVAL
#define MQTT_RES_PING_INTERVAL   MQTT_RES_CONF_PING_INTERVAL
#else
#define MQTT_RES_PING_INTERVAL   0
#endif
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t sensors_restart_event;
extern process_event_t sensors_stop_event;
/*---------------------------------------------------------------------------*/
/* Function to retrieve the Client ID (uses the IBM Bluemix format */
void mqtt_res_client_id(char *buf, uint8_t len);

/* Function to enable/disable subscribed sensors */
void activate_sensors(uint8_t state);

/* Return the latest RSSI parent value (from the latest ping) */
void mqtt_res_parent_rssi(char *buf, uint8_t len);

/* Return the uptime in number of seconds */
void mqtt_res_uptime(char *buf, uint8_t len);

/* Return the parent's 64-bit address */
void mqtt_res_parent_addr(char *buf, uint8_t len);

/* In case of a numeric chunk, checks the atoi() conversion */
int mqtt_check_int_chunk_len(uint16_t chunk, uint8_t chunk_len);

/* Returns the device's 16-bit address */
void mqtt_res_u16_addr(char *buf, uint8_t len);
/*---------------------------------------------------------------------------*/
#endif /* MQTT_RES_H_ */
/** @} */

