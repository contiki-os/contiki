/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 *         orpl.c header file
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __ORPL_H__
#define __ORPL_H__

#include "net/rpl/rpl.h"

#ifdef ORPL_CONF_EDC_W
#define ORPL_EDC_W ORPL_CONF_EDC_W
#else /* ORPL_CONF_EDC_W */
#define ORPL_EDC_W 64
#endif /* ORPL_CONF_EDC_W */

#ifdef ORPL_CONF_WITH_FP_RECOVERY
#define ORPL_WITH_FP_RECOVERY ORPL_CONF_WITH_FP_RECOVERY
#else /* ORPL_CONF_WITH_FP_RECOVERY */
#define ORPL_WITH_FP_RECOVERY 1
#endif /* ORPL_CONF_WITH_FP_RECOVERY */

/* Default implementation for logging functions */
#ifndef ORPL_LOG
#define ORPL_LOG(...) PRINTF(...)
#define ORPL_LOG_FROM_APPDATAPTR(appdataptr, ...) PRINTF(...)
#define ORPL_LOG_NULL(...) PRINTF(...)
#define ORPL_LOG_FROM_UIP(...) PRINTF(...)
#define ORPL_LOG_FROM_PACKETBUF(...) PRINTF(...)
#define ORPL_LOG_IPADDR(addr) PRINT6ADDR(addr)
#define ORPL_LOG_LLADDR(addr) PRINTLLADDR(addr)
#define ORPL_LOG_NODEID_FROM_RIMEADDR(addr) ((addr).u8[RIMEADDR_SIZE-1])
#define ORPL_LOG_NODEID_FROM_IPADDR(addr) ((addr).u8[15])
#define ORPL_LOG_INC_HOPCOUNT_FROM_PACKETBUF()
#define ORPL_LOG_INC_FPCOUNT_FROM_PACKETBUF()
#endif /* ORPL_LOG */

/* Fixed point divisor */
#define EDC_DIVISOR 128
/* From rtimer ticks to EDC fixed point metric */
#define EDC_TICKS_TO_METRIC(edc) (uint16_t)((edc) / (CONTIKIMAC_CONF_CYCLE_TIME / EDC_DIVISOR))

/* The global IPv6 address in use */
extern uip_ipaddr_t global_ipv6;

/* Flag used to tell lower layers that the current UDP transmission
 * is a routing set, so that the desired callback function is called
 * after each transmission attempt */
extern int sending_routing_set;

/* Total number of broadcast sent */
extern uint32_t orpl_broadcast_count;

/* Set the 32-bit ORPL sequence number in packetbuf */
void orpl_packetbuf_set_seqno(uint32_t seqno);
/* Get the 32-bit ORPL sequence number from packetbuf */
uint32_t orpl_packetbuf_seqno();
/* Set the current ORPL sequence number before sending */
void orpl_set_curr_seqno(uint32_t seqno);
/* Get the current ORPL sequence number */
uint32_t orpl_get_curr_seqno();
/* Get a new ORPL sequence number */
uint32_t orpl_get_new_seqno();
/* Build a global link-layer address from an IPv6 based on its UUID64 */
void lladdr_from_ipaddr_uuid(uip_lladdr_t *lladdr, const uip_ipaddr_t *ipaddr);
/* Returns 1 if EDC is frozen, i.e. we are not allowed to change edc */
int orpl_is_edc_frozen();
/* Returns 1 routing sets are active, i.e. we can start inserting and merging */
int orpl_are_routing_set_active();
/* Returns 1 if the node is root of ORPL */
int orpl_is_root();
/* Returns current EDC of the node */
rpl_rank_t orpl_current_edc();
/* Returns 1 if addr is the global ip of a reachable neighbor */
int orpl_is_reachable_neighbor(const uip_ipaddr_t *ipaddr);
/* Insert a packet sequence number to the blacklist
 * (used for false positive recovery) */
void orpl_blacklist_insert(uint32_t seqno);
/* Returns 1 is the sequence number is contained in the blacklist */
int orpl_blacklist_contains(uint32_t seqno);
/* A packet was routed downwards successfully, insert it into our
 * history. Used during false positive recovery. */
void orpl_acked_down_insert(uint32_t seqno, const rimeaddr_t *child);
/* Returns 1 if a given packet is in the acked down history */
int orpl_acked_down_contains(uint32_t seqno, const rimeaddr_t *child);
/* Callback function called after routing set transmissions */
void orpl_routing_set_sent(void *ptr, int status, int transmissions);
/* Function called when the trickle timer expires */
void orpl_trickle_callback(rpl_instance_t *instance);
/* Callback function for every ACK received while broadcasting.
 * Used for beacon counting. */
void orpl_broadcast_acked(const rimeaddr_t *receiver);
/* Callback function at the end of a every broadcast
 * Used for beacon counting. */
void orpl_broadcast_done();
/* Update the current EDC (rank of the node) */
void orpl_update_edc(rpl_rank_t edc);
/* ORPL initialization */
void orpl_init(const uip_ipaddr_t *ipaddr, int is_root, int up_only);
/* Function that computes the metric EDC */
rpl_rank_t orpl_calculate_edc(int verbose);

#endif /* __ORPL_H__ */
