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
 * $Id: rpl-dag.c,v 1.18 2010/06/03 18:37:47 joxe Exp $
 */
/**
 * \file
 *         Logic for Directed Acyclic Graphs in RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/rpl/rpl.h"

#include "net/uip.h"
#include "net/uip-nd6.h"
#include "net/rime/ctimer.h"
#include "lib/list.h"
#include "lib/memb.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_ANNOTATE

#include "net/uip-debug.h"

/************************************************************************/
extern rpl_of_t rpl_of_etx;
static rpl_of_t *objective_functions[] = {&rpl_of_etx, NULL};
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

#define POISON_ROUTES 1
/************************************************************************/
static void
remove_parents(rpl_dag_t *dag, rpl_parent_t *exception, int poison_routes)
{
  rpl_parent_t *p, *p2;

  PRINTF("RPL: Removing parents %s poisoning routes\n",
	 poison_routes == POISON_ROUTES ? "and" : "without");

  for(p = list_head(dag->parents); p != NULL;) {
   if(p != exception) {
     ANNOTATE("#L %u 0\n", p->addr.u8[sizeof(p->addr) - 1]);

     if(poison_routes == POISON_ROUTES) {
       /* Send no-DAOs to old parents. */
       dao_output(p, ZERO_LIFETIME);
     }

     p2 = p->next;
     rpl_remove_parent(dag, p);
     p = p2;
   } else {
     p = p->next;
   }
 }
}
/************************************************************************/
rpl_dag_t *
rpl_set_root(uip_ipaddr_t *dag_id)
{
  rpl_dag_t *dag;
  int version;

  version = -1;
  dag = rpl_get_dag(RPL_DEFAULT_OCP);
  if(dag != NULL) {
    PRINTF("RPL: Dropping a joined DAG when setting this node as root");
    version = dag->version;
    rpl_free_dag(dag);
  }

  dag = rpl_alloc_dag();
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG\n");
    return NULL;
  }

  dag->joined = 1;
  dag->version = version + 1;
  dag->grounded = RPL_GROUNDED;
  dag->rank = ROOT_RANK;
  dag->of = rpl_find_of(RPL_DEFAULT_OCP);
  dag->best_parent = NULL;

  memcpy(&dag->dag_id, dag_id, sizeof(dag->dag_id));

  dag->dio_intdoubl = DEFAULT_DIO_INTERVAL_DOUBLINGS;
  dag->dio_intmin = DEFAULT_DIO_INTERVAL_MIN;
  dag->dio_redundancy = DEFAULT_DIO_REDUNDANCY;
  dag->max_rankinc = DEFAULT_MAX_RANKINC;
  dag->min_hoprankinc = DEFAULT_MIN_HOPRANKINC;

  PRINTF("RPL: Node set to be a DAG root with DAG ID ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

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
    if(DEFAULT_ROUTE_LIFETIME == INFINITE_LIFETIME) {
      dag->def_route = uip_ds6_defrt_add(from, 0);
    } else {
      dag->def_route = uip_ds6_defrt_add(from, DEFAULT_ROUTE_LIFETIME);
    }
    if(dag->def_route == NULL) {
      return 0;
    }
  }

  return 1;
}
/************************************************************************/
rpl_dag_t *
rpl_alloc_dag(void)
{
  int i;

  for(i = 0; i < RPL_MAX_DAG_ENTRIES; i++) {
    if(dag_table[i].used == 0) {
      memset(&dag_table[i], 0, sizeof(dag_table[0]));
      dag_table[i].parents = &dag_table[i].parent_list;
      list_init(dag_table[i].parents);
      dag_table[i].def_route = NULL;
      return &dag_table[i];
    }
  }
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
  remove_parents(dag, NULL, !POISON_ROUTES);
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
    return NULL;
  }

  memcpy(&p->addr, addr, sizeof(p->addr));
  p->local_confidence = 0;
  p->dag = dag;
  p->rank = dio->dag_rank;

  list_add(dag->parents, p);

  /* Draw a line between the node and its parent in Cooja. */
  ANNOTATE("#L %u 1\n",
           addr->u8[sizeof(*addr) - 1]);

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
rpl_preferred_parent(rpl_dag_t *dag)
{
  rpl_parent_t *p;
  rpl_parent_t *best;

  best = NULL;
  for(p = list_head(dag->parents); p != NULL; p = p->next) {
    if(best == NULL) {
      best = p;
    } else {
      best = dag->of->best_parent(best, p);
    }
  }
  if(dag->best_parent != best) {
    dag->best_parent = best; /* Cache the value. */
    rpl_set_default_route(dag, &best->addr);
  }
  return best;
}
/************************************************************************/
int
rpl_remove_parent(rpl_dag_t *dag, rpl_parent_t *parent)
{
  uip_ds6_defrt_t *defrt;

  ANNOTATE("#L %u 0\n",
           parent->addr.u8[sizeof(parent->addr) - 1]);

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
  rpl_of_t *of;

  for(of = objective_functions[0]; of != NULL; of++) {
    if(of->ocp == ocp) {
      return of;
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

  dag = rpl_alloc_dag();
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

  p->local_confidence = 0;	/* The lowest confidence for new parents. */

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
  dag->preference = dio->preference;
  dag->grounded = dio->grounded;
  dag->instance_id = dio->instance_id;
  dag->rank = dag->of->increment_rank(dio->dag_rank, p);
  dag->min_rank = dag->rank; /* So far this is the lowest rank we know */
  dag->version = dio->version;
  dag->dio_intdoubl = dio->dag_intdoubl;
  dag->dio_intmin = dio->dag_intmin;
  dag->dio_redundancy = dio->dag_redund;

  dag->max_rankinc = dio->dag_max_rankinc;
  dag->min_hoprankinc = dio->dag_min_hoprankinc;

  memcpy(&dag->dag_id, &dio->dag_id, sizeof(dio->dag_id));

  /* copy prefix information into the dag */
  memcpy(&dag->prefix_info, &dio->prefix_info, sizeof(rpl_prefix_t));

  PRINTF("RPL: Joined DAG with instance ID %u, rank %hu, DAG ID ",
         dio->instance_id, dag->rank);
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  rpl_reset_dio_timer(dag, 1);
  rpl_set_default_route(dag, from);

  if(dio->dst_adv_supported && dio->dst_adv_trigger) {
    rpl_schedule_dao(dag);
  } else {
    PRINTF("RPL: dst_adv_trigger not set in incoming DIO!\n");
  }
}
/************************************************************************/
static void
global_repair(uip_ipaddr_t *from, rpl_dag_t *dag, rpl_dio_t *dio)
{
  rpl_parent_t *p;

  remove_parents(dag, NULL, !POISON_ROUTES);
  dag->version = dio->version;
  dag->of->reset(dag);
  if((p = rpl_add_parent(dag, dio, from)) == NULL) {
    PRINTF("RPL: Failed to add a parent during the global repair\n");
    dag->rank = INFINITE_RANK;
  } else {
    rpl_set_default_route(dag, from);
    dag->rank = dag->of->increment_rank(dio->dag_rank, p);
    rpl_reset_dio_timer(dag, 1);
  }
  PRINTF("RPL: Participating in a global repair (version=%u, rank=%hu)\n",
         dag->version, dag->rank);

}
/************************************************************************/
int
rpl_repair_dag(rpl_dag_t *dag)
{
  if(dag->rank == ROOT_RANK) {
    dag->version++;
    rpl_reset_dio_timer(dag, 1);
    return 1;
  }
  return 0;
}
/************************************************************************/
void
rpl_process_dio(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;
  rpl_parent_t *preferred_parent;
  uint8_t new_rank;
  uint8_t new_parent;

  dag = rpl_get_dag(dio->instance_id);
  if(dag == NULL) {
    /* Always join the first possible DAG that is not of INF_RANK. */
    if(dio->dag_rank != INFINITE_RANK) {
      join_dag(from, dio);
    } else {
      PRINTF("RPL: Ignoring DIO from node with infinite rank: ");
      PRINT6ADDR(from);
      PRINTF("\n");
    }
    return;
  }

  if(dag->instance_id != dio->instance_id) {
    /* We avoid joining more than one RPL instance. */
    PRINTF("RPL: Cannot join another RPL instance\n");
    return;
  }

  if(memcmp(&dag->dag_id, &dio->dag_id, sizeof(dag->dag_id))) {
    PRINTF("RPL: Ignoring DIO for another DAG within our instance\n");
    return;
  }

  if(dio->version > dag->version) {
    if(dag->rank == ROOT_RANK) {
      PRINTF("RPL: Root received inconsistent DIO version number\n");
      dag->version = dio->version + 1;
      rpl_reset_dio_timer(dag, 1);
      return;
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

  /* This DIO pertains to a DAG that we are already part of. */
  p = rpl_find_parent(dag, from);
  if(p != NULL) {
    if(dio->dst_adv_supported && dio->dst_adv_trigger &&
       p == rpl_preferred_parent(dag)) {
      rpl_schedule_dao(dag);
    }

    if(p->rank > dio->dag_rank) {
      p->rank = dio->dag_rank;
      rpl_reset_dio_timer(dag, 1);
    } else if(p->rank < dio->dag_rank) {
      PRINTF("RPL: Existing parent ");
      PRINT6ADDR(from);
      PRINTF(" got a higher rank (%hu -> %hu)\n",
             p->rank, dio->dag_rank);
      p->rank = dio->dag_rank;
      if(RPL_PARENT_COUNT(dag) > 1) {
        /* Since we have alternative parents, we can simply drop this one. */
        rpl_remove_parent(dag, p);
	p = rpl_preferred_parent(dag);
	if(p != NULL) {
          rpl_set_default_route(dag, &p->addr);
	}
	return;
      } else if(dag->of->increment_rank(dio->dag_rank, p) <= dag->min_rank + dag->max_rankinc) {
        preferred_parent = rpl_preferred_parent(dag);
        if(p == preferred_parent) {
          new_rank = dag->of->increment_rank(dio->dag_rank, p);
          rpl_set_default_route(dag, &p->addr);
	  dag->rank = new_rank;
	  PRINTF("RPL: New rank is %hu, max is %hu\n",
		dag->rank, dag->min_rank + dag->max_rankinc);
        }
      } else {
        PRINTF("RPL: Cannot find an acceptable preferred parent\n");
        /* do local repair - jump down the DAG */
        remove_parents(dag, NULL, POISON_ROUTES);
        dag->rank = INFINITE_RANK;
      }
      rpl_reset_dio_timer(dag, 1);
    } else {
      /* Assume consistency and increase the DIO counter. */
      PRINTF("RPL: Received a consistent DIO\n");
      dag->dio_counter++;
    }
  }

  if(dio->dag_rank < dag->rank) {
    /* Message from a node closer to the root, but we might still be out of allowed rank-range */
    if(dag->max_rankinc > 0 && dag->min_rank + dag->max_rankinc <
       dag->of->increment_rank(dio->dag_rank, NULL)) {
        PRINTF("RPL: Could not add parent, resulting rank too high\n");
        return;
    }
    new_parent = 0;
    if(p == NULL) {
      p = rpl_add_parent(dag, dio, from);
      if(p == NULL) {
        PRINTF("RPL: Could not add parent\n");
        return;
      }

      PRINTF("RPL: New parent with rank %hu ", p->rank);
      PRINT6ADDR(from);
      PRINTF("\n");
      new_parent = 1;
    }

    new_rank = dag->of->increment_rank(dio->dag_rank, p);
    if(new_rank < dag->rank) {
      PRINTF("RPL: Moving up within the DAG from rank %hu to %hu\n",
             dag->rank, new_rank);
      dag->rank = new_rank;
      dag->min_rank = new_rank; /* So far this is the lowest rank we know */
      rpl_reset_dio_timer(dag, 1);

      /* Remove old def-route and add the new */
      /* fix handling of destination prefix   */
      rpl_set_default_route(dag, from);

      if(new_parent) {
        remove_parents(dag, p, POISON_ROUTES);
      }
    }
  } else if(dio->dag_rank == dag->rank) {
    /* Message from a sibling. */
  } else {
    /* Message from a node at a longer distance from the root. If the
       node is in the parent list, we just remove it. */
    if(p != NULL && p->rank < dio->dag_rank) {
      PRINTF("RPL: Parent ");
      PRINT6ADDR(&p->addr);
      PRINTF(" has increased in rank from %hu to %hu. Removing it.\n",
             p->rank, dio->dag_rank);
      rpl_remove_parent(dag, p);
      if(RPL_PARENT_COUNT(dag) == 0) {
        dag->rank = INFINITE_RANK;
      }
    }
  }
}
/************************************************************************/
void
rpl_ds6_neighbor_callback(uip_ds6_nbr_t *nbr)
{
  if(nbr->isused) {
    PRINTF("RPL: Neighbor state %u: ", nbr->state);
    PRINT6ADDR(&nbr->ipaddr);
    PRINTF("\n");
  } else {
    rpl_dag_t *dag;
    rpl_parent_t *p;
    char acceptable_rank_increase;

    p = NULL;

    PRINTF("RPL: Removed neighbor ");
    PRINT6ADDR(&nbr->ipaddr);
    PRINTF("\n");

    dag = rpl_get_dag(RPL_ANY_INSTANCE);
    if(dag != NULL) {
      p = rpl_find_parent(dag, &nbr->ipaddr);
      if(p != NULL) {
        rpl_remove_parent(dag, p);
      }
    }
    if(dag != NULL && dag->def_route != NULL &&
       uip_ipaddr_cmp(&dag->def_route->ipaddr, &p->addr)) {
      p = rpl_preferred_parent(dag);
      if(p == NULL) {
	rpl_free_dag(dag);
	return;
      }

      acceptable_rank_increase = !dag->max_rankinc || 
	dag->of->increment_rank(p->rank, p) <= dag->min_rank + dag->max_rankinc;
      if(acceptable_rank_increase) {
        dag->rank = dag->of->increment_rank(p->rank, p);
        if(dag->rank < dag->min_rank) {
           dag->min_rank = dag->rank;
        }
        PRINTF("RPL: New rank is %hu, max is %hu\n",
		dag->rank, dag->min_rank + dag->max_rankinc);
        rpl_set_default_route(dag, &p->addr);
      } else {
        PRINTF("RPL: Cannot select the preferred parent\n");
        /* do local repair - jump down the DAG */
        remove_parents(dag, NULL, POISON_ROUTES);
        dag->rank = INFINITE_RANK;
        rpl_reset_dio_timer(dag, 1);
      }
    }
  }
}
