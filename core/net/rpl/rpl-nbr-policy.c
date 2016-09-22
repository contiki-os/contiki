/*
 * Copyright (c) 2014-2015, Yanzi Networks AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holders nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/**
 * \addtogroup uip6
 * @{
 */


/**
 * \file
 *
 * Default RPL NBR policy
 * decides when to add a new discovered node to the nbr table from RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>
 * Contributors: Niclas Finne <nfi@sics.se>, Oriol Piñol <oriol@yanzi.se>,
 *
 */

#include "net/rpl/rpl-private.h"
#include "net/nbr-table.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/*
 * Policy for neighbor adds
 * - one node is locked (default route)
 * - max X children (nexthops)
 * - max Y "best parents"
 * => at least MAX_NBRS - (Y + X + 1) free slots for other.
 *
 * NOTE: this policy assumes that all neighbors end up being IPv6
 * neighbors and are not only MAC neighbors.
 */
#define MAX_CHILDREN (NBR_TABLE_MAX_NEIGHBORS - 2)
#define UIP_IP_BUF       ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static int num_parents; /* any node that are possible parents */
static int num_children;  /* all children that we have as nexthop */
static int num_free;
static linkaddr_t *worst_rank_nbr; /* the parent that has the worst rank */
static rpl_rank_t worst_rank;
/*---------------------------------------------------------------------------*/
#if DEBUG == DEBUG_FULL
/*
 * This create a periodic call of the update_nbr function that will print
 * useful debugging information when in DEBUG_FULL mode
 */
static void update_nbr(void);
static struct ctimer periodic_timer;
static int timer_init = 0;
static void
handle_periodic_timer(void *ptr)
{
  update_nbr();
  ctimer_restart(&periodic_timer);
}
#endif /* DEBUG == DEBUG_FULL */
/*---------------------------------------------------------------------------*/
static void
update_nbr(void)
{
  uip_ds6_nbr_t *nbr;
  rpl_parent_t *parent;
  int num_used;
  int is_used;
  rpl_rank_t rank;

#if DEBUG == DEBUG_FULL
  if(!timer_init) {
    timer_init = 1;
    ctimer_set(&periodic_timer, 60 * CLOCK_SECOND,
               &handle_periodic_timer, NULL);
  }
#endif /* DEBUG == DEBUG_FULL */

  worst_rank = 0;
  worst_rank_nbr = NULL;
  num_used = 0;
  num_parents = 0;
  num_children = 0;

  nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL) {
    linkaddr_t *lladdr = nbr_table_get_lladdr(ds6_neighbors, nbr);
    is_used = 0;

    /*
     * Check if this neighbor is used as nexthop and therefor being a
     * RPL child.
    */

    if(uip_ds6_route_is_nexthop(&nbr->ipaddr) != 0) {
      is_used++;
      num_children++;
    }

    parent = rpl_get_parent((uip_lladdr_t *)lladdr);
    if(parent != NULL) {
      num_parents++;

      if(parent->dag != NULL && parent->dag->preferred_parent == parent) {
        /*
         * This is the preferred parent for the DAG and must not be removed
         * Note: this assumes that only RPL adds default routes.
         */
      } else if(is_used == 0 && worst_rank < INFINITE_RANK &&
                parent->rank > 0 &&
                parent->dag != NULL &&
                parent->dag->instance != NULL &&
                (rank = parent->dag->instance->of->rank_via_parent(parent)) > worst_rank) {
        /* This is the worst-rank neighbor - this is a good candidate for removal */
        worst_rank = rank;
        worst_rank_nbr = lladdr;
      }
      /* add to is_used after evaluation of is_used above */
      is_used++;
    }

    if(is_used == 0) {
      /* This neighbor is neither parent or child and can be safely removed */
      worst_rank_nbr = lladdr;
      worst_rank = INFINITE_RANK;
    } else if(is_used > 1) {
      PRINTF("NBR-POLICY: *** Neighbor is both child and candidate parent: ");
      PRINTLLADDR((uip_lladdr_t *)lladdr);
      PRINTF("\n");
    }

    nbr = nbr_table_next(ds6_neighbors, nbr);
    num_used++;
  }
  /* how many more IP neighbors can be have? */
  num_free = NBR_TABLE_MAX_NEIGHBORS - num_used;

  PRINTF("NBR-POLICY: Free: %d, Children: %d, Parents: %d Routes: %d\n",
	 num_free, num_children, num_parents, uip_ds6_route_num_routes());
}
/*---------------------------------------------------------------------------*/
/* Called whenever we get a unicast DIS - e.g. someone that already
   have this node in its table - since it is a unicast */
const linkaddr_t *
find_removable_dis(uip_ipaddr_t *from)
{

  update_nbr();
  if(num_free > 0) {
    /* there are free entries (e.g. unsused by RPL and ND6) but since it is
       used by other modules we can not pick these entries for removal. */
    PRINTF("Num-free > 0 = %d - Other for RPL/ND6 unused NBR entry exists .",
           num_free);
  }
  if(num_children < MAX_CHILDREN) {
    return worst_rank_nbr;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
const linkaddr_t *
find_removable_dio(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_instance_t *instance;

  update_nbr();

  instance = rpl_get_instance(dio->instance_id);
  if(instance == NULL || instance->current_dag == NULL) {
    PRINTF("Did not find instance id: %d\n", dio->instance_id);
    return NULL;
  }

  /* Add the new neighbor only if it is better than the worst parent. */
  if(dio->rank + instance->min_hoprankinc < worst_rank - instance->min_hoprankinc / 2) {
    /* Found *great* neighbor - add! */
    PRINTF("Found better neighbor %d < %d - add to cache...\n",
           dio->rank, worst_rank);

    return worst_rank_nbr;
  }

  PRINTF("Found worse neighbor with new %d and old %d - NOT add to cache.\n",
         dio->rank, worst_rank);
  return NULL;
}
/*---------------------------------------------------------------------------*/
const linkaddr_t *
find_removable_dao(uip_ipaddr_t *from, rpl_instance_t *instance)
{
  int max = MAX_CHILDREN;
  update_nbr();

  if(instance != NULL) {
    /* No need to reserve space for parents for RPL ROOT */
    if(instance->current_dag->rank == ROOT_RANK(instance)) {
      max = NBR_TABLE_MAX_NEIGHBORS;
    }
  }
  
  /* Check if this DAO sender is not yet neighbor and there is already too
     many children. */
  if(num_children >= max) {
    PRINTF("Can not add another child - already at max.\n");
    return NULL;
  }
  /* remove the worst ranked nbr */
  return worst_rank_nbr;
}
/*---------------------------------------------------------------------------*/
const linkaddr_t *
rpl_nbr_policy_find_removable(nbr_table_reason_t reason,void * data)
{
  /* When we get the DIO/DAO/DIS we know that UIP contains the
     incoming packet */
  switch(reason) {
  case NBR_TABLE_REASON_RPL_DIO:
    return find_removable_dio(&UIP_IP_BUF->srcipaddr, data);
  case NBR_TABLE_REASON_RPL_DAO:
    return find_removable_dao(&UIP_IP_BUF->srcipaddr, data);
  case NBR_TABLE_REASON_RPL_DIS:
    return find_removable_dis(&UIP_IP_BUF->srcipaddr);
  default:
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
/** @}*/
