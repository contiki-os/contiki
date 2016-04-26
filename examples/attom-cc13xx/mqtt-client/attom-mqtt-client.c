#include "contiki-net.h"
#include "mqtt.h"
#include "uip-debug.h"
#include "ip64-addr.h"
#include "lib/sensors.h"
#include "batmon-sensor.h"
#include "debug.h"

/*-----------------------------------------------------------------------------------*/
#define DEBUG DEBUG_PRINT
/*-----------------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_process);
PROCESS(mqtt_pub_process, "MQTT Periodic Publisher");
AUTOSTART_PROCESSES(&mqtt_process, &mqtt_pub_process);
/*-----------------------------------------------------------------------------------*/
#define ATTO_HARDWARE_ID                     150072
#define ATTO_LOGVIEW_URL                     "mqtt.logview.com.br"
#define ATTO_API_VERSION                     1 
#define MQTT_PERIODIC_PUB_TIME               (CLOCK_SECOND * 5)
#define MQTT_CLIENT_CONN_KEEP_ALIVE          (CLOCK_SECOND * 20)
#define MQTT_CLIENT_CONN_RECONNECT           (CLOCK_SECOND * 10)
#define MQTT_CLIENT_MAX_SEGMENT_SIZE         32
#define MAX_APPLICATION_BUFFER_SIZE          64
#define MAX_PATHLEN                          64
#define MAX_HOSTLEN                          64
/*---------------------------------------------------------------------------*/
static int broker_port = 1883;
static char *client_id = "contiki";
static struct mqtt_connection conn;
static struct ctimer publish_msg_timer;
static struct ctimer mqtt_conn_timer;
static char broker_ip[MAX_APPLICATION_BUFFER_SIZE];
static char publish_topic[MAX_APPLICATION_BUFFER_SIZE];
static char subscribe_topic[MAX_APPLICATION_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static unsigned int
create_mqtt_topic(char * out, const char* resource)
{
  unsigned int len = sprintf(out, "v%d/%d/%s", ATTO_API_VERSION, ATTO_HARDWARE_ID, resource);
  printf("APP - create topic [%s] with len[%d]\n", out, len);
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
    printf("null url\n");
    return 0;
  }

  /* Don't even try to go further if the URL is empty. */
  if(strlen(url) == 0) {
    printf("empty url\n");
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
static void publish_message()
{
  char* topic;
  char* str_data;
  uint8_t data[MAX_APPLICATION_BUFFER_SIZE];
  int data_size;

  int temp_value = get_temperature_reading();
  if (temp_value == CC26XX_SENSOR_READING_ERROR) {
    printf("APP - msg error");
  }

  str_data = (char*) data;
  data_size = sprintf(str_data, "%d.00", temp_value);
  topic = publish_topic;

  uint16_t mid = 0;
  mqtt_qos_level_t qos = 0;

  printf("APP - Trying to publish %s for %s with size %d\n", str_data, topic, data_size);

  if(mqtt_ready(&conn)) {
    mqtt_status_t status = mqtt_publish(&conn, &mid, topic, data, data_size, qos, MQTT_RETAIN_OFF);
    if (status != MQTT_STATUS_OK) {
      printf("APP - Error[%d] on publish\n", status);
    }
  }

  printf("APP - Reprogramming the next message\n");
  ctimer_reset(&publish_msg_timer);
}
/*-----------------------------------------------------------------------------------*/
static void mqtt_start_connection()
{
  printf("APP - Trying MQTT connection with broker ip[%s] port[%d]\n", broker_ip, broker_port);
  mqtt_status_t status = mqtt_connect(&conn, broker_ip, broker_port, MQTT_CLIENT_CONN_KEEP_ALIVE);
  if (status != MQTT_STATUS_OK) {
    printf("APP - Got error %i on connection\n", status);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
mqtt_event_handler(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
    case MQTT_EVENT_CONNECTED:
      {
        printf("APP - Application has a MQTT connection\n");
        printf("APP - Application will start periocally send messages\n");
        ctimer_stop(&mqtt_conn_timer);
        ctimer_set(&publish_msg_timer, MQTT_PERIODIC_PUB_TIME, publish_message, NULL);
        break;
      }
    case MQTT_EVENT_DISCONNECTED:
      {
        printf("APP - MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));
        ctimer_stop(&publish_msg_timer);
        ctimer_set(&mqtt_conn_timer, MQTT_CLIENT_CONN_RECONNECT, mqtt_start_connection, NULL);
        break;
      }
    case MQTT_EVENT_PUBLISH:
      {
        struct mqtt_message * msg = (struct mqtt_message *) data;
        printf("APP - Application received a publish on topic '%s'. Payload " "size is %i bytes. Content:\n\n", msg->topic, msg->payload_length);
        break;
      }
    case MQTT_EVENT_SUBACK:
      {
        printf("APP - Application is subscribed to topic successfully\n");
        break;
      }
    case MQTT_EVENT_UNSUBACK:
      {
        printf("APP - Application is unsubscribed to topic successfully\n");
        break;
      }
    case MQTT_EVENT_PUBACK:
      {
        printf("APP - Publishing complete.\n");
        break;
      }
    default:
      printf("APP - Application got a unhandled MQTT event: %i\n", event);
      break;
  }
}
/*-----------------------------------------------------------------------------------*/
unsigned int uip_ipaddr_to_string(uip_ipaddr_t * addr, char *buf)
{
  printf("APP - IP to string");
  uip_debug_ipaddr_print(addr);
  printf("\n");
  return sprintf(buf, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3],
                ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7],
                ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11],
                ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15]);
  }
/*-----------------------------------------------------------------------------------*/
static inline void
update_broker_address(uip_ipaddr_t * addr, int port)
{
  uip_ipaddr_to_string(addr, broker_ip);
  broker_port = port;
}
/*-----------------------------------------------------------------------------------*/
static bool
resolv_hostname(const char* url, uip_ipaddr_t * addr)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;
  char host[MAX_HOSTLEN];
  char path[MAX_PATHLEN];
  uint16_t port;
  int ret;

  if(parse_url(url, host, &port, path)) {

    printf("url %s host %s port %d path %s\n",
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
          update_broker_address(addr, port);
          return true;
        } else {
          return false;
        }
      }
    }
    update_broker_address(addr, port);
    return true;
  } else {
    return false;
  }
}
/*-----------------------------------------------------------------------------------*/
static bool
mqtt_connection_is_available(struct mqtt_connection * conn)
{
  return ((!mqtt_connected(conn))
          && (conn->state != MQTT_CONN_STATE_CONNECTING_TO_BROKER)
          && (conn->state != MQTT_CONN_STATE_DISCONNECTING));
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

  uip_ipaddr(&ip4addr, 8,8,8,8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  etimer_set(&et, CLOCK_SECOND * 60);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  mqtt_status_t status = mqtt_register(&conn, &mqtt_pub_process, client_id,
      mqtt_event_handler,
      MQTT_CLIENT_MAX_SEGMENT_SIZE);

  if (status != MQTT_STATUS_OK) {
    printf("APP - Can't initializate MQTT, dying...\n");
    PROCESS_EXIT();
  }

  create_mqtt_topic(publish_topic, "temperature");
  create_mqtt_topic(subscribe_topic, "command");

  while(1) {
    dns_resolved = resolv_hostname(ATTO_LOGVIEW_URL, &hostaddr);
    etimer_set(&et, CLOCK_SECOND * 30);
    PROCESS_WAIT_EVENT();
    if (ev == resolv_event_found) {
      printf("APP - DNS found!\n");
      if (dns_resolved) {
        printf("APP - DNS resolved!\n");
        if (mqtt_connection_is_available(&conn)) {
          mqtt_start_connection();
        }
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
