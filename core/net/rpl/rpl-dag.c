/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         Logic for Directed Acyclic Graphs in RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */


#include "contiki.h"
#include "net/rpl/rpl-private.h"
#include "net/uip.h"
#include "net/uip-nd6.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "sys/ctimer.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "net/neighbor-info.h"

/************************************************************************/
extern rpl_of_t RPL_OF;
static rpl_of_t * const objective_functions[] = {&RPL_OF};
/************************************************************************/

#ifndef RPL_CONF_MAX_DAG_ENTRIES
#define RPL_MAX_DAG_ENTRIES     2
#else
#define RPL_MAX_DAG_ENTRIES     RPL_CONF_MAX_DAG_ENTRIES
#endif /* !RPL_CONF_MAX_DAG_ENTRIES */

#ifndef RPL_CONF_MAX_PARENTS
#define RPL_MAX_PARENTS       8
#else
#define RPL_MAX_PARENTS       RPL_CONF_MAX_PARENTS
#endif /* !RPL_CONF_MAX_PARENTS */
/************************************************************************/
/* RPL definitions. */

#ifndef RPL_CONF_GROUNDED
#define RPL_GROUNDED                    0
#else
#define RPL_GROUNDED                    RPL_CONF_GROUNDED
#endif /* !RPL_CONF_GROUNDED */

#ifndef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_DIO_INTERVAL_MIN            DEFAULT_DIO_INTERVAL_MIN
#else
#define RPL_DIO_INTERVAL_MIN            RPL_CONF_DIO_INTERVAL_MIN
#endif /* !RPL_CONF_DIO_INTERVAL_MIN */

#ifndef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_DIO_INTERVAL_DOUBLINGS      DEFAULT_DIO_INTERVAL_DOUBLINGS
#else
#define RPL_DIO_INTERVAL_DOUBLINGS      RPL_CONF_DIO_INTERVAL_DOUBLINGS
#endif /* !RPL_CONF_DIO_INTERVAL_DOUBLINGS */

/************************************************************************/
/* Allocate parents from the same static MEMB chunk to reduce memory waste. */
MEMB(parent_memb, struct rpl_parent, RPL_MAX_PARENTS);

static rpl_dag_t dag_table[RPL_MAX_DAG_ENTRIES];
/************************************************************************/
/* Remove DAG parents with a rank that is at least the same as minimum_rank. */
static void
remove_parents(rpl_dag_t *dag, rpl_rank_t minimum_rank)
{
  rpl_parent_t *p, *p2;

  PRINTF("RPL: Removing parents (minimum rank %u)\n",
	minimum_rank);

  for(p = list_head(dag->parents); p != NULL; p = p2) {
    p2 = p->next;
    if(p->rank >= minimum_rank) {
      rpl_remove_parent(dag, p);
    }
  }
}
/************************************************************************/
static void
remove_worst_parent(rpl_dag_t *dag, rpl_rank_t min_worst_rank)
{
  rpl_parent_t *p, *worst;

  PRINTF("RPL: Removing the worst parent\n");

  /* Find the parent with the highest rank. */
  worst = NULL;
  for(p = list_head(dag->parents); p != NULL; p = list_item_next(p)) {
    if(p != dag->preferred_parent &&
       (worst == NULL || p->rank > worst->rank)) {
      worst = p;
    }
  }
  /* Remove the neighbor if its rank is worse than the minimum worst
     rank. */
  if(worst != NULL && worst->rank > min_worst_rank) {
    rpl_remove_parent(dag, worst);
  }
}
/************************************************************************/
static int
should_send_dao(rpl_dag_t *dag, rpl_dio_t *dio, rpl_parent_t *p)
{
  /* if MOP is set to no downward routes no DAO should be sent */
  if(dag->mop == RPL_MOP_NO_DOWNWARD_ROUTES) return 0;
  return dio->dtsn > p->dtsn && p == dag->preferred_parent;
}
/************************************************************************/
static int
acceptable_rank(rpl_dag_t *dag, rpl_rank_t rank)
{
  return rank != INFINITE_RANK &&
    (dag->max_rankinc == 0 ||
     DAG_RANK(rank, dag) <= DAG_RANK(dag->min_rank + dag->max_rankinc, dag));
}
/************************************************************************/
rpl_dag_t *
rpl_set_root(uip_ipaddr_t *dag_id)
{
  rpl_dag_t *dag;
  int version;

  version = -1;
  dag = rpl_get_dag(RPL_DEFAULT_INSTANCE);
  if(dag != NULL) {
    PRINTF("RPL: Dropping a joined DAG when setting this node as root");
    version = dag->version;
    rpl_free_dag(dag);
  }

  dag = rpl_alloc_dag(RPL_DEFAULT_INSTANCE);
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG\n");
    return NULL;
  }

  dag->joined = 1;
  dag->version = version + 1;
  dag->grounded = RPL_GROUNDED;
  dag->mop = RPL_MOP_DEFAULT;
  dag->of = &RPL_OF;
  dag->preferred_parent = NULL;
  dag->dtsn_out = 1; /* Trigger DAOs from the beginning. */

  memcpy(&dag->dag_id, dag_id, sizeof(dag->dag_id));

  dag->dio_intdoubl = DEFAULT_DIO_INTERVAL_DOUBLINGS;
  dag->dio_intmin = DEFAULT_DIO_INTERVAL_MIN;
  dag->dio_redundancy = DEFAULT_DIO_REDUNDANCY;
  dag->max_rankinc = DEFAULT_MAX_RANKINC;
  dag->min_hoprankinc = DEFAULT_MIN_HOPRANKINC;

  dag->default_lifetime = RPL_DEFAULT_LIFETIME;
  dag->lifetime_unit = RPL_DEFAULT_LIFETIME_UNIT;

  dag->rank = ROOT_RANK(dag);

  dag->of->update_metric_container(dag);

  PRINTF("RPL: Node set to be a DAG root with DAG ID ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  ANNOTATE("#A root=%u\n",dag->dag_id.u8[sizeof(dag->dag_id) - 1]);


  rpl_reset_dio_timer(dag, 1);

  return dag;
}
/************************************************************************/
int
rpl_set_prefix(rpl_dag_t *dag, uip_ipaddr_t *prefix, int len)
{
  if(len <= 128) {
    memset(&dag->prefix_info.prefix, 0, 16);
    memcpy(&dag->prefix_info.prefix, prefix, (len + 7) / 8);
    dag->prefix_info.length = len;
    dag->prefix_info.flags = UIP_ND6_RA_FLAG_AUTONOMOUS;
    PRINTF("RPL: Prefix set - will announce this in DIOs\n");
    return 1;
  }
  return 0;
}
/************************************************************************/
int
rpl_set_default_route(rpl_dag_t *dag, uip_ipaddr_t *from)
{
  if(dag->def_route != NULL) {
    PRINTF("RPL: Removing default route through ");
    PRINT6ADDR(&dag->def_route->ipaddr);
    PRINTF("\n");
    uip_ds6_defrt_rm(dag->def_route);
  }

  if(from != NULL) {
    PRINTF("RPL: Adding default route through ");
    PRINT6ADDR(from);
    PRINTF("\n");
    dag->def_route = uip_ds6_defrt_add(from,
                                       RPL_LIFETIME(dag,
                                                    dag->default_lifetime));
    if(dag->def_route == NULL) {
      return 0;
    }
  }

  return 1;
}
/************************************************************************/
rpl_dag_t *
rpl_alloc_dag(uint8_t instance_id)
{
  rpl_dag_t *dag;
  rpl_dag_t *end;

  for(dag = &dag_table[0], end = dag + RPL_MAX_DAG_ENTRIES; dag < end; dag++) {
    if(dag->used == 0) {
      memset(dag, 0, sizeof(*dag));
      dag->parents = &dag->parent_list;
      list_init(dag->parents);
      dag->instance_id = instance_id;
      dag->def_route = NULL;
      dag->rank = INFINITE_RANK;
      dag->min_rank = INFINITE_RANK;
      return dag;
    }
  }

  RPL_STAT(rpl_stats.mem_overflows++);
  return NULL;
}
/************************************************************************/
void
rpl_free_dag(rpl_dag_t *dag)
{
  PRINTF("RPL: Leaving the DAG ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  /* Remove routes installed by DAOs. */
  rpl_remove_routes(dag);

  /* Remove parents and the default route. */
  remove_parents(dag, 0);
  rpl_set_default_route(dag, NULL);

  ctimer_stop(&dag->dio_timer);
  ctimer_stop(&dag->dao_timer);

  dag->used = 0;
  dag->joined = 0;
}
/************************************************************************/
rpl_parent_t *
rpl_add_parent(rpl_dag_t *dag, rpl_dio_t *dio, uip_ipaddr_t *addr)
{
  rpl_parent_t *p;

  p = memb_alloc(&parent_memb);
  if(p == NULL) {
    RPL_STAT(rpl_stats.mem_overflows++);
    return NULL;
  }

  memcpy(&p->addr, addr, sizeof(p->addr));
  p->dag = dag;
  p->rank = dio->rank;
  p->link_metric = INITIAL_LINK_METRIC;
  p->dtsn = 0;

  memcpy(&p->mc, &dio->mc, sizeof(p->mc));

  list_add(dag->parents, p);

  return p;
}
/************************************************************************/
rpl_parent_t *
rpl_find_parent(rpl_dag_t *dag, uip_ipaddr_t *addr)
{
  rpl_parent_t *p;

  for(p = list_head(dag->parents); p != NULL; p = p->next) {
    if(uip_ipaddr_cmp(&p->addr, addr)) {
      return p;
    }
  }

  return NULL;
}

/************************************************************************/
rpl_parent_t *
rpl_select_parent(rpl_dag_t *dag)
{
  rpl_parent_t *p;
  rpl_parent_t *best;

  best = NULL;
  for(p = list_head(dag->parents); p != NULL; p = p->next) {
    if(p->rank == INFINITE_RANK) {
      /* ignore this neighbor */
    } else if(best == NULL) {
      best = p;
    } else {
      best = dag->of->best_parent(best, p);
    }
  }

  if(best == NULL) {
    /* need to handle update of best... */
    return NULL;
  }

  if(dag->preferred_parent != best) {
    PRINTF("RPL: Sending a No-Path DAO to old DAO parent\n");
    dao_output(dag->preferred_parent, ZERO_LIFETIME);

    dag->preferred_parent = best; /* Cache the value. */
    dag->of->update_metric_container(dag);
    rpl_set_default_route(dag, &best->addr);
    /* The DAO parent set changed - schedule a DAO transmission. */
    if(dag->mop != RPL_MOP_NO_DOWNWARD_ROUTES) {
      rpl_schedule_dao(dag);
    }
    rpl_reset_dio_timer(dag, 1);
    PRINTF("RPL: New preferred parent, rank changed from %u to %u\n",
	   (unsigned)dag->rank, dag->of->calculate_rank(best, 0));
    RPL_STAT(rpl_stats.parent_switch++);
  }

  /* Update the DAG rank, since link-layer information may have changed
     the local confidence. */
  dag->rank = dag->of->calculate_rank(best, 0);
  if(dag->rank < dag->min_rank) {
    dag->min_rank = dag->rank;
  } else if(!acceptable_rank(dag, best->rank)) {
    /* Send a No-Path DAO to the soon-to-be-removed preferred parent. */
    dao_output(best, ZERO_LIFETIME);

    remove_parents(dag, 0);
    return NULL;
  }

  return best;
}
/************************************************************************/
int
rpl_remove_parent(rpl_dag_t *dag, rpl_parent_t *parent)
{
  uip_ds6_defrt_t *defrt;

  /* Remove uIPv6 routes that have this parent as the next hop. **/
  uip_ds6_route_rm_by_nexthop(&parent->addr);
  defrt = uip_ds6_defrt_lookup(&parent->addr);
  if(defrt != NULL) {
    PRINTF("RPL: Removing default route ");
    PRINT6ADDR(&parent->addr);
    PRINTF("\n");
    uip_ds6_defrt_rm(defrt);
    dag->def_route = NULL;
  }

  PRINTF("RPL: Removing parent ");
  PRINT6ADDR(&parent->addr);
  PRINTF("\n");

  if(parent == dag->preferred_parent) {
    dag->preferred_parent = NULL;
  }

  list_remove(dag->parents, parent);
  memb_free(&parent_memb, parent);
  return 0;
}
/************************************************************************/
rpl_dag_t *
rpl_get_dag(int instance_id)
{
  int i;

  for(i = 0; i < RPL_MAX_DAG_ENTRIES; i++) {
    if(dag_table[i].joined && (instance_id == RPL_ANY_INSTANCE ||
			       dag_table[i].instance_id == instance_id)) {
      return &dag_table[i];
    }
  }
  return NULL;
}
/************************************************************************/
rpl_of_t *
rpl_find_of(rpl_ocp_t ocp)
{
  unsigned int i;

  for(i = 0;
      i < sizeof(objective_functions) / sizeof(objective_functions[0]); 
      i++) {
    if(objective_functions[i]->ocp == ocp) {
      return objective_functions[i];
    }
  }

  return NULL;
}
/************************************************************************/
static void
join_dag(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;
  rpl_of_t *of;

  dag = rpl_alloc_dag(dio->instance_id);
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG object!\n");
    return;
  }

  p = rpl_add_parent(dag, dio, from);
  PRINTF("RPL: Adding ");
  PRINT6ADDR(from);
  PRINTF(" as a parent: ");
  if(p == NULL) {
    PRINTF("failed\n");
    return;
  }
  PRINTF("succeeded\n");

  /* Determine the objective function by using the
     objective code point of the DIO. */
  of = rpl_find_of(dio->ocp);
  if(of == NULL) {
    PRINTF("RPL: DIO for DAG instance %u does not specify a supported OF\n",
        dio->instance_id);
    return;
  }

  /* Autoconfigure an address if this node does not already have an address
     with this prefix. */
  if((dio->prefix_info.flags & UIP_ND6_RA_FLAG_AUTONOMOUS)) {
    uip_ipaddr_t ipaddr;
    /* assume that the prefix ends with zeros! */
    memcpy(&ipaddr, &dio->prefix_info.prefix, 16);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    if(uip_ds6_addr_lookup(&ipaddr) == NULL) {
      PRINTF("RPL: adding global IP address ");
      PRINT6ADDR(&ipaddr);
      PRINTF("\n");
      uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
    }
  }

  dag->joined = 1;
  dag->used = 1;
  dag->of = of;
  dag->grounded = dio->grounded;
  dag->mop = dio->mop;
  dag->preference = dio->preference;
  dag->instance_id = dio->instance_id;

  dag->max_rankinc = dio->dag_max_rankinc;
  dag->min_hoprankinc = dio->dag_min_hoprankinc;

  dag->version = dio->version;
  dag->preferred_parent = p;
  dag->of->update_metric_container(dag);

  dag->dio_intdoubl = dio->dag_intdoubl;
  dag->dio_intmin = dio->dag_intmin;
  dag->dio_redundancy = dio->dag_redund;

  memcpy(&dag->dag_id, &dio->dag_id, sizeof(dio->dag_id));

  /* copy prefix information into the dag */
  memcpy(&dag->prefix_info, &dio->prefix_info, sizeof(rpl_prefix_t));

  dag->rank = dag->of->calculate_rank(p, dio->rank);
  dag->min_rank = dag->rank; /* So far this is the lowest rank we know of. */

  PRINTF("RPL: Joined DAG with instance ID %u, rank %hu, DAG ID ",
         dio->instance_id, dag->rank);
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  ANNOTATE("#A join=%u\n",dag->dag_id.u8[sizeof(dag->dag_id) - 1]);


  dag->default_lifetime = dio->default_lifetime;
  dag->lifetime_unit = dio->lifetime_unit;

  rpl_reset_dio_timer(dag, 1);
  rpl_set_default_route(dag, from);

  if(should_send_dao(dag, dio, p)) {
    rpl_schedule_dao(dag);
  } else {
    PRINTF("RPL: The DIO does not meet the prerequisites for sending a DAO\n");
  }
}
/************************************************************************/
static void
global_repair(uip_ipaddr_t *from, rpl_dag_t *dag, rpl_dio_t *dio)
{
  rpl_parent_t *p;

  remove_parents(dag, 0);
  dag->version = dio->version;
  dag->dtsn_out = 1;
  dag->of->reset(dag);
  if((p = rpl_add_parent(dag, dio, from)) == NULL) {
    PRINTF("RPL: Failed to add a parent during the global repair\n");
    dag->rank = INFINITE_RANK;
  } else {
    rpl_set_default_route(dag, from);
    dag->rank = dag->of->calculate_rank(NULL, dio->rank);
    dag->min_rank = dag->rank;
    rpl_reset_dio_timer(dag, 1);
    if(should_send_dao(dag, dio, p)) {
      rpl_schedule_dao(dag);
    }
  }
  PRINTF("RPL: Participating in a global repair (version=%u, rank=%hu)\n",
         dag->version, dag->rank);

  RPL_STAT(rpl_stats.global_repairs++);
}
/************************************************************************/
int
rpl_repair_dag(rpl_dag_t *dag)
{
  if(dag->rank == ROOT_RANK(dag)) {
    dag->version++;
    dag->dtsn_out = 1;
    rpl_reset_dio_timer(dag, 1);
    return 1;
  }
  return 0;
}
/************************************************************************/
void
rpl_local_repair(rpl_dag_t *dag)
{
  PRINTF("RPL: Starting a local DAG repair\n");

  dag->rank = INFINITE_RANK;
  remove_parents(dag, 0);
  rpl_reset_dio_timer(dag, 1);

  RPL_STAT(rpl_stats.local_repairs++);
}
/************************************************************************/
void
rpl_recalculate_ranks(void)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;

  /*
   * We recalculate ranks when we receive feedback from the system rather
   * than RPL protocol messages. This periodical recalculation is called
   * from a timer in order to keep the stack depth reasonably low.
   */
  dag = rpl_get_dag(RPL_ANY_INSTANCE);
  if(dag != NULL) {
    for(p = list_head(dag->parents); p != NULL; p = p->next) {
      if(p->updated) {
	p->updated = 0;
	rpl_process_parent_event(dag, p);
	/*
	 * Stop calculating here because the parent list may have changed.
	 * If more ranks need to be recalculated, it will be taken care of
	 * in subsequent calls to this functions.
	 */
	break;
      }
    }
  }
}
/************************************************************************/
int
rpl_process_parent_event(rpl_dag_t *dag, rpl_parent_t *p)
{
  rpl_rank_t parent_rank;
  rpl_rank_t old_rank;

  /* Update the parent rank. */
  parent_rank = p->rank;
  old_rank = dag->rank;

  if(rpl_select_parent(dag) == NULL) {
    /* No suitable parent; trigger a local repair. */
    PRINTF("RPL: No parents found in a DAG\n");
    rpl_local_repair(dag);
    return 1;
  }

  if(DAG_RANK(old_rank, dag) != DAG_RANK(dag->rank, dag)) {
    if(dag->rank < dag->min_rank) {
      dag->min_rank = dag->rank;
    }
    PRINTF("RPL: Moving in the DAG from rank %hu to %hu\n",
	   DAG_RANK(old_rank, dag), DAG_RANK(dag->rank, dag));
    PRINTF("RPL: The preferred parent is ");
    PRINT6ADDR(&dag->preferred_parent->addr);
    PRINTF(" (rank %u)\n",
           (unsigned)DAG_RANK(dag->preferred_parent->rank, dag));
    rpl_reset_dio_timer(dag, 1);
  }

  if(parent_rank == INFINITE_RANK ||
     !acceptable_rank(dag, dag->of->calculate_rank(NULL, parent_rank))) {
    /* The candidate parent is no longer valid: the rank increase resulting
       from the choice of it as a parent would be too high. */
    return 0;
  }

  return 1;
}
/************************************************************************/
void
rpl_process_dio(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;

  if(dio->mop != RPL_MOP_DEFAULT) {
    PRINTF("RPL: Ignoring a DIO with an unsupported MOP: %d\n", dio->mop);
    return;
  }

  dag = rpl_get_dag(dio->instance_id);
  if(dag == NULL) {
    /* Join the first possible DAG of this RPL instance. */
    if(dio->rank != INFINITE_RANK) {
      join_dag(from, dio);
    } else {
      PRINTF("RPL: Ignoring DIO from node with infinite rank: ");
      PRINT6ADDR(from);
      PRINTF("\n");
    }
    return;
  }

  if(memcmp(&dag->dag_id, &dio->dag_id, sizeof(dag->dag_id))) {
    PRINTF("RPL: Ignoring DIO for another DAG within our instance\n");
    return;
  }

  if(dio->version > dag->version) {
    if(dag->rank == ROOT_RANK(dag)) {
      PRINTF("RPL: Root received inconsistent DIO version number\n");
      dag->version = dio->version + 1;
      rpl_reset_dio_timer(dag, 1);
    } else {
      global_repair(from, dag, dio);
    }
    return;
  } else if(dio->version < dag->version) {
    /* Inconsistency detected - someone is still on old version */
    PRINTF("RPL: old version received => inconsistency detected\n");
    rpl_reset_dio_timer(dag, 1);
    return;
  }

  if(dio->rank == INFINITE_RANK) {
    rpl_reset_dio_timer(dag, 1);
  } else if(dio->rank < ROOT_RANK(dag)) {
    PRINTF("RPL: Ignoring DIO with too low rank: %u\n",
           (unsigned)dio->rank);
    return;
  }

  if(dag->rank == ROOT_RANK(dag)) {
    if(dio->rank != INFINITE_RANK) {
      dag->dio_counter++;
    }
    return;
  }

  /*
   * At this point, we know that this DIO pertains to a DAG that
   * we are already part of. We consider the sender of the DIO to be
   * a candidate parent, and let rpl_process_parent_event decide
   * whether to keep it in the set.
   */

  p = rpl_find_parent(dag, from);
  if(p == NULL) {
    if(RPL_PARENT_COUNT(dag) == RPL_MAX_PARENTS) {
      /* Make room for a new parent. */
      remove_worst_parent(dag, dio->rank);
    }
    
    /* Add the DIO sender as a candidate parent. */
    p = rpl_add_parent(dag, dio, from);
    if(p == NULL) {
      PRINTF("RPL: Failed to add a new parent (");
      PRINT6ADDR(from);
      PRINTF(")\n");
      return;
    }

    PRINTF("RPL: New candidate parent with rank %u: ", (unsigned)p->rank);
    PRINT6ADDR(from);
    PRINTF("\n");
  } else if(DAG_RANK(p->rank, dag) == DAG_RANK(dio->rank, dag)) {
    PRINTF("RPL: Received consistent DIO\n");
    dag->dio_counter++;
  }
  
  /* We have allocated a candidate parent; process the DIO further. */

  memcpy(&p->mc, &dio->mc, sizeof(p->mc));    
  p->rank = dio->rank;
  if(rpl_process_parent_event(dag, p) == 0) {
    /* The candidate parent no longer exists. */
    return;
  }
  
  if(should_send_dao(dag, dio, p)) {
    rpl_schedule_dao(dag);
  }
  
  p->dtsn = dio->dtsn;
}
/************************************************************************/

