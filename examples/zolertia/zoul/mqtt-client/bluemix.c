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
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/random.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "dev/leds.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "dev/sys-ctrl.h"
#include "mqtt-client.h"
#include "mqtt-sensors.h"
#include "bluemix.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define SENSORS_NAME_EXPAND(x, y) x##y
#define SENSORS_NAME(x, y) SENSORS_NAME_EXPAND(x, y)
/*---------------------------------------------------------------------------*/
/* Payload length of ICMPv6 echo requests used to measure RSSI with def rt */
#define ECHO_REQ_PAYLOAD_LEN   20
/*---------------------------------------------------------------------------*/
#define APP_BUFFER_SIZE 512
static char *buf_ptr;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/* Topic placeholders */
static char data_topic[CONFIG_PUB_TOPIC_LEN];
/*---------------------------------------------------------------------------*/
PROCESS(bluemix_process, "IBM bluemix MQTT process");
/*---------------------------------------------------------------------------*/
/* Include there the sensors processes to include */
PROCESS_NAME(SENSORS_NAME(MQTT_SENSORS, _sensors_process));
/*---------------------------------------------------------------------------*/
static struct etimer alarm_expired;
/*---------------------------------------------------------------------------*/
static uint16_t seq_nr_value;
/*---------------------------------------------------------------------------*/
/* Parent RSSI functionality */
static struct uip_icmp6_echo_reply_notification echo_reply_notification;
static int def_rt_rssi = 0;
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
static void
ping_parent(void)
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
    PRINTF("bluemix: Parent not available\n");
    return;
  }

  uip_icmp6_send(uip_ds6_defrt_choose(), ICMP6_ECHO_REQUEST, 0,
                 ECHO_REQ_PAYLOAD_LEN);
}
/*---------------------------------------------------------------------------*/
void
activate_sensors(uint8_t state)
{
  if(state) {
    process_start(&SENSORS_NAME(MQTT_SENSORS, _sensors_process), NULL);
  } else {
    process_exit(&SENSORS_NAME(MQTT_SENSORS, _sensors_process));
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
    PRINTF("bluemix: null buffer or lenght less than zero\n");
    return -1;
  }

  if(first) {
    len = snprintf(buf_ptr, length, "%s", "{\"d\":{");
    pos = len;
    buf_ptr += len;
  }

  len = snprintf(buf_ptr, (length - pos),
                 "\"%s\":\"%s\"", meaning, value);
 
  if(len < 0 || pos >= length) {
    PRINTF("bluemix: Buffer too short. Have %d, need %d + \\0\n", length, len);
    return -1;
  }

  pos += len;
  buf_ptr += len;

  if(more) {
    len = snprintf(buf_ptr, (length - pos), "%s", ",");
  } else {
    len = snprintf(buf_ptr, (length - pos), "%s", "}}");
  }

  pos += len;
  buf_ptr += len;

  return pos;
}
/*---------------------------------------------------------------------------*/
static void
publish_alarm(sensor_val_t *sensor)
{
  uint16_t aux_int, aux_res;

  if(etimer_expired(&alarm_expired)) {

    /* Clear buffer */
    memset(app_buffer, 0, APP_BUFFER_SIZE);

    PRINTF("bluemix: Alarm! %s --> %u\n", sensor->alarm_name, sensor->value);
    aux_int = sensor->value;
    aux_res = sensor->value;

    if(sensor->pres > 0) {
      aux_int /= sensor->pres;
      aux_res %= sensor->pres;
    } else {
      aux_res = 0;
    }

    snprintf(app_buffer, APP_BUFFER_SIZE,
             "{\"d\":{\"%s\":%d.%02u}}",
             sensor->alarm_name, aux_int, aux_res);

    publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));

    /* Schedule the timer to prevent flooding the broker with the same event */
    etimer_set(&alarm_expired, (CLOCK_SECOND * DEFAULT_ALARM_TIME));
  }
}
/*---------------------------------------------------------------------------*/
static void
publish_event(sensor_values_t *msg)
{
  char aux[64];
  int len = 0;
  uint8_t i;
  uint16_t aux_int, aux_res;
  int remain = APP_BUFFER_SIZE;

  /* Clear buffer */
  memset(app_buffer, 0, APP_BUFFER_SIZE);

  /* Use the buf_ptr as pointer to the actual application buffer */
  buf_ptr = app_buffer;

  /* Retrieve our own IPv6 address
   * This is the starting value to be sent, the `first` argument should be 1,
   * and the `more` argument 1 as well, as we want to add more values to our
   * list
   */
  memset(aux, 0, sizeof(aux));

  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_ID, DEVICE_ID, 1, 1);
  remain =- len;

  /* Include the sensor values, if `sensor_name` is NULL discard */
  for(i=0; i < msg->num; i++) {
    if(msg->sensor[i].sensor_name != NULL) {
      memset(aux, 0, sizeof(aux));

      aux_int = msg->sensor[i].value;
      aux_res = msg->sensor[i].value;

      if(msg->sensor[i].pres > 0) {
        aux_int /= msg->sensor[i].pres;
        aux_res %= msg->sensor[i].pres;
      } else {
        aux_res = 0;
      }

      snprintf(aux, sizeof(aux), "%d.%02u", aux_int, aux_res);
      len = add_pub_topic(remain, msg->sensor[i].sensor_name, aux, 0, 1);
      remain =- len;
    }
  }

  memset(aux, 0, sizeof(aux));
  snprintf(aux, sizeof(aux), "%lu", clock_seconds());
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_UPTIME, aux, 0, 1);
  remain =- len;

  memset(aux, 0, sizeof(aux));
  ipaddr_sprintf(aux, sizeof(aux), uip_ds6_defrt_choose());
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_PARENT, aux, 0, 1);
  remain =- len;

  /* The last value to be sent, the `more` argument should be zero */
  memset(aux, 0, sizeof(aux));
  snprintf(aux, sizeof(aux), "%d", def_rt_rssi);
  len = add_pub_topic(remain, DEFAULT_PUBLISH_EVENT_RSSI, aux, 0, 0);

  PRINTF("bluemix: publish %s (%u)\n", app_buffer, strlen(app_buffer));
  publish((uint8_t *)app_buffer, data_topic, strlen(app_buffer));
}
/*---------------------------------------------------------------------------*/
/* This function handler receives publications to which we are subscribed */
static void
bluemix_pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  PRINTF("bluemix: no subscription supported right now\n");
}
/*---------------------------------------------------------------------------*/
static void
init_platform(void)
{
  /* Register the publish callback handler */
  MQTT_PUB_REGISTER_HANDLER(bluemix_pub_handler);

  /* Configures a callback for a ping request to our parent node, to retrieve
   * the RSSI value
   */
  def_rt_rssi = 0x8000000;
  uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
                                    echo_reply_handler);

  /* Create client id */
  snprintf(conf.client_id, DEFAULT_CONF_IP_ADDR_STR_LEN,
           "d:%s:%s:%02x%02x%02x%02x%02x%02x", DEFAULT_ORG_ID, "Zolertia",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
           linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* Create topics */
  snprintf(data_topic, CONFIG_PUB_TOPIC_LEN, "iot-2/evt/%s/fmt/json",
           DEFAULT_PUB_STRING);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(bluemix_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize platform-specific */
  init_platform();

  printf("\nIBM bluemix process started\n");
  printf("  Client ID:    %s\n", conf.client_id);
  printf("  Data topic:   %s\n", data_topic);

  while(1) {

    PROCESS_YIELD();

    if(ev == mqtt_client_event_connected) {
      seq_nr_value = 0;

      /* Ping our current parent to retrieve the RSSI signal level */
      ping_parent();

      /* No subscription implemented at the moment, continue */

      /* Enable the sensor */
      activate_sensors(0x01);
    }

    if(ev == mqtt_client_event_disconnected) {
      /* We are not connected, disable the sensors */
      activate_sensors(0x00);
    }

    /* Check for periodic publish events */
    if(ev == SENSORS_NAME(MQTT_SENSORS,_sensors_data_event)) {
      seq_nr_value++;

      /* The `pub_interval_check` is an external struct defined in mqtt-client */
      if(!(seq_nr_value % conf.pub_interval_check)) {
        sensor_values_t *msgPtr = (sensor_values_t *) data;
        publish_event(msgPtr);
      }
    }

    /* Check for alarms */
    if(ev == SENSORS_NAME(MQTT_SENSORS,_sensors_alarm_event)) {
      sensor_val_t *sensorPtr = (sensor_val_t *) data;
      publish_alarm(sensorPtr);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
