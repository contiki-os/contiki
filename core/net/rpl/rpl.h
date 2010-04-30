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
 * $Id: rpl.h,v 1.2 2010/04/30 15:03:55 nvt-se Exp $
 */

#ifndef RPL_H
#define RPL_H

#include "lib/list.h"
#include "net/uip.h"
#include "sys/clock.h"
#include "net/rime/ctimer.h"
#include "net/uip-ds6.h"

/* set to 1 for some statistics on trickle / DIO */
#ifndef RPL_CONF_STATS
#define RPL_CONF_STATS 1
#endif /* RPL_CONF_STATS */

/* The RPL Codes for the message types */
#define RPL_CODE_DIS                     1   /* DIS message */
#define RPL_CODE_DIO                     2   /* DIO message */
#define RPL_CODE_DAO                     4   /* DAO message */

/* RPL DIO suboption types */
#define RPL_DIO_SUBOPT_PAD1              0   /* Pad1 */
#define RPL_DIO_SUBOPT_PADN              1   /* PadN */
#define RPL_DIO_SUBOPT_DAG_MC            2   /* DAG metric container */
#define RPL_DIO_SUBOPT_DEST_PREFIX       3   /* Destination prefix */
#define RPL_DIO_SUBOPT_DAG_CONF          4   /* DAG configuration */
#define RPL_DIO_SUBOPT_OCP               5   /* OCP */

/*---------------------------------------------------------------------------*/
/* Default values for RPL constants and variables. */

#define DEFAULT_DAO_LATENCY             (CLOCK_SECOND * 10)

/* Special value indicating immediate removal. */
#define ZERO_LIFETIME                   0

/* Special value indicating that a DAO should not expire. */
#define INFINITE_LIFETIME               0xffffffff

/* Default route lifetime in seconds. */
#define DEFAULT_ROUTE_LIFETIME          INFINITE_LIFETIME

/* Rank of a node outside the LLN. */
#define BASE_RANK                       0

/* Rank of a root node. */
#define ROOT_RANK                       1

#define INFINITE_RANK                   0xffff

#define RPL_DEFAULT_INSTANCE            0
#define RPL_ANY_INSTANCE               -1

#define RPL_DEFAULT_OCP                 0

/* TODO: pick these from OCP later? */
#define DEFAULT_MAX_RANKINC             16
#define DEFAULT_MIN_HOPRANKINC          4

/* Represents 2^n ms. */
#define DEFAULT_DIO_INTERVAL_MIN        12

/* Maximum amount of timer doublings. */
#define DEFAULT_DIO_INTERVAL_DOUBLINGS  8

/* Desired DIO redundancy. */
#define DEFAULT_DIO_REDUNDANCY          1

/* Expire DAOs from neighbors that do not respond in this time. (seconds) */
#define DAO_EXPIRATION_TIMEOUT          60
/*---------------------------------------------------------------------------*/

#define RPL_ROUTE_FROM_INTERNAL         0
#define RPL_ROUTE_FROM_UNICAST_DAO      1
#define RPL_ROUTE_FROM_MULTICAST_DAO    2
#define RPL_ROUTE_FROM_DIO              3

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
#define RPL_DIS_INTERVAL                60
#define RPL_DIS_START_DELAY             5

typedef uint16_t rpl_rank_t;
typedef uint16_t rpl_ocp_t;

struct rpl_neighbor {
  struct rpl_neighbor *next;
  void *dag;
  uip_ipaddr_t addr;
  rpl_rank_t rank;
  uint8_t local_confidence;
};

typedef struct rpl_neighbor rpl_neighbor_t;

struct rpl_of {
  rpl_neighbor_t *(*best_parent)(rpl_neighbor_t *, rpl_neighbor_t *);
  rpl_rank_t (*increment_rank)(rpl_rank_t, rpl_neighbor_t *);
  rpl_ocp_t ocp;
};


typedef struct rpl_of rpl_of_t;

/* Logical representation of a DAG Information Object (DIO.) */
struct rpl_dio {
  uip_ipaddr_t dag_id;
  rpl_ocp_t ocp;
  rpl_rank_t dag_rank;
  uint8_t grounded;
  uint8_t dst_adv_trigger;
  uint8_t dst_adv_supported;
  uint8_t preference;
  uint8_t sequence_number;
  uint8_t instance_id;
  uint8_t dtsn;
  uint8_t dag_intdoubl;
  uint8_t dag_intmin;
  uint8_t dag_redund;
  uint8_t dag_max_rankinc;
  uint8_t dag_min_hoprankinc;
};

typedef struct rpl_dio rpl_dio_t;

struct rpl_prefix {
  uip_ipaddr_t prefix;
  uint32_t lifetime;
  uint8_t length;
  uint8_t preference;
};

typedef struct rpl_prefix rpl_prefix_t;

/* Directed Acyclic Graph */
struct rpl_dag {
  /* DAG configuration */
  rpl_of_t *of;
  uip_ipaddr_t dag_id;
  /* this is  the current def-router that is set - used for routing "upwards" */
  uip_ds6_defrt_t *def_route;
  rpl_rank_t rank;
  rpl_rank_t min_rank; /* should be nullified per dodag iteration! */
  uint8_t dtsn;
  uint8_t instance_id;
  uint8_t sequence_number;
  uint8_t preference;
  uint8_t grounded;
  uint8_t dio_intdoubl;
  uint8_t dio_intmin;
  uint8_t dio_redundancy;
  uint8_t max_rankinc;
  uint8_t min_hoprankinc;
  uint8_t used;

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
  uint16_t dio_next_delay; /* delay for completion of dio interval */
  struct ctimer dio_timer;
  struct ctimer dao_timer;
  rpl_neighbor_t *best_parent;
  void *neighbor_list;
  list_t neighbors;
};

typedef struct rpl_dag rpl_dag_t;

/*---------------------------------------------------------------------------*/
/* RPL macro functions. */
#define RPL_PARENT_COUNT(dag)                           \
                                list_length((dag)->neighbors)
#define RPL_NEIGHBOR_IS_CHILD(dag, neighbor)            \
                                ((neighbor)->rank > (dag)->rank)
#define RPL_NEIGHBOR_IS_SIBLING(dag, neighbor)          \
                                ((neighbor)->rank == (dag)->rank)
#define RPL_NEIGHBOR_IS_PARENT(dag, neighbor)           \
                                ((neighbor)->rank < (dag)->rank)
/*---------------------------------------------------------------------------*/
/* ICMPv6 functions for RPL. */
void dis_output(uip_ipaddr_t *addr);
void dio_output(rpl_dag_t *, uip_ipaddr_t *uc_addr);
void dao_output(rpl_neighbor_t *, uint32_t lifetime);
void uip_rpl_input(void);

/* RPL logic functions. */
int rpl_set_root(uip_ipaddr_t *);
int rpl_repair_dag(rpl_dag_t *dag);
int rpl_set_default_route(rpl_dag_t *dag, uip_ipaddr_t *from);
void rpl_process_dio(uip_ipaddr_t *, rpl_dio_t *);

/* DAG allocation and deallocation. */
rpl_dag_t *rpl_alloc_dag(void);
void rpl_free_dag(rpl_dag_t *);

/* DAG parent management function. */
rpl_neighbor_t *rpl_add_neighbor(rpl_dag_t *, uip_ipaddr_t *);
rpl_neighbor_t *rpl_find_neighbor(rpl_dag_t *, uip_ipaddr_t *);
int rpl_remove_neighbor(rpl_dag_t *, rpl_neighbor_t *);
rpl_neighbor_t *rpl_first_parent(rpl_dag_t *dag);
rpl_neighbor_t *rpl_find_best_parent(rpl_dag_t *dag);

void rpl_join_dag(rpl_dag_t *);
rpl_dag_t *rpl_get_dag(int instance_id);
rpl_dag_t *rpl_find_dag(unsigned char aucIndex);

/* RPL routing table functions. */
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
void rpl_poison_routes(rpl_dag_t *, rpl_neighbor_t *);
/*---------------------------------------------------------------------------*/
void rpl_init(void);

#endif /* RPL_H */
