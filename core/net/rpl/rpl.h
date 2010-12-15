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
 * Author: Joakim Eriksson, Nicolas Tsiftes
 *
 * $Id: rpl.h,v 1.26 2010/12/15 12:24:00 nvt-se Exp $
 */

#ifndef RPL_H
#define RPL_H

/*
 * ContikiRPL - an implementation of the routing protocol for low power and
 * lossy networks. See: draft-ietf-roll-rpl-09.
 *
 * --
 * The DIOs handle prefix information option for setting global IP addresses
 * on the nodes, but the current handling is not awaiting the join of the DAG
 * so it does not currently support multiple DAGs.
 */

#include "lib/list.h"
#include "net/uip.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "net/uip-ds6.h"

/* set to 1 for some statistics on trickle / DIO */
#ifndef RPL_CONF_STATS
#define RPL_CONF_STATS 1
#endif /* RPL_CONF_STATS */

/* The RPL Codes for the message types */
#define RPL_CODE_DIS                     0   /* DIS message */
#define RPL_CODE_DIO                     1   /* DIO message */
#define RPL_CODE_DAO                     2   /* DAO message */
#define RPL_CODE_DAO_ACK                 3   /* DAO ACK message */

#define RPL_CODE_SEC_DIS               0x80   /* DIS message */
#define RPL_CODE_SEC_DIO               0x81   /* DIO message */
#define RPL_CODE_SEC_DAO               0x82   /* DAO message */
#define RPL_CODE_SEC_DAO_ACK           0x83   /* DAO ACK message */

/* RPL DIO/DAO suboption types */
#define RPL_DIO_SUBOPT_PAD1              0   /* Pad1 */
#define RPL_DIO_SUBOPT_PADN              1   /* PadN */
#define RPL_DIO_SUBOPT_DAG_MC            2   /* DAG metric container */
#define RPL_DIO_SUBOPT_ROUTE_INFO        3   /* Route information */
#define RPL_DIO_SUBOPT_DAG_CONF          4   /* DAG configuration */
#define RPL_DIO_SUBOPT_TARGET            5   /* Target */
#define RPL_DIO_SUBOPT_TRANSIT           6   /* Transit information */
#define RPL_DIO_SUBOPT_SOLICITED_INFO    7   /* Solicited information */
#define RPL_DIO_SUBOPT_PREFIX_INFO       8   /* Prefix information option */

#define RPL_DAO_K_FLAG                   0x80 /* DAO ACK requested */
#define RPL_DAO_D_FLAG                   0x40 /* DODAG ID Present */

/*---------------------------------------------------------------------------*/
/* Default values for RPL constants and variables. */

#define DEFAULT_DAO_LATENCY             (CLOCK_SECOND * (1 + (random_rand() & 0xf)))

/* Special value indicating immediate removal. */
#define ZERO_LIFETIME                   0

/* Special value indicating that a DAO should not expire. */
#define INFINITE_LIFETIME               0xffffffff

/* Default route lifetime in seconds. */
#define DEFAULT_ROUTE_LIFETIME          INFINITE_LIFETIME

#define DEFAULT_RPL_LIFETIME_UNIT       0xffff
#define DEFAULT_RPL_DEF_LIFETIME        0xff

#define DEFAULT_MIN_HOPRANKINC          256
#define DEFAULT_MAX_RANKINC             3*DEFAULT_MIN_HOPRANKINC

#define DAG_RANK(fixpt_rank, dag)	((fixpt_rank) / dag->min_hoprankinc)

/* Rank of a node outside the LLN. */
#define BASE_RANK                       0

/* Rank of a root node. */
#define ROOT_RANK                       DEFAULT_MIN_HOPRANKINC

#define INFINITE_RANK                   0xffff

#define RPL_DEFAULT_INSTANCE            0
#define RPL_ANY_INSTANCE               -1

#define RPL_DEFAULT_OCP                 1

/* Represents 2^n ms. */
/* Default alue according to the specification is 3 which
   means 8 milliseconds - this is not a reasonable value if
   using power-saving / duty-cycling    */
#define DEFAULT_DIO_INTERVAL_MIN        12

/* Maximum amount of timer doublings. */
#define DEFAULT_DIO_INTERVAL_DOUBLINGS  8

/* Default DIO redundancy. */
#define DEFAULT_DIO_REDUNDANCY          10

/* Expire DAOs from neighbors that do not respond in this time. (seconds) */
#define DAO_EXPIRATION_TIMEOUT          60
/*---------------------------------------------------------------------------*/
#define RPL_INSTANCE_LOCAL_FLAG         0x80
#define RPL_INSTANCE_D_FLAG             0x40

#define RPL_ROUTE_FROM_INTERNAL         0
#define RPL_ROUTE_FROM_UNICAST_DAO      1
#define RPL_ROUTE_FROM_MULTICAST_DAO    2
#define RPL_ROUTE_FROM_DIO              3

/* DAG Mode of Operation */
#define RPL_MOP_NO_DOWNWARD_ROUTES      0
#define RPL_MOP_NON_STORING             1
#define RPL_MOP_STORING_NO_MULTICAST    2
#define RPL_MOP_STORING_MULTICAST       3
#define RPL_MOP_DEFAULT                 RPL_MOP_STORING_NO_MULTICAST

/* DAG Metric Container Object Types, to be confirmed by IANA. */
#define RPL_DAG_MC_NSA                  1
#define RPL_DAG_MC_NE                   2
#define RPL_DAG_MC_HC                   3
#define RPL_DAG_MC_THROUGHPUT           4
#define RPL_DAG_MC_LATENCY              5
#define RPL_DAG_MC_LQL                  6
#define RPL_DAG_MC_ETX                  7
#define RPL_DAG_MC_LC                   8

/* DIS related */
#define RPL_DIS_SEND                    1
#ifdef  RPL_DIS_INTERVAL_CONF
#define RPL_DIS_INTERVAL                RPL_DIS_INTERVAL_CONF
#else
#define RPL_DIS_INTERVAL                60
#endif
#define RPL_DIS_START_DELAY             5

typedef uint16_t rpl_rank_t;
typedef uint16_t rpl_ocp_t;

struct rpl_parent {
  struct rpl_parent *next;
  void *dag;
  uip_ipaddr_t addr;
  rpl_rank_t rank;
  uint8_t local_confidence;
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
 */
struct rpl_of {
  void (*reset)(void *);
  void (*parent_state_callback)(rpl_parent_t *, int, int);
  rpl_parent_t *(*best_parent)(rpl_parent_t *, rpl_parent_t *);
  rpl_rank_t (*calculate_rank)(rpl_parent_t *, rpl_rank_t);
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

/* Logical representation of a DAG Information Object (DIO.) */
struct rpl_dio {
  uip_ipaddr_t dag_id;
  rpl_ocp_t ocp;
  rpl_rank_t rank;
  uint8_t grounded;
  uint8_t mop;
  uint8_t preference;
  uint8_t version;
  uint8_t instance_id;
  uint8_t dtsn;
  uint8_t dag_intdoubl;
  uint8_t dag_intmin;
  uint8_t dag_redund;
  uint8_t default_lifetime;
  uint16_t lifetime_unit;
  rpl_rank_t dag_max_rankinc;
  rpl_rank_t dag_min_hoprankinc;
  rpl_prefix_t destination_prefix;
  rpl_prefix_t prefix_info;
};

typedef struct rpl_dio rpl_dio_t;

#if RPL_CONF_STATS
/* Statistics for fault management. */
struct rpl_stats {
  uint16_t mem_overflows;
  uint16_t local_repairs;
  uint16_t global_repairs;
  uint16_t malformed_msgs;
  uint16_t resets;
};
typedef struct rpl_stats rpl_stats_t;

extern rpl_stats_t rpl_stats;

#define RPL_STAT(code)	(code) 
#else
#define RPL_STAT(code)
#endif /* RPL_CONF_STATS */

/* Directed Acyclic Graph */
struct rpl_dag {
  /* DAG configuration */
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
  uint8_t dio_send; /* for keeping track of which mode the timer is in */
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
/* RPL macros. */
#define RPL_PARENT_COUNT(dag)	list_length((dag)->parents)
/*---------------------------------------------------------------------------*/
/* ICMPv6 functions for RPL. */
void dis_output(uip_ipaddr_t *addr);
void dio_output(rpl_dag_t *, uip_ipaddr_t *uc_addr);
void dao_output(rpl_parent_t *, uint32_t lifetime);
void dao_ack_output(rpl_dag_t *, uip_ipaddr_t *, uint8_t);
void uip_rpl_input(void);

/* RPL logic functions. */
rpl_dag_t *rpl_set_root(uip_ipaddr_t *);
int rpl_set_prefix(rpl_dag_t *dag, uip_ipaddr_t *prefix, int len);
int rpl_repair_dag(rpl_dag_t *dag);
void rpl_local_repair(rpl_dag_t *dag);
int rpl_set_default_route(rpl_dag_t *dag, uip_ipaddr_t *from);
void rpl_process_dio(uip_ipaddr_t *, rpl_dio_t *);
int rpl_process_parent_event(rpl_dag_t *, rpl_parent_t *);

/* DAG allocation and deallocation. */
rpl_dag_t *rpl_alloc_dag(uint8_t);
void rpl_free_dag(rpl_dag_t *);

/* DAG parent management function. */
rpl_parent_t *rpl_add_parent(rpl_dag_t *, rpl_dio_t *dio, uip_ipaddr_t *);
rpl_parent_t *rpl_find_parent(rpl_dag_t *, uip_ipaddr_t *);
int rpl_remove_parent(rpl_dag_t *, rpl_parent_t *);
rpl_parent_t *rpl_select_parent(rpl_dag_t *dag);
void rpl_recalculate_ranks(void);

void rpl_join_dag(rpl_dag_t *);
rpl_dag_t *rpl_get_dag(int instance_id);
rpl_dag_t *rpl_find_dag(unsigned char aucIndex);

/* RPL routing table functions. */
void rpl_remove_routes(rpl_dag_t *dag);
uip_ds6_route_t *rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix,
                               int prefix_len, uip_ipaddr_t *next_hop);
void rpl_purge_routes(void);

/* Objective function. */
rpl_of_t *rpl_find_of(rpl_ocp_t);

/* Timer functions. */
void rpl_schedule_dao(rpl_dag_t *);
void rpl_reset_dio_timer(rpl_dag_t *, uint8_t);
void rpl_reset_periodic_timer(void);

/* Route poisoning. */
void rpl_poison_routes(rpl_dag_t *, rpl_parent_t *);
/*---------------------------------------------------------------------------*/
void rpl_init(void);

#endif /* RPL_H */
