/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2017, Antonio Lignan - antonio.lignan@gmail.com
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
#if WITH_WEBSERVER
#include "httpd-simple.h"
#endif
#include "dev/sys-ctrl.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_CLIENT
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
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
#ifdef RECONNECT_ATTEMPTS_CONF
#define RECONNECT_ATTEMPTS            RECONNECT_ATTEMPTS_CONF
#else
#define RECONNECT_ATTEMPTS            RETRY_FOREVER
#endif

#define CONNECTION_STABLE_TIME        (CLOCK_SECOND * 5)
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
PROCESS_NAME(mqtt_client_process);
PROCESS_NAME(PLATFORM_NAME(MQTT_PLATFORM,_process));
AUTOSTART_PROCESSES(&mqtt_client_process);
/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
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
static void init_config(void);
/*---------------------------------------------------------------------------*/
/* Declare process events */
process_event_t mqtt_client_event_connected;
process_event_t mqtt_client_event_disconnected;
/*---------------------------------------------------------------------------*/
/* Holds the MQTT configuration */
mqtt_client_config_t conf;
static uint8_t *pCfg;
/*---------------------------------------------------------------------------*/
#if WITH_WEBSERVER
static int
auth_user_post_handler(char *key, int key_len, char *val, int val_len)
{
  if(key_len != strlen("auth_user") ||
     strncasecmp(key, "auth_user", strlen(key)) != 0) {
    return HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;
  }

  if((val_len > DEFAULT_AUTH_USER_LEN) ||
    (strlen(val) > DEFAULT_AUTH_USER_LEN)) {
    PRINTF("Client: invalid Auth User configuration\n");
    return HTTPD_SIMPLE_POST_HANDLER_ERROR;
  }

  memcpy(conf.auth_user, val, val_len);
  PRINTF("Client: New Auth User config --> %s\n", conf.auth_user);
  mqtt_write_config_to_flash("mqtt_config", pCfg, sizeof(mqtt_client_config_t));
  return HTTPD_SIMPLE_POST_HANDLER_OK;
}
/*---------------------------------------------------------------------------*/
static int
auth_token_post_handler(char *key, int key_len, char *val, int val_len)
{
  if(key_len != strlen("auth_token") ||
     strncasecmp(key, "auth_token", strlen(key)) != 0) {
    return HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;
  }

  if((val_len > DEFAULT_AUTH_TOKEN_LEN) ||
    (strlen(val) > DEFAULT_AUTH_TOKEN_LEN)) {
    PRINTF("Client: invalid Auth Token configuration\n");
    return HTTPD_SIMPLE_POST_HANDLER_ERROR;
  }

  memcpy(conf.auth_token, val, val_len);
  PRINTF("Client: New Auth Token config --> %s\n", conf.auth_token);
  mqtt_write_config_to_flash("mqtt_config", pCfg, sizeof(mqtt_client_config_t));
  return HTTPD_SIMPLE_POST_HANDLER_OK;
}
/*---------------------------------------------------------------------------*/
HTTPD_SIMPLE_POST_HANDLER(auth_user, auth_user_post_handler);
HTTPD_SIMPLE_POST_HANDLER(auth_token, auth_token_post_handler);
#endif /* WITH_WEBSERVER */
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_client_process, "Zolertia MQTT Client");
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
int
mqtt_write_config_to_flash(char *name, uint8_t *buf, uint16_t len)
{
  int fd;
  uint16_t crc, mw;

  /* magic word */
  mw = (uint16_t) COFFEE_MAGIC_WORD;
  buf[0] = (uint8_t)mw & 0x00FF;
  buf[1] = (uint8_t)((mw & 0xFF00) >> 8);

  crc = crc16_data(buf, (len- 2), 0);
  buf[len-2] = ((uint8_t *)&crc)[0];
  buf[len-1] = ((uint8_t *)&crc)[1];

  fd = cfs_open(name, CFS_READ | CFS_WRITE);

  if(fd >= 0) {
    if(cfs_write(fd, buf, len) > 0) {
      PRINTF("Client: saved in flash (MW 0x%02X, CRC16 %u, len %u)\n",
              buf[0] + (buf[1] << 8), buf[len-2] + (buf[len-1] << 8), len);
      print_config_info();
      cfs_close(fd);
    } else {
      PRINTF("Client: failed to write file %s\n", name);
      cfs_close(fd);
      return -1;
    }
  } else {
    PRINTF("Client: failed to open file %s\n", name);
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
mqtt_read_config_from_flash(char *name, uint8_t *buf, uint16_t len)
{
  int fd;
  uint16_t crc;
  uint16_t mw;
  uint16_t crc_read;

  fd = cfs_open(name, CFS_READ | CFS_WRITE);

  if(fd >= 0) {
    if(cfs_read(fd, buf, len) > 0) {
      mw = buf[0] + (buf[1] << 8);
      crc_read = buf[len-2] + (buf[len-1] << 8);

      PRINTF("Client: Read from flash (MW 0x%02X, CRC16 %u len %u)\n", mw,
             crc_read, len);
      crc = crc16_data(buf, (len-2), 0);

      if((mw == COFFEE_MAGIC_WORD) && (crc == crc_read)) {
        PRINTF("Client: magic word and CRC check OK\n");
        print_config_info();

      } else {
        PRINTF("Client: invalid magic word or CRC, MQ 0x%02X, CRC16 %u vs %u\n",
               mw, crc_read, crc);
        cfs_close(fd);
        init_config();
        return -1;
      }

    } else {
      PRINTF("Client: not found in flash\n");
      cfs_close(fd);
      return -1;
    }
    cfs_close(fd);
  } else {
    PRINTF("Client: failed to open file %s\n", name);
    return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
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
    PRINTF("\nClient: Application has a MQTT connection\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    PRINTF("\nClient: MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&mqtt_client_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    /* Implement first_flag in publish message? */
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      PRINTF("Client: Application received a publish on topic '%s'. Payload "
          "size is %i bytes\n\n",
          msg_ptr->topic, msg_ptr->payload_length);
    }

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk,
                msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
    PRINTF("\nClient: Application is subscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    PRINTF("\nClient: Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    PRINTF("\nClient: Publishing complete.\n");
    break;
  }
  default:
    PRINTF("\nClient: Application got a MQTT error event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
init_config(void)
{
  /* Fill in the MQTT client configuration info */
  memset(&conf, 0, sizeof(mqtt_client_config_t));
  conf.pub_interval = DEFAULT_SAMPLING_INTERVAL;
  conf.pub_interval_check = DEFAULT_PUBLISH_INTERVAL;

  if(strlen(DEFAULT_AUTH_USER)) {
    memcpy(conf.auth_user, DEFAULT_AUTH_USER, DEFAULT_AUTH_USER_LEN);
  } else {
    conf.auth_user[0] = '\0';
    printf("Warning: No hardcoded Auth User\n");
  }

  if(strlen(DEFAULT_AUTH_TOKEN)) {
    memcpy(conf.auth_token, DEFAULT_AUTH_TOKEN, DEFAULT_AUTH_TOKEN_LEN);
  } else {
    conf.auth_token[0] = '\0';
    printf("Warning: No hardcoded Auth Token\n");
  }

  /* The client ID should be given by the platform process */
  conf.client_id[0] = '\0';
}
/*---------------------------------------------------------------------------*/
void
publish(uint8_t *app_buffer, char *pub_topic, uint16_t len)
{
  PRINTF("Client: Publish %s to %s\n", app_buffer, pub_topic);

  if(mqtt_publish(&conn, NULL, pub_topic, app_buffer, len, MQTT_QOS_LEVEL_0,
                  MQTT_RETAIN_OFF) != MQTT_STATUS_OK) {
    PRINTF("Client: *** ongoing Publication already, not queued!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  /* Connect to MQTT server */
  mqtt_connect(&conn, MQTT_DEMO_BROKER_IP_ADDR, DEFAULT_BROKER_PORT,
               conf.pub_interval_check * 3);
  connect_attempt = 0;
  state = STATE_CONNECTING;
  PRINTF("Client: Connecting");
}
/*---------------------------------------------------------------------------*/
static void
state_machine(void)
{
  switch(state) {
  case STATE_INIT:
    mqtt_register(&conn, &mqtt_client_process, conf.client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);

#if DEFAULT_CONF_AUTH_IS_REQUIRED
#if DEFAULT_AUTH_USER_ONLY_REQUIRED
    mqtt_set_username_password(&conn, conf.auth_user, NULL);
#else
    mqtt_set_username_password(&conn, conf.auth_user, conf.auth_token);
#endif
#endif
    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    PRINTF("Client: MQTT session created\n");

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
    connect_attempt++;

    if(connect_attempt < CONNECT_MAX_WAITING_PERIOD ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Not connected yet. Wait */
      printf(".");
      break;
    } else {
      connect_attempt = 0;
      state = STATE_DISCONNECTED;
    }

  case STATE_CONNECTED:
    /* Notice there's no "break" here, it will continue to subscribe */
    connect_attempt = 0;

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

        /* Notiy the platforms we are connected and ready to the broker */
        process_post(&PLATFORM_NAME(MQTT_PLATFORM, _process),
                     mqtt_client_event_connected, NULL);

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
    process_post(&PLATFORM_NAME(MQTT_PLATFORM, _process),
                 mqtt_client_event_disconnected, NULL);

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
    PRINTF("Client: Bad configuration(\n");
    return;

  case STATE_ERROR:
  default:
    leds_on(STATUS_LED);
    /*
     * This should never happen.
     */
    PRINTF("Client: System error, reboot... :'(\n");
    sys_ctrl_reset();
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_client_process, ev, data)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;
  radio_value_t aux;

  PROCESS_BEGIN();

  /* Retrieve radio configuration */
  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);

  printf("\nZolertia MQTT client\n");
  printf("  Broker IP:      %s\n", MQTT_DEMO_BROKER_IP_ADDR);
  printf("  Broken port:    %u\n", DEFAULT_BROKER_PORT);
  printf("  Radio/Encrypt:  ch%02u llsec (%u)\n", aux, WITH_LLSEC_ENABLED);
  printf("  Webserver/cfg:  %u\n", WITH_WEBSERVER);

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
  printf("Connecting to the WSN network... \n\n");
  etimer_set(&publish_periodic_timer, CLOCK_SECOND * 35);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&publish_periodic_timer));
  leds_off(LEDS_RED);

  /* Allocate processes */
  mqtt_client_event_connected = process_alloc_event();
  mqtt_client_event_disconnected = process_alloc_event();

  /* Initialize the publish callback handler */
  pub_handler = NULL;

  /* Initialize configuration strings */

  /* Start the webserver */
#if WITH_WEBSERVER
  process_start(&httpd_simple_process, NULL);

  /* The HTTPD_SIMPLE_POST_HANDLER macro should have already created the
   * respective handlers
   */
  httpd_simple_register_post_handler(&auth_user_handler);
  httpd_simple_register_post_handler(&auth_token_handler);
#endif
  /* Check if we can start the state machine with the stored values, or we need
   * to bootstrap until configured over httpd.  When flashing a new image, the
   * configuration values are lost.  Default is to leave both DEFAULT_AUTH_USER
   * and DEFAULT_AUTH_TOKEN as empty strings.  If both strings are not NULL or
   * empty, then we start with the hard-coded config
   */

  init_config();

  pCfg = (uint8_t *) &conf;
  if(mqtt_read_config_from_flash("mqtt_config", pCfg,
                                 sizeof(mqtt_client_config_t)) == -1) {

#if DEFAULT_CONF_AUTH_IS_REQUIRED
#if !DEFAULT_AUTH_USER_ONLY_REQUIRED
    if((strlen(DEFAULT_AUTH_USER)) && (strlen(DEFAULT_AUTH_TOKEN))) {
      printf("Hardcoded Auth User is %s\n", conf.auth_user);
      printf("Hardcoded Auth Token is %s\n\n", conf.auth_token);
      print_config_info();

    } else {
      printf("No client information found!\n");
#if WITH_WEBSERVER
      /* Bootstrap until we get the credentials over webservice */
      printf("Awaiting provisioning over the httpd webserver\n");

      while(1) {
        if((strlen(conf.auth_token)) && (strlen(conf.auth_user))) {
          printf("Configuration found, continuing...\n");
          break;
        }
        /* Bootstrap and wait until we received a valid configuration */
        PROCESS_WAIT_EVENT_UNTIL(httpd_simple_event_new_config);
        printf("*** New configuration over httpd\n");
      }
#else
      /* No further configuration is possible if the info is not harcoded */
      PROCESS_EXIT();
#endif /* WITH_WEBSERVER */
    }
#endif /* DEFAULT_AUTH_USER_ONLY_REQUIRED */
#endif /* DEFAULT_CONF_AUTH_IS_REQUIRED */
#if DEFAULT_AUTH_USER_ONLY_REQUIRED
    if(!(strlen(DEFAULT_AUTH_USER))) {
      printf("\nNo hardcoded User ID found\n");
#if WITH_WEBSERVER
      while(1) {
        if(strlen(conf.auth_user)) {
          printf("Configuration found, continuing...\n");
          break;
        }
        /* Bootstrap and wait until we received a valid configuration */
        PROCESS_WAIT_EVENT_UNTIL(httpd_simple_event_new_config);
        printf("*** New configuration over httpd\n");
      }
#else
      /* No further configuration is possible if the info is not harcoded */
      PROCESS_EXIT();
#endif /* WITH_WEBSERVER */
    }
#endif /* DEFAULT_AUTH_USER_ONLY_REQUIRED */
  }

  /* Start the platform process */
  process_start(&PLATFORM_NAME(MQTT_PLATFORM,_process), NULL);

  if(!(strlen(conf.client_id))) {
    printf("\nFATAL! no Client ID found!!!\n");
    printf("The configuration over webservice is not currently enabled\n");
    PROCESS_EXIT();
  }

  /* Schedule next timer event ASAP */
  etimer_set(&publish_periodic_timer, 0);

  while(1) {

    /* The update_config() should schedule a timer right away */
    PROCESS_YIELD();
#if WITH_WEBSERVER
    if(ev == httpd_simple_event_new_config) {
      PRINTF("Client: *** New configuration over httpd\n");
      etimer_set(&publish_periodic_timer, 0);
    }
#endif
    /* We are waiting for the timer to kick the state_machine() */
    if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
       ev == PROCESS_EVENT_POLL) {
      state_machine();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
