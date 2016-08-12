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
 *   Private declarations for ContikiRPL.
 * \author
 *   Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef RPL_PRIVATE_H
#define RPL_PRIVATE_H

#include "net/rpl/rpl.h"

#include "lib/list.h"
#include "net/ip/uip.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/rpl/rpl-ns.h"
#include "net/ipv6/multicast/uip-mcast6.h"

/*---------------------------------------------------------------------------*/
/** \brief Is IPv6 address addr the link-local, all-RPL-nodes
    multicast address? */
#define uip_is_addr_linklocal_rplnodes_mcast(addr)	    \
  ((addr)->u8[0] == 0xff) &&				    \
  ((addr)->u8[1] == 0x02) &&				    \
  ((addr)->u16[1] == 0) &&				    \
  ((addr)->u16[2] == 0) &&				    \
  ((addr)->u16[3] == 0) &&				    \
  ((addr)->u16[4] == 0) &&				    \
  ((addr)->u16[5] == 0) &&				    \
  ((addr)->u16[6] == 0) &&				    \
  ((addr)->u8[14] == 0) &&				    \
  ((addr)->u8[15] == 0x1a))

/** \brief Set IP address addr to the link-local, all-rpl-nodes
    multicast address. */
#define uip_create_linklocal_rplnodes_mcast(addr)	\
  uip_ip6addr((addr), 0xff02, 0, 0, 0, 0, 0, 0, 0x001a)
/*---------------------------------------------------------------------------*/
/* RPL message types */
#define RPL_CODE_DIS                   0x00   /* DAG Information Solicitation */
#define RPL_CODE_DIO                   0x01   /* DAG Information Option */
#define RPL_CODE_DAO                   0x02   /* Destination Advertisement Option */
#define RPL_CODE_DAO_ACK               0x03   /* DAO acknowledgment */
#define RPL_CODE_SEC_DIS               0x80   /* Secure DIS */
#define RPL_CODE_SEC_DIO               0x81   /* Secure DIO */
#define RPL_CODE_SEC_DAO               0x82   /* Secure DAO */
#define RPL_CODE_SEC_DAO_ACK           0x83   /* Secure DAO ACK */

/* RPL control message options. */
#define RPL_OPTION_PAD1                  0
#define RPL_OPTION_PADN                  1
#define RPL_OPTION_DAG_METRIC_CONTAINER  2
#define RPL_OPTION_ROUTE_INFO            3
#define RPL_OPTION_DAG_CONF              4
#define RPL_OPTION_TARGET                5
#define RPL_OPTION_TRANSIT               6
#define RPL_OPTION_SOLICITED_INFO        7
#define RPL_OPTION_PREFIX_INFO           8
#define RPL_OPTION_TARGET_DESC           9

#define RPL_DAO_K_FLAG                   0x80 /* DAO ACK requested */
#define RPL_DAO_D_FLAG                   0x40 /* DODAG ID present */

#define RPL_DAO_ACK_UNCONDITIONAL_ACCEPT 0
#define RPL_DAO_ACK_ACCEPT               1   /* 1 - 127 is OK but not good */
#define RPL_DAO_ACK_UNABLE_TO_ACCEPT     128 /* >127 is fail */
#define RPL_DAO_ACK_UNABLE_TO_ADD_ROUTE_AT_ROOT 255 /* root can not accept */

#define RPL_DAO_ACK_TIMEOUT              -1

/*---------------------------------------------------------------------------*/
/* RPL IPv6 extension header option. */
#define RPL_HDR_OPT_LEN			4
#define RPL_HOP_BY_HOP_LEN		(RPL_HDR_OPT_LEN + 2 + 2)
#define RPL_RH_LEN     4
#define RPL_SRH_LEN    4
#define RPL_RH_TYPE_SRH   3
#define RPL_HDR_OPT_DOWN		0x80
#define RPL_HDR_OPT_DOWN_SHIFT  	7
#define RPL_HDR_OPT_RANK_ERR		0x40
#define RPL_HDR_OPT_RANK_ERR_SHIFT   	6
#define RPL_HDR_OPT_FWD_ERR		0x20
#define RPL_HDR_OPT_FWD_ERR_SHIFT   	5
/*---------------------------------------------------------------------------*/
/* Default values for RPL constants and variables. */

/* DAO transmissions are always delayed by RPL_DAO_DELAY +/- RPL_DAO_DELAY/2 */
#ifdef RPL_CONF_DAO_DELAY
#define RPL_DAO_DELAY                 RPL_CONF_DAO_DELAY
#else /* RPL_CONF_DAO_DELAY */
#define RPL_DAO_DELAY                 (CLOCK_SECOND * 4)
#endif /* RPL_CONF_DAO_DELAY */

/* Delay between reception of a no-path DAO and actual route removal */
#ifdef RPL_CONF_NOPATH_REMOVAL_DELAY
#define RPL_NOPATH_REMOVAL_DELAY          RPL_CONF_NOPATH_REMOVAL_DELAY
#else /* RPL_CONF_NOPATH_REMOVAL_DELAY */
#define RPL_NOPATH_REMOVAL_DELAY          60
#endif /* RPL_CONF_NOPATH_REMOVAL_DELAY */

#ifdef RPL_CONF_DAO_MAX_RETRANSMISSIONS
#define RPL_DAO_MAX_RETRANSMISSIONS RPL_CONF_DAO_MAX_RETRANSMISSIONS
#else
#define RPL_DAO_MAX_RETRANSMISSIONS     5
#endif /* RPL_CONF_DAO_MAX_RETRANSMISSIONS */

#ifdef RPL_CONF_DAO_RETRANSMISSION_TIMEOUT
#define RPL_DAO_RETRANSMISSION_TIMEOUT RPL_CONF_DAO_RETRANSMISSION_TIMEOUT
#else
#define RPL_DAO_RETRANSMISSION_TIMEOUT  (5 * CLOCK_SECOND)
#endif /* RPL_CONF_DAO_RETRANSMISSION_TIMEOUT */

/* Special value indicating immediate removal. */
#define RPL_ZERO_LIFETIME               0

#define RPL_LIFETIME(instance, lifetime) \
          ((unsigned long)(instance)->lifetime_unit * (lifetime))

#ifndef RPL_CONF_MIN_HOPRANKINC
/* RFC6550 defines the default MIN_HOPRANKINC as 256.
 * However, we use MRHOF as a default Objective Function (RFC6719),
 * which recommends setting MIN_HOPRANKINC with care, in particular
 * when used with ETX as a metric. ETX is computed as a fixed point
 * real with a divisor of 128 (RFC6719, RFC6551). We choose to also
 * use 128 for RPL_MIN_HOPRANKINC, resulting in a rank equal to the
 * ETX path cost. Larger values may also be desirable, as discussed
 * in section 6.1 of RFC6719. */
#if RPL_OF_OCP == RPL_OCP_MRHOF
#define RPL_MIN_HOPRANKINC          128
#else /* RPL_OF_OCP == RPL_OCP_MRHOF */
#define RPL_MIN_HOPRANKINC          256
#endif /* RPL_OF_OCP == RPL_OCP_MRHOF */
#else /* RPL_CONF_MIN_HOPRANKINC */
#define RPL_MIN_HOPRANKINC          RPL_CONF_MIN_HOPRANKINC
#endif /* RPL_CONF_MIN_HOPRANKINC */

#ifndef RPL_CONF_MAX_RANKINC
#define RPL_MAX_RANKINC             (7 * RPL_MIN_HOPRANKINC)
#else /* RPL_CONF_MAX_RANKINC */
#define RPL_MAX_RANKINC             RPL_CONF_MAX_RANKINC
#endif /* RPL_CONF_MAX_RANKINC */

#define DAG_RANK(fixpt_rank, instance) \
  ((fixpt_rank) / (instance)->min_hoprankinc)

/* Rank of a virtual root node that coordinates DAG root nodes. */
#define BASE_RANK                       0

/* Rank of a root node. */
#define ROOT_RANK(instance)             (instance)->min_hoprankinc

#define INFINITE_RANK                   0xffff

/*---------------------------------------------------------------------------*/
#define RPL_INSTANCE_LOCAL_FLAG         0x80
#define RPL_INSTANCE_D_FLAG             0x40

/* Values that tell where a route came from. */
#define RPL_ROUTE_FROM_INTERNAL         0
#define RPL_ROUTE_FROM_UNICAST_DAO      1
#define RPL_ROUTE_FROM_MULTICAST_DAO    2
#define RPL_ROUTE_FROM_DIO              3

/* DAG Mode of Operation */
#define RPL_MOP_NO_DOWNWARD_ROUTES      0
#define RPL_MOP_NON_STORING             1
#define RPL_MOP_STORING_NO_MULTICAST    2
#define RPL_MOP_STORING_MULTICAST       3

/* RPL Mode of operation */
#ifdef  RPL_CONF_MOP
#define RPL_MOP_DEFAULT                 RPL_CONF_MOP
#else /* RPL_CONF_MOP */
#if RPL_WITH_MULTICAST
#define RPL_MOP_DEFAULT                 RPL_MOP_STORING_MULTICAST
#else
#define RPL_MOP_DEFAULT                 RPL_MOP_STORING_NO_MULTICAST
#endif /* RPL_WITH_MULTICAST */
#endif /* RPL_CONF_MOP */

/*
 * Embed support for storing mode
 */
#ifdef RPL_CONF_WITH_STORING
#define RPL_WITH_STORING RPL_CONF_WITH_STORING
#else /* RPL_CONF_WITH_STORING */
/* By default: embed support for non-storing if and only if the configured MOP is not non-storing */
#define RPL_WITH_STORING (RPL_MOP_DEFAULT != RPL_MOP_NON_STORING)
#endif /* RPL_CONF_WITH_STORING */

/*
 * Embed support for non-storing mode
 */
#ifdef RPL_CONF_WITH_NON_STORING
#define RPL_WITH_NON_STORING RPL_CONF_WITH_NON_STORING
#else /* RPL_CONF_WITH_NON_STORING */
/* By default: embed support for non-storing if and only if the configured MOP is non-storing */
#define RPL_WITH_NON_STORING (RPL_MOP_DEFAULT == RPL_MOP_NON_STORING)
#endif /* RPL_CONF_WITH_NON_STORING */

#if RPL_WITH_STORING && (UIP_DS6_ROUTE_NB == 0)
#error "RPL with storing mode included but #routes == 0. Set UIP_CONF_MAX_ROUTES accordingly."
#if !RPL_WITH_NON_STORING && (RPL_NS_LINK_NUM > 0)
#error "You might also want to set RPL_NS_CONF_LINK_NUM to 0."
#endif
#endif

#if RPL_WITH_NON_STORING && (RPL_NS_LINK_NUM == 0)
#error "RPL with non-storing mode included but #links == 0. Set RPL_NS_CONF_LINK_NUM accordingly."
#if !RPL_WITH_STORING && (UIP_DS6_ROUTE_NB > 0)
#error "You might also want to set UIP_CONF_MAX_ROUTES to 0."
#endif
#endif

#define RPL_IS_STORING(instance) (RPL_WITH_STORING && ((instance) != NULL) && ((instance)->mop > RPL_MOP_NON_STORING))
#define RPL_IS_NON_STORING(instance) (RPL_WITH_NON_STORING && ((instance) != NULL) && ((instance)->mop == RPL_MOP_NON_STORING))

/* Emit a pre-processor error if the user configured multicast with bad MOP */
#if RPL_WITH_MULTICAST && (RPL_MOP_DEFAULT != RPL_MOP_STORING_MULTICAST)
#error "RPL Multicast requires RPL_MOP_DEFAULT==3. Check contiki-conf.h"
#endif

/* Multicast Route Lifetime as a multiple of the lifetime unit */
#ifdef RPL_CONF_MCAST_LIFETIME
#define RPL_MCAST_LIFETIME RPL_CONF_MCAST_LIFETIME
#else
#define RPL_MCAST_LIFETIME 3
#endif

/* DIS related */
#define RPL_DIS_SEND                    1

/*---------------------------------------------------------------------------*/
/* Lollipop counters */

#define RPL_LOLLIPOP_MAX_VALUE           255
#define RPL_LOLLIPOP_CIRCULAR_REGION     127
#define RPL_LOLLIPOP_SEQUENCE_WINDOWS    16
#define RPL_LOLLIPOP_INIT                (RPL_LOLLIPOP_MAX_VALUE - RPL_LOLLIPOP_SEQUENCE_WINDOWS + 1)
#define RPL_LOLLIPOP_INCREMENT(counter)                                 \
  do {                                                                  \
    if((counter) > RPL_LOLLIPOP_CIRCULAR_REGION) {                      \
      (counter) = ((counter) + 1) & RPL_LOLLIPOP_MAX_VALUE;             \
    } else {                                                            \
      (counter) = ((counter) + 1) & RPL_LOLLIPOP_CIRCULAR_REGION;       \
    }                                                                   \
  } while(0)

#define RPL_LOLLIPOP_IS_INIT(counter)		\
  ((counter) > RPL_LOLLIPOP_CIRCULAR_REGION)
/*---------------------------------------------------------------------------*/
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
  struct rpl_metric_container mc;
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
  uint16_t parent_switch;
  uint16_t forward_errors;
  uint16_t loop_errors;
  uint16_t loop_warnings;
  uint16_t root_repairs;
};
typedef struct rpl_stats rpl_stats_t;

extern rpl_stats_t rpl_stats;
#endif


/*---------------------------------------------------------------------------*/
/* RPL macros. */

#if RPL_CONF_STATS
#define RPL_STAT(code)	(code) 
#else
#define RPL_STAT(code)
#endif /* RPL_CONF_STATS */
/*---------------------------------------------------------------------------*/
/* Instances */
extern rpl_instance_t instance_table[];
extern rpl_instance_t *default_instance;

/* ICMPv6 functions for RPL. */
void dis_output(uip_ipaddr_t *addr);
void dio_output(rpl_instance_t *, uip_ipaddr_t *uc_addr);
void dao_output(rpl_parent_t *, uint8_t lifetime);
void dao_output_target(rpl_parent_t *, uip_ipaddr_t *, uint8_t lifetime);
void dao_ack_output(rpl_instance_t *, uip_ipaddr_t *, uint8_t, uint8_t);
void rpl_icmp6_register_handlers(void);
uip_ds6_nbr_t *rpl_icmp6_update_nbr_table(uip_ipaddr_t *from,
                                          nbr_table_reason_t r, void *data);

/* RPL logic functions. */
void rpl_join_dag(uip_ipaddr_t *from, rpl_dio_t *dio);
void rpl_join_instance(uip_ipaddr_t *from, rpl_dio_t *dio);
void rpl_local_repair(rpl_instance_t *instance);
void rpl_process_dio(uip_ipaddr_t *, rpl_dio_t *);
int rpl_process_parent_event(rpl_instance_t *, rpl_parent_t *);

/* DAG object management. */
rpl_dag_t *rpl_alloc_dag(uint8_t, uip_ipaddr_t *);
rpl_instance_t *rpl_alloc_instance(uint8_t);
void rpl_free_dag(rpl_dag_t *);
void rpl_free_instance(rpl_instance_t *);
void rpl_purge_dags(void);

/* DAG parent management function. */
rpl_parent_t *rpl_add_parent(rpl_dag_t *, rpl_dio_t *dio, uip_ipaddr_t *);
rpl_parent_t *rpl_find_parent(rpl_dag_t *, uip_ipaddr_t *);
rpl_parent_t *rpl_find_parent_any_dag(rpl_instance_t *instance, uip_ipaddr_t *addr);
void rpl_nullify_parent(rpl_parent_t *);
void rpl_remove_parent(rpl_parent_t *);
void rpl_move_parent(rpl_dag_t *dag_src, rpl_dag_t *dag_dst, rpl_parent_t *parent);
rpl_parent_t *rpl_select_parent(rpl_dag_t *dag);
rpl_dag_t *rpl_select_dag(rpl_instance_t *instance,rpl_parent_t *parent);
void rpl_recalculate_ranks(void);

/* RPL routing table functions. */
void rpl_remove_routes(rpl_dag_t *dag);
void rpl_remove_routes_by_nexthop(uip_ipaddr_t *nexthop, rpl_dag_t *dag);
uip_ds6_route_t *rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix,
                               int prefix_len, uip_ipaddr_t *next_hop);
void rpl_purge_routes(void);

/* Objective function. */
rpl_of_t *rpl_find_of(rpl_ocp_t);

/* Timer functions. */
void rpl_schedule_dao(rpl_instance_t *);
void rpl_schedule_dao_immediately(rpl_instance_t *);
void rpl_schedule_unicast_dio_immediately(rpl_instance_t *instance);
void rpl_cancel_dao(rpl_instance_t *instance);
void rpl_schedule_probing(rpl_instance_t *instance);

void rpl_reset_dio_timer(rpl_instance_t *);
void rpl_reset_periodic_timer(void);

/* Route poisoning. */
void rpl_poison_routes(rpl_dag_t *, rpl_parent_t *);


rpl_instance_t *rpl_get_default_instance(void);

#endif /* RPL_PRIVATE_H */
