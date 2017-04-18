/*
 * Copyright (c) 2016, Atto Engenharia de Sistemas - http://attosistemas.com.br/
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

#include "contiki-net.h"
#include "mqtt.h"
#include "uip-debug.h"
#include "ip64-addr.h"
#include "lib/sensors.h"
#include "batmon-sensor.h"
#include "debug.h"
#include "assert.h"

/*-----------------------------------------------------------------------------------*/
#define DEBUG DEBUG_PRINT
/*-----------------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_process);
PROCESS(mqtt_pub_process, "AttoM MQTT Periodic Publisher");
AUTOSTART_PROCESSES(&mqtt_process, &mqtt_pub_process);
/*-----------------------------------------------------------------------------------*/
#define ATTOM_API_VERSION                    1
#define ATTOM_HARDWARE_ID                    999999
#define MAX_APPLICATION_BUFFER_SIZE          64
#define MIN_APPLICATION_BUFFER_SIZE          32
#define MAX_PATHLEN                          64
#define MAX_HOSTLEN                          64
#define MAX_MSG_COUNTER                      65000
#define MQTT_BROKER_URL                      "your.mqtt.broker.host"
#define MQTT_USERNAME                        "mqtt.username"
#define MQTT_PASSWORD                        "mqtt.password"
#define MQTT_PERIODIC_PUB_TIME               (CLOCK_SECOND * 15)
#define MQTT_CLIENT_CONN_KEEP_ALIVE          (CLOCK_SECOND * 40)
#define MQTT_CLIENT_CONN_RECONNECT           (CLOCK_SECOND * 20)
#define MQTT_CLIENT_MAX_SEGMENT_SIZE         32
/*---------------------------------------------------------------------------*/
static int broker_port = 1883;
static char *client_id = "mqtt.client_id";
static struct mqtt_connection conn;
static struct ctimer publish_msg_timer;
static struct ctimer publish_counter_timer;
static struct ctimer mqtt_conn_timer;
static char broker_ip[MAX_APPLICATION_BUFFER_SIZE] = { 0 };
static char pub_temp_topic[MAX_APPLICATION_BUFFER_SIZE] = { 0 };
static char pub_error_topic[MAX_APPLICATION_BUFFER_SIZE] = { 0 };
static char pub_self_topic[MAX_APPLICATION_BUFFER_SIZE] = { 0 };
static char mqtt_username[MIN_APPLICATION_BUFFER_SIZE] = { 0 };
static char mqtt_password[MIN_APPLICATION_BUFFER_SIZE] = { 0 };
/*---------------------------------------------------------------------------*/
static unsigned int
create_mqtt_topic(char *out, const char *resource)
{
  unsigned int len = sprintf(out, "v%d/%d/%s", ATTOM_API_VERSION, ATTOM_HARDWARE_ID, resource);
  printf("APP - create topic [%s] with len[%d]\r\n", out, len);
  return len;
}
/*---------------------------------------------------------------------------*/
static int
parse_url(const char *url, char *host, uint16_t *portptr, char *path)
{
  const char *urlptr;
  int i;
  const char *file;
  uint16_t port;

  if(url == NULL) {
    printf("null url\r\n");
    return 0;
  }

  /* Don't even try to go further if the URL is empty. */
  if(strlen(url) == 0) {
    printf("empty url\r\n");
    return 0;
  }

  /* See if the URL starts with http:// and remove it. Otherwise, we
     assume it is an implicit http://. */
  if(strncmp(url, "http://", strlen("http://")) == 0) {
    urlptr = url + strlen("http://");
  } else {
    urlptr = url;
  }

  /* Find host part of the URL. */
  if(*urlptr == '[') {
    /* Handle IPv6 addresses - scan for matching ']' */
    urlptr++;
    for(i = 0; i < MAX_HOSTLEN; ++i) {
      if(*urlptr == ']') {
        if(host != NULL) {
          host[i] = 0;
        }
        break;
      }
      if(host != NULL) {
        host[i] = *urlptr;
      }
      ++urlptr;
    }
  } else {
    for(i = 0; i < MAX_HOSTLEN; ++i) {
      if(*urlptr == 0 ||
         *urlptr == '/' ||
         *urlptr == ' ' ||
         *urlptr == ':') {
        if(host != NULL) {
          host[i] = 0;
        }
        break;
      }
      if(host != NULL) {
        host[i] = *urlptr;
      }
      ++urlptr;
    }
  }

  /* Find the port. Default is 1883. */
  port = 1883;
  if(*urlptr == ':') {
    port = 0;
    do {
      ++urlptr;
      if(*urlptr >= '0' && *urlptr <= '9') {
        port = (10 * port) + (*urlptr - '0');
      }
    } while(*urlptr >= '0' &&
            *urlptr <= '9');
  }
  if(portptr != NULL) {
    *portptr = port;
  }
  /* Find file part of the URL. */
  while(*urlptr != '/' && *urlptr != 0) {
    ++urlptr;
  }
  if(*urlptr == '/') {
    file = urlptr;
  } else {
    file = "/";
  }
  if(path != NULL) {
    strncpy(path, file, MAX_PATHLEN);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
get_temperature_reading()
{
  return batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
}
/*---------------------------------------------------------------------------*/
static void
publish_message(char *topic, mqtt_qos_level_t qos, uint8_t *data, size_t size)
{
  if(mqtt_ready(&conn)) {
    uint16_t mid = 0;
    mqtt_status_t status = mqtt_publish(&conn, &mid, topic, data, size, qos, MQTT_RETAIN_OFF);
    if(status != MQTT_STATUS_OK) {
      printf("APP - Error[%d] on publish\r\n", status);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int counter = 0;
static void
publish_counter_message()
{
  mqtt_qos_level_t qos = 0;
  counter = (counter + 1) % MAX_MSG_COUNTER;
  uint8_t counterStr[10] = { 0 };
  char *counterPtr = (char *)counterStr;
  size_t counterStrSize = snprintf(counterPtr, 10, "%d", counter);
  publish_message(pub_self_topic, qos, counterStr, counterStrSize);
  printf("APP - Reprogramming the next counter message\r\n");
  ctimer_reset(&publish_counter_timer);
}
/*---------------------------------------------------------------------------*/
static void
publish_periodic_message()
{
  char *topic;
  uint8_t data[MAX_APPLICATION_BUFFER_SIZE];
  int data_size;
  mqtt_qos_level_t qos = 0;
  char *str_data = (char *)data;

  int temp_value = get_temperature_reading();
  if(temp_value == CC26XX_SENSOR_READING_ERROR) {
    printf("APP - msg error");
    data_size = snprintf(str_data, MAX_APPLICATION_BUFFER_SIZE, "Sensor reading error");
    topic = pub_error_topic;
  } else {
    data_size = snprintf(str_data, MAX_APPLICATION_BUFFER_SIZE, "%d", temp_value);
    topic = pub_temp_topic;

    printf("APP - Trying to publish %s for %s with size %d\r\n", str_data, topic, data_size);
  }

  publish_message(topic, qos, data, data_size);

  printf("APP - Reprogramming the next message\r\n");
  ctimer_reset(&publish_msg_timer);
}
/*-----------------------------------------------------------------------------------*/
static void
mqtt_start_connection()
{
  printf("APP - Trying MQTT connection with broker ip[%s] port[%d]\r\n", broker_ip, broker_port);
  mqtt_status_t status = mqtt_connect(&conn, broker_ip, broker_port, MQTT_CLIENT_CONN_KEEP_ALIVE);
  if(status != MQTT_STATUS_OK) {
    printf("APP - Got error %i on connection\r\n", status);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
mqtt_event_handler(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED:
  {
    printf("APP - Application has a MQTT connection\r\n");
    printf("APP - Application will start periocally send messages\r\n");
    ctimer_stop(&mqtt_conn_timer);
    ctimer_set(&publish_msg_timer, MQTT_PERIODIC_PUB_TIME, publish_periodic_message, NULL);
    ctimer_set(&publish_counter_timer, MQTT_PERIODIC_PUB_TIME, publish_counter_message, NULL);
    break;
  }
  case MQTT_EVENT_DISCONNECTED:
  {
    printf("APP - MQTT Disconnect. Reason %u\r\n", *((mqtt_event_t *)data));
    ctimer_stop(&publish_msg_timer);
    ctimer_stop(&publish_counter_timer);
    ctimer_set(&mqtt_conn_timer, MQTT_CLIENT_CONN_RECONNECT, mqtt_start_connection, NULL);
    break;
  }
  case MQTT_EVENT_PUBLISH:
  {
    struct mqtt_message *msg = (struct mqtt_message *)data;
    printf("APP - Application received a publish on topic '%s'. Payload " "size is %i bytes. Content: %s\r\n", msg->topic, msg->payload_length, msg->payload_chunk);
    break;
  }
  case MQTT_EVENT_SUBACK:
  {
    printf("APP - Application is subscribed to topic successfully\r\n");
    break;
  }
  case MQTT_EVENT_UNSUBACK:
  {
    printf("APP - Application is unsubscribed to topic successfully\r\n");
    break;
  }
  case MQTT_EVENT_PUBACK:
  {
    printf("APP - Publishing complete.\r\n");
    break;
  }
  default:
    printf("APP - Application got a unhandled MQTT event: 0x%02x\r\n", event);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
unsigned int
uip_ipaddr_to_string(uip_ipaddr_t *addr, char *buf)
{
  if(addr == NULL) {
    return 0;
  }
  printf("APP - IP to string");
  uip_debug_ipaddr_print(addr); /* TODO: */
  printf("\r\n");
  return sprintf(buf, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3],
                 ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7],
                 ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11],
                 ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15]);
}
/*-----------------------------------------------------------------------------------*/
static inline int
update_broker_address(uip_ipaddr_t *addr, int port)
{
  if(uip_ipaddr_to_string(addr, broker_ip) > 0) {
    broker_port = port;
    return true;
  }
  return false;
}
/*-----------------------------------------------------------------------------------*/
static bool
resolv_hostname(const char *url, uip_ipaddr_t *addr)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;
  char host[MAX_HOSTLEN];
  char path[MAX_PATHLEN];
  uint16_t port;
  int ret;

  if(parse_url(url, host, &port, path)) {

    printf("url %s host %s port %d path %s\r\n",
           url, host, port, path);

    if(uiplib_ip6addrconv(host, &ip6addr) == 0) {
      /* First check if the host is an IP address. */
      if(uiplib_ip4addrconv(host, &ip4addr) != 0) {
        ip64_addr_4to6(&ip4addr, &ip6addr);
      } else {
        /* Try to lookup the hostname. If it fails, we initiate a hostname
           lookup. */
        ret = resolv_lookup(host, &addr);
        if(ret == RESOLV_STATUS_UNCACHED ||
           ret == RESOLV_STATUS_EXPIRED) {
          resolv_query(host);
          puts("Resolving host...");
          return false;
        }
        if(addr != NULL) {
          return update_broker_address(addr, port);
        } else {
          return false;
        }
      }
    }
    return update_broker_address(addr, port);
  } else {
    return false;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
mqtt_config_setup()
{
  snprintf(mqtt_username, MIN_APPLICATION_BUFFER_SIZE, "%s", MQTT_USERNAME);
  snprintf(mqtt_password, MIN_APPLICATION_BUFFER_SIZE, "%s", MQTT_PASSWORD);
  printf("APP - username[%s] password [%s]\r\n", mqtt_username, mqtt_password);
}
/*-----------------------------------------------------------------------------------*/
static bool
mqtt_connection_is_available(struct mqtt_connection *conn)
{
  return (!mqtt_connected(conn))
         && (conn->state != MQTT_CONN_STATE_CONNECTING_TO_BROKER)
         && (conn->state != MQTT_CONN_STATE_DISCONNECTING);
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_pub_process, ev, data)
{
  static uip_ip4addr_t ip4addr;
  static uip_ip6addr_t ip6addr;
  static uip_ipaddr_t hostaddr;
  static struct etimer et;
  static bool dns_resolved = false;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(batmon_sensor);

  uip_ipaddr(&ip4addr, 8, 8, 8, 8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  etimer_set(&et, CLOCK_SECOND * 60);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  mqtt_config_setup();

  mqtt_status_t status = mqtt_register(&conn, &mqtt_pub_process, client_id,
                                       mqtt_event_handler,
                                       MQTT_CLIENT_MAX_SEGMENT_SIZE);

  assert(status == MQTT_STATUS_OK);

  mqtt_set_username_password(&conn, mqtt_username, mqtt_password);

  create_mqtt_topic(pub_temp_topic, "sensor/temperature");
  create_mqtt_topic(pub_error_topic, "error");
  create_mqtt_topic(pub_self_topic, "status/self");

  while(1) {
    if(!dns_resolved) {
      dns_resolved = resolv_hostname(MQTT_BROKER_URL, &hostaddr);
      if(ev == resolv_event_found) {
        printf("APP - DNS found!\r\n");
        if(dns_resolved) {
          printf("APP - DNS resolved!\r\n");
          if(mqtt_connection_is_available(&conn)) {
            mqtt_start_connection();
          }
        }
      }
    }
    etimer_set(&et, CLOCK_SECOND * 30);
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
