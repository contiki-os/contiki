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
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "ip64-addr.h"
#include "lib/crc16.h"
#include "mqtt-client.h"
#include "httpd-simple.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/*
 * Publish to an MQTT broker
 */
static const char *broker_ip = MQTT_DEMO_BROKER_IP_ADDR;
/*---------------------------------------------------------------------------*/
/*
 * A timeout used when waiting for something to happen (e.g. to connect or to
 * disconnect)
 */
#define STATE_MACHINE_PERIODIC        (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
/* Provide visible feedback via LEDS during various states */
/* When connecting to broker */
#define CONNECTING_LED_DURATION       (CLOCK_SECOND >> 2)

/* Each time we try to publish */
#define PUBLISH_LED_ON_DURATION       (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER                 0xFF
#define RECONNECT_INTERVAL            (CLOCK_SECOND * 2)

/*
 * Number of times to try reconnecting to the broker.
 * Can be a limited number (e.g. 3, 10 etc) or can be set to RETRY_FOREVER
 */
#define RECONNECT_ATTEMPTS            RETRY_FOREVER
#define CONNECTION_STABLE_TIME        (CLOCK_SECOND * 5)
/*
 * Sensor and device data publication interval
 */
#define DEFAULT_PUBLISH_TIME          (DEFAULT_PUBLISH_INTERVAL * CLOCK_SECOND)
#define DEFAULT_KEEP_ALIVE_TIMER      ((DEFAULT_PUBLISH_INTERVAL * 3)/2)
/*---------------------------------------------------------------------------*/
/* A timeout used when waiting to connect to a network */
#define NET_CONNECT_PERIODIC          (CLOCK_SECOND >> 2)
#define NO_NET_LED_DURATION           (NET_CONNECT_PERIODIC >> 1)
/*---------------------------------------------------------------------------*/
static struct timer connection_life;
static uint8_t connect_attempt;
/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;

#define STATE_INIT                    0
#define STATE_REGISTERED              1
#define STATE_CONNECTING              2
#define STATE_CONNECTED               3
#define STATE_PUBLISHING              4
#define STATE_DISCONNECTED            5
#define STATE_NEWCONFIG               6
#define STATE_CONFIG_ERROR         0xFE
#define STATE_ERROR                0xFF
/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_demo_process);
AUTOSTART_PROCESSES(&mqtt_demo_process);
/*---------------------------------------------------------------------------*/
/* Include there the platforms processes to include */
PROCESS_NAME(relayr_process);
/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
/*---------------------------------------------------------------------------*/
/*
 * Buffers for ID and tokens
 * Make sure they are large enough to hold the entire respective string
 */
static char client_id[CONFIG_IP_ADDR_STR_LEN];
static char *pub_topic = DEFAULT_PUBLISH_EVENT;
static char *cfg_topic = DEFAULT_SUBSCRIBE_CFG;
static char *cmd_topic = DEFAULT_SUBSCRIBE_CMD;
/*---------------------------------------------------------------------------*/
static struct mqtt_connection conn;
/*---------------------------------------------------------------------------*/
void (*pub_handler)(const char *topic, uint16_t topic_len,
                           const uint8_t *chunk, uint16_t chunk_len);
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
static struct etimer publish_periodic_timer;
static struct ctimer ct;
/*---------------------------------------------------------------------------*/
/* Declare process events */
process_event_t mqtt_client_event_connected;
process_event_t mqtt_client_event_disconnected;
/*---------------------------------------------------------------------------*/
/* Holds the MQTT configuration */
mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
static int
interval_post_handler(char *key, int key_len, char *val, int val_len)
{
  return HTTPD_SIMPLE_POST_HANDLER_OK;
}
/*---------------------------------------------------------------------------*/
HTTPD_SIMPLE_POST_HANDLER(interval, interval_post_handler);
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_demo_process, "Zolertia MQTT Client");
/*---------------------------------------------------------------------------*/
static void
publish_led_off(void *d)
{
  leds_off(STATUS_LED);
}
/*---------------------------------------------------------------------------*/
static void
print_config_info(void)
{
  printf("Auth User --------------> %s\n", conf.auth_user);
  printf("Auth Token -------------> %s\n", conf.auth_token);
  printf("Pub Interval -----------> %u\n", conf.pub_interval_check);

  /* FIXME: include here an arch function to print the sensor config */
}
/*---------------------------------------------------------------------------*/
#if 0
static int
write_config_to_flash(void)
{
  int fd;
  uint8_t *pCfg;
  config_flash_t store;

  pCfg = (uint8_t *) &store;

  store.magic_word = COFFEE_MAGIC_WORD;
  memcpy(store.auth_user, conf.auth_user, CONFIG_AUTH_USER_LEN);
  memcpy(store.auth_token, conf.auth_token, CONFIG_AUTH_TOKEN_LEN);
  store.pub_interval_check = conf.pub_interval_check;
  store.crc = crc16_data(pCfg, (sizeof(config_flash_t) - 2), 0);

  fd = cfs_open("mqtt_config", CFS_READ | CFS_WRITE);

  if(fd >= 0) {
    if(cfs_write(fd, pCfg, sizeof(config_flash_t)) > 0) {
      PRINTF("Client: saved in flash (MW 0x%02X, CRC16 %u, len %u)\n",
              store.magic_word, store.crc, sizeof(config_flash_t));
      print_config_info();
      cfs_close(fd);
    } else {
      PRINTF("Client: failed to write file\n");
      cfs_close(fd);
      return -1;
    }
  } else {
    PRINTF("Client: failed to open file\n");
    return -1;
  }

  return 0;
}
#endif
/*---------------------------------------------------------------------------*/
static int
read_config_from_flash(void)
{
  int fd;
  uint8_t *pCfg;
  uint16_t crc;
  config_flash_t store;

  pCfg = (uint8_t *) &store;

  /* FIXME: temporal until fixing the flash problem */
  return -1;

  fd = cfs_open("mqtt_config", CFS_READ | CFS_WRITE);

  if(fd >= 0) {
    if(cfs_read(fd, pCfg, sizeof(config_flash_t)) > 0) {
      PRINTF("Client: Read from flash (MW 0x%02X, CRC16 %u len %u)\n",
             store.magic_word, store.crc, sizeof(config_flash_t));
      crc = crc16_data(pCfg, (sizeof(config_flash_t) - 2), 0);

      if((store.magic_word == COFFEE_MAGIC_WORD) && (crc == store.crc)) {
        PRINTF("Client: magic word and CRC check OK\n");
        memcpy(conf.auth_user, store.auth_user, CONFIG_AUTH_USER_LEN);
        memcpy(conf.auth_token, store.auth_token, CONFIG_AUTH_TOKEN_LEN);
        conf.pub_interval_check = store.pub_interval_check;
        print_config_info();

      } else {
        PRINTF("Client: invalid magic word or CRC, using defaults\n");
        cfs_close(fd);
        return -1;
      }

    } else {
      PRINTF("Client: not found in flash\n");
      cfs_close(fd);
      return -1;
    }
    cfs_close(fd);
  } else {
    PRINTF("Client: failed to open file\n");
    return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
subscribe(char * topic)
{
  mqtt_status_t status;
  status = mqtt_subscribe(&conn, NULL, topic, MQTT_QOS_LEVEL_0);

  PRINTF("Client: Subscribing to %s\n", topic);
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    PRINTF("Client: Tried to subscribe but command queue was full!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    PRINTF("Client: Application has a MQTT connection\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    PRINTF("Client: MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&mqtt_demo_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    /* Implement first_flag in publish message? */
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      PRINTF("Client: Application received a publish on topic '%s'. Payload "
          "size is %i bytes. Content:\n\n",
          msg_ptr->topic, msg_ptr->payload_length);
    }

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk,
                msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
    PRINTF("Client: Application is subscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    PRINTF("Client: Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    PRINTF("Client: Publishing complete.\n");
    break;
  }
  default:
    PRINTF("Client: Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
construct_client_id(void)
{
  int len = snprintf(client_id, CONFIG_IP_ADDR_STR_LEN, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  if(len < 0 || len >= CONFIG_IP_ADDR_STR_LEN) {
    PRINTF("Client: buffer size too small for client ID: %d\n", len);
  }
}
/*---------------------------------------------------------------------------*/
static void
init_platform_config(void)
{
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));

  if(strlen(DEFAULT_AUTH_USER)) {
    memcpy(conf.auth_user, DEFAULT_USER_ID, CONFIG_AUTH_USER_LEN);
  } else {
    printf("Warning: No hardcoded Auth User\n");
  }

  if(strlen(DEFAULT_AUTH_TOKEN)) {
    memcpy(conf.auth_token, DEFAULT_AUTH_TOKEN, CONFIG_AUTH_TOKEN_LEN);
  } else {
    printf("Warning: No hardcoded Auth Token\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
init_config(void)
{
  /* Fill in the MQTT client configuration info */
  memset(&conf, 0, sizeof(mqtt_client_config_t));
  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_SAMPLING_INTERVAL;
  conf.pub_interval_check = DEFAULT_PUBLISH_INTERVAL;

  /* Formats the device's IP address in a string */
  construct_client_id();
}
/*---------------------------------------------------------------------------*/
void
publish(uint8_t *app_buffer, uint16_t len)
{
  PRINTF("Client: Publish %s to %s\n", app_buffer, pub_topic);

  mqtt_publish(&conn, NULL, pub_topic, app_buffer, len, MQTT_QOS_LEVEL_0,
               MQTT_RETAIN_OFF);
}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  /* Connect to MQTT server */
  mqtt_connect(&conn, conf.broker_ip, conf.broker_port, conf.pub_interval * 3);
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

    mqtt_set_username_password(&conn, conf.auth_user, conf.auth_token);

    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    PRINTF("Client: Init\n");

    /* Notice there is no "break" here, it will continue to the
     * STATE_REGISTERED
     */
  case STATE_REGISTERED:
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) {
      /* Registered and with a public IP. Connect */
      PRINTF("Client: Registered. Connect attempt %u\n", connect_attempt);
      connect_to_broker();

    } else {
      leds_on(STATUS_LED);
      ctimer_set(&ct, NO_NET_LED_DURATION, publish_led_off, NULL);
    }
    etimer_set(&publish_periodic_timer, NET_CONNECT_PERIODIC);
    return;
    break;

  case STATE_CONNECTING:
    leds_on(STATUS_LED);
    ctimer_set(&ct, CONNECTING_LED_DURATION, publish_led_off, NULL);
    /* Not connected yet. Wait */
    PRINTF("Client: Connecting (%u)\n", connect_attempt);
    break;

  case STATE_CONNECTED:
    /* Notice there's no "break" here, it will continue to subscribe */

  case STATE_PUBLISHING:
    /* If the timer expired, the connection is stable. */
    if(timer_expired(&connection_life)) {
      /*
       * Intentionally using 0 here instead of 1: We want RECONNECT_ATTEMPTS
       * attempts if we disconnect after a successful connect
       */
      connect_attempt = 0;
    }

    if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      /* Connected and Publish */
      if(state == STATE_CONNECTED) {

        /* FIXME: there is only room for one subscription, limited by the MQTT driver */
        subscribe(cfg_topic);
        // subscribe(cmd_topic);

        /* Notiy the platforms we are connected and ready to the broker */
        process_post(PROCESS_BROADCAST, mqtt_client_event_connected, NULL);

        state = STATE_PUBLISHING;
      }

      /* Return here so we don't end up rescheduling the timer */
      return;

    } else {
      /*
       * Our publish timer fired, but some MQTT packet is already in flight
       * (either not sent at all, or sent but not fully ACKd).
       *
       * This can mean that we have lost connectivity to our broker or that
       * simply there is some network delay. In both cases, we refuse to
       * trigger a new message and we wait for TCP to either ACK the entire
       * packet after retries, or to timeout and notify us.
       */
      PRINTF("Client: Publishing... (MQTT state=%d, q=%u)\n", conn.state,
          conn.out_queue_full);
    }
    break;

  case STATE_DISCONNECTED:
    PRINTF("Client: Disconnected\n");

    /* Notiy the platforms we are connected to the broker */
    process_post(PROCESS_BROADCAST, mqtt_client_event_disconnected, NULL);

    if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
      mqtt_disconnect(&conn);
      connect_attempt++;

      interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
        RECONNECT_INTERVAL << 3;

      PRINTF("Client: Disconnected. Attempt %u in %lu ticks\n",
                                                     connect_attempt, interval);

      etimer_set(&publish_periodic_timer, interval);

      state = STATE_REGISTERED;
      return;

    } else {
      /* Max reconnect attempts reached. Enter error state */
      state = STATE_ERROR;
      PRINTF("Client: Aborting connection after %u attempts\n", connect_attempt - 1);
    }
    break;

  case STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
    PRINTF("Client: Bad configuration.\n");
    return;

  case STATE_ERROR:
  default:
    leds_on(STATUS_LED);
    /*
     * This should never happen.
     * If we enter here it's because of some error. Stop timers. The only thing
     * that can bring us out is a new config event
     */
    PRINTF("Client: Default case: State=0x%02x\n", state);
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_demo_process, ev, data)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  printf("\nZolertia MQTT client\n");
  printf("  Broker IP:    %s\n", conf.broker_ip);
  printf("  Data topic:   %s\n", pub_topic);
  printf("  Config topic: %s\n", cfg_topic);
  printf("  Cmd topic:    %s\n\n", cmd_topic);

  /* Set the initial state */
  state = STATE_INIT;

  /* Retrieve nameserver configuration, not really used since we use a NAT64
   * address
   */
  uip_ipaddr(&ip4addr, 8, 8, 8, 8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  /* Stop and wait until the node joins the network */
  leds_on(LEDS_RED);
  printf("Connecting to the network... \n");
  etimer_set(&publish_periodic_timer, CLOCK_SECOND * 35);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&publish_periodic_timer));
  leds_off(LEDS_RED);

  /* Allocate processes */
  mqtt_client_event_connected = process_alloc_event();
  mqtt_client_event_disconnected = process_alloc_event();

  /* Initialize the publish callback handler */
  pub_handler = NULL;

  /* Start the webserver */
  process_start(&httpd_simple_process, NULL);

  /* The HTTPD_SIMPLE_POST_HANDLER macro should have already created the
   * interval_handler()
   */
  httpd_simple_register_post_handler(&interval_handler);

  /* Check if we can start the state machine with the stored values, or we need
   * to bootstrap until configured over httpd.  When flashing a new image, the
   * configuration values are lost.  Default is to leave both DEFAULT_USER_ID
   * and DEFAULT_AUTH_TOKEN as empty strings.  If both strings are not NULL or
   * empty, then we start with the hard-coded config
   */

  if(read_config_from_flash() == -1) {

    init_config();
    init_platform_config();

    if((strlen(DEFAULT_USER_ID)) && (strlen(DEFAULT_AUTH_TOKEN))) {

      printf("Hardcoded Auth User is %s\n", conf.auth_user);
      printf("Hardcoded Auth Token is %s\n\n", conf.auth_token);
      print_config_info();

    } else {
      printf("No client information found!\n");
      printf("Awaiting provisioning over the httpd webserver\n");

      /* Bootstrap and wait until we received a valid configuration */
      PROCESS_WAIT_EVENT_UNTIL(httpd_simple_event_new_config);

      printf("*** New configuration over httpd\n");
    }
  }

  /* Start the platform process */
  process_start(&relayr_process, NULL);

  /* Schedule next timer event ASAP */
  etimer_set(&publish_periodic_timer, 0);

  while(1) {

    /* The update_config() should schedule a timer right away */
    PROCESS_YIELD();

    if(ev == httpd_simple_event_new_config) {
      PRINTF("Client: *** New configuration over httpd\n");
      etimer_set(&publish_periodic_timer, 0);
    }

    /* We are waiting for the timer to kick the state_machine() */
    if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
       ev == PROCESS_EVENT_POLL) {
      state_machine();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
