#include "contiki-net.h"
#include "http-socket.h"
#include "ip64-addr.h"
#include "batmon-sensor.h"
#include "lib/sensors.h"

#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
#define ATTO_HARDWARE_ID 150072
#define ATTO_JSON_VERSION 2
#define MAX_PAYLOAD_LEN   64
#define HTTP_PERIODIC_POST_TIME (30 * CLOCK_SECOND)
#define ATTO_LOGVIEW_URL "http://io.logview.com.br/v1"
/*---------------------------------------------------------------------------*/
static struct http_socket s;
static int bytes_received = 0;
/*---------------------------------------------------------------------------*/
PROCESS(http_post_logview_process, "HTTP POST LogView Example");
AUTOSTART_PROCESSES(&http_post_logview_process);
/*---------------------------------------------------------------------------*/
static int last_temp_value = 0;
static void
get_batmon_reading(void)
{
  int value;
  value = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
  if(value == CC26XX_SENSOR_READING_ERROR) {
    printf("Temperature Read Error\r\n");
  }
  last_temp_value = value;
}
/*---------------------------------------------------------------------------*/
static unsigned int postdatalen; 
static char postdata[MAX_PAYLOAD_LEN];
static void
update_post_data(void)
{
  static int seq = 0;
  int value = last_temp_value;

  memset(postdata, 0, MAX_PAYLOAD_LEN);
  postdatalen = sprintf(postdata, "{\"v\":\"%d\", \"h\":\"%d\", \"n\":\"%02d\", \"s\":\"2886\", \"temp\":\"%d.00\"}",
      ATTO_JSON_VERSION, ATTO_HARDWARE_ID, seq, value);
  seq = (seq + 1) % 100;
}
/*---------------------------------------------------------------------------*/
static void
http_callback(struct http_socket *s, void *ptr,
         http_socket_event_t e,
         const uint8_t *data, uint16_t datalen)
{
  if(e == HTTP_SOCKET_ERR) {
    printf("HTTP socket error\n");
  } else if(e == HTTP_SOCKET_TIMEDOUT) {
    printf("HTTP socket error: timed out\n");
  } else if(e == HTTP_SOCKET_ABORTED) {
    printf("HTTP socket error: aborted\n");
  } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
    printf("HTTP socket error: hostname not found\n");
  } else if(e == HTTP_SOCKET_CLOSED) {
    printf("HTTP socket closed, %d bytes received\n", bytes_received);
  } else if(e == HTTP_SOCKET_DATA) {
    bytes_received += datalen;
    printf("HTTP socket received %d bytes of data\n", datalen);
  } else if(e == HTTP_SOCKET_OK) {
    printf("HTTP socket OK\n");
  } else {
    printf("Other HTTP event: %i\n", e);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(http_post_logview_process, ev, data)
{
  static struct etimer et;
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(batmon_sensor);

  uip_ipaddr(&ip4addr, 8,8,8,8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  etimer_set(&et, CLOCK_SECOND * 60);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  while(1) {
    http_socket_init(&s);
    get_batmon_reading();
    update_post_data();
    http_socket_post(&s, ATTO_LOGVIEW_URL, postdata,
                     postdatalen, "application/json",
                     http_callback, NULL);
    http_socket_close(&s);
    etimer_set(&et, HTTP_PERIODIC_POST_TIME);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
