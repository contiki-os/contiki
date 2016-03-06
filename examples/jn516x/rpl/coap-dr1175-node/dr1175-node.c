/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Theo van Daele <theo.van.daele@nxp.com>
 *
 */

#include "contiki.h"
#include "net/ip/uip.h"
#include "tools/rpl-tools.h"
#include "rest-engine.h"
#include "light-sensor.h"
#include "ht-sensor.h"
#include "dev/leds.h"
#include "dev/leds-extension.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <stdlib.h>

static void event_sensors_dr1175_handler(void);
static void get_sensors_dr1175_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_light_sensor_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_light_sensor_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_temperature_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_temperature_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_humidity_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_humidity_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_white_led_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_rgb_led_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d3_1174_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d6_1174_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) { content_len += snprintf(content + content_len, sizeof(content) - content_len, __VA_ARGS__); } }

#define CLIP(value, level) if(value > level) { \
    value = level; \
}
#define SET_LED(LED)            if(atoi((const char *)request_content) != 0) {   \
                                  leds_on(LED);                                  \
                                } else {                                         \
                                  leds_off(LED);                                 \
                                }
/*---------------------------------------------------------------------------*/
PROCESS(start_app, "START_APP");
AUTOSTART_PROCESSES(&start_app, &sensors_process);

/*---------------------------------------------------------------------------*/

/*********** CoAP sensor/ resource *************************************************/

/*******************************************************************/
/* Observable resource and event handler to obtain all sensor data */
/*******************************************************************/
EVENT_RESOURCE(resource_sensors_dr1175,               /* name */
               "obs;title=\"All_DR1175_sensors\"",    /* attributes */
               get_sensors_dr1175_handler,            /* GET handler */
               NULL,                                  /* POST handler */
               NULL,                                  /* PUT handler */
               NULL,                                  /* DELETE handler */
               event_sensors_dr1175_handler);         /* event handler */
static void
get_sensors_dr1175_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.APPLICATION_JSON) {
    content_len = 0;
    CONTENT_PRINTF("{\"DR1175\":[");
    CONTENT_PRINTF("{\"Humidity\":\"%d\"},", ht_sensor.value(HT_SENSOR_HUM));
    CONTENT_PRINTF("{\"Light\":\"%d\"},", light_sensor.value(0));
    CONTENT_PRINTF("{\"Temp\":\"%d\"}", ht_sensor.value(HT_SENSOR_TEMP));
    CONTENT_PRINTF("]}");
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
static void
event_sensors_dr1175_handler()
{
  /* Registered observers are notified and will trigger the GET handler to create the response. */
  REST.notify_subscribers(&resource_sensors_dr1175);
}
/*****************************************************/
/* Resource and handler to obtain light sensor value */
/*****************************************************/
RESOURCE(resource_light_sensor_value,
         "title=\"light sensor value\"",
         get_light_sensor_value_handler,
         NULL,
         NULL,
         NULL);
static void
get_light_sensor_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%d", light_sensor.value(0));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/***************************************************/
/* Resource and handler to obtain light unit value */
/***************************************************/
RESOURCE(resource_light_sensor_unit,
         "title=\"light sensor unit\"",
         get_light_sensor_unit_handler,
         NULL,
         NULL,
         NULL);
static void
get_light_sensor_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("Lux");
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/***********************************************************/
/* Resource and handler to obtain temperature sensor value */
/***********************************************************/
RESOURCE(resource_temperature_value,
         "title=\"temperature value\"",
         get_temperature_value_handler,
         NULL,
         NULL,
         NULL);
static void
get_temperature_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%d", ht_sensor.value(HT_SENSOR_TEMP));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/*********************************************************/
/* Resource and handler to obtain temperature unit value */
/*********************************************************/
RESOURCE(resource_temperature_unit,
         "title=\"temperature unit\"",
         get_temperature_unit_handler,
         NULL,
         NULL,
         NULL);
static void
get_temperature_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("degrees C");
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/********************************************************/
/* Resource and handler to obtain humidity sensor value */
/********************************************************/
RESOURCE(resource_humidity_value,
         "title=\"humidity value\"",
         get_humidity_value_handler,
         NULL,
         NULL,
         NULL);
static void
get_humidity_value_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%d", ht_sensor.value(HT_SENSOR_HUM));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/******************************************************/
/* Resource and handler to obtain humidity unit value */
/******************************************************/
RESOURCE(resource_humidity_unit,
         "title=\"humidity unit\"",
         get_humidity_unit_handler,
         NULL,
         NULL,
         NULL);
static void
get_humidity_unit_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("relative %%");
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/***************************************************/
/* Resource and handler to control White LED level */
/***************************************************/
RESOURCE(resource_white_led,
         "title=\"WhiteLED <[0..255]>\"",
         NULL,
         put_post_white_led_handler,
         put_post_white_led_handler,
         NULL);
static void
put_post_white_led_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  int level;

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    level = atoi((const char *)request_content);
    CLIP(level, 255)
    leds_set_level(level, LEDS_WHITE);
  }
}
/*************************************************/
/* Resource and handler to control RGB LED level */
/*************************************************/
RESOURCE(resource_rgb_led,
         "title=\"RGB LED <[0..255] [0..255] [0..255]>\"",
         NULL,
         put_post_rgb_led_handler,
         put_post_rgb_led_handler,
         NULL);
static void
put_post_rgb_led_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  char *pch;
  int RGB[] = { 0, 0, 0 };
  int index = 0;

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    pch = strtok((char *)request_content, " ");
    while((pch != NULL) && (index != sizeof(RGB) / sizeof(int))) {
      /* Convert token to int */
      RGB[index] = atoi(pch);
      CLIP(RGB[index], 255)
      index++;
      /* Get next token */
      pch = strtok(NULL, " ");
    }
    leds_set_level(RGB[0], LEDS_RED);
    leds_set_level(RGB[1], LEDS_GREEN);
    leds_set_level(RGB[2], LEDS_BLUE);
  }
}
/************************************************/
/* Resource and handler to control D3 on DR1174 */
/************************************************/
RESOURCE(resource_led_d3_1174,
         "title=\"LED D3 1174<[0,1]>\"",
         NULL,
         put_post_led_d3_1174_handler,
         put_post_led_d3_1174_handler,
         NULL);
static void
put_post_led_d3_1174_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    SET_LED(LEDS_GP0);
  }
}
/************************************************/
/* Resource and handler to control D6 on DR1174 */
/************************************************/
RESOURCE(resource_led_d6_1174,
         "title=\"LED D6 1174<[0,1]>\"",
         NULL,
         put_post_led_d6_1174_handler,
         put_post_led_d6_1174_handler,
         NULL);
static void
put_post_led_d6_1174_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    SET_LED(LEDS_GP1);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_app, ev, data)
{
  PROCESS_BEGIN();

  static int is_coordinator = 0;
  static struct etimer et;

  /* Make sensor active for measuring */
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(ht_sensor);

  /* Start net stack */
  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_tools_init(&prefix);
  } else {
    rpl_tools_init(NULL);
  } printf("Starting RPL node\n");

  rest_init_engine();
  rest_activate_resource(&resource_light_sensor_value, "DR1175/LightSensor/Value");
  rest_activate_resource(&resource_light_sensor_unit, "DR1175/LightSensor/Unit");
  rest_activate_resource(&resource_temperature_unit, "DR1175/Temperature/Unit");
  rest_activate_resource(&resource_temperature_value, "DR1175/Temperature/Value");
  rest_activate_resource(&resource_humidity_unit, "DR1175/Humidity/Unit");
  rest_activate_resource(&resource_humidity_value, "DR1175/Humidity/Value");
  rest_activate_resource(&resource_white_led, "DR1175/WhiteLED");
  rest_activate_resource(&resource_rgb_led, "DR1175/ColorLED/RGBValue");
  rest_activate_resource(&resource_led_d3_1174, "DR1175/LED/D3On1174");
  rest_activate_resource(&resource_led_d6_1174, "DR1175/LED/D6On1174");
  rest_activate_resource(&resource_sensors_dr1175, "DR1175/AllSensors");

  /* Level of LEDS=0, so no light after start-up */
  leds_on(LEDS_WHITE | LEDS_RED | LEDS_GREEN | LEDS_BLUE);

  /* contiki-jn516x-main switches all leds off after process start-up.
     Switch on required leds after rescheduling, otherwise level change needs to be
     accompanied with leds_on() at least once in resource handler. */
  etimer_set(&et, CLOCK_SECOND / 10);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  /* Level of LEDS=0, so no light after start-up. However, they are enabled
     A level change will directly be visible. */
  leds_on(LEDS_WHITE | LEDS_RED | LEDS_GREEN | LEDS_BLUE);

  /* If sensor process generates an event, call event_handler of resource.
     This will make this resource observable by the client */
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) &&
                             ((data == &light_sensor) || (data == &ht_sensor)));
    event_sensors_dr1175_handler();
  }

  PROCESS_END();
}
