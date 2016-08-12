#include "contiki-net.h"
#include "http-socket.h"
#include "ip64-addr.h"
#include "dev/leds.h"
#include "rpl.h"

#include <stdio.h>

static struct http_socket s;
static int bytes_received = 0;
static int restarts;
static struct ctimer reconnect_timer;
static int connect = 0;

static void callback(struct http_socket *s, void *ptr,
                     http_socket_event_t e,
                     const uint8_t *data, uint16_t datalen);

/*---------------------------------------------------------------------------*/
PROCESS(http_example_process, "HTTP Example");
AUTOSTART_PROCESSES(&http_example_process);
/*---------------------------------------------------------------------------*/
static void
reconnect(void *dummy)
{
  printf("#A color=orange\n");
  rpl_set_mode(RPL_MODE_MESH);
  connect = 1;
}
/*---------------------------------------------------------------------------*/
static void
restart(void)
{
  int scale;
  restarts++;
  printf("restart %d\n", restarts);
  rpl_set_mode(RPL_MODE_FEATHER);
  printf("#A color=red\n");

  scale = restarts;
  if(scale > 5) {
    scale = 5;
  }
  ctimer_set(&reconnect_timer, random_rand() % ((CLOCK_SECOND * 10) << scale),
              reconnect, NULL);
}
/*---------------------------------------------------------------------------*/
static void
callback(struct http_socket *s, void *ptr,
         http_socket_event_t e,
         const uint8_t *data, uint16_t datalen)
{
  if(e == HTTP_SOCKET_ERR) {
    printf("HTTP socket error\n");
  } else if(e == HTTP_SOCKET_TIMEDOUT) {
    printf("HTTP socket error: timed out\n");
    restart();
  } else if(e == HTTP_SOCKET_ABORTED) {
    printf("HTTP socket error: aborted\n");
    restart();
  } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
    printf("HTTP socket error: hostname not found\n");
    restart();
  } else if(e == HTTP_SOCKET_CLOSED) {
    if(bytes_received > 0) {
      printf("HTTP socket closed, %d bytes received\n", bytes_received);
      leds_off(LEDS_RED);
      printf("#A color=blue\n");
      rpl_set_mode(RPL_MODE_FEATHER);
    } else {
      restart();
    }
  } else if(e == HTTP_SOCKET_DATA) {
    bytes_received += datalen;
    printf("HTTP socket received %d bytes of data\n", datalen);
  }
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(http_example_process, ev, data)
{
  static struct etimer et;
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  uip_ipaddr(&ip4addr, 8,8,8,8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  etimer_set(&et, CLOCK_SECOND * 20);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  http_socket_init(&s);
  connect = 1;
  leds_on(LEDS_RED);
  restarts = 0;
  etimer_set(&et, CLOCK_SECOND * 5);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
    if(connect && rpl_has_downward_route()) {
      printf("#A color=green\n");
      http_socket_get(&s, "http://www.contiki-os.org/", 0, 0,
		      callback, NULL);
      connect = 0;
    } else if(connect) {
      connect++;
      /* If connect have been "tried" 5 timer we quit trying now... */
      if(connect > 5)  {
	restart();
	connect = 0;
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
