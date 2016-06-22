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
 *         The Minimum Rank with Hysteresis Objective Function (MRHOF), RFC6719
 *
 *         This implementation uses the estimated number of
 *         transmissions (ETX) as the additive routing metric,
 *         and also provides stubs for the energy metric.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

/**
 * \addtogroup uip6
 * @{
 */

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/nbr-table.h"
#include "net/link-stats.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/* RFC6551 and RFC6719 do not mandate the use of a specific formula to
 * compute the ETX value. This MRHOF implementation relies on the value
 * computed by the link-stats module. It has an optional feature,
 * RPL_MRHOF_CONF_SQUARED_ETX, that consists in squaring this value.
 * This basically penalizes bad links while preserving the semantics of ETX
 * (1 = perfect link, more = worse link). As a result, MRHOF will favor
 * good links over short paths. Recommended when reliability is a priority.
 * Without this feature, a hop with 50% PRR (ETX=2) is equivalent to two
 * perfect hops with 100% PRR (ETX=1+1=2). With this feature, the former
 * path obtains ETX=2*2=4 and the former ETX=1*1+1*1=2. */
#ifdef RPL_MRHOF_CONF_SQUARED_ETX
#define RPL_MRHOF_SQUARED_ETX RPL_MRHOF_CONF_SQUARED_ETX
#else /* RPL_MRHOF_CONF_SQUARED_ETX */
#define RPL_MRHOF_SQUARED_ETX 0
#endif /* RPL_MRHOF_CONF_SQUARED_ETX */

#if !RPL_MRHOF_SQUARED_ETX
/* Configuration parameters of RFC6719. Reject parents that have a higher
 * link metric than the following. The default value is 512 but we use 1024. */
#define MAX_LINK_METRIC     1024 /* Eq ETX of 8 */
/* Hysteresis of MRHOF: the rank must differ more than PARENT_SWITCH_THRESHOLD_DIV
 * in order to switch preferred parent. Default in RFC6719: 192, eq ETX of 1.5.
 * We use a more aggressive setting: 96, eq ETX of 0.75.
 */
#define PARENT_SWITCH_THRESHOLD 96 /* Eq ETX of 0.75 */
#else /* !RPL_MRHOF_SQUARED_ETX */
#define MAX_LINK_METRIC     2048 /* Eq ETX of 4 */
#define PARENT_SWITCH_THRESHOLD 160 /* Eq ETX of 1.25 (results in a churn comparable
to the threshold of 96 in the non-squared case) */
#endif /* !RPL_MRHOF_SQUARED_ETX */

/* Reject parents that have a higher path cost than the following. */
#define MAX_PATH_COST      32768   /* Eq path ETX of 256 */

/*---------------------------------------------------------------------------*/
static void
reset(rpl_dag_t *dag)
{
  PRINTF("RPL: Reset MRHOF\n");
}
/*---------------------------------------------------------------------------*/
#if RPL_WITH_DAO_ACK
static void
dao_ack_callback(rpl_parent_t *p, int status)
{
  if(status == RPL_DAO_ACK_UNABLE_TO_ADD_ROUTE_AT_ROOT) {
    return;
  }
  /* here we need to handle failed DAO's and other stuff */
  PRINTF("RPL: MRHOF - DAO ACK received with status: %d\n", status);
  if(status >= RPL_DAO_ACK_UNABLE_TO_ACCEPT) {
    /* punish the ETX as if this was 10 packets lost */
    link_stats_packet_sent(rpl_get_parent_lladdr(p), MAC_TX_OK, 10);
  } else if(status == RPL_DAO_ACK_TIMEOUT) { /* timeout = no ack */
    /* punish the total lack of ACK with a similar punishment */
    link_stats_packet_sent(rpl_get_parent_lladdr(p), MAC_TX_OK, 10);
  }
}
#endif /* RPL_WITH_DAO_ACK */
/*---------------------------------------------------------------------------*/
static uint16_t
parent_link_metric(rpl_parent_t *p)
{
  const struct link_stats *stats = rpl_get_parent_link_stats(p);
  if(stats != NULL) {
#if RPL_MRHOF_SQUARED_ETX
    uint32_t squared_etx = ((uint32_t)stats->etx * stats->etx) / LINK_STATS_ETX_DIVISOR;
    return (uint16_t)MIN(squared_etx, 0xffff);
#else /* RPL_MRHOF_SQUARED_ETX */
  return stats->etx;
#endif /* RPL_MRHOF_SQUARED_ETX */
  }
  return 0xffff;
}
/*---------------------------------------------------------------------------*/
static uint16_t
parent_path_cost(rpl_parent_t *p)
{
  uint32_t path_cost = 0;

  if(p == NULL || p->dag == NULL || p->dag->instance == NULL) {
    return 0xffff;
  }

#if RPL_WITH_MC
  rpl_metric_object_t *metric_object;
  /* Handle the different MC object types */
  metric_object = rpl_find_metric_any_routing_type(&p->mc, RPL_DAG_MC_METRIC_OBJECT);
  if(metric_object == NULL){
    path_cost = p->rank + parent_link_metric(p);
  }
  else{
    switch(metric_object->type){
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ENERGY)
    case RPL_DAG_MC_ENERGY:
      path_cost = metric_object->obj.energy.energy_est + parent_link_metric(p);
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ENERGY) */
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_HOPCOUNT)
    case RPL_DAG_MC_HOPCOUNT:
      path_cost = p->rank + 1;
      break;
#endif
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_LATENCY)
    case RPL_DAG_MC_LATENCY:
      
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_LATENCY) */
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ETX)
    case RPL_DAG_MC_ETX:
      /*
      As specified in RFC 6719, Section 3.4, Approximation of ETX MUST be made
      from advertised rank and ETX Metric object in MC must be ignored.
      */
      path_cost = p->rank + parent_link_metric(p);
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ETX) */
    default:
      PRINTF("RPL: MRHOF, Unhandled DAG MC Object type: %u\n", (unsigned)metric_object->type);
      break;
    }
  }
#else /* RPL_WITH_MC */
  path_cost = p->rank + parent_link_metric(p);
#endif /* RPL_WITH_MC */

  /* path cost upper bound: 0xffff */
  return MIN(path_cost, 0xffff);
}
/*---------------------------------------------------------------------------*/
static rpl_rank_t
rank_via_parent(rpl_parent_t *p)
{
  uint16_t min_hoprankinc;
  uint16_t path_cost;

  if(p == NULL || p->dag == NULL || p->dag->instance == NULL) {
    return INFINITE_RANK;
  }

  min_hoprankinc = p->dag->instance->min_hoprankinc;
  path_cost = parent_path_cost(p);

  /* Rank lower-bound: parent rank + min_hoprankinc */
  return MAX(MIN((uint32_t)p->rank + min_hoprankinc, 0xffff), path_cost);
}
/*---------------------------------------------------------------------------*/
static int
parent_is_acceptable(rpl_parent_t *p)
{
  uint16_t link_metric = parent_link_metric(p);
  uint16_t path_cost = parent_path_cost(p);
  /* Exclude links with too high link metrics or path cost (RFC6719, 3.2.2) */
  return link_metric <= MAX_LINK_METRIC && path_cost <= MAX_PATH_COST;
}
/*---------------------------------------------------------------------------*/
static int
parent_has_usable_link(rpl_parent_t *p)
{
  uint16_t link_metric = parent_link_metric(p);
  /* Exclude links with too high link metrics  */
  return link_metric <= MAX_LINK_METRIC;
}
/*---------------------------------------------------------------------------*/
static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  rpl_dag_t *dag;
  uint16_t p1_cost;
  uint16_t p2_cost;
  int p1_is_acceptable;
  int p2_is_acceptable;

  p1_is_acceptable = p1 != NULL && parent_is_acceptable(p1);
  p2_is_acceptable = p2 != NULL && parent_is_acceptable(p2);

  if(!p1_is_acceptable) {
    return p2_is_acceptable ? p2 : NULL;
  }
  if(!p2_is_acceptable) {
    return p1_is_acceptable ? p1 : NULL;
  }

  dag = p1->dag; /* Both parents are in the same DAG. */
  p1_cost = parent_path_cost(p1);
  p2_cost = parent_path_cost(p2);

  /* Maintain stability of the preferred parent in case of similar ranks. */
  if(p1 == dag->preferred_parent || p2 == dag->preferred_parent) {
    if(p1_cost < p2_cost + PARENT_SWITCH_THRESHOLD &&
       p1_cost > p2_cost - PARENT_SWITCH_THRESHOLD) {
      return dag->preferred_parent;
    }
  }

  return p1_cost < p2_cost ? p1 : p2;
}
/*---------------------------------------------------------------------------*/
static rpl_dag_t *
best_dag(rpl_dag_t *d1, rpl_dag_t *d2)
{
  if(d1->grounded != d2->grounded) {
    return d1->grounded ? d1 : d2;
  }

  if(d1->preference != d2->preference) {
    return d1->preference > d2->preference ? d1 : d2;
  }

  return d1->rank < d2->rank ? d1 : d2;
}
/*---------------------------------------------------------------------------*/
#if !RPL_WITH_MC
static void
update_metric_container(rpl_instance_t *instance)
{
  metric_object = rpl_find_metric_any_routing_type(&instance->mc, RPL_DAG_MC_METRIC_OBJECT);
  if(metric_object != NULL){
    rpl_remove_metric(metric_object);
  }
}
#else /* RPL_WITH_MC */
static void
update_metric_container(rpl_instance_t *instance)
{
  rpl_metric_object_t *metric_object, *parent_metric_object;
  uint16_t path_cost;
  rpl_dag_t *dag;
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ENERGY)
  uint8_t type;
#endif

  dag = instance->current_dag;
  if(dag == NULL || !dag->joined) {
    PRINTF("RPL: Cannot update the metric container when not joined\n");
    return;
  }

  metric_object = rpl_find_metric_any_routing_type(&instance->mc, RPL_DAG_MC_METRIC_OBJECT);
  parent_metric_object = rpl_find_metric_any_routing_type(&dag->preferred_parent->mc, RPL_DAG_MC_METRIC_OBJECT);

  if(dag->rank == ROOT_RANK(instance)) {
    /* Configure MC at root only, other nodes are auto-configured when joining */
#if RPL_DAG_MC != RPL_DAG_MC_NONE
    if(metric_object == NULL){
      metric_object = rpl_alloc_metric(&instance->mc);
      if(metric_object == NULL){
        PRINTF("RPL: Cannot update the metric container, no metric object available\n");
        return;
      }
      metric_object->type = RPL_DAG_MC;
      metric_object->flags = RPL_DAG_MC_FLAG_P;
      metric_object->aggr = RPL_DAG_MC_AGGR_ADDITIVE;
      metric_object->prec = 0;

      switch(metric_object->type){
        case RPL_DAG_MC_HOPCOUNT:
          metric_object->obj.hopcount.count = 1;
          break;
        case RPL_DAG_MC_LATENCY:
          metric_object->obj.latency.subobject = 0;
          break;
      }
    }
#endif /* RPL_DAG_MC != RPL_DAG_MC_NONE */
    path_cost = dag->rank;
  } else {
    path_cost = parent_path_cost(dag->preferred_parent);
  }

  /* Handle the different MC object types */
  switch(metric_object->type){
    case RPL_DAG_MC_NONE:
      
      break;
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ENERGY)
    case RPL_DAG_MC_ENERGY:
      metric_object->length = 2;
#ifdef RPL_DAG_MC_ENERGY_TYPE
      type = RPL_DAG_MC_ENERGY_TYPE;
#else /* RPL_DAG_MC_ENERGY_TYPE */
      if(dag->rank == ROOT_RANK(instance)) {
        type = RPL_DAG_MC_ENERGY_TYPE_MAINS;
      } else {
        type = RPL_DAG_MC_ENERGY_TYPE_BATTERY;
      }
#endif /* RPL_DAG_MC_ENERGY_TYPE */
      metric_object->obj.energy.flags = type << RPL_DAG_MC_ENERGY_FLAG_TYPE;
      metric_object->obj.energy.energy_est = path_cost;
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ENERGY) */
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_HOPCOUNT)
    case RPL_DAG_MC_HOPCOUNT:
      metric_object->obj.hopcount.count = parent_metric_object->obj.hopcount.count + 1;
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_HOPCOUNT) */
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_LATENCY)
    case RPL_DAG_MC_LATENCY:
      
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_LATENCY) */
#if RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ETX)
    case RPL_DAG_MC_ETX:
      PRINTF("RPL: Removed ETX Metric Obj. in MC, Rank used instead\n");
      /*
      As specified in RFC 6719, Section 3.4, ETX metric MUST NOT be advertised
      in the Metric Container.
      */
      rpl_remove_metric(metric_object);
      break;
#endif /* RPL_IS_METRIC_SUPPORTED(RPL_DAG_MC_ETX) */
    default:
      PRINTF("RPL: MRHOF, Unhandled DAG MC Object type: %u\n", (unsigned)metric_object->type);
      return;
  }
}
#endif /* RPL_WITH_MC */
/*---------------------------------------------------------------------------*/
rpl_of_t rpl_mrhof = {
  reset,
#if RPL_WITH_DAO_ACK
  dao_ack_callback,
#endif
  parent_link_metric,
  parent_has_usable_link,
  parent_path_cost,
  rank_via_parent,
  best_parent,
  best_dag,
  update_metric_container,
  RPL_OCP_MRHOF
};

/** @}*/
