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
 *
 * $Id: rpl.c,v 1.2 2010/05/18 16:43:56 nvt-se Exp $
 */
/**
 * \file
 *         ContikiRPL, an implementation of IETF ROLL RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/uip.h"
#include "net/tcpip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "net/neighbor-info.h"

#define DEBUG DEBUG_ANNOTATE
#include "net/uip-debug.h"

#include <limits.h>

/************************************************************************/
#ifndef RPL_CONF_MAX_ROUTE_ENTRIES
#define RPL_MAX_ROUTE_ENTRIES   10
#else
#define RPL_MAX_ROUTE_ENTRIES   RPL_CONF_MAX_ROUTE_ENTRIES
#endif /* !RPL_CONF_MAX_ROUTE_ENTRIES */

/************************************************************************/
extern uip_ds6_route_t uip_ds6_routing_table[UIP_DS6_ROUTE_NB];

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
uip_ds6_route_t *
rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix, int prefix_len,
              uip_ipaddr_t *next_hop)
{
  uip_ds6_route_t *rep;

  if((rep = uip_ds6_route_lookup(prefix)) == NULL) {

    if((rep = uip_ds6_route_add(prefix, prefix_len, next_hop, 0)) == NULL) {
      PRINTF("RPL: No space for more route entries\n");
      return NULL;
    }

    rep->state.dag = dag;
    rep->state.lifetime = DEFAULT_ROUTE_LIFETIME;
    rep->state.learned_from = RPL_ROUTE_FROM_INTERNAL;

    PRINTF("RPL: Added a route to ");
    PRINT6ADDR(prefix);
    PRINTF("/%d via ", prefix_len);
    PRINT6ADDR(next_hop);
    PRINTF("\n");
  }
  return rep;
}
/************************************************************************/
static void
neighbor_callback(const rimeaddr_t *addr, int known, int etx)
{
  uip_ipaddr_t ipaddr;
  rpl_dag_t *dag;
  rpl_neighbor_t *parent;
  uip_ds6_route_t *rep;

  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, (uip_lladdr_t *)addr);
  PRINTF("RPL: Neighbor ");
  PRINT6ADDR(&ipaddr);
  PRINTF(" is %sknown. ETX = %d\n", known ? "" : "no longer ", etx);

  dag = rpl_get_dag(RPL_DEFAULT_INSTANCE);
  if(dag == NULL) {
    return;
  }

  parent = rpl_find_neighbor(dag, &ipaddr);
  if(parent == NULL) {
    rep = uip_ds6_route_lookup(&ipaddr);
    if(rep != NULL) {
      if(!known) {
        rep->state.lifetime = DAO_EXPIRATION_TIMEOUT;
        rep->state.saved_lifetime = rep->state.lifetime;
      } else {
        rep->state.lifetime = rep->state.saved_lifetime;
      }
    }
  } else if(!known) {
    PRINTF("RPL: Removing parent ");
    PRINT6ADDR(&parent->addr);
    PRINTF(" because of bad connectivity (ETX %d)\n", etx);
    rpl_remove_neighbor(dag, parent);
    if(RPL_PARENT_COUNT(dag) == 0) {
      rpl_free_dag(dag);
    } else {
      /* Select a new default route. */
      parent = rpl_find_best_parent(dag);
      if(parent != NULL) {
        rpl_set_default_route(dag, &parent->addr);
      }
    }
  } else {
    parent->local_confidence = ~0 - etx;
    PRINTF("RPL: Updating the local confidence value for this neighbor to %d\n",
           parent->local_confidence);
    if(parent != dag->best_parent &&
       dag->of->best_parent(parent, dag->best_parent) == parent) {
      dag->best_parent = parent;
      rpl_set_default_route(dag, &parent->addr);
    }
  }
}
/************************************************************************/
void
rpl_init(void)
{
  PRINTF("RPL started\n");

  rpl_reset_periodic_timer();
  neighbor_info_subscribe(neighbor_callback);
}
/************************************************************************/
