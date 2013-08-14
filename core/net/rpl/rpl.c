/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         ContikiRPL, an implementation of RPL: IPv6 Routing Protocol
 *         for Low-Power and Lossy Networks (IETF RFC 6550)
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/uip.h"
#include "net/tcpip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl-private.h"
#include "net/neighbor-info.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include <limits.h>
#include <string.h>

#if RPL_CONF_STATS
rpl_stats_t rpl_stats;
#endif

/************************************************************************/
extern uip_ds6_route_t uip_ds6_routing_table[UIP_DS6_ROUTE_NB];
/************************************************************************/
void
rpl_purge_routes(void)
{
  int i;

  for(i = 0; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].isused) {
      if(uip_ds6_routing_table[i].state.lifetime <= 1) {
        uip_ds6_route_rm(&uip_ds6_routing_table[i]);
      } else {
        uip_ds6_routing_table[i].state.lifetime--;
      }
    }
  }
}
/************************************************************************/
void
rpl_remove_routes(rpl_dag_t *dag)
{
  int i;

  for(i = 0; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].state.dag == dag) {
      uip_ds6_route_rm(&uip_ds6_routing_table[i]);
    }
  }
}
/************************************************************************/
void
rpl_remove_routes_by_nexthop(uip_ipaddr_t *nexthop, rpl_dag_t *dag)
{
  uip_ds6_route_t *locroute;

  for(locroute = uip_ds6_routing_table;
      locroute < uip_ds6_routing_table + UIP_DS6_ROUTE_NB;
      locroute++) {
    if(locroute->isused
        && uip_ipaddr_cmp(&locroute->nexthop, nexthop)
        && locroute->state.dag == dag) {
      locroute->isused = 0;
    }
  }
  ANNOTATE("#L %u 0\n",nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
}
/************************************************************************/
uip_ds6_route_t *
rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix, int prefix_len,
              uip_ipaddr_t *next_hop)
{
  uip_ds6_route_t *rep;

  rep = uip_ds6_route_lookup(prefix);
  if(rep == NULL) {
    if((rep = uip_ds6_route_add(prefix, prefix_len, next_hop, 0)) == NULL) {
      PRINTF("RPL: No space for more route entries\n");
      return NULL;
    }
  } else {
    PRINTF("RPL: Updated the next hop for prefix ");
    PRINT6ADDR(prefix);
    PRINTF(" to ");
    PRINT6ADDR(next_hop);
    PRINTF("\n");
    uip_ipaddr_copy(&rep->nexthop, next_hop);
  }
  rep->state.dag = dag;
  rep->state.lifetime = RPL_LIFETIME(dag->instance, dag->instance->default_lifetime);
  rep->state.learned_from = RPL_ROUTE_FROM_INTERNAL;

  PRINTF("RPL: Added a route to ");
  PRINT6ADDR(prefix);
  PRINTF("/%d via ", prefix_len);
  PRINT6ADDR(next_hop);
  PRINTF("\n");

  return rep;
}
/************************************************************************/
static void
rpl_link_neighbor_callback(const rimeaddr_t *addr, int known, int etx)
{
  uip_ipaddr_t ipaddr;
  rpl_parent_t *parent;
  rpl_instance_t *instance;
  rpl_instance_t *end;

  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, (uip_lladdr_t *)addr);
  PRINTF("RPL: Neighbor ");
  PRINT6ADDR(&ipaddr);
  PRINTF(" is %sknown. ETX = %u\n", known ? "" : "no longer ", NEIGHBOR_INFO_FIX2ETX(etx));

  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance) {
    if(instance->used == 1 ) {
      parent = rpl_find_parent_any_dag(instance, &ipaddr);
      if(parent != NULL) {
        /* Trigger DAG rank recalculation. */
        parent->updated = 1;
        parent->link_metric = etx;

        if(instance->of->parent_state_callback != NULL) {
          instance->of->parent_state_callback(parent, known, etx);
        }
        if(!known) {
          PRINTF("RPL: Removing parent ");
          PRINT6ADDR(&parent->addr);
          PRINTF(" in instance %u because of bad connectivity (ETX %d)\n", instance->instance_id, etx);
          parent->rank = INFINITE_RANK;
        }
      }
    }
  }

  if(!known) {
    PRINTF("RPL: Deleting routes installed by DAOs received from ");
    PRINT6ADDR(&ipaddr);
    PRINTF("\n");
    uip_ds6_route_rm_by_nexthop(&ipaddr);
  }
}
/************************************************************************/
void
rpl_ipv6_neighbor_callback(uip_ds6_nbr_t *nbr)
{
  rpl_parent_t *p;
  rpl_instance_t *instance;
  rpl_instance_t *end;

  if(!nbr->isused) {
    PRINTF("RPL: Removing neighbor ");
    PRINT6ADDR(&nbr->ipaddr);
    PRINTF("\n");
    for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance) {
      if(instance->used == 1 ) {
        p = rpl_find_parent_any_dag(instance, &nbr->ipaddr);
        if(p != NULL) {
          p->rank = INFINITE_RANK;
          /* Trigger DAG rank recalculation. */
          p->updated = 1;
        }
      }
    }
  }
}
/************************************************************************/
void
rpl_init(void)
{
  uip_ipaddr_t rplmaddr;
  PRINTF("RPL started\n");
  default_instance = NULL;

  rpl_reset_periodic_timer();
  neighbor_info_subscribe(rpl_link_neighbor_callback);

  /* add rpl multicast address */
  uip_create_linklocal_rplnodes_mcast(&rplmaddr);
  uip_ds6_maddr_add(&rplmaddr);

#if RPL_CONF_STATS
  memset(&rpl_stats, 0, sizeof(rpl_stats));
#endif
}
/************************************************************************/
