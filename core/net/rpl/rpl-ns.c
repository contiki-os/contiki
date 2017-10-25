/*
 * Copyright (c) 2016, Inria.
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
 *         RPL non-storing mode specific functions. Includes support for
 *         source routing.
 *
 * \author Simon Duquennoy <simon.duquennoy@inria.fr>
 */

#include "net/rpl/rpl-conf.h"

#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/rpl/rpl-private.h"
#include "net/rpl/rpl-ns.h"
#include "lib/list.h"
#include "lib/memb.h"

#if RPL_WITH_NON_STORING

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#include <limits.h>
#include <string.h>

/* Total number of nodes */
static int num_nodes;

/* Every known node in the network */
LIST(nodelist);
MEMB(nodememb, rpl_ns_node_t, RPL_NS_LINK_NUM);

/*---------------------------------------------------------------------------*/
int
rpl_ns_num_nodes(void)
{
  return num_nodes;
}
/*---------------------------------------------------------------------------*/
static int
node_matches_address(const rpl_dag_t *dag, const rpl_ns_node_t *node, const uip_ipaddr_t *addr)
{
  return addr != NULL
      && node != NULL
      && dag != NULL
      && dag == node->dag
      && !memcmp(addr, &node->dag->dag_id, 8)
      && !memcmp(((const unsigned char *)addr) + 8, node->link_identifier, 8);
}
/*---------------------------------------------------------------------------*/
rpl_ns_node_t *
rpl_ns_get_node(const rpl_dag_t *dag, const uip_ipaddr_t *addr)
{
  rpl_ns_node_t *l;
  for(l = list_head(nodelist); l != NULL; l = list_item_next(l)) {
    /* Compare prefix and node identifier */
    if(node_matches_address(dag, l, addr)) {
      return l;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
int
rpl_ns_is_node_reachable(const rpl_dag_t *dag, const uip_ipaddr_t *addr)
{
  int max_depth = RPL_NS_LINK_NUM;
  rpl_ns_node_t *node = rpl_ns_get_node(dag, addr);
  rpl_ns_node_t *root_node = rpl_ns_get_node(dag, dag != NULL ? &dag->dag_id : NULL);
  while(node != NULL && node != root_node && max_depth > 0) {
    node = node->parent;
    max_depth--;
  }
  return node != NULL && node == root_node;
}
/*---------------------------------------------------------------------------*/
void
rpl_ns_expire_parent(rpl_dag_t *dag, const uip_ipaddr_t *child, const uip_ipaddr_t *parent)
{
  rpl_ns_node_t *l = rpl_ns_get_node(dag, child);
  /* Check if parent matches */
  if(l != NULL && node_matches_address(dag, l->parent, parent)) {
    l->lifetime = RPL_NOPATH_REMOVAL_DELAY;
  }
}
/*---------------------------------------------------------------------------*/
rpl_ns_node_t *
rpl_ns_update_node(rpl_dag_t *dag, const uip_ipaddr_t *child, const uip_ipaddr_t *parent, uint32_t lifetime)
{
  rpl_ns_node_t *child_node = rpl_ns_get_node(dag, child);
  rpl_ns_node_t *parent_node = rpl_ns_get_node(dag, parent);
  rpl_ns_node_t *old_parent_node;

  if(parent != NULL) {
    /* No node for the parent, add one with infinite lifetime */
    if(parent_node == NULL) {
      parent_node = rpl_ns_update_node(dag, parent, NULL, 0xffffffff);
      if(parent_node == NULL) {
        return NULL;
      }
    }
  }

  /* No node for this child, add one */
  if(child_node == NULL) {
    child_node = memb_alloc(&nodememb);
    /* No space left, abort */
    if(child_node == NULL) {
      return NULL;
    }
    child_node->parent = NULL;
    list_add(nodelist, child_node);
    num_nodes++;
  }

  /* Initialize node */
  child_node->dag = dag;
  child_node->lifetime = lifetime;
  memcpy(child_node->link_identifier, ((const unsigned char *)child) + 8, 8);

  /* Is the node reachable before the update? */
  if(rpl_ns_is_node_reachable(dag, child)) {
    old_parent_node = child_node->parent;
    /* Update node */
    child_node->parent = parent_node;
    /* Has the node become unreachable? May happen if we create a loop. */
    if(!rpl_ns_is_node_reachable(dag, child)) {
      /* The new parent makes the node unreachable, restore old parent.
       * We will take the update next time, with chances we know more of
       * the topology and the loop is gone. */
      child_node->parent = old_parent_node;
    }
  } else {
    child_node->parent = parent_node;
  }

  return child_node;
}
/*---------------------------------------------------------------------------*/
void
rpl_ns_init(void)
{
  num_nodes = 0;
  memb_init(&nodememb);
  list_init(nodelist);
}
/*---------------------------------------------------------------------------*/
rpl_ns_node_t *
rpl_ns_node_head(void)
{
  return list_head(nodelist);
}
/*---------------------------------------------------------------------------*/
rpl_ns_node_t *
rpl_ns_node_next(rpl_ns_node_t *item)
{
  return list_item_next(item);
}
/*---------------------------------------------------------------------------*/
void
rpl_ns_get_node_global_addr(uip_ipaddr_t *addr, rpl_ns_node_t *node)
{
  if(addr != NULL && node != NULL && node->dag != NULL) {
    memcpy(addr, &node->dag->dag_id, 8);
    memcpy(((unsigned char *)addr) + 8, &node->link_identifier, 8);
  }
}
/*---------------------------------------------------------------------------*/
void
rpl_ns_periodic(void)
{
  rpl_ns_node_t *l;
  /* First pass, decrement lifetime for all nodes with non-infinite lifetime */
  for(l = list_head(nodelist); l != NULL; l = list_item_next(l)) {
    /* Don't touch infinite lifetime nodes */
    if(l->lifetime != 0xffffffff && l->lifetime > 0) {
      l->lifetime--;
    }
  }
  /* Second pass, for all expire nodes, deallocate them iff no child points to them */
  for(l = list_head(nodelist); l != NULL; l = list_item_next(l)) {
    if(l->lifetime == 0) {
      rpl_ns_node_t *l2;
      for(l2 = list_head(nodelist); l2 != NULL; l2 = list_item_next(l2)) {
        if(l2->parent == l) {
          break;
        }
      }
      /* No child found, deallocate node */
      list_remove(nodelist, l);
      memb_free(&nodememb, l);
      num_nodes--;
    }
  }
}

#endif /* RPL_WITH_NON_STORING */
