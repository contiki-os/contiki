#include "contiki.h"

#include "websocket.h"

#include <stdio.h>

static struct websocket s;

static void callback(struct websocket *s, websocket_result_t r,
                     const uint8_t *data, uint16_t datalen);

#define RECONNECT_INTERVAL 10 * CLOCK_SECOND
static struct ctimer reconnect_timer;

/*---------------------------------------------------------------------------*/
PROCESS(websocket_example_process, "Websocket Example");
AUTOSTART_PROCESSES(&websocket_example_process);
/*---------------------------------------------------------------------------*/
static void
reconnect_callback(void *ptr)
{
  websocket_open(&s, "ws://172.16.0.1:8080/",
                 "contiki", NULL, callback);
}
/*---------------------------------------------------------------------------*/
static void
callback(struct websocket *s, websocket_result_t r,
         const uint8_t *data, uint16_t datalen)
{
  if(r == WEBSOCKET_CLOSED ||
     r == WEBSOCKET_RESET ||
     r == WEBSOCKET_HOSTNAME_NOT_FOUND ||
     r == WEBSOCKET_TIMEDOUT) {
    ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, s);
  } else if(r == WEBSOCKET_CONNECTED) {
    websocket_send_str(s, "Connected");
  } else if(r == WEBSOCKET_DATA) {
    printf("websocket-example: Received data '%.*s' (len %d)\n", datalen,
           data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(websocket_example_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, &s);

  websocket_init(&s);
  while(1) {
    etimer_set(&et, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    char buf[] = "012345678";
    static int count;
    buf[0] = (count % 9) + '0';
    count++;
    websocket_send_str(&s, buf);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
