#include "contiki.h"
#include "contiki-net.h"
#include "ip64.h"
#include "net/netstack.h"
#include "net/rpl/rpl-dag-root.h"
#include "net/rpl/rpl.h"
#include "net/ipv6/uip-ds6-route.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(router_node_process, "Router node");
AUTOSTART_PROCESSES(&router_node_process);
/*---------------------------------------------------------------------------*/
uip_lladdr_t * uip_ds6_route_nexthop_lladdr(uip_ds6_route_t *route);


PROCESS_THREAD(router_node_process, ev, data)
{
  uip_ipaddr_t *nexthop = NULL;
  uip_ds6_defrt_t *defrt;
  uip_ipaddr_t *ipaddr;
  uip_ds6_route_t *r;
  static struct etimer et;
  PROCESS_BEGIN();

  /* Set us up as a RPL root node. */
  rpl_dag_root_init_dag();

  /* Initialize the IP64 module so we'll start translating packets */
  ip64_init();

  /* etimer_set(&et, CLOCK_SECOND * 60); */
  /* PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); */

  /* ... and do nothing more. */
  while(1) {
    etimer_set(&et, CLOCK_SECOND * 20);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    defrt = NULL;
    if((ipaddr = uip_ds6_defrt_choose()) != NULL) {
      defrt = uip_ds6_defrt_lookup(ipaddr);
    }
    if(defrt != NULL) {
      printf("DefRT: :: -> %02d", defrt->ipaddr.u8[15]);
      printf(" lt:%lu inf:%d\n", stimer_remaining(&defrt->lifetime),
    	     defrt->isinfinite);
    } else {
      printf("DefRT: :: -> NULL\n");
    }

    if(uip_ds6_route_head() != NULL) {
      printf("found head\n");
      for(r = uip_ds6_route_head();
    	  r != NULL;
    	  r = uip_ds6_route_next(r)) {
    	nexthop = uip_ds6_route_nexthop(r);
    	if(nexthop != NULL) {
	  printf("Route: %02d -> %02d", r->ipaddr.u8[15], nexthop->u8[15]);
    	} else {
    	  //printf("Route: %p %02d -> ? nbr-routes:%p", r, r->ipaddr.u8[15],
	  //r->neighbor_routes);
    	}
    	printf(" lt:%lu\n", r->state.lifetime);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
