/*
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
/**
 * \file
 *         border-router
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "rich.h"

#include "net/netstack.h"
#include "dev/slip.h"

#include "rich.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

PROCESS(border_router_process, "Border router process");

AUTOSTART_PROCESSES(&border_router_process);
/*---------------------------------------------------------------------------*/
static void
print_network_status(void)
{
  int i;
  uint8_t state;
  uip_ds6_defrt_t *default_route;
  uip_ds6_route_t *route;

  PRINTA("--- Network status ---\n");

  /* Our IPv6 addresses */
  PRINTA("- Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA("-- ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }

  /* Our default route */
  PRINTA("- Default route:\n");
  default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
  if(default_route != NULL) {
    PRINTA("-- ");
    uip_debug_ipaddr_print(&default_route->ipaddr);;
    PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
  } else {
    PRINTA("-- None\n");
  }

  /* Our routing entries */
  PRINTA("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
  route = uip_ds6_route_head();
  while(route != NULL) {
    PRINTA("-- ");
    uip_debug_ipaddr_print(&route->ipaddr);
    PRINTA(" via ");
    uip_debug_ipaddr_print(uip_ds6_route_nexthop(route));
    PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
    route = uip_ds6_route_next(route);
  }

  PRINTA("----------------------\n");
}


/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
//  uip_buf[2] = '\n';
  uip_len = 2;
  slip_send();
  uip_len = 0;
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  memcpy(&prefix, prefix_64, 16);
  prefix_set = 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;

  PROCESS_PAUSE();

  PRINTF("RPL-Border router started\n");

  
  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    PRINTF("Waiting for prefix\n");
  }

  PRINTF("Obtained prefix: ");
  uip_debug_ipaddr_print(&prefix);
  PRINTF("\n");

  rich_init(&prefix);

  etimer_set(&et, CLOCK_SECOND * 60);
  while(1) {
    print_network_status();
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
