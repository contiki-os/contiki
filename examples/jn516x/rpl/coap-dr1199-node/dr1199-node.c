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
#include "dev/leds.h"
#include "button-sensor.h"
#include "pot-sensor.h"
#include <stdio.h>
#include <stdlib.h>

static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;

static void event_sensors_dr1199_handler();
static void get_sensors_dr1199_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_switch_sw1_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_switch_sw2_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_switch_sw3_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_switch_sw4_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_switch_dio8_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_pot_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d1_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d2_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d3_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d3_1174_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_d6_1174_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_post_led_all_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) { content_len += snprintf(content + content_len, sizeof(content) - content_len, __VA_ARGS__); } }

#define PARSE_SWITCH(POSITION)  if(button_sensor.value(0) & (1 << POSITION)) { \
                                  CONTENT_PRINTF("PRESSED"); \
                                } else { \
                                  CONTENT_PRINTF("RELEASED"); \
                                }

#define SET_LED(LED)            if(atoi((const char *)request_content) != 0) {   \
                                  leds_on(LED);                                  \
                                } else {                                         \
                                  leds_off(LED);                                 \
                                }

/*---------------------------------------------------------------------------*/
PROCESS(start_app, "START_APP");
AUTOSTART_PROCESSES(&sensors_process, &start_app);

/*---------------------------------------------------------------------------*/

/*********** CoAP sensor/ resource *************************************************/

/*******************************************************************/
/* Observable resource and event handler to obtain all sensor data */
/*******************************************************************/
EVENT_RESOURCE(resource_sensors_dr1199,               /* name */
               "obs;title=\"All_DR1199_sensors\"",    /* attributes */
               get_sensors_dr1199_handler,            /* GET handler */
               NULL,                                  /* POST handler */
               NULL,                                  /* PUT handler */
               NULL,                                  /* DELETE handler */
               event_sensors_dr1199_handler);         /* event handler */
static void
get_sensors_dr1199_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.APPLICATION_JSON) {
    content_len = 0;
    CONTENT_PRINTF("{\"DR1199\":[");
    CONTENT_PRINTF("{\"Switch\":\"0x%X\"},", button_sensor.value(0));
    CONTENT_PRINTF("{\"Pot\":\"%d\"}", pot_sensor.value(0));
    CONTENT_PRINTF("]}");
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
static void
event_sensors_dr1199_handler()
{
  /* Registered observers are notified and will trigger the GET handler to create the response. */
  REST.notify_subscribers(&resource_sensors_dr1199);
}
/***********************************************/
/* Resource and handler to obtain switch value */
/***********************************************/
RESOURCE(resource_switch_sw1,
         "title=\"SW1\"",
         get_switch_sw1_handler,
         NULL,
         NULL,
         NULL);
static void
get_switch_sw1_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  content_len = 0;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    PARSE_SWITCH(1)
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
RESOURCE(resource_switch_sw2,
         "title=\"SW2\"",
         get_switch_sw2_handler,
         NULL,
         NULL,
         NULL);
static void
get_switch_sw2_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  content_len = 0;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    PARSE_SWITCH(2)
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
RESOURCE(resource_switch_sw3,
         "title=\"SW3\"",
         get_switch_sw3_handler,
         NULL,
         NULL,
         NULL);
static void
get_switch_sw3_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  content_len = 0;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    PARSE_SWITCH(3)
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
RESOURCE(resource_switch_sw4,
         "title=\"SW4\"",
         get_switch_sw4_handler,
         NULL,
         NULL,
         NULL);
static void
get_switch_sw4_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  content_len = 0;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    PARSE_SWITCH(4)
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
RESOURCE(resource_switch_dio8,
         "title=\"DIO8\"",
         get_switch_dio8_handler,
         NULL,
         NULL,
         NULL);
static void
get_switch_dio8_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  content_len = 0;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    PARSE_SWITCH(0)
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/*******************************************************/
/*  Resource and handler to obtain potentiometer value */
/*******************************************************/
RESOURCE(resource_pot,
         "title=\"Potentiometer\"",
         get_pot_handler,
         NULL,
         NULL,
         NULL);
static void
get_pot_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  content_len = 0;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    CONTENT_PRINTF("%d", pot_sensor.value(0));
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/************************************/
/* Resource and handler to set leds */
/************************************/
RESOURCE(resource_led_d1,
         "title=\"LED D1 <[0,1]>\"",
         NULL,
         put_post_led_d1_handler,
         put_post_led_d1_handler,
         NULL);
static void
put_post_led_d1_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    SET_LED(LEDS_GREEN)
  }
}
RESOURCE(resource_led_d2,
         "title=\"LED D2 <[0,1]>\"",
         NULL,
         put_post_led_d2_handler,
         put_post_led_d2_handler,
         NULL);
static void
put_post_led_d2_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    SET_LED(LEDS_BLUE)
  }
}
RESOURCE(resource_led_d3,
         "title=\"LED D3 <[0,1]>\"",
         NULL,
         put_post_led_d3_handler,
         put_post_led_d3_handler,
         NULL);
static void
put_post_led_d3_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    SET_LED(LEDS_RED)
  }
}
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
RESOURCE(resource_led_all,
         "title=\"LED All <[0,1]>\"",
         NULL,
         put_post_led_all_handler,
         put_post_led_all_handler,
         NULL);
static void
put_post_led_all_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    if(atoi((const char *)request_content) != 0) {
      leds_on(LEDS_ALL);
    } else {
      leds_off(LEDS_ALL);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_app, ev, data)
{
  PROCESS_BEGIN();

  static int is_coordinator = 0;

  /* is_coordinator = node_id == 1; */

  /* Make sensor active for measuring */
  SENSORS_ACTIVATE(button_sensor);
  SENSORS_ACTIVATE(pot_sensor);

  /* Start net stack */
  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_tools_init(&prefix);
  } else {
    rpl_tools_init(NULL);
  } printf("Starting RPL node\n");

  rest_init_engine();
  rest_activate_resource(&resource_switch_sw1, "DR1199/Switch/SW1");
  rest_activate_resource(&resource_switch_sw2, "DR1199/Switch/SW2");
  rest_activate_resource(&resource_switch_sw3, "DR1199/Switch/SW3");
  rest_activate_resource(&resource_switch_sw4, "DR1199/Switch/SW4");
  rest_activate_resource(&resource_switch_dio8, "DR1199/Switch/DIO8");
  rest_activate_resource(&resource_pot, "DR1199/Potentiometer");
  rest_activate_resource(&resource_led_d1, "DR1199/LED/D1");
  rest_activate_resource(&resource_led_d2, "DR1199/LED/D2");
  rest_activate_resource(&resource_led_d3, "DR1199/LED/D3");
  rest_activate_resource(&resource_led_d3_1174, "DR1199/LED/D3On1174");
  rest_activate_resource(&resource_led_d6_1174, "DR1199/LED/D6On1174");
  rest_activate_resource(&resource_led_all, "DR1199/LED/All");
  rest_activate_resource(&resource_sensors_dr1199, "DR1199/AllSensors");
  /* If sensor process generates an event, call event_handler of resource.
     This will make this resource observable by the client */
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) &&
                             ((data == &button_sensor) || (data == &pot_sensor)));
    event_sensors_dr1199_handler();
  }

  PROCESS_END();
}

