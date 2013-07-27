#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdio.h>

#define UDP_PORT 61618

#define SEND_INTERVAL		(4 * CLOCK_SECOND)

static struct simple_udp_connection broadcast_connection;

#ifndef SIZE
#define SIZE 100
#endif

/*---------------------------------------------------------------------------*/
PROCESS(udp_process, "UDP broadcast process");
AUTOSTART_PROCESSES(&udp_process);
/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  simple_udp_register(&broadcast_connection, UDP_PORT,
                      NULL, UDP_PORT,
                      receiver);

  etimer_set(&periodic_timer, 20 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    uint8_t buf[SIZE];
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    printf("Sending broadcast\n");
    uip_create_linklocal_allnodes_mcast(&addr);
    simple_udp_sendto(&broadcast_connection, buf, sizeof(buf), &addr);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
