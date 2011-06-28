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
 * \file
 *	ContikiRPL - an implementation of the routing protocol for low 
 *	power and lossy networks. See: draft-ietf-roll-rpl-17.
 * \author
 *	Joakim Eriksson <joakime@sics.se> & Nicolas Tsiftes <nvt@sics.se>
 *
 */

#ifndef RPL_H
#define RPL_H

#include "lib/list.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "sys/ctimer.h"

/* set to 1 for some statistics on trickle / DIO */
#ifndef RPL_CONF_STATS
#define RPL_CONF_STATS 0
#endif /* RPL_CONF_STATS */

/* 
 * Select routing metric supported at runtime. This must be a valid
 * DAG Metric Container Object Type (see below). Currently, we only 
 * support RPL_DAG_MC_ETX and RPL_DAG_MC_ENERGY.
 */
#ifdef RPL_CONF_DAG_MC
#define RPL_DAG_MC RPL_CONF_DAG_MC
#else
#define RPL_DAG_MC RPL_DAG_MC_ETX
#endif /* RPL_CONF_DAG_MC */

/*
 * The objective function used by RPL is configurable through the 
 * RPL_CONF_OF parameter. This should be defined to be the name of an 
 * rpl_of_t object linked into the system image, e.g., rpl_of0.
 */
#ifdef RPL_CONF_OF
#define RPL_OF RPL_CONF_OF
#else
/* ETX is the default objective function. */
#define RPL_OF rpl_of_etx
#endif /* RPL_CONF_OF */

/* This value decides which DAG instance we should participate in by default. */
#define RPL_DEFAULT_INSTANCE		0

/* This value is used to access an arbitrary DAG. It will likely be 
   replaced when we support multiple DAGs more. */
#define RPL_ANY_INSTANCE               -1
/*---------------------------------------------------------------------------*/
/* The amount of parents that this node has in a particular DAG. */
#define RPL_PARENT_COUNT(dag)   list_length((dag)->parents)
/*---------------------------------------------------------------------------*/
typedef uint16_t rpl_rank_t;
typedef uint8_t rpl_lifetime_t;
typedef uint16_t rpl_ocp_t;

/*---------------------------------------------------------------------------*/
/* DAG Metric Container Object Types, to be confirmed by IANA. */
#define RPL_DAG_MC_NONE			0 /* Local identifier for empty MC */
#define RPL_DAG_MC_NSA                  1 /* Node State and Attributes */
#define RPL_DAG_MC_ENERGY               2 /* Node Energy */
#define RPL_DAG_MC_HOPCOUNT             3 /* Hop Count */
#define RPL_DAG_MC_THROUGHPUT           4 /* Throughput */
#define RPL_DAG_MC_LATENCY              5 /* Latency */
#define RPL_DAG_MC_LQL                  6 /* Link Quality Level */
#define RPL_DAG_MC_ETX                  7 /* Expected Transmission Count */
#define RPL_DAG_MC_LC                   8 /* Link Color */

/* DAG Metric Container flags. */
#define RPL_DAG_MC_FLAG_P               0x8
#define RPL_DAG_MC_FLAG_C               0x4
#define RPL_DAG_MC_FLAG_O               0x2
#define RPL_DAG_MC_FLAG_R               0x1

/* DAG Metric Container aggregation mode. */
#define RPL_DAG_MC_AGGR_ADDITIVE        0
#define RPL_DAG_MC_AGGR_MAXIMUM         1
#define RPL_DAG_MC_AGGR_MINIMUM         2
#define RPL_DAG_MC_AGGR_MULTIPLICATIVE  3

/* The bit index within the flags field of
   the rpl_metric_object_energy structure. */
#define RPL_DAG_MC_ENERGY_INCLUDED	3
#define RPL_DAG_MC_ENERGY_TYPE		1
#define RPL_DAG_MC_ENERGY_ESTIMATION	0

#define RPL_DAG_MC_ENERGY_TYPE_MAINS		0
#define RPL_DAG_MC_ENERGY_TYPE_BATTERY		1
#define RPL_DAG_MC_ENERGY_TYPE_SCAVENGING	2

struct rpl_metric_object_energy {
  uint8_t flags;
  uint8_t energy_est;
};

/* Logical representation of a DAG Metric Container. */
struct rpl_metric_container {
  uint8_t type;
  uint8_t flags;
  uint8_t aggr;
  uint8_t prec;
  uint8_t length;
  union metric_object {
    struct rpl_metric_object_energy energy;
    uint16_t etx;
  } obj;
};
typedef struct rpl_metric_container rpl_metric_container_t;
/*---------------------------------------------------------------------------*/
struct rpl_dag;
/*---------------------------------------------------------------------------*/
struct rpl_parent {
  struct rpl_parent *next;
  struct rpl_dag *dag;
  rpl_metric_container_t mc;
  uip_ipaddr_t addr;
  rpl_rank_t rank;
  uint8_t link_metric;
  uint8_t dtsn;
  uint8_t updated;
};
typedef struct rpl_parent rpl_parent_t;
/*---------------------------------------------------------------------------*/
/*
 * API for RPL objective functions (OF)
 *
 * reset(dag)
 *
 *  Resets the objective function state for a specific DAG. This function is
 *  called when doing a global repair on the DAG.
 *
 * parent_state_callback(parent, known, etx)
 *
 *  Receives link-layer neighbor information. The parameter "known" is set
 *  either to 0 or 1. The "etx" parameter specifies the current
 *  ETX(estimated transmissions) for the neighbor.
 *
 * best_parent(parent1, parent2)
 *
 *  Compares two parents and returns the best one, according to the OF.
 *
 * calculate_rank(parent, base_rank)
 *
 *  Calculates a rank value using the parent rank and a base rank.
 *  If "parent" is NULL, the objective function selects a default increment
 *  that is adds to the "base_rank". Otherwise, the OF uses information known
 *  about "parent" to select an increment to the "base_rank".
 *
 * update_metric_container(dag)
 *
 *  Updates the metric container for outgoing DIOs in a certain DAG.
 *  If the objective function of the DAG does not use metric containers, 
 *  the function should set the object type to RPL_DAG_MC_NONE.
 */
struct rpl_of {
  void (*reset)(struct rpl_dag *);
  void (*parent_state_callback)(rpl_parent_t *, int, int);
  rpl_parent_t *(*best_parent)(rpl_parent_t *, rpl_parent_t *);
  rpl_rank_t (*calculate_rank)(rpl_parent_t *, rpl_rank_t);
  void (*update_metric_container)(struct rpl_dag *);
  rpl_ocp_t ocp;
};
typedef struct rpl_of rpl_of_t;
/*---------------------------------------------------------------------------*/
/* RPL DIO prefix suboption */
struct rpl_prefix {
  uip_ipaddr_t prefix;
  uint32_t lifetime;
  uint8_t length;
  uint8_t flags;
};
typedef struct rpl_prefix rpl_prefix_t;
/*---------------------------------------------------------------------------*/
/* Directed Acyclic Graph */
struct rpl_dag {
  /* DAG configuration */
  rpl_metric_container_t mc;
  rpl_of_t *of;
  uip_ipaddr_t dag_id;
  /* The current default router - used for routing "upwards" */
  uip_ds6_defrt_t *def_route;
  rpl_rank_t rank;
  rpl_rank_t min_rank; /* should be reset per DODAG iteration! */
  uint8_t dtsn_out;
  uint8_t instance_id;
  uint8_t version;
  uint8_t grounded;
  uint8_t mop;
  uint8_t preference;
  uint8_t dio_intdoubl;
  uint8_t dio_intmin;
  uint8_t dio_redundancy;
  rpl_rank_t max_rankinc;
  rpl_rank_t min_hoprankinc;
  uint8_t used;
  uint8_t default_lifetime;
  uint16_t lifetime_unit; /* lifetime in seconds = l_u * d_l */
  /* live data for the DAG */
  uint8_t joined;
  uint8_t dio_intcurrent;
  uint8_t dio_send; /* for keeping track of which mode the timer is in 
*/
  uint8_t dio_counter;
#if RPL_CONF_STATS
  uint16_t dio_totint;
  uint16_t dio_totsend;
  uint16_t dio_totrecv;
#endif /* RPL_CONF_STATS */
  uint32_t dio_next_delay; /* delay for completion of dio interval */
  struct ctimer dio_timer;
  struct ctimer dao_timer;
  rpl_parent_t *preferred_parent;
  void *parent_list;
  list_t parents;
  rpl_prefix_t prefix_info;
};
typedef struct rpl_dag rpl_dag_t;
/*---------------------------------------------------------------------------*/
/* Public RPL functions. */
void rpl_init(void);
rpl_dag_t *rpl_set_root(uip_ipaddr_t *);
int rpl_set_prefix(rpl_dag_t *dag, uip_ipaddr_t *prefix, int len);
int rpl_repair_dag(rpl_dag_t *dag);
int rpl_set_default_route(rpl_dag_t *dag, uip_ipaddr_t *from);
rpl_dag_t *rpl_get_dag(int instance_id);
/*---------------------------------------------------------------------------*/
#endif /* RPL_H */
