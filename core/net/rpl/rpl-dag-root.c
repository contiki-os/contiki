/*
 * Copyright (c) 2012-2014, Thingsquare, http://www.thingsquare.com/.
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
 *
 */

#include "contiki.h"
#include "contiki-net.h"

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/rpl/rpl-dag-root.h"

#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#define RPL_DAG_GRACE_PERIOD (CLOCK_SECOND * 20 * 1)

static struct uip_ds6_notification n;
static uint8_t to_become_root;
static struct ctimer c;
/*---------------------------------------------------------------------------*/
static const uip_ipaddr_t *
dag_root(void)
{
  rpl_dag_t *dag;

  dag = rpl_get_any_dag();
  if(dag != NULL) {
    return &dag->dag_id;
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
static const uip_ipaddr_t *
get_global_address(void)
{
  int i;
  uint8_t state;
  uip_ipaddr_t *ipaddr = NULL;

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       state == ADDR_PREFERRED &&
       !uip_is_addr_linklocal(&uip_ds6_if.addr_list[i].ipaddr)) {
      ipaddr = &uip_ds6_if.addr_list[i].ipaddr;
    }
  }
  return ipaddr;
}
/*---------------------------------------------------------------------------*/
static void
create_dag_callback(void *ptr)
{
  const uip_ipaddr_t *root, *ipaddr;

  root = dag_root();
  ipaddr = get_global_address();

  if(root == NULL || uip_ipaddr_cmp(root, ipaddr)) {
    /* The RPL network we are joining is one that we created, so we
       become root. */
    if(to_become_root) {
      rpl_dag_root_init_dag_immediately();
      to_become_root = 0;
    }
  } else {
    rpl_dag_t *dag;

    dag = rpl_get_any_dag();
#if DEBUG
    printf("Found a network we did not create\n");
    printf("version %d grounded %d preference %d used %d joined %d rank %d\n",
           dag->version, dag->grounded,
           dag->preference, dag->used,
           dag->joined, dag->rank);
#endif /* DEBUG */

    /* We found a RPL network that we did not create so we just join
       it without becoming root. But if the network has an infinite
       rank, we assume the network has broken, and we become the new
       root of the network. */

    if(dag->rank == INFINITE_RANK) {
      if(to_become_root) {
        rpl_dag_root_init_dag_immediately();
        to_become_root = 0;
      }
    }

    /* Try again after the grace period */
    ctimer_set(&c, RPL_DAG_GRACE_PERIOD, create_dag_callback, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static void
route_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr,
               int numroutes)
{
  if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    if(route != NULL && ipaddr != NULL &&
       !uip_is_addr_unspecified(route) &&
       !uip_is_addr_unspecified(ipaddr)) {
      if(to_become_root) {
        ctimer_set(&c, 0, create_dag_callback, NULL);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  /* Assign a unique local address (RFC4193,
     http://tools.ietf.org/html/rfc4193). */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
void
rpl_dag_root_init(void)
{
  static uint8_t initialized = 0;

  if(!initialized) {
    to_become_root = 0;
    set_global_address();
    uip_ds6_notification_add(&n, route_callback);
    initialized = 1;
  }
}
/*---------------------------------------------------------------------------*/
int
rpl_dag_root_init_dag_immediately(void)
{
  struct uip_ds6_addr *root_if;
  int i;
  uint8_t state;
  uip_ipaddr_t *ipaddr = NULL;

  rpl_dag_root_init();

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       state == ADDR_PREFERRED &&
       !uip_is_addr_linklocal(&uip_ds6_if.addr_list[i].ipaddr)) {
      ipaddr = &uip_ds6_if.addr_list[i].ipaddr;
    }
  }

  if(ipaddr != NULL) {
    root_if = uip_ds6_addr_lookup(ipaddr);
    if(root_if != NULL) {
      rpl_dag_t *dag;
      uip_ipaddr_t prefix;

      rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
      dag = rpl_get_any_dag();

      /* If there are routes in this dag, we remove them all as we are
         from now on the new dag root and the old routes are wrong */
      rpl_remove_routes(dag);
      if(dag->instance != NULL &&
         dag->instance->def_route != NULL) {
	uip_ds6_defrt_rm(dag->instance->def_route);
        dag->instance->def_route = NULL;
      }

      uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
      rpl_set_prefix(dag, &prefix, 64);
      PRINTF("rpl_dag_root_init_dag: created a new RPL dag\n");
      return 0;
    } else {
      PRINTF("rpl_dag_root_init_dag: failed to create a new RPL DAG\n");
      return -1;
    }
  } else {
    PRINTF("rpl_dag_root_init_dag: failed to create a new RPL DAG, no preferred IP address found\n");
    return -2;
  }
}
/*---------------------------------------------------------------------------*/
void
rpl_dag_root_init_dag(void)
{
  rpl_dag_root_init();

  ctimer_set(&c, RPL_DAG_GRACE_PERIOD, create_dag_callback, NULL);
  to_become_root = 1;

  /* Send a DIS packet to request RPL info from neighbors. */
  dis_output(NULL);
}
/*---------------------------------------------------------------------------*/
int
rpl_dag_root_is_root(void)
{
  rpl_instance_t *instance;

  instance = rpl_get_default_instance();

  if(instance == NULL) {
    return 0;
  }

  if(instance->current_dag &&
     instance->current_dag->rank == ROOT_RANK(instance)) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
