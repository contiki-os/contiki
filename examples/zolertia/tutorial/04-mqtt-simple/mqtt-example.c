/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \file
 *         An example showing how to use the MQTT API
 * \author
 *         Antonio Lignan <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "rpl/rpl-private.h"
#include "mqtt.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include <string.h>
/*---------------------------------------------------------------------------*/
#define PUB_INTERVAL         (30 * CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
#define INIT                 0
#define CONNECTED            1
#define READY                2
#define DISCONNECTED         3
/*---------------------------------------------------------------------------*/
#define DEFAULT_ORG_ID       "zolertia"
#define DEFAULT_BROKER_IP    "fd00::1"
#define DEFAULT_BROKER_PORT  1883
/*---------------------------------------------------------------------------*/
#define DEFAULT_CONF_USER    NULL
#define DEFAULT_CONF_TOKEN   NULL
/*---------------------------------------------------------------------------*/
#define DEFAULT_PUB_TOPIC    "zolertia/test"
#define DEFAULT_SUB_TOPIC    "zolertia/leds"
/*---------------------------------------------------------------------------*/
static uint8_t state = INIT;
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_demo_process);
AUTOSTART_PROCESSES(&mqtt_demo_process);
/*---------------------------------------------------------------------------*/
static struct mqtt_connection conn;
/*---------------------------------------------------------------------------*/
#define APP_BUFFER_SIZE 64
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_demo_process, "MQTT Simple Demo");
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED:
    printf("MQTT connection OK\n");
    state = CONNECTED;
    process_poll(&mqtt_demo_process);
    break;

  case MQTT_EVENT_DISCONNECTED:
    state = DISCONNECTED;
    process_poll(&mqtt_demo_process);
    /* Print the disconnection reason */
    printf("MQTT Disconnected=%u\n", *((mqtt_event_t *)data));
    break;

  case MQTT_EVENT_PUBLISH:
    msg_ptr = data;
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      printf("Received PUB: topic '%s' (%i bytes)\n", msg_ptr->topic,
                                                      strlen(msg_ptr->topic));
    }
    printf("Payload %s (%u bytes)\n", msg_ptr->payload_chunk,
                                      msg_ptr->payload_length);
    leds_toggle(LEDS_RED);
    break;

  case MQTT_EVENT_SUBACK:
    state = READY;
    process_poll(&mqtt_demo_process);
    break;

  case MQTT_EVENT_UNSUBACK:
  case MQTT_EVENT_PUBACK:
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_demo_process, ev, data)
{
  PROCESS_BEGIN();
  printf("\nMQTT simple demo\n");
  static uint16_t counter;

  /* Create the connection */
  mqtt_register(&conn, &mqtt_demo_process, DEFAULT_ORG_ID, mqtt_event,
                MAX_TCP_SEGMENT_SIZE);

  /* User and token if required */
  mqtt_set_username_password(&conn, DEFAULT_CONF_USER, DEFAULT_CONF_TOKEN);

  /* Wait until we got a valid global address */
  while(1) {
    etimer_set(&et, CLOCK_SECOND / 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if((uip_ds6_get_global(ADDR_PREFERRED) != NULL)) {
      printf("Connected to the wireless network\n");
      break;
    }
    etimer_reset(&et);
  }

  /* Schedule a timer to kick the next PROCESS_YIELD */
  etimer_set(&et, CLOCK_SECOND / 2);

  /* Basic state machine handling the connection */
  while(1) {
    PROCESS_YIELD();

    switch(state) {
      case INIT:
      /* Attempt to connect to the broker */
      printf("MQTT connecting...\n");
      mqtt_connect(&conn, DEFAULT_BROKER_IP, DEFAULT_BROKER_PORT,
                   PUB_INTERVAL * 3);
      break;

      case CONNECTED:
      printf("MQTT connected, subscribing...\n");
      mqtt_subscribe(&conn, NULL, DEFAULT_SUB_TOPIC, MQTT_QOS_LEVEL_0);
      break;

      case READY:
      printf("MQTT subscribed to %s\n", DEFAULT_SUB_TOPIC);
      break;

      case DISCONNECTED:
      state = INIT;
      printf("MQTT disconnected\n");
      mqtt_disconnect(&conn);
      break;
    }

    /* Break the loop and move to the next one */
    if(state == READY) break;
  }

  /* Set the periodic timer */
  counter = 0;
  etimer_set(&et, PUB_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    snprintf(app_buffer, APP_BUFFER_SIZE, "{\"counter\":%u}", counter);

    if(mqtt_publish(&conn, NULL, DEFAULT_PUB_TOPIC, (uint8_t *)app_buffer,
                    strlen(app_buffer), MQTT_QOS_LEVEL_0,
                    MQTT_RETAIN_OFF) == MQTT_STATUS_OK) {
      printf("MQTT Publish %u to %s\n", counter, DEFAULT_PUB_TOPIC);
      counter++;

    } else {
      printf("MQTT failed to publish\n");
    }
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
