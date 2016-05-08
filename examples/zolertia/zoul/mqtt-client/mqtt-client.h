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
/**
 * \addtogroup relayr-mqtt
 * @{
 *
 * \file
 * Relayr MQTT application header
 *
 * \author
 *         Antonio Lignan <antonio.lignan@gmail.com>
 *                        <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef MQTT_CLIENT_H_
#define MQTT_CLIENT_H_
/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */
typedef struct mqtt_client_config {
  char auth_user[CONFIG_AUTH_USER_LEN];
  clock_time_t pub_interval;
  char broker_ip[CONFIG_IP_ADDR_STR_LEN];
  uint16_t pub_interval_check;
  char auth_token[CONFIG_AUTH_TOKEN_LEN];
  uint16_t broker_port;
} mqtt_client_config_t;

extern mqtt_client_config_t conf; 
/*---------------------------------------------------------------------------*/
typedef struct config_flash {
  uint16_t magic_word;
  char auth_user[CONFIG_AUTH_USER_LEN];
  char auth_token[CONFIG_AUTH_TOKEN_LEN];
  uint16_t temp_threshold;
  uint16_t humd_threshold;
  uint16_t pub_interval_check;
  uint16_t crc;
} config_flash_t;
/*---------------------------------------------------------------------------*/
/* MQTT client available functions */
void publish(uint8_t *app_buffer, uint16_t len);
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
#define SENSOR_NAME_STRING  20
typedef struct sensor_val {
  int16_t value;
  int16_t threshold;
  int16_t min;
  int16_t max;
  char sensor_name[SENSOR_NAME_STRING];
  char alarm_name[SENSOR_NAME_STRING];
  char sensor_config[SENSOR_NAME_STRING];
} sensor_val_t;

typedef struct sensors_values {
  uint8_t num;
  sensor_val_t sensor[DEFAULT_CONF_SENSORS_NUM];
} sensor_values_t;
/*---------------------------------------------------------------------------*/
#endif /* MQTT_CLIENT_H_ */
/** @} */

