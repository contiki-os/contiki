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
 *	Public API declarations for ContikiRPL.
 * \author
 *	Joakim Eriksson <joakime@sics.se> & Nicolas Tsiftes <nvt@sics.se>
 *
 */

#ifndef RPL_H
#define RPL_H

#include "rpl-conf.h"

#include "lib/list.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "sys/ctimer.h"

/*---------------------------------------------------------------------------*/
typedef uint16_t rpl_rank_t;
typedef uint16_t rpl_ocp_t;
/*---------------------------------------------------------------------------*/
/* IANA Routing Metric/Constraint Type as defined in RFC6551 */
#define RPL_DAG_MC_NONE                 0 /* Local identifier for empty MC */
#define RPL_DAG_MC_NSA                  1 /* Node State and Attributes */
#define RPL_DAG_MC_ENERGY               2 /* Node Energy */
#define RPL_DAG_MC_HOPCOUNT             3 /* Hop Count */
#define RPL_DAG_MC_THROUGHPUT           4 /* Throughput */
#define RPL_DAG_MC_LATENCY              5 /* Latency */
#define RPL_DAG_MC_LQL                  6 /* Link Quality Level */
#define RPL_DAG_MC_ETX                  7 /* Expected Transmission Count */
#define RPL_DAG_MC_LC                   8 /* Link Color */

/* IANA Routing Metric/Constraint Common Header Flag field as defined in RFC6551 (bit indexes) */
#define RPL_DAG_MC_FLAG_P               5
#define RPL_DAG_MC_FLAG_C               6
#define RPL_DAG_MC_FLAG_O               7
#define RPL_DAG_MC_FLAG_R               8

/* IANA Routing Metric/Constraint Common Header A Field as defined in RFC6551 */
#define RPL_DAG_MC_AGGR_ADDITIVE        0
#define RPL_DAG_MC_AGGR_MAXIMUM         1
#define RPL_DAG_MC_AGGR_MINIMUM         2
#define RPL_DAG_MC_AGGR_MULTIPLICATIVE  3

/* The bit index within the flags field of the rpl_metric_object_energy structure. */
#define RPL_DAG_MC_ENERGY_INCLUDED      3
#define RPL_DAG_MC_ENERGY_TYPE          1
#define RPL_DAG_MC_ENERGY_ESTIMATION    0

/* IANA Node Type Field as defined in RFC6551 */
#define RPL_DAG_MC_ENERGY_TYPE_MAINS       0
#define RPL_DAG_MC_ENERGY_TYPE_BATTERY     1
#define RPL_DAG_MC_ENERGY_TYPE_SCAVENGING  2

/* IANA Objective Code Point as defined in RFC6550 */
#define RPL_OCP_OF0     0
#define RPL_OCP_MRHOF   1

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
struct rpl_instance;
struct rpl_dag;
/*---------------------------------------------------------------------------*/
#define RPL_PARENT_FLAG_UPDATED           0x1
#define RPL_PARENT_FLAG_LINK_METRIC_VALID 0x2

struct rpl_parent {
  struct rpl_dag *dag;
#if RPL_WITH_MC
  rpl_metric_container_t mc;
#endif /* RPL_WITH_MC */
  rpl_rank_t rank;
  uint8_t dtsn;
  uint8_t flags;
};
typedef struct rpl_parent rpl_parent_t;
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
  uip_ipaddr_t dag_id;
  rpl_rank_t min_rank; /* should be reset per DAG iteration! */
  uint8_t version;
  uint8_t grounded;
  uint8_t preference;
  uint8_t used;
  /* live data for the DAG */
  uint8_t joined;
  rpl_parent_t *preferred_parent;
  rpl_rank_t rank;
  struct rpl_instance *instance;
  rpl_prefix_t prefix_info;
  uint32_t lifetime;
};
typedef struct rpl_dag rpl_dag_t;
typedef struct rpl_instance rpl_instance_t;
/*---------------------------------------------------------------------------*/
/* Security Section for DIS reply and DAO forward (RFC 6550 10.2)*/
#if RPL_SECURITY
struct rpl_sec_section {
  uint8_t responded;
  uint8_t timestamp;
  uint8_t kim;
  uint8_t lvl;
  uint32_t key_source[2];
  uint8_t key_index;
};
typedef struct rpl_sec_section rpl_sec_section_t;

struct rpl_sec_node {
  uint32_t sec_counter;
  uint16_t lifetime;
};
typedef struct rpl_sec_node rpl_sec_node_t;
#endif
/*---------------------------------------------------------------------------*/
/*
 * API for RPL objective functions (OF)
 *
 * reset(dag)
 *
 *  Resets the objective function state for a specific DAG. This function is
 *  called when doing a global repair on the DAG.
 *
 * parent_link_metric(parent)
 *
 *  Returns the link metric of a parent
 *
 * parent_has_usable_link(parent)
 *
 *  Returns 1 iff we have a usable link to this parent
 *
 * parent_path_cost(parent)
 *
 *  Returns the path cost of a parent
 *
 * rank_via_parent(parent)
 *
 *  Returns our rank if we select a given parent as preferred parent
 *
 * parent_is_acceptable
 *
 *  Returns 1 if a parent is usable as preferred parent, 0 otherwise
 *
 * best_parent(parent1, parent2)
 *
 *  Compares two parents and returns the best one, according to the OF.
 *
 * best_dag(dag1, dag2)
 *
 *  Compares two DAGs and returns the best one, according to the OF.
 *
 * update_metric_container(dag)
 *
 *  Updates the metric container for outgoing DIOs in a certain DAG.
 *  If the objective function of the DAG does not use metric containers,
 *  the function should set the object type to RPL_DAG_MC_NONE.
 *
 * dao_ack_callback(parent, status)
 *
 * A callback on the result of the DAO ACK. Similar to the neighbor link
 * callback. A failed DAO_ACK (NACK) can be used for switching to another
 * parent via changed link metric or other mechanisms.
 */
struct rpl_of {
  void (*reset)(struct rpl_dag *);
#if RPL_WITH_DAO_ACK
  void (*dao_ack_callback)(rpl_parent_t *, int status);
#endif
  uint16_t (*parent_link_metric)(rpl_parent_t *);
  int (*parent_has_usable_link)(rpl_parent_t *);
  uint16_t (*parent_path_cost)(rpl_parent_t *);
  rpl_rank_t (*rank_via_parent)(rpl_parent_t *);
  rpl_parent_t *(*best_parent)(rpl_parent_t *, rpl_parent_t *);
  rpl_dag_t *(*best_dag)(rpl_dag_t *, rpl_dag_t *);
  void (*update_metric_container)(rpl_instance_t *);
  rpl_ocp_t ocp;
};
typedef struct rpl_of rpl_of_t;

/*---------------------------------------------------------------------------*/
/* Instance */
struct rpl_instance {
  /* DAG configuration */
  rpl_metric_container_t mc;
  rpl_of_t *of;
  rpl_dag_t *current_dag;
  rpl_dag_t dag_table[RPL_MAX_DAG_PER_INSTANCE];
  /* The current default router - used for routing "upwards" */
  uip_ds6_defrt_t *def_route;
  uint8_t instance_id;
  uint8_t used;
  uint8_t dtsn_out;
  uint8_t mop;
  uint8_t dio_intdoubl;
  uint8_t dio_intmin;
  uint8_t dio_redundancy;
  uint8_t default_lifetime;
  uint8_t dio_intcurrent;
  uint8_t dio_send; /* for keeping track of which mode the timer is in */
  uint8_t dio_counter;
  /* my last registered DAO that I might be waiting for ACK on */
  uint8_t my_dao_seqno;
  uint8_t my_dao_transmissions;
  /* this is intended to keep track if this instance have a route downward */
  uint8_t has_downward_route;
  rpl_rank_t max_rankinc;
  rpl_rank_t min_hoprankinc;
  uint16_t lifetime_unit; /* lifetime in seconds = l_u * d_l */
#if RPL_CONF_STATS
  uint16_t dio_totint;
  uint16_t dio_totsend;
  uint16_t dio_totrecv;
#endif /* RPL_CONF_STATS */
  clock_time_t dio_next_delay; /* delay for completion of dio interval */
#if RPL_WITH_PROBING
  struct ctimer probing_timer;
  rpl_parent_t *urgent_probing_target;
#endif /* RPL_WITH_PROBING */
  struct ctimer dio_timer;
  struct ctimer dao_timer;
  struct ctimer dao_lifetime_timer;
  struct ctimer unicast_dio_timer;
  rpl_parent_t *unicast_dio_target;
#if RPL_WITH_DAO_ACK
  struct ctimer dao_retransmit_timer;
#if RPL_SECURITY
  uint8_t root_counter_trusted;
  uint32_t root_sec_counter;
#endif
#endif /* RPL_WITH_DAO_ACK */
};

/*---------------------------------------------------------------------------*/
/* Public RPL functions. */
void rpl_init(void);
void uip_rpl_input(void);
rpl_dag_t *rpl_set_root(uint8_t instance_id, uip_ipaddr_t *dag_id);
int rpl_set_prefix(rpl_dag_t *dag, uip_ipaddr_t *prefix, unsigned len);
int rpl_repair_root(uint8_t instance_id);
int rpl_set_default_route(rpl_instance_t *instance, uip_ipaddr_t *from);
rpl_dag_t *rpl_get_dag(const uip_ipaddr_t *addr);
rpl_dag_t *rpl_get_any_dag(void);
rpl_instance_t *rpl_get_instance(uint8_t instance_id);
int rpl_update_header(void);
int rpl_finalize_header(uip_ipaddr_t *addr);
int rpl_verify_hbh_header(int);
void rpl_insert_header(void);
void rpl_remove_header(void);
const struct link_stats *rpl_get_parent_link_stats(rpl_parent_t *p);
int rpl_parent_is_fresh(rpl_parent_t *p);
int rpl_parent_is_reachable(rpl_parent_t *p);
uint16_t rpl_get_parent_link_metric(rpl_parent_t *p);
rpl_rank_t rpl_rank_via_parent(rpl_parent_t *p);
const linkaddr_t *rpl_get_parent_lladdr(rpl_parent_t *p);
uip_ipaddr_t *rpl_get_parent_ipaddr(rpl_parent_t *nbr);
rpl_parent_t *rpl_get_parent(uip_lladdr_t *addr);
rpl_rank_t rpl_get_parent_rank(uip_lladdr_t *addr);
void rpl_dag_init(void);
uip_ds6_nbr_t *rpl_get_nbr(rpl_parent_t *parent);
void rpl_print_neighbor_list(void);
int rpl_process_srh_header(void);
int rpl_srh_get_next_hop(uip_ipaddr_t *ipaddr);

/* Per-parent RPL information */
NBR_TABLE_DECLARE(rpl_parents);

#if RPL_SECURITY
NBR_TABLE_DECLARE(rpl_sec_nodes);
#endif

/**
 * RPL modes
 *
 * The RPL module can be in either of three modes: mesh mode
 * (RPL_MODE_MESH), feater mode (RPL_MODE_FEATHER), and leaf mode
 * (RPL_MODE_LEAF). In mesh mode, nodes forward data for other nodes,
 * and are reachable by others. In feather mode, nodes can forward
 * data for other nodes, but are not reachable themselves. In leaf
 * mode, nodes do not forward data for others, but are reachable by
 * others. */
enum rpl_mode {
  RPL_MODE_MESH = 0,
  RPL_MODE_FEATHER = 1,
  RPL_MODE_LEAF = 2,
};

/**
 * Set the RPL mode
 *
 * \param mode The new RPL mode
 * \retval The previous RPL mode
 */
enum rpl_mode rpl_set_mode(enum rpl_mode mode);

/**
 * Get the RPL mode
 *
 * \retval The RPL mode
 */
enum rpl_mode rpl_get_mode(void);

/**
 * Get the RPL's best guess on if we have downward route or not.
 *
 * \retval 1 if we have a downward route from RPL Root, 0 if not.
 */
int rpl_has_downward_route(void);

/*---------------------------------------------------------------------------*/
#endif /* RPL_H */
