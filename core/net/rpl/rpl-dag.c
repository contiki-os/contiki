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
 * $Id: rpl-dag.c,v 1.2 2010/05/04 14:41:55 joxe Exp $
 */
/**
 * \file
 *         Logic for Directed Acyclic Graphs in RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/rpl/rpl.h"

#include "net/uip.h"
#include "net/rime/ctimer.h"
#include "lib/list.h"
#include "lib/memb.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_ANNOTATE

#include "net/uip-debug.h"

/************************************************************************/
#ifdef RPL_CONF_OBJECTIVE_FUNCTION
#define RPL_OF                  RPL_CONF_OBJECTIVE_FUNCTION
#else
extern rpl_of_t rpl_of0;
#define RPL_OF                  rpl_of0
#endif /* RPL_CONF_OBJECTIVE_FUNCTION */

#ifndef RPL_CONF_MAX_DAG_ENTRIES
#define RPL_MAX_DAG_ENTRIES     2
#else
#define RPL_MAX_DAG_ENTRIES     RPL_CONF_MAX_DAG_ENTRIES
#endif /* !RPL_CONF_MAX_DAG_ENTRIES */

#ifndef RPL_CONF_MAX_NEIGHBORS
#define RPL_MAX_NEIGHBORS       10
#else
#define RPL_MAX_NEIGHBORS       RPL_CONF_MAX_NEIGHBORS
#endif /* !RPL_CONF_MAX_NEIGHBORS */
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
/* Allocate neighbors from the same static MEMB chunk to reduce memory waste. */
MEMB(neighbor_memb, struct rpl_neighbor, RPL_MAX_NEIGHBORS);

static rpl_dag_t dag_table[RPL_MAX_DAG_ENTRIES];
/************************************************************************/
static void
poison_routes(rpl_dag_t *dag, rpl_neighbor_t *exception)
{
  rpl_neighbor_t *p, *p2;

  PRINTF("RPL: Poisoning routes\n");

  for(p = list_head(dag->neighbors); p != NULL;) {
   if(p != exception && RPL_NEIGHBOR_IS_PARENT(dag, p)) {
     ANNOTATE("#L %u 0\n", p->addr.u8[sizeof(p->addr) - 1]);

     /* Send no-DAOs to old parents. */
     dao_output(p, ZERO_LIFETIME);

     p2 = p->next;
     rpl_remove_neighbor(dag, p);
     p = p2;
   } else {
     p = p->next;
   }
 }
}
/************************************************************************/
int
rpl_set_root(uip_ipaddr_t *dag_id)
{
  rpl_dag_t *dag;

  dag = rpl_get_dag(RPL_DEFAULT_OCP);
  if(dag != NULL) {
    PRINTF("RPL: Dropping a joined DAG when setting this node as root");
    rpl_free_dag(dag);
  }

  dag = rpl_alloc_dag();
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG\n");
    return -1;
  }

  dag->joined = 1;
  dag->grounded = RPL_GROUNDED;
  dag->rank = ROOT_RANK;
  dag->of = rpl_find_of(RPL_DEFAULT_OCP);
  dag->best_parent = NULL;

  memcpy(&dag->dag_id, dag_id, sizeof(dag->dag_id));

  dag->dio_intdoubl = DEFAULT_DIO_INTERVAL_DOUBLINGS;
  dag->dio_intmin = DEFAULT_DIO_INTERVAL_MIN;
  dag->dio_redundancy = DEFAULT_DIO_REDUNDANCY;

  PRINTF("RPL: Node set to be a DAG root with DAG ID ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  rpl_reset_dio_timer(dag, 1);

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
    dag->def_route = uip_ds6_defrt_add(from, 100000);
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
      dag_table[i].neighbors = &dag_table[i].neighbor_list;
      dag_table[i].def_route = NULL;
      list_init(dag_table[i].neighbors);
      return &dag_table[i];
    }
  }
  return NULL;
}
/************************************************************************/
void
rpl_free_dag(rpl_dag_t *dag)
{
  uip_ds6_route_t *rep;

  PRINTF("RPL: Leaving the DAG ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  rep = uip_ds6_route_lookup(&dag->dag_id);
  if(rep != NULL) {
    uip_ds6_route_rm(rep);
  }

  rpl_set_default_route(dag, NULL);

  poison_routes(dag, NULL);

  ctimer_stop(&dag->dio_timer);
  ctimer_stop(&dag->dao_timer);
  dag->used = 0;
  dag->joined = 0;
}
/************************************************************************/
rpl_neighbor_t *
rpl_add_neighbor(rpl_dag_t *dag, uip_ipaddr_t *addr)
{
  rpl_neighbor_t *n;

  n = memb_alloc(&neighbor_memb);
  if(n == NULL) {
    return NULL;
  }

  memcpy(&n->addr, addr, sizeof(n->addr));
  n->local_confidence = 0;
  n->dag = dag;

  list_add(dag->neighbors, n);

  /* Draw a line between the node and its parent in Cooja. */
  ANNOTATE("#L %u 1\n",
           addr->u8[sizeof(*addr) - 1]);

  return n;
}
/************************************************************************/
rpl_neighbor_t *
rpl_find_neighbor(rpl_dag_t *dag, uip_ipaddr_t *addr)
{
  rpl_neighbor_t *n;

  for(n = list_head(dag->neighbors); n != NULL; n = n->next) {
    if(uip_ipaddr_cmp(&n->addr, addr)) {
      return n;
    }
  }

  return NULL;
}

/************************************************************************/
rpl_neighbor_t *
rpl_first_parent(rpl_dag_t *dag)
{
  //return list_head(dag->parents);

  rpl_neighbor_t *n;

    for(n = list_head(dag->neighbors); n != NULL; n = n->next) {
      if(RPL_NEIGHBOR_IS_PARENT(dag, n)) {
          return n;
      }
    }
    return n;
}
/************************************************************************/
rpl_neighbor_t *
rpl_find_best_parent(rpl_dag_t *dag)
{
  rpl_neighbor_t *n;
  rpl_neighbor_t *best;

  best = NULL;
  for(n = list_head(dag->neighbors); n != NULL; n = n->next) {
    if(RPL_NEIGHBOR_IS_PARENT(dag, n)) {
      if(best == NULL) {
        best = n;
      } else {
        best = dag->of->best_parent(best, n);
      }
    }
  }
  dag->best_parent = best; /* Cached value. */
  return best;
}
/************************************************************************/
int
rpl_remove_neighbor(rpl_dag_t *dag, rpl_neighbor_t *parent)
{
  uip_ds6_defrt_t *defrt;

  ANNOTATE("#L %u 0\n",
           parent->addr.u8[sizeof(parent->addr) - 1]);

  /* Remove uIPv6 routes that have this neighbor as the next hop. **/
  uip_ds6_route_rm_by_nexthop(&parent->addr);
  defrt = uip_ds6_defrt_lookup(&parent->addr);
  if(defrt != NULL) {
    PRINTF("RPL: Removing default route ");
    PRINT6ADDR(&parent->addr);
    PRINTF("\n");
    uip_ds6_defrt_rm(defrt);
    dag->def_route = NULL;
  }

  PRINTF("RPL: Removing neighbor ");
  PRINT6ADDR(&parent->addr);
  PRINTF("\n");

  list_remove(dag->neighbors, parent);
  memb_free(&neighbor_memb, parent);
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
rpl_dag_t *
rpl_find_dag(unsigned char aucIndex)
{
  int i;

  for(i = aucIndex; i < RPL_MAX_DAG_ENTRIES; i++) {
    if(dag_table[i].joined ) {
      return &dag_table[i];
    }
  }
  return NULL;
}
/************************************************************************/
rpl_of_t *
rpl_find_of(rpl_ocp_t ocp)
{
  if(RPL_OF.ocp == ocp) {
    return &RPL_OF;
  }
  return NULL;
}
/************************************************************************/
void
rpl_join_dag(rpl_dag_t *dag)
{
  dag->joined = 1;

  rpl_reset_dio_timer(dag, 1);
}
/************************************************************************/
static void
join_dag(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_dag_t *dag;
  rpl_neighbor_t *n;
  rpl_of_t *of;

  dag = rpl_alloc_dag();
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG object!\n");
    return;
  }

  n = rpl_add_neighbor(dag, from);
  PRINTF("RPL: Adding ");
  PRINT6ADDR(from);
  PRINTF(" as a parent: ");
  if(n == NULL) {
    PRINTF("failed\n");
    return;
  }
  PRINTF("succeeded\n");

  n->local_confidence = 0; /* Extract packet LQI here. */
  n->rank = dio->dag_rank;

  /* Determine the objective function by using the
     objective code point of the DIO. */
  of = rpl_find_of(dio->ocp);
  if(of == NULL) {
    PRINTF("RPL: DIO for DAG instance %d does not specify a supported OF\n",
        dio->instance_id);
    return;
  }

  dag->used = 1;
  dag->of = of;
  dag->preference = dio->preference;
  dag->grounded = dio->grounded;
  dag->instance_id = dio->instance_id;
  dag->rank = dag->of->increment_rank(dio->dag_rank, n);
  dag->min_rank = dag->rank; /* So far this is the lowest rank we know */
  dag->sequence_number = dio->sequence_number;
  dag->dio_intdoubl = dio->dag_intdoubl;
  dag->dio_intmin = dio->dag_intmin;
  dag->dio_redundancy = dio->dag_redund;

  dag->max_rankinc = dio->dag_max_rankinc;
  dag->min_hoprankinc = dio->dag_min_hoprankinc;

  memcpy(&dag->dag_id, &dio->dag_id, sizeof(dio->dag_id));

  rpl_join_dag(dag);

  PRINTF("RPL: Joined DAG with instance ID %d, rank %d, DAG ID ",
         dio->instance_id, dag->rank);
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  rpl_set_default_route(dag, from);

  if(dio->dst_adv_trigger) {
    rpl_schedule_dao(dag);
  } else {
    PRINTF("RPL: dst_adv_trigger not set in incoming DIO!\n");
  }
}
/************************************************************************/
static void
global_repair(uip_ipaddr_t *from, rpl_dag_t *dag, rpl_dio_t *dio)
{
  rpl_neighbor_t *n;

  poison_routes(dag, NULL);
  dag->sequence_number = dio->sequence_number;
  if((n = rpl_add_neighbor(dag, from)) == NULL) {
    PRINTF("RPL: Failed to add a parent during the global repair\n");
    dag->rank = INFINITE_RANK;
  } else {
    rpl_set_default_route(dag, from);
    dag->rank = dag->of->increment_rank(dio->dag_rank, n);
    rpl_reset_dio_timer(dag, 1);
  }
  PRINTF("RPL: Participating in a global DAG repair. New DAG sequence number: %d NewRank: %d\n",
         dag->sequence_number, dag->rank);

}
/************************************************************************/
int
rpl_repair_dag(rpl_dag_t *dag)
{
  if(dag->rank == ROOT_RANK) {
    dag->sequence_number++;
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
  rpl_neighbor_t *n;
  uint8_t new_rank;
  uint8_t new_parent;

  /* if(from->u8[15] != 0xe7) { */
  /*   printf("last byte did not match e7 %x\n", from->u8[15]); */
  /*   return; */
  /* } */

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

  if(dio->sequence_number > dag->sequence_number) {
    global_repair(from, dag, dio);
    return;
  }

  /* This DIO pertains to a DAG that we are already part of. */
  n = rpl_find_neighbor(dag, from);
  if(n != NULL) {
    if(dio->dst_adv_trigger) {
      rpl_schedule_dao(dag);
    } else {
      PRINTF("RPL: dst_adv_trigger is not set in incoming DIO\n");
    }

    if(n->rank > dio->dag_rank) {
      n->rank = dio->dag_rank;
      rpl_reset_dio_timer(dag, 1);
    } else if(n->rank < dio->dag_rank) {
      PRINTF("RPL: Existing parent ");
      PRINT6ADDR(from);
      PRINTF(" got a higher rank (%d -> %d)\n",
             n->rank, dio->dag_rank);
      n->rank = dio->dag_rank;
      if(RPL_PARENT_COUNT(dag) > 1) {
        /* Since we have alternative parents, we can simply drop this one. */
        rpl_remove_neighbor(dag, n);
      } else if(dag->of->increment_rank(dio->dag_rank, n) <= dag->min_rank + dag->max_rankinc) {
        dag->rank = dag->of->increment_rank(dio->dag_rank, n);
        PRINTF("RPL: New rank is %i, max is %i\n", dag->rank, dag->min_rank + dag->max_rankinc);
        rpl_set_default_route(dag, &n->addr);
      } else {
        PRINTF("RPL: Cannot find acceptable best neighbor\n");
        /* do local repair - jump down the DAG */
        poison_routes(dag, NULL);
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
    if(dag->min_rank + dag->max_rankinc < dag->of->increment_rank(dio->dag_rank, n)) {
        PRINTF("RPL: Could not add parent, resulting rank too high\n");
        return;
    }
    new_parent = 0;
    if(n == NULL) {
      n = rpl_add_neighbor(dag, from);
      if(n == NULL) {
        PRINTF("RPL: Could not add parent\n");
        return;
      }

      n->rank = dio->dag_rank;
      PRINTF("RPL: New parent with rank %d ", n->rank);
      PRINT6ADDR(from);
      PRINTF("\n");
      new_parent = 1;
    }

    new_rank = dag->of->increment_rank(dio->dag_rank, n);
    if(new_rank < dag->rank) {
      PRINTF("RPL: Moving up within the DAG from rank %d to %d\n",
             dag->rank, new_rank);
      dag->rank = new_rank;
      dag->min_rank = new_rank; /* So far this is the lowest rank we know */
      rpl_reset_dio_timer(dag, 1);

      /* Remove old def-route and add the new */
      /* fix handling of destination prefix   */
      if(dag->grounded) {
        rpl_set_default_route(dag, from);
      }

      if(new_parent) {
        poison_routes(dag, n);
      }
    }
  } else if(dio->dag_rank == dag->rank) {
    /* Message from a sibling. */
  } else {
    /* Message from a node at a longer distance from the root. If the
       node is in the parent list, we just remove it. */
    if(n != NULL && n->rank < dio->dag_rank) {
      PRINTF("RPL: Parent ");
      PRINT6ADDR(&n->addr);
      PRINTF(" has increased in rank from %d to %d. Removing it.\n",
             n->rank, dio->dag_rank);
      rpl_remove_neighbor(dag, n);
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
    rpl_neighbor_t *n;
    n = NULL;

    PRINTF("RPL: Removed neighbor ");
    PRINT6ADDR(&nbr->ipaddr);
    PRINTF("\n");

    dag = rpl_get_dag(RPL_ANY_INSTANCE);
    if(dag != NULL) {
      n = rpl_find_neighbor(dag, &nbr->ipaddr);
      if(n != NULL) {
        rpl_remove_neighbor(dag, n);
      }
    }
    if(dag->def_route != NULL &&
       uip_ipaddr_cmp(&dag->def_route->ipaddr, &n->addr)) {
      n = rpl_find_best_parent(dag);
      if(n != NULL && dag->of->increment_rank(n->rank, n) <= dag->min_rank + dag->max_rankinc) {
        dag->rank = dag->of->increment_rank(n->rank, n);
        if(dag->rank < dag->min_rank) {
           dag->min_rank = dag->rank;
        }
        PRINTF("New rank is %i, max is %i\n", dag->rank, dag->min_rank + dag->max_rankinc);
        rpl_set_default_route(dag, &n->addr);
      } else {
        PRINTF("RPL: Cannot find the best neighbor\n");
        /* do local repair - jump down the DAG */
        poison_routes(dag, NULL);
        dag->rank = INFINITE_RANK;
        rpl_reset_dio_timer(dag, 1);
      }
    }
  }
}
