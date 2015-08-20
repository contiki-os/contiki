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
 */

#define MAX_CHILDREN (NBR_TABLE_MAX_NEIGHBORS - 3)

static int num_parents; /* any node that are possible parents */
static int num_children;  /* all children that we have as nexthop */
static int num_free;
static uip_ds6_nbr_t *worst_rank_nbr; /* the parent that has the worst rank */
static rpl_rank_t worst_rank;
/*---------------------------------------------------------------------------*/
static void
update_nbr(void)
{
  uip_ds6_nbr_t *nbr;
  rpl_parent_t *parent;
  int num_used;
  int is_used;
  rpl_rank_t rank;

  worst_rank = 0;
  worst_rank_nbr = NULL;
  num_used = 0;
  num_parents = 0;
  num_children = 0;

  nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL) {
    linkaddr_t *lladdr = nbr_table_get_lladdr(ds6_neighbors, nbr);
    is_used = 0;

    parent = rpl_get_parent((uip_lladdr_t *)lladdr);
    if(parent != NULL) {
      num_parents++;
      is_used++;

      if(parent->dag != NULL && parent->dag->preferred_parent == parent) {
        /* This is the preferred parent for the DAG and must not be removed */

        /* Note: this assumes that only RPL adds default routes. */

      } else if(worst_rank < INFINITE_RANK &&
                parent->rank > 0 &&
                parent->dag != NULL &&
                parent->dag->instance != NULL &&
                (rank = parent->dag->instance->of->calculate_rank(parent, 0)) > worst_rank) {
        /* This is the worst-rank neighbor - this is a good candidate for removal */
        worst_rank = rank;
        worst_rank_nbr = nbr;
      }
    }

    /* Check if this neighbor is used as nexthop and therefor being a
       RPL child. */
    if(uip_ds6_route_is_nexthop((uip_lladdr_t *)lladdr) != 0) {
      is_used++;
      num_children++;
    }

    if(is_used == 0) {
      /* This neighbor is neither parent or child and can be safely removed */
      worst_rank_nbr = nbr;
      worst_rank = INFINITE_RANK;
    } else if(is_used > 1) {
      /* Both parent and child - this should never happen! */
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
static int
remove_worst_nbr(void)
{
  /* we assume that it is possible to remove the worst parent at the moment */
  if(worst_rank_nbr != NULL) {
    PRINTF("Removing worst ranked nbr ");
    PRINTLLADDR((uip_lladdr_t*)nbr_table_get_lladdr(ds6_neighbors, worst_rank_nbr));
    PRINTF(" with rank %d\n", worst_rank);
    if(uip_ds6_nbr_rm(worst_rank_nbr)) {
      worst_rank_nbr = NULL;
      return 1;
    }
    PRINTF("FAILED to remove worst ranked nbr!\n");
    return 0;
  }
  PRINTF("FAILED to remove worst rank nbr - no found\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Called whenever we get a unicast DIS - e.g. someone that already
   have this node in its table - since it is a unicast */
static int
check_add_from_dis(uip_ipaddr_t *from)
{

  /* do a lookup to see if it is alread there - then allow add/update */
  if(uip_ds6_nbr_lookup(from)) {
    return 1;
  }
  update_nbr();
  if(num_free > 0) {
    return 1;
  }
  if(num_children < MAX_CHILDREN) {
    return remove_worst_nbr();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
check_add_from_dio(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_instance_t *instance;
  rpl_rank_t rank;

  /* Do a lookup to see if it is already there - then allow add/update. */
  if(uip_ds6_nbr_lookup(from)) {
    return 1;
  }

  update_nbr();

  /* If there is room for this neighbor just add it. */
  if(num_free > 0) {
    return 1;
  }

  instance = rpl_get_instance(dio->instance_id);
  if(instance == NULL || instance->current_dag == NULL) {
    PRINTF("Did not find instance id: %d\n", dio->instance_id);
    return 0;
  }

  /* Add the new neighbor only if it is better than the preferred parent. */
  rank = instance->of->calculate_rank(NULL, dio->rank);
  if(rank < worst_rank - instance->min_hoprankinc / 2) {
    /* Found *great* neighbor - add! */
    PRINTF("Found better neighbor %d < %d - add to cache...\n",
           rank, worst_rank);

    return remove_worst_nbr();
  }

  PRINTF("Found worse neighbor with new %d and old %d - NOT add to cache.\n",
         rank, worst_rank);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
check_add_from_dao(uip_ipaddr_t *from)
{
  /* Do a lookup to see if it is alread there - then allow add/update. */
  if(uip_ds6_nbr_lookup(from)) {
    return 1;
  }

  update_nbr();

  /* Check if this DAO sender is not yet neighbor and there is already too
     many children. */
  if(num_children >= MAX_CHILDREN) {
    PRINTF("Can not add another child - already at max.\n");
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
const struct nbr_policy rpl_nbr_policy = {
  check_add_from_dis,
  check_add_from_dio,
  check_add_from_dao
};
/*---------------------------------------------------------------------------*/
/** @}*/
