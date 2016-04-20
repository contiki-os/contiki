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
#include "contiki-conf.h"
#include "rpl/rpl-private.h"
#include "mqtt.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/leds.h"
#include <string.h>
/*---------------------------------------------------------------------------*/
/*
 * Publish to an MQTT broker
 */
static const char *broker_ip = MQTT_DEMO_BROKER_IP_ADDR;
/*---------------------------------------------------------------------------*/
/* State machine ticking and led signalling */
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
#define CONNECTING_LED_DURATION    (CLOCK_SECOND >> 2)
#define PUBLISH_LED_ON_DURATION    (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER               0xFF
#define RECONNECT_INTERVAL          (CLOCK_SECOND * 2)
#define RECONNECT_ATTEMPTS          RETRY_FOREVER
#define CONNECTION_STABLE_TIME      (CLOCK_SECOND * 5)
#define NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 2)
#define NO_NET_LED_DURATION         (NET_CONNECT_PERIODIC >> 1)
static struct timer connection_life;
static uint8_t connect_attempt;
/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;
#define STATE_INIT            0
#define STATE_REGISTERED      1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_PUBLISHING      4
#define STATE_DISCONNECTED    5
#define STATE_NEWCONFIG       6
#define STATE_CONFIG_ERROR 0xFE
#define STATE_ERROR        0xFF
/*---------------------------------------------------------------------------*/
#define CONFIG_ALARM_TYPE_ID_LEN   32
#define CONFIG_BUTTON_TYPE_ID_LEN  32
#define CONFIG_IP_ADDR_STR_LEN     64
/*---------------------------------------------------------------------------*/
#define SCROLL_PERIOD    (CLOCK_SECOND / 6)
/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_demo_process);
AUTOSTART_PROCESSES(&mqtt_demo_process);
/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */
typedef struct mqtt_client_config {
  char alarm_type[CONFIG_ALARM_TYPE_ID_LEN];
  char button_type[CONFIG_BUTTON_TYPE_ID_LEN];
  char broker_ip[CONFIG_IP_ADDR_STR_LEN];
  uint16_t broker_port;
  clock_time_t pub_interval;
} mqtt_client_config_t;
/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
/*---------------------------------------------------------------------------*/
#define BUFFER_SIZE 64
static char client_id[BUFFER_SIZE];
static char alarm_topic[BUFFER_SIZE];
static char button_topic[BUFFER_SIZE];
static struct mqtt_connection conn;
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
static struct etimer publish_periodic_timer;
static struct ctimer ct;
static struct ctimer ct_lcd_off;
static struct etimer et;
/*---------------------------------------------------------------------------*/
static mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_demo_process, "MQTT Demo");
/*---------------------------------------------------------------------------*/
static void
publish_led_off(void *d)
{
  leds_off(STATUS_LED);
}
/*---------------------------------------------------------------------------*/
static void
clear_lcd(void *d)
{
  leds_off(LEDS_ALL);
}
/*---------------------------------------------------------------------------*/
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  printf("Pub Handler: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len,
      chunk_len);

  if(strncmp(topic, "zolertia/alarm/button", 21) == 0) {

    leds_on(LEDS_BLUE);
    printf("RFID alarm received\n");
 
  } else if(strncmp(topic, "zolertia/alarm/temp", 19) == 0) {

    leds_on(LEDS_RED);
    printf("Fridge: door open!\n");

  } else {
    return;
  }

  ctimer_set(&ct_lcd_off, (CLOCK_SECOND * 10), clear_lcd, NULL);
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    printf("APP - Application has a MQTT connection\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    printf("APP - MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&mqtt_demo_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    /* Implement first_flag in publish message? */
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      printf("APP - Application received a publish on topic '%s'. Payload "
          "size is %i bytes. Content:\n\n",
          msg_ptr->topic, msg_ptr->payload_length);
    }

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk,
                msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
    printf("APP - Application is subscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    printf("APP - Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    printf("APP - Publishing complete.\n");
    break;
  }
  default:
    printf("APP - Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
construct_sub_topic(void)
{
  snprintf(alarm_topic, BUFFER_SIZE, "zolertia/alarm/%s", conf.alarm_type);
  snprintf(button_topic, BUFFER_SIZE, "zolertia/alarm/%s", conf.button_type);

  printf("Subscription topic %s\n", alarm_topic);
  printf("Subscription topic %s\n", button_topic);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_client_id(void)
{
  int len = snprintf(client_id, BUFFER_SIZE, "d:%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Client ID: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
update_config(void)
{
  if(construct_client_id() == 0) {
    /* Fatal error. Client ID larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_sub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  state = STATE_INIT;
  etimer_set(&publish_periodic_timer, 0);
  return;
}
/*---------------------------------------------------------------------------*/
static int
init_config()
{
  /* Populate configuration with default values */
  memset(&conf, 0, sizeof(mqtt_client_config_t));
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));
  memcpy(conf.alarm_type, DEFAULT_ALARM_TYPE_ID, strlen(DEFAULT_ALARM_TYPE_ID));
  memcpy(conf.button_type, DEFAULT_BUTTON_TYPE_ID, strlen(DEFAULT_BUTTON_TYPE_ID));
  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_PUBLISH_INTERVAL;
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
subscribe(void)
{
  mqtt_status_t status;
  status = mqtt_subscribe(&conn, NULL, alarm_topic, MQTT_QOS_LEVEL_0);

  printf("APP - Subscribing!\n");
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    printf("APP - Tried to subscribe but command queue was full!\n");
  }

  status = mqtt_subscribe(&conn, NULL, button_topic, MQTT_QOS_LEVEL_0);

  printf("APP - Subscribing!\n");
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    printf("APP - Tried to subscribe but command queue was full!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  /* Connect to MQTT server */
  mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
               conf.pub_interval * 3);

  state = STATE_CONNECTING;
}
/*---------------------------------------------------------------------------*/
static void
state_machine(void)
{
  switch(state) {
  case STATE_INIT:
    mqtt_register(&conn, &mqtt_demo_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);
    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    printf("Init\n");

  case STATE_REGISTERED:
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) {
      /* Registered and with a public IP. Connect */
      printf("Registered. Connect attempt %u\n", connect_attempt);
      connect_to_broker();

    } else {
      leds_on(STATUS_LED);
      ctimer_set(&ct, NO_NET_LED_DURATION, publish_led_off, NULL);
    }
    etimer_set(&publish_periodic_timer, NET_CONNECT_PERIODIC);
    return;

  case STATE_CONNECTING:
    leds_on(STATUS_LED);
    ctimer_set(&ct, CONNECTING_LED_DURATION, publish_led_off, NULL);
    /* Not connected yet. Wait */
    printf("Connecting (%u)\n", connect_attempt);
    break;

  case STATE_CONNECTED:
    subscribe();
    return;

  case STATE_DISCONNECTED:
    printf("Disconnected\n");
    if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
      mqtt_disconnect(&conn);
      connect_attempt++;

      interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
        RECONNECT_INTERVAL << 3;

      printf("Disconnected. Attempt %u in %lu ticks\n", connect_attempt, interval);

      etimer_set(&publish_periodic_timer, interval);

      state = STATE_REGISTERED;
      return;

    } else {
      /* Max reconnect attempts reached. Enter error state */
      state = STATE_ERROR;
      printf("Aborting connection after %u attempts\n", connect_attempt - 1);
    }
    break;

  case STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
    printf("Bad configuration.\n");
    return;

  case STATE_ERROR:
  default:
    leds_on(STATUS_LED);
    /*
     * This should never happen.
     * If we enter here it's because of some error. Stop timers. The only thing
     * that can bring us out is a new config event
     */
    printf("Default case: State=0x%02x\n", state);
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_demo_process, ev, data)
{
  static uint8_t i = 0;

  PROCESS_BEGIN();

  printf("MQTT Demo Process\n");

  /* wait 15 seconds */
  etimer_set(&et, (CLOCK_SECOND * 15));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  if(init_config() != 1) {
    PROCESS_EXIT();
  }

  update_config();

  while(1) {

    PROCESS_YIELD();

    if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
       ev == PROCESS_EVENT_POLL || (ev == sensors_event)) {
      state_machine();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
