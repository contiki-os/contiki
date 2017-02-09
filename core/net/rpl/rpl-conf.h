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
 *	Public configuration and API declarations for ContikiRPL.
 * \author
 *	Joakim Eriksson <joakime@sics.se> & Nicolas Tsiftes <nvt@sics.se>
 *
 */

#ifndef RPL_CONF_H
#define RPL_CONF_H

#include "contiki-conf.h"

/* Set to 1 to enable RPL statistics */
#ifndef RPL_CONF_STATS
#define RPL_CONF_STATS 0
#endif /* RPL_CONF_STATS */

/*
 * The objective function (OF) used by a RPL root is configurable through
 * the RPL_CONF_OF_OCP parameter. This is defined as the objective code
 * point (OCP) of the OF, RPL_OCP_OF0 or RPL_OCP_MRHOF. This flag is of
 * no relevance to non-root nodes, which run the OF advertised in the
 * instance they join.
 * Make sure the selected of is inRPL_SUPPORTED_OFS.
 */
#ifdef RPL_CONF_OF_OCP
#define RPL_OF_OCP RPL_CONF_OF_OCP
#else /* RPL_CONF_OF_OCP */
#define RPL_OF_OCP RPL_OCP_MRHOF
#endif /* RPL_CONF_OF_OCP */

/*
 * The set of objective functions supported at runtime. Nodes are only
 * able to join instances that advertise an OF in this set. To include
 * both OF0 and MRHOF, use {&rpl_of0, &rpl_mrhof}.
 */
#ifdef RPL_CONF_SUPPORTED_OFS
#define RPL_SUPPORTED_OFS RPL_CONF_SUPPORTED_OFS
#else /* RPL_CONF_SUPPORTED_OFS */
#define RPL_SUPPORTED_OFS {&rpl_mrhof}
#endif /* RPL_CONF_SUPPORTED_OFS */

/*
 * Enable/disable RPL Metric Containers (MC). The actual MC in use
 * for a given DODAG is decided at runtime, when joining. Note that
 * OF0 (RFC6552) operates without MC, and so does MRHOF (RFC6719) when
 * used with ETX as a metric (the rank is the metric). We disable MC
 * by default, but note it must be enabled to support joining a DODAG
 * that requires MC (e.g., MRHOF with a metric other than ETX).
 */
#ifdef RPL_CONF_WITH_MC
#define RPL_WITH_MC RPL_CONF_WITH_MC
#else /* RPL_CONF_WITH_MC */
#define RPL_WITH_MC 0
#endif /* RPL_CONF_WITH_MC */

/* The MC advertised in DIOs and propagating from the root */
#ifdef RPL_CONF_DAG_MC
#define RPL_DAG_MC RPL_CONF_DAG_MC
#else
#define RPL_DAG_MC RPL_DAG_MC_NONE
#endif /* RPL_CONF_DAG_MC */

/* This value decides which DAG instance we should participate in by default. */
#ifdef RPL_CONF_DEFAULT_INSTANCE
#define RPL_DEFAULT_INSTANCE RPL_CONF_DEFAULT_INSTANCE
#else
#define RPL_DEFAULT_INSTANCE	       0x1e
#endif /* RPL_CONF_DEFAULT_INSTANCE */

/*
 * This value decides if this node must stay as a leaf or not
 * as allowed by draft-ietf-roll-rpl-19#section-8.5
 */
#ifdef RPL_CONF_LEAF_ONLY
#define RPL_LEAF_ONLY RPL_CONF_LEAF_ONLY
#else
#define RPL_LEAF_ONLY 0
#endif

/*
 * Maximum of concurent RPL instances.
 */
#ifdef RPL_CONF_MAX_INSTANCES
#define RPL_MAX_INSTANCES     RPL_CONF_MAX_INSTANCES
#else
#define RPL_MAX_INSTANCES     1
#endif /* RPL_CONF_MAX_INSTANCES */

/*
 * Maximum number of DAGs within an instance.
 */
#ifdef RPL_CONF_MAX_DAG_PER_INSTANCE
#define RPL_MAX_DAG_PER_INSTANCE     RPL_CONF_MAX_DAG_PER_INSTANCE
#else
#define RPL_MAX_DAG_PER_INSTANCE     2
#endif /* RPL_CONF_MAX_DAG_PER_INSTANCE */

/*
 * RPL Default route lifetime
 * The RPL route lifetime is used for the downward routes and for the default
 * route. In a high density network with DIO suppression activated it may happen
 * that a node will never send a DIO once the DIO interval becomes high as it
 * has heard DIO from many neighbors already. As the default route to the
 * preferred parent has a lifetime reset by receiving DIO from the parent, it
 * means that the default route can be destroyed after a while. Setting the
 * default route with infinite lifetime secures the upstream route.
 */
#ifdef RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME
#define RPL_DEFAULT_ROUTE_INFINITE_LIFETIME                    RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME
#else
#define RPL_DEFAULT_ROUTE_INFINITE_LIFETIME                    1
#endif /* RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME */

/*
 * Maximum lifetime of a DAG
 * When a DODAG is not updated since RPL_CONF_DAG_LIFETIME times the DODAG
 * maximum DIO interval the DODAG is removed from the list of DODAGS of the
 * related instance, except if it is the currently joined DODAG.
 */
#ifdef RPL_CONF_DAG_LIFETIME
#define RPL_DAG_LIFETIME                    RPL_CONF_DAG_LIFETIME
#else
#define RPL_DAG_LIFETIME                    3
#endif /* RPL_CONF_DAG_LIFETIME */

/*
 * 
 */
#ifndef RPL_CONF_DAO_SPECIFY_DAG
  #if RPL_MAX_DAG_PER_INSTANCE > 1
    #define RPL_DAO_SPECIFY_DAG 1
  #else
    #define RPL_DAO_SPECIFY_DAG 0
  #endif /* RPL_MAX_DAG_PER_INSTANCE > 1 */
#else
  #define RPL_DAO_SPECIFY_DAG RPL_CONF_DAO_SPECIFY_DAG
#endif /* RPL_CONF_DAO_SPECIFY_DAG */

/*
 * The DIO interval (n) represents 2^n ms.
 *
 * According to the specification, the default value is 3 which
 * means 8 milliseconds. That is far too low when using duty cycling
 * with wake-up intervals that are typically hundreds of milliseconds.
 * ContikiRPL thus sets the default to 2^12 ms = 4.096 s.
 */
#ifdef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_DIO_INTERVAL_MIN        RPL_CONF_DIO_INTERVAL_MIN
#else
#define RPL_DIO_INTERVAL_MIN        12
#endif

/*
 * Maximum amount of timer doublings.
 *
 * The maximum interval will by default be 2^(12+8) ms = 1048.576 s.
 * RFC 6550 suggests a default value of 20, which of course would be
 * unsuitable when we start with a minimum interval of 2^12.
 */
#ifdef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_DIO_INTERVAL_DOUBLINGS  RPL_CONF_DIO_INTERVAL_DOUBLINGS
#else
#define RPL_DIO_INTERVAL_DOUBLINGS  8
#endif

/*
 * DIO redundancy. To learn more about this, see RFC 6206.
 *
 * RFC 6550 suggests a default value of 10. It is unclear what the basis
 * of this suggestion is. Network operators might attain more efficient
 * operation by tuning this parameter for specific deployments.
 */
#ifdef RPL_CONF_DIO_REDUNDANCY
#define RPL_DIO_REDUNDANCY          RPL_CONF_DIO_REDUNDANCY
#else
#define RPL_DIO_REDUNDANCY          10
#endif

/*
 * Default route lifetime unit. This is the granularity of time
 * used in RPL lifetime values, in seconds.
 */
#ifndef RPL_CONF_DEFAULT_LIFETIME_UNIT
#define RPL_DEFAULT_LIFETIME_UNIT       60
#else
#define RPL_DEFAULT_LIFETIME_UNIT       RPL_CONF_DEFAULT_LIFETIME_UNIT
#endif

/*
 * Default route lifetime as a multiple of the lifetime unit.
 */
#ifndef RPL_CONF_DEFAULT_LIFETIME
#define RPL_DEFAULT_LIFETIME            30
#else
#define RPL_DEFAULT_LIFETIME            RPL_CONF_DEFAULT_LIFETIME
#endif

/*
 * DAG preference field
 */
#ifdef RPL_CONF_PREFERENCE
#define RPL_PREFERENCE              RPL_CONF_PREFERENCE
#else
#define RPL_PREFERENCE              0
#endif

/*
 * RPL DAO ACK support. When enabled, DAO ACK will be sent and requested.
 * This will also enable retransmission of DAO when no ack is received.
 * */
#ifdef RPL_CONF_WITH_DAO_ACK
#define RPL_WITH_DAO_ACK RPL_CONF_WITH_DAO_ACK
#else
#define RPL_WITH_DAO_ACK 0
#endif /* RPL_CONF_WITH_DAO_ACK */

/*
 * RPL REPAIR ON DAO NACK. When enabled, DAO NACK will trigger a local
 * repair in order to quickly find a new parent to send DAO's to.
 * NOTE: this is too agressive in some cases so use with care.
 * */
#ifdef RPL_CONF_RPL_REPAIR_ON_DAO_NACK
#define RPL_REPAIR_ON_DAO_NACK RPL_CONF_RPL_REPAIR_ON_DAO_NACK
#else
#define RPL_REPAIR_ON_DAO_NACK 0
#endif /* RPL_CONF_RPL_REPAIR_ON_DAO_NACK */

/*
 * Setting the DIO_REFRESH_DAO_ROUTES will make the RPL root always
 * increase the DTSN (Destination Advertisement Trigger Sequence Number)
 * when sending multicast DIO. This is to get all children to re-register
 * their DAO route. This is needed when DAO-ACK is not enabled to add
 * reliability to route maintenance.
 * */
#ifdef RPL_CONF_DIO_REFRESH_DAO_ROUTES
#define RPL_DIO_REFRESH_DAO_ROUTES RPL_CONF_DIO_REFRESH_DAO_ROUTES
#else
#define RPL_DIO_REFRESH_DAO_ROUTES 1
#endif /* RPL_CONF_DIO_REFRESH_DAO_ROUTES */

/*
 * RPL probing. When enabled, probes will be sent periodically to keep
 * parent link estimates up to date.
 */
#ifdef RPL_CONF_WITH_PROBING
#define RPL_WITH_PROBING RPL_CONF_WITH_PROBING
#else
#define RPL_WITH_PROBING 1
#endif

/*
 * RPL probing interval.
 */
#ifdef RPL_CONF_PROBING_INTERVAL
#define RPL_PROBING_INTERVAL RPL_CONF_PROBING_INTERVAL
#else
#define RPL_PROBING_INTERVAL (120 * CLOCK_SECOND)
#endif

/*
 * Function used to select the next parent to be probed.
 */
#ifdef RPL_CONF_PROBING_SELECT_FUNC
#define RPL_PROBING_SELECT_FUNC RPL_CONF_PROBING_SELECT_FUNC
#else
#define RPL_PROBING_SELECT_FUNC get_probing_target
#endif

/*
 * Function used to send RPL probes.
 * To probe with DIO, use:
 * #define RPL_CONF_PROBING_SEND_FUNC(instance, addr) dio_output((instance), (addr))
 * To probe with DIS, use:
 * #define RPL_CONF_PROBING_SEND_FUNC(instance, addr) dis_output((addr))
 * Any other custom probing function is also acceptable.
 */
#ifdef RPL_CONF_PROBING_SEND_FUNC
#define RPL_PROBING_SEND_FUNC RPL_CONF_PROBING_SEND_FUNC
#else
#define RPL_PROBING_SEND_FUNC(instance, addr) dio_output((instance), (addr))
#endif

/*
 * Function used to calculate next RPL probing interval
 */
#ifdef RPL_CONF_PROBING_DELAY_FUNC
#define RPL_PROBING_DELAY_FUNC RPL_CONF_PROBING_DELAY_FUNC
#else
#define RPL_PROBING_DELAY_FUNC get_probing_delay
#endif

/*
 * Interval of DIS transmission
 */
#ifdef  RPL_CONF_DIS_INTERVAL
#define RPL_DIS_INTERVAL                RPL_CONF_DIS_INTERVAL
#else
#define RPL_DIS_INTERVAL                60
#endif

/*
 * Added delay of first DIS transmission after boot
 */
#ifdef  RPL_CONF_DIS_START_DELAY
#define RPL_DIS_START_DELAY             RPL_CONF_DIS_START_DELAY
#else
#define RPL_DIS_START_DELAY             5
#endif

#endif /* RPL_CONF_H */
