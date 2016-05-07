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
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "ip64-addr.h"
#include "dev/sht25.h"
#include "net/ip/uip-debug.h"
#include "dev/sys-ctrl.h"
#include "lib/crc16.h"
#include "mqtt-relayr.h"
#include "httpd-simple.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"

#include <string.h>
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
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
/* Provide visible feedback via LEDS during various states */
/* When connecting to broker */
#define CONNECTING_LED_DURATION    (CLOCK_SECOND >> 2)

/* Each time we try to publish */
#define PUBLISH_LED_ON_DURATION    (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER              0xFF
#define RECONNECT_INTERVAL         (CLOCK_SECOND * 2)

/*
 * Number of times to try reconnecting to the broker.
 * Can be a limited number (e.g. 3, 10 etc) or can be set to RETRY_FOREVER
 */
#define RECONNECT_ATTEMPTS         RETRY_FOREVER
#define CONNECTION_STABLE_TIME     (CLOCK_SECOND * 5)

/*
 * Sensor and device data publication interval
 */
#define DEFAULT_PUBLISH_TIME          (DEFAULT_PUBLISH_INTERVAL * CLOCK_SECOND)
#define DEFAULT_KEEP_ALIVE_TIMER      ((DEFAULT_PUBLISH_INTERVAL * 3)/2)
/*---------------------------------------------------------------------------*/
/* Payload length of ICMPv6 echo requests used to measure RSSI with def rt */
#define ECHO_REQ_PAYLOAD_LEN   20
/*---------------------------------------------------------------------------*/
static struct timer connection_life;
static struct etimer alarm_expired;
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
/* A timeout used when waiting to connect to a network */
#define NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 2)
#define NO_NET_LED_DURATION         (NET_CONNECT_PERIODIC >> 1)
/*---------------------------------------------------------------------------*/
static uint8_t sensors_status = DEFAULT_SENSOR_STATE;
/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_demo_process);
AUTOSTART_PROCESSES(&mqtt_demo_process);
/*---------------------------------------------------------------------------*/
static uint16_t temp_threshold = DEFAULT_TEMP_THRESH;
static uint16_t humd_threshold = DEFAULT_HUMD_THRESH;
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
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define APP_BUFFER_SIZE 512
static struct mqtt_connection conn;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
static struct etimer publish_periodic_timer;
static struct ctimer ct;
static char *buf_ptr;
static uint16_t seq_nr_value = 0;
/*---------------------------------------------------------------------------*/
/* Parent RSSI functionality */
static struct uip_icmp6_echo_reply_notification echo_reply_notification;
static int def_rt_rssi = 0;
/*---------------------------------------------------------------------------*/
/* Holds the MQTT configuration */
mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
static int
interval_post_handler(char *key, int key_len, char *val, int val_len)
{
/*
  int fd;
  int rv = 0;
  uint8_t buf[2];

  if(key_len != strlen("interval") ||
     strncasecmp(key, "interval", strlen("interval")) != 0) {
    return HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;
  }

  rv = atoi(val);

  if(rv < WEATHER_STATION_WS_INTERVAL_MIN ||
     rv > WEATHER_STATION_WS_INTERVAL_MAX) {
    return HTTPD_SIMPLE_POST_HANDLER_ERROR;
  }

  ws_config.interval = rv * CLOCK_SECOND;
  PRINTF("WS: new interval tick is: %u\n", rv);

  fd = cfs_open("WS_int", CFS_READ | CFS_WRITE);
  if(fd >= 0) {
    buf[0] = ((uint8_t *)&ws_config.interval)[0];
    buf[1] = ((uint8_t *)&ws_config.interval)[1];
    if(cfs_write(fd, &buf, 2) > 0) {
      PRINTF("WS: interval saved in flash\n");
    }
    cfs_close(fd);
  }

*/

  return HTTPD_SIMPLE_POST_HANDLER_OK;
}
/*---------------------------------------------------------------------------*/
HTTPD_SIMPLE_POST_HANDLER(interval, interval_post_handler);
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_demo_process, "Relayr MQTT App");
/*---------------------------------------------------------------------------*/
/* Converts the IPv6 address to string */
static int
ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr)
{
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}
/*---------------------------------------------------------------------------*/
static void
ipaddr_ownaddr(char *buf)
{
  uint8_t i, state;
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
      (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      printf("IPv6 own address: ");
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
      uip_ip6addr_copy(buf, &uip_ds6_if.addr_list[i].ipaddr);
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Handles the ping response and updates the RSSI value */
static void
echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data,
                   uint16_t datalen)
{
  if(uip_ip6addr_cmp(source, uip_ds6_defrt_choose())) {
    def_rt_rssi = sicslowpan_get_last_rssi();
  }
}
/*---------------------------------------------------------------------------*/
/* Helper function, when publishing to a topic it will turn the STATUS_LED after
 * we are done
 */
static void
publish_led_off(void *d)
{
  leds_off(STATUS_LED);
}
/*---------------------------------------------------------------------------*/
/* Helper function, enable or disable the sensor based on a received command */
static void
activate_sensors(uint8_t state)
{
  if(state) {
    printf("*** Activating sensors!\n");
    SENSORS_ACTIVATE(sht25);
    return;
  }
  printf("*** De-activating sensors!\n");
  SENSORS_DEACTIVATE(sht25);
}
/*---------------------------------------------------------------------------*/
static void
print_config_info(void)
{
  printf("Auth User --------------> %s\n", conf.auth_user);
  printf("Auth Token -------------> %s\n", conf.auth_token);
  printf("Pub Interval -----------> %u\n", conf.pub_interval_check);
  printf("Temperature threshold --> %u\n", temp_threshold);
  printf("Humidity threshold -----> %u\n", humd_threshold);
}
/*---------------------------------------------------------------------------*/
static int
write_config_to_flash(void)
{
  int fd;
  uint8_t *pCfg;
  config_flash_t store;

  pCfg = (uint8_t *) &store;

  store.magic_word = 0xABCD;
  memcpy(store.auth_user, conf.auth_user, CONFIG_AUTH_USER_LEN);
  memcpy(store.auth_token, conf.auth_token, CONFIG_AUTH_TOKEN_LEN);
  store.pub_interval_check = conf.pub_interval_check;
  store.temp_threshold = temp_threshold;
  store.humd_threshold = humd_threshold;
  store.crc = crc16_data(pCfg, (sizeof(config_flash_t) - 2), 0);

  fd = cfs_open("relayr_config", CFS_READ | CFS_WRITE);

  if(fd >= 0) {
    if(cfs_write(fd, pCfg, sizeof(config_flash_t)) > 0) {
      printf("Config: saved in flash (MW 0x%02X, CRC16 %u, len %u)\n",
              store.magic_word, store.crc, sizeof(config_flash_t));
      print_config_info();
      cfs_close(fd);
    } else {
      printf("Config: failed to write file\n");
      cfs_close(fd);
      return -1;
    }
  } else {
    printf("Config: failed to open file\n");
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
read_config_from_flash(void)
{
  int fd;
  uint8_t *pCfg;
  uint16_t crc;
  config_flash_t store;

  pCfg = (uint8_t *) &store;

  // FIXME: temporal until fixing the flash problem
  return -1;

  fd = cfs_open("relayr_config", CFS_READ | CFS_WRITE);

  if(fd >= 0) {
    if(cfs_read(fd, pCfg, sizeof(config_flash_t)) > 0) {
      printf("Config: Read from flash (MW 0x%02X, CRC16 %u len %u)\n",
             store.magic_word, store.crc, sizeof(config_flash_t));
      crc = crc16_data(pCfg, (sizeof(config_flash_t) - 2), 0);

      if((store.magic_word == 0xABCD) && (crc == store.crc)) {
        printf("Config: magic word and CRC check OK\n");
        memcpy(conf.auth_user, store.auth_user, CONFIG_AUTH_USER_LEN);
        memcpy(conf.auth_token, store.auth_token, CONFIG_AUTH_TOKEN_LEN);
        conf.pub_interval_check = store.pub_interval_check;
        temp_threshold = store.temp_threshold;
        humd_threshold = store.humd_threshold;
        print_config_info();

      } else {
        printf("Config: invalid magic word or CRC, using defaults\n");
        cfs_close(fd);
        return -1;
      }

    } else {
      printf("Config: not found in flash\n");
      cfs_close(fd);
      return -1;
    }
    cfs_close(fd);
  } else {
    printf("Config: failed to open file\n");
    return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* This function handler receives publications to which we are subscribed */
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  uint16_t aux;

  printf("Pub Handler: topic='%s' (len=%u), chunk='%s', chunk_len=%u\n", topic,
         topic_len, chunk, chunk_len);

  /* Most of the commands follow a boolean-logic at least */
  if(chunk_len <= 0) {
    printf("Error: Chunk should be at least a single digit integer or string\n");
    return;
  }

  /* This is a command event, it uses "true" and "false" strings
   * We expect commands to have the following syntax:
   * {"name":"enable_sensor","value":false}
   * That is why we use an index of "9" to search for the command string
   */
  if(strncmp(topic, DEFAULT_SUBSCRIBE_CMD, CONFIG_SUB_CMD_TOPIC_LEN) == 0) {

    /* Toggle a given LED */
    if(strncmp((const char *)&chunk[9], DEFAULT_SUBSCRIBE_CMD_LEDS,
               strlen(DEFAULT_SUBSCRIBE_CMD_LEDS)) == 0) {
      printf("Command received: toggle LED\n");

      if(strncmp((const char *)&chunk[strlen(DEFAULT_SUBSCRIBE_CMD_LEDS) + 19],
        "true", 4) == 0) {
        leds_on(CMD_LED);
      } else if(strncmp((const char *)&chunk[strlen(DEFAULT_SUBSCRIBE_CMD_LEDS) + 19],
        "false", 5) == 0) {
        leds_off(CMD_LED);
      } else {
        printf("Error: invalid command argument (expected boolean)!\n");
        return;
      }

    /* Restart the device */
    } else if(strncmp((const char *)&chunk[9], DEFAULT_SUBSCRIBE_CMD_REBOOT,
               strlen(DEFAULT_SUBSCRIBE_CMD_REBOOT)) == 0) {
      printf("Command received: reboot\n");

      /* This is fixed to check only "true" arguments */
      if(strncmp((const char *)&chunk[strlen(DEFAULT_SUBSCRIBE_CMD_REBOOT) + 19],
        "true", 4) == 0) {
        sys_ctrl_reset();
      } else {
        printf("Error: invalid command argument (expected only 'true')!\n");
        return;
      }

    /* Enable or disable external sensors */
    } else if(strncmp((const char *)&chunk[9], DEFAULT_SUBSCRIBE_CMD_SENSOR,
               strlen(DEFAULT_SUBSCRIBE_CMD_SENSOR)) == 0) {
      printf("Command received: enable/disable sensor\n");

      if(strncmp((const char *)&chunk[strlen(DEFAULT_SUBSCRIBE_CMD_SENSOR) + 19],
        "true", 4) == 0) {
        sensors_status = 1;
      } else if(strncmp((const char *)&chunk[strlen(DEFAULT_SUBSCRIBE_CMD_SENSOR) + 19],
        "false", 5) == 0) {
        sensors_status = 0;
      } else {
        printf("Error: invalid command argument (expected boolean)!\n");
        return;
      }
      activate_sensors(sensors_status);

    } else {
      printf("Command not recognized\n");
    }

  /* This is a configuration event
   * We expect the configuration payload to follow the next syntax:
   * {"name":"update_period","value":61}
   */
  } else if(strncmp(topic, DEFAULT_SUBSCRIBE_CFG,
                           CONFIG_SUB_CFG_TOPIC_LEN) == 0) {

    /* Change the update period */
    if(strncmp((const char *)&chunk[9], DEFAULT_SUBSCRIBE_CFG_EVENT,
               strlen(DEFAULT_SUBSCRIBE_CFG_EVENT)) == 0) {

      /* Take integers as configuration value */
      aux = atoi((const char*) &chunk[strlen(DEFAULT_SUBSCRIBE_CFG_EVENT) + 19]);

      /* Check for allowed values */
      if((aux < DEFAULT_UPDATE_PERIOD_MIN) || (aux > DEFAULT_UPDATE_PERIOD_MAX)) {
        printf("Error: update interval should be between %u and %u\n", 
                DEFAULT_UPDATE_PERIOD_MIN, DEFAULT_UPDATE_PERIOD_MAX);
        return;
      }

      printf("Config: New update interval --> %u secs\n", aux);
      conf.pub_interval_check = aux;
      // write_config_to_flash();

    /* Change the temperature threshold (over) */
    } else if(strncmp((const char *)&chunk[9], DEFAULT_SUBSCRIBE_CFG_TEMPTHR,
                      strlen(DEFAULT_SUBSCRIBE_CFG_TEMPTHR)) == 0) {

      /* Take integers as configuration value */
      aux = atoi((const char*) &chunk[strlen(DEFAULT_SUBSCRIBE_CFG_TEMPTHR) + 19]);

      if((aux < DEFAULT_SHT25_TEMP_MIN) || (aux > DEFAULT_SHT25_TEMP_MAX)) {
        printf("Error: temperature threshold should be between %d and %d\n",
               DEFAULT_SHT25_TEMP_MIN, DEFAULT_SHT25_TEMP_MAX);
        return;
      }

      printf("New temperature threshold --> %u\n", aux);
      temp_threshold = aux;
      // write_config_to_flash();

    /* Change the humidity threshold (over) */
    } else if(strncmp((const char *)&chunk[9], DEFAULT_SUBSCRIBE_CFG_HUMDTHR,
                      strlen(DEFAULT_SUBSCRIBE_CFG_HUMDTHR)) == 0) {

      /* Take integers as configuration value */
      aux = atoi((const char*) &chunk[strlen(DEFAULT_SUBSCRIBE_CFG_HUMDTHR) + 19]);

      if((aux < DEFAULT_SHT25_HUMD_MIN) || (aux > DEFAULT_SHT25_HUMD_MAX)) {
        printf("Error: humidity threshold should be between %u and %u\n",
               DEFAULT_SHT25_HUMD_MIN, DEFAULT_SHT25_HUMD_MAX);
        return;
      }

      printf("New humidity threshold --> %u\n", aux);
      humd_threshold = aux;
      // write_config_to_flash();

    /* Invalid configuration topic */
    } else {
      printf("Configuration parameter not recognized\n");
    }

  } else {
    printf("Incorrect topic or chunk len. Ignored\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
subscribe(char * topic)
{
  mqtt_status_t status;
  status = mqtt_subscribe(&conn, NULL, topic, MQTT_QOS_LEVEL_0);

  printf("APP - Subscribing to %s\n", topic);
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    printf("APP - Tried to subscribe but command queue was full!\n");
  }
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
static void
construct_client_id(void)
{
  int len = snprintf(client_id, CONFIG_IP_ADDR_STR_LEN, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  if(len < 0 || len >= CONFIG_IP_ADDR_STR_LEN) {
    printf("Error: buffer size too small for client ID: %d\n", len);
  }
}
/*---------------------------------------------------------------------------*/
static void
init_relayr_config(void)
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

  /* Configures a callback for a ping request to our parent node, to retrieve
   * the RSSI value
   */
  def_rt_rssi = 0x8000000;
  uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
                                    echo_reply_handler);

  /* Formats the device's IP address in a string */
  construct_client_id();
}
/*---------------------------------------------------------------------------*/
static void
publish_alarm(char *alarm, uint16_t value, uint16_t value_thresh)
{
  if(etimer_expired(&alarm_expired)) {
    printf("*** Alarm! %s %u over %u\n", alarm, value, value_thresh);
    snprintf(app_buffer, APP_BUFFER_SIZE,
             "[{\"meaning\":\"%s\",\"value\":%d.%u}]",
             alarm, value / 100, value % 100);

    printf("APP - Publish %s to %s\n", app_buffer, pub_topic);

    mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
                 strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);

    etimer_set(&alarm_expired, (CLOCK_SECOND * 15));
  }
}
/*---------------------------------------------------------------------------*/
static int
add_pub_topic(uint16_t length, char *meaning, char *value,
              uint8_t first, uint8_t more)
{
  int len = 0;
  int pos = 0;

  if((buf_ptr == NULL) || (length <= 0)){
    printf("Error: null buffer or lenght less than zero\n");
    return -1;
  }

  if(first) {
    len = snprintf(buf_ptr, length, "%s", "[");
    pos = len;
    buf_ptr += len;
  }

  len = snprintf(buf_ptr, (length - pos),
                 "{\"meaning\":\"%s\",\"value\":\"%s\"}",
                 meaning, value);
 
  if(len < 0 || pos >= length) {
    printf("Buffer too short. Have %d, need %d + \\0\n", length, len);
    return -1;
  }

  pos += len;
  buf_ptr += len;

  if(more) {
    len = snprintf(buf_ptr, (length - pos), "%s", ",");
  } else {
    len = snprintf(buf_ptr, (length - pos), "%s", "]");
  }

  pos += len;
  buf_ptr += len;

  return pos;
}
/*---------------------------------------------------------------------------*/
static void
publish_event(uint16_t temp, uint16_t humd)
{
  char aux[64];
  int len = 0;
  int remain = APP_BUFFER_SIZE;

  /* Use the buf_ptr as pointer to the actual application buffer */
  buf_ptr = app_buffer;

  /* Retrieve our own IPv6 address
   * This is the starting value to be sent, the `first` argument should be 1,
   * and the `more` argument 1 as well, as we want to add more values to our
   * list
   */
  memset(aux, 0, sizeof(aux));
  ipaddr_ownaddr(aux);
  printf("Checking IPv6 own address %s\n", aux);
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_ID, "hola", 1, 1);

  remain =- len;

  memset(aux, 0, sizeof(aux));
  snprintf(aux, sizeof(aux), "%lu", clock_seconds());
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_UPTIME, aux, 0, 1);

  remain =- len;

  memset(aux, 0, sizeof(aux));
  ipaddr_sprintf(aux, sizeof(aux), uip_ds6_defrt_choose());
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_PARENT, aux, 0, 1);

  remain =- len;

  memset(aux, 0, sizeof(aux));
  snprintf(aux, sizeof(aux), "%d", def_rt_rssi);
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_RSSI, aux, 0, 1);

  remain =- len;

  memset(aux, 0, sizeof(aux));
  snprintf(aux, sizeof(aux), "%d.%02u", temp / 100, temp % 100);
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_TEMP, aux, 0, 1);

  remain =- len;

  /* The last value to be sent, the `more` argument should be zero */
  memset(aux, 0, sizeof(aux));
  snprintf(aux, sizeof(aux), "%u.%02u", humd / 100, humd % 100);
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_HUMD, aux, 0, 0);

  mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
               strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);

  printf("APP - Publish %s to %s\n", app_buffer, pub_topic);
}
/*---------------------------------------------------------------------------*/
static void
publish(void)
{
  uint16_t temp;
  uint16_t humd;

  seq_nr_value++;

  /* clear buffer */
  memset(app_buffer, 0, APP_BUFFER_SIZE);

  /* Sample sensors and check for alarms only if sensors are enabled */
  if(sensors_status) {
    temp = sht25.value(SHT25_VAL_TEMP);
    humd = sht25.value(SHT25_VAL_HUM);

    printf("APP - Temperature %d.%02u Humidity %u.%02u\n", temp / 100, temp % 100,
                                                           humd / 100, humd % 100);

    /* Check for valid values, if a mishap is found (i.e sensor not present but
     * enabled, then use the default unused-value and let is skip the checks
     * further below
     */
    if((temp < DEFAULT_SHT25_TEMP_MIN) || (temp > DEFAULT_SHT25_TEMP_MAX)) {
      printf("Error: temperature value invalid: should be between %d and %d\n",
             DEFAULT_SHT25_TEMP_MIN, DEFAULT_SHT25_TEMP_MAX);
      temp = DEFAULT_TEMP_NOT_USED;
    }

    if((humd < DEFAULT_SHT25_HUMD_MIN) || (humd > DEFAULT_SHT25_HUMD_MAX)) {
      printf("Error: humidity value invalid: should be between %d and %d\n",
             DEFAULT_SHT25_HUMD_MIN, DEFAULT_SHT25_HUMD_MAX);
      humd = DEFAULT_HUMD_NOT_USED;
    }

    /* No alarm and no periodic report event, discard */
    if((temp < temp_threshold) && (humd < humd_threshold) &&
       (seq_nr_value % conf.pub_interval_check)) {
      return;
    }

    /* Publish a temperature alarm
     * It has a higher priority than the humidity alarm, to avoid publishing
     * to two topics at the same time
     */
    if(temp >= temp_threshold) {
      publish_alarm(DEFAULT_PUBLISH_ALARM_TEMP, temp, temp_threshold);
      return;
    }

    if(humd >= humd_threshold) {
      publish_alarm(DEFAULT_PUBLISH_ALARM_HUMD, humd, humd_threshold);
      return;
    }

  /* Return default temperature/humidity values */
  } else {
    temp = DEFAULT_TEMP_NOT_USED;
    humd = DEFAULT_HUMD_NOT_USED;
  }

  /* Check again for period threshold as there should not be any alarm, also
   * in case sensors are disabled
   */
  if(seq_nr_value % conf.pub_interval_check) {
    return;
  }

  /* Publish our periodic data */
  publish_event(temp, humd);
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
ping_parent(void)
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
    printf("Parent not available\n");
    return;
  }

  uip_icmp6_send(uip_ds6_defrt_choose(), ICMP6_ECHO_REQUEST, 0,
                 ECHO_REQ_PAYLOAD_LEN);
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
    printf("Init\n");

    /* Notice there is no "break" here, it will continue to the
     * STATE_REGISTERED
     */
  case STATE_REGISTERED:
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) {
      /* Registered and with a public IP. Connect */
      printf("Registered. Connect attempt %u\n", connect_attempt);
      ping_parent();
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
    printf("Connecting (%u)\n", connect_attempt);
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
      /* Connected. Publish */
      if(state == STATE_CONNECTED) {

        /* Subscribe to topics */
        /* FIXME: there is only room for one subscription, limited by the MQTT driver */

        subscribe(cfg_topic);
        // subscribe(cmd_topic);

        state = STATE_PUBLISHING;

      } else {
        leds_on(STATUS_LED);
        ctimer_set(&ct, PUBLISH_LED_ON_DURATION, publish_led_off, NULL);
        publish();
      }
      etimer_set(&publish_periodic_timer, conf.pub_interval);

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
      printf("Publishing... (MQTT state=%d, q=%u)\n", conn.state,
          conn.out_queue_full);
    }
    break;

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
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  printf("\nZolertia & Relayr MQTT Demo Process\n");
  printf("Client information:\n");
  printf("  Broker IP:    %s\n", conf.broker_ip);
  printf("  Data topic:   %s\n", pub_topic);
  printf("  Config topic: %s\n", cfg_topic);
  printf("  Cmd topic:    %s\n\n", cmd_topic);

  /* Reset the counter */
  seq_nr_value = 0;

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
  etimer_set(&publish_periodic_timer, CLOCK_SECOND * 35);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&publish_periodic_timer));
  leds_off(LEDS_RED);

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
    init_relayr_config();

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

  /* Start/disable sensors */
  activate_sensors(sensors_status);

  /* Schedule next timer event ASAP */
  etimer_set(&publish_periodic_timer, 0);

  while(1) {

    /* The update_config() should schedule a timer right away */
    PROCESS_YIELD();

    if(ev == httpd_simple_event_new_config) {
      printf("*** New configuration over httpd\n");
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
