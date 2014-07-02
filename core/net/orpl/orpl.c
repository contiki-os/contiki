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
 *         ORPL core functions
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "orpl.h"
#include "orpl-anycast.h"
#include "orpl-routing-set.h"
#include "net/packetbuf.h"
#include "net/simple-udp.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl-private.h"
#include "lib/random.h"
#include "dev/leds.h"
#include <string.h>

#if WITH_ORPL

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

/* The global IPv6 address in use */
uip_ipaddr_t global_ipv6;

/* Flag used to tell lower layers that the current UDP transmission
 * is a routing set, so that the desired callback function is called
 * after each transmission attempt */
int sending_routing_set = 0;

/* Total number of broadcast sent */
uint32_t orpl_broadcast_count = 0;

/* Defines whether all neighbors we have a good link to should be included
 * in our routing set, regardless of them being children or not. */
#define ORPL_ALL_NEIGHBORS_IN_ROUTING_SET 1

/* When set:
 * - stop updating EDC after N seconds
 * - start updating Routing sets only after N+1 seconds
 * - don't age routing sets */
#ifndef FREEZE_TOPOLOGY
#define FREEZE_TOPOLOGY 1
#endif

#if FREEZE_TOPOLOGY
#define UPDATE_EDC_MAX_TIME 4*60
#define UPDATE_ROUTING_SET_MIN_TIME 5*60
#else
#define UPDATE_EDC_MAX_TIME 0
#define UPDATE_ROUTING_SET_MIN_TIME 0
#endif

/* PRR threshold for considering a neighbor as usable */
#define NEIGHBOR_PRR_THRESHOLD 50

/* Rank changes of more than RANK_MAX_CHANGE trigger a trickle timer reset */
#define RANK_MAX_CHANGE (2*EDC_DIVISOR)
/* The last boradcasted EDC */
static uint16_t last_broadcasted_edc = 0xffff;

/* Set to 1 when only upwards routing is enabled */
static int orpl_up_only = 0;
/* A flag that tells whether we are root or not */
static int is_root_flag = 0;

/* UDP port used for routing set broadcasting */
#define ROUTING_SET_PORT 4444
/* UDP connection used for routing set broadcasting */
static struct simple_udp_connection routing_set_connection;
/* Multicast IP address used for routing set broadcasting */
static uip_ipaddr_t routing_set_addr;
/* Data structure used for routing set broadcasting. Also includes
 * current edc. */
struct routing_set_broadcast_s {
  uint16_t edc;
  union {
    struct routing_set_s rs;
    uint8_t padding[64];
  };
};

/* Timer for periodic broadcast of routing sets */
static struct ctimer routing_set_broadcast_timer;

/* Data structure for storing the history of packets that were
 * acked while routing downwards. Used during recovery to ensure
 * only parents that forwarded the packet down before will take
 * it back (avoids duplicates during in-depth exploration) */
struct packet_acked_down_s {
  uint32_t seqno;
  rimeaddr_t child;
};
/* Size of the packet acked down history */
#define ACKED_DOWN_SIZE 32
/* The histrory of packets acked down */
static struct packet_acked_down_s acked_down[ACKED_DOWN_SIZE];

/* The current RPL instance */
static rpl_instance_t *curr_instance;

/* Routing set false positive blacklist */
#define BLACKLIST_SIZE 16
static uint32_t blacklisted_seqnos[BLACKLIST_SIZE];

static void broadcast_routing_set(void *ptr);

/* Seqno of the next packet to be sent */
static uint32_t current_seqno = 0;

/* Set the 32-bit ORPL sequence number in packetbuf */
void
orpl_packetbuf_set_seqno(uint32_t seqno)
{
  packetbuf_set_attr(PACKETBUF_ATTR_ORPL_SEQNO0, seqno >> 16);
  packetbuf_set_attr(PACKETBUF_ATTR_ORPL_SEQNO1, seqno);
}

/* Get the 32-bit ORPL sequence number from packetbuf */
uint32_t
orpl_packetbuf_seqno()
{
  return ((uint32_t)packetbuf_attr(PACKETBUF_ATTR_ORPL_SEQNO0) << 16) |
    packetbuf_attr(PACKETBUF_ATTR_ORPL_SEQNO1);
}

/* Get the current ORPL sequence number */
uint32_t
orpl_get_curr_seqno()
{
  uint32_t ret = current_seqno;
  current_seqno = 0; /* The app must set the seqno before next transmission */
  return ret;
}

/* Get a new ORPL sequence number */
uint32_t
orpl_get_new_seqno()
{
  if(current_seqno == 0) {
    current_seqno = random_rand();
  }
  return ++current_seqno;
}

/* Set the current ORPL sequence number before sending */
void orpl_set_curr_seqno(uint32_t seqno)
{
  current_seqno = seqno;
}

/* Build a global link-layer address from an IPv6 based on its UUID64 */
void
lladdr_from_ipaddr_uuid(uip_lladdr_t *lladdr, const uip_ipaddr_t *ipaddr)
{
#if (UIP_LLADDR_LEN == 8)
  memcpy(lladdr, ipaddr->u8 + 8, UIP_LLADDR_LEN);
  lladdr->addr[0] ^= 0x02;
#else
#error orpl.c supports only EUI-64 identifiers
#endif
}

/* Build a global IPv6 address from a link-local IPv6 address */
static void
global_ipaddr_from_llipaddr(uip_ipaddr_t *gipaddr, const uip_ipaddr_t *llipaddr)
{
  uip_ip6addr(gipaddr, 0, 0, 0, 0, 0, 0, 0, 0);
  memcpy(gipaddr, &global_ipv6, 8);
  memcpy(gipaddr->u8+8, llipaddr->u8+8, 8);
}

/* Returns 1 if EDC is frozen, i.e. we are not allowed to change edc */
int
orpl_is_edc_frozen()
{
  return FREEZE_TOPOLOGY && orpl_up_only == 0 && clock_seconds() > UPDATE_EDC_MAX_TIME;
}

/* Returns 1 routing sets are active, i.e. we can start inserting and merging */
int
orpl_are_routing_set_active()
{
  return orpl_up_only == 0 && (FREEZE_TOPOLOGY || clock_seconds() > UPDATE_ROUTING_SET_MIN_TIME);
}

/* Returns 1 if the node is root of ORPL */
int
orpl_is_root()
{
  return is_root_flag;
}

/* Returns current EDC of the node */
rpl_rank_t
orpl_current_edc()
{
  rpl_dag_t *dag = rpl_get_any_dag();
  return dag == NULL ? 0xffff : dag->rank;
}

/* Returns 1 if addr is link-layer address of a reachable neighbor */
static int
orpl_is_reachable_neighbor_from_lladdr(const uip_lladdr_t *lladdr)
{
  /* We don't consider neighbors as reachable before we have send
   * at least 4 broadcasts to estimate link quality */
  if(lladdr != NULL && orpl_broadcast_count >= 4) {
    rpl_parent_t *p = rpl_get_parent(lladdr);
    uint16_t bc_count = p == NULL ? 0 : p->bc_ackcount;
    return 100*bc_count/orpl_broadcast_count >= NEIGHBOR_PRR_THRESHOLD;
  } else {
    return 0;
  }
}

/* Returns 1 if addr is the global ip of a reachable neighbor */
int
orpl_is_reachable_neighbor(const uip_ipaddr_t *ipaddr)
{
  uip_lladdr_t lladdr;
  lladdr_from_ipaddr_uuid(&lladdr, ipaddr);
  return orpl_is_reachable_neighbor_from_lladdr(&lladdr);
}

/* Returns 1 if addr is the global ip of a reachable child */
static int
orpl_is_reachable_child(const uip_ipaddr_t *ipaddr)
{
  if(ipaddr) {
    uip_lladdr_t lladdr;
    lladdr_from_ipaddr_uuid(&lladdr, ipaddr);
    if(orpl_is_reachable_neighbor_from_lladdr(&lladdr)) {
      rpl_rank_t curr_edc = orpl_current_edc();
      rpl_rank_t neighbor_edc = rpl_get_parent_rank(&lladdr);
      return neighbor_edc > ORPL_EDC_W && (neighbor_edc - ORPL_EDC_W) > curr_edc;
    }
  }
  return 0;
}

/* Insert a packet sequence number to the blacklist
 * (used for false positive recovery) */
void
orpl_blacklist_insert(uint32_t seqno)
{
  ORPL_LOG("ORPL: blacklisting %lx\n", seqno);
  int i;
  for(i = BLACKLIST_SIZE - 1; i > 0; --i) {
    blacklisted_seqnos[i] = blacklisted_seqnos[i - 1];
  }
  blacklisted_seqnos[0] = seqno;
}

/* Returns 1 is the sequence number is contained in the blacklist */
int
orpl_blacklist_contains(uint32_t seqno)
{
  int i;
  for(i = 0; i < BLACKLIST_SIZE; ++i) {
    if(seqno == blacklisted_seqnos[i]) {
      return 1;
    }
  }
  return 0;
}

/* A packet was routed downwards successfully, insert it into our
 * history. Used during false positive recovery. */
void
orpl_acked_down_insert(uint32_t seqno, const rimeaddr_t *child)
{
  ORPL_LOG("ORPL: inserted ack down %lx %u\n", seqno,
      ORPL_LOG_NODEID_FROM_RIMEADDR(child));
  int i;
  for(i = ACKED_DOWN_SIZE - 1; i > 0; --i) {
    acked_down[i] = acked_down[i - 1];
  }
  acked_down[0].seqno = seqno;
  rimeaddr_copy(&acked_down[0].child, child);
}

/* Returns 1 if a given packet is in the acked down history */
int
orpl_acked_down_contains(uint32_t seqno, const rimeaddr_t *child)
{
  int i;
  for(i = 0; i < ACKED_DOWN_SIZE; ++i) {
    if(seqno == acked_down[i].seqno && rimeaddr_cmp(child, &acked_down[i].child)) {
      return 1;
    }
  }
  return 0;
}

/* Schedule a routing set broadcast in a few seconds */
static void
request_routing_set_broadcast()
{
  ORPL_LOG("ORPL: requesting routing set broadcast\n");
  ctimer_set(&routing_set_broadcast_timer, random_rand() % (32 * CLOCK_SECOND), broadcast_routing_set, NULL);
}

/* Broadcast our routing set to all neighbors */
static void
broadcast_routing_set(void *ptr)
{
  if(!orpl_are_routing_set_active()) {
    request_routing_set_broadcast();
  } else {
    struct routing_set_broadcast_s routing_set_broadcast;
    rpl_rank_t curr_edc = orpl_current_edc();

    ORPL_LOG("ORPL: broadcast routing set (edc=%u)\n", curr_edc);

    /* Build data structure to be broadcasted */
    last_broadcasted_edc = curr_edc;
    routing_set_broadcast.edc = curr_edc;
    memcpy(&routing_set_broadcast.rs, orpl_routing_set_get_active(), sizeof(struct routing_set_s));

    /* Proceed to UDP transmission */
    sending_routing_set = 1;
    simple_udp_sendto(&routing_set_connection, &routing_set_broadcast, sizeof(struct routing_set_broadcast_s), &routing_set_addr);
    sending_routing_set = 0;
  }
}

/* Callback function called after routing set transmissions */
void
orpl_routing_set_sent(void *ptr, int status, int transmissions)
{
  if(status == MAC_TX_COLLISION) {
    request_routing_set_broadcast();
  }
}

/* UDP callback function for received routing sets */
static void
udp_received_routing_set(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *payload,
         uint16_t datalen)
{
  struct routing_set_broadcast_s *data = (struct routing_set_broadcast_s *)payload;

  /* EDC: store edc as neighbor attribute, update metric */
  uint16_t neighbor_edc = data->edc;
  rpl_set_parent_rank((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER), neighbor_edc);
  rpl_recalculate_ranks();

  /* Calculate neighbor's global IP address */
  uip_ipaddr_t sender_global_ipaddr;
  global_ipaddr_from_llipaddr(&sender_global_ipaddr, sender_addr);

  if(orpl_are_routing_set_active() && orpl_is_reachable_neighbor(&sender_global_ipaddr)) {
    int bit_count_before = orpl_routing_set_count_bits();
    int bit_count_after;
    int is_reachable_child = orpl_is_reachable_child(&sender_global_ipaddr);

    if(is_reachable_child || ORPL_ALL_NEIGHBORS_IN_ROUTING_SET) {
      /* Insert the neighbor in our routing set */
      orpl_routing_set_insert(&sender_global_ipaddr);
      ORPL_LOG("ORPL: inserting neighbor into routing set: %u ",
          ORPL_LOG_NODEID_FROM_IPADDR(&sender_global_ipaddr));
      ORPL_LOG_IPADDR(&sender_global_ipaddr);
      ORPL_LOG("\n");
    }

    if(is_reachable_child) {
      /* The neighbor is a child, merge its routing set in ours */
      orpl_routing_set_merge((const struct routing_set_s *)
          &((struct routing_set_broadcast_s*)data)->rs);
      ORPL_LOG("ORPL: merging routing set from: %u ",
          ORPL_LOG_NODEID_FROM_IPADDR(&sender_global_ipaddr));
      ORPL_LOG_IPADDR(&sender_global_ipaddr);
      ORPL_LOG("\n");
    }

    /* Broadcast our routing set again if it has changed */
    bit_count_after = orpl_routing_set_count_bits();
    if(curr_instance && bit_count_after != bit_count_before) {
      request_routing_set_broadcast();
    }
  }
}

/* Function called when the trickle timer expires */
void
orpl_trickle_callback(rpl_instance_t *instance)
{
  curr_instance = instance;

  if(orpl_are_routing_set_active()) {
#if !FREEZE_TOPOLOGY
    /* Swap routing sets to implement ageing */
    ORPL_LOG("ORPL: swapping routing sets\n");
    orpl_routing_set_swap();
#endif /* FREEZE_TOPOLOGY */

    /* Request transmission of routing set */
    request_routing_set_broadcast();
  }

  /* We recalculate the ranks periodically */
  rpl_recalculate_ranks();
}

/* Callback function for every ACK received while broadcasting.
 * Used for beacon counting. */
void
orpl_broadcast_acked(const rimeaddr_t *receiver)
{
  rpl_parent_t *p = rpl_get_parent((uip_lladdr_t *)receiver);
  if(p != NULL) {
    p->bc_ackcount++;
    if(p->bc_ackcount > orpl_broadcast_count+1) {
      p->bc_ackcount = orpl_broadcast_count+1;
    }
  }
}

/* Callback function at the end of a every broadcast
 * Used for beacon counting. */
void
orpl_broadcast_done()
{
  /* Update global broacast count */
  orpl_broadcast_count++;

  /* Loop over all neighbors and insert the reachable ones into
     out routing set */
  if(orpl_are_routing_set_active()) {
    rpl_parent_t *p;
    for(p = nbr_table_head(rpl_parents);
        p != NULL;
        p = nbr_table_next(rpl_parents, p)) {
      uip_ipaddr_t *nbr_ipaddr = rpl_get_parent_ipaddr(p);
      uip_ipaddr_t nbr_global_ipaddr;
      global_ipaddr_from_llipaddr(&nbr_global_ipaddr, nbr_ipaddr);

      if(orpl_is_reachable_child(&nbr_global_ipaddr)) {
        orpl_routing_set_insert(&nbr_global_ipaddr);
        ORPL_LOG("ORPL: inserting neighbor into routing set: %u ",
            ORPL_LOG_NODEID_FROM_IPADDR(&nbr_global_ipaddr));
        ORPL_LOG_IPADDR(&nbr_global_ipaddr);
        ORPL_LOG("\n");
      }
    }
  }
}

/* Update the current EDC (rank of the node) */
void
orpl_update_edc(rpl_rank_t edc)
{
  rpl_rank_t curr_edc = orpl_current_edc();
  rpl_dag_t *dag = rpl_get_any_dag();

  if(dag) {
    dag->rank = edc;
  }

  /* Reset DIO timer if the edc changed significantly */
  if(curr_instance && last_broadcasted_edc != 0xffff &&
      ((last_broadcasted_edc > curr_edc && last_broadcasted_edc - curr_edc > RANK_MAX_CHANGE) ||
      (curr_edc > last_broadcasted_edc && curr_edc - last_broadcasted_edc > RANK_MAX_CHANGE))) {
    PRINTF("ORPL: reset DIO timer (edc changed from %u to %u)\n", last_broadcasted_edc, curr_edc);
    last_broadcasted_edc = curr_edc;
    rpl_reset_dio_timer(curr_instance);
  }

  /* Update EDC annotation */
  if(edc != curr_edc) {
    ANNOTATE("#A edc=%u.%u\n", edc/EDC_DIVISOR,
        (10 * (edc % EDC_DIVISOR)) / EDC_DIVISOR);
  }

  /* Update EDC */
  curr_edc = edc;
}

/* ORPL initialization */
void
orpl_init(const uip_ipaddr_t *ipaddr, int is_root, int up_only)
{
  orpl_up_only = up_only;
  is_root_flag = is_root;

  if(is_root) {
    ANNOTATE("#A color=red\n");
    ANNOTATE("#A edc=0.0\n");
    /* Set root EDC to 0 */
    orpl_update_edc(0);
  }

  /* Initialize global address */
  memcpy(&global_ipv6, ipaddr, 16);

  /* Initialize routing set module */
  orpl_anycast_init();
  orpl_routing_set_init();

  /* Set up multicast UDP connectoin for dissemination of routing sets */
  uip_create_linklocal_allnodes_mcast(&routing_set_addr);
  simple_udp_register(&routing_set_connection, ROUTING_SET_PORT,
                        NULL, ROUTING_SET_PORT,
                        udp_received_routing_set);

}

#endif /* WITH_ORPL */
