/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         Orchestra: a slotframe dedicated to unicast data transmission.
 *         Nodes transmit at a timeslot defined as hash(MAC) % ORCHESTRA_SB_UNICAST_PERIOD
 *         Nodes listen at: for each nbr in RPL children and RPL preferred parent,
 *                                             hash(nbr.MAC) % ORCHESTRA_SB_UNICAST_PERIOD
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "orchestra.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/rime/rime.h" /* Needed for so-called rime-sniffer */

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#if ORCHESTRA_COLLISION_FREE_HASH
#define UNICAST_SLOT_IS_SHARED    (ORCHESTRA_SB_UNICAST_PERIOD < ORCHESTRA_MAX_HASH)
#else /* ORCHESTRA_COLLISION_FREE_HASH */
#define UNICAST_SLOT_IS_SHARED    1
#endif /* ORCHESTRA_COLLISION_FREE_HASH */

static struct tsch_slotframe *sf_sb;
/* A net-layer sniffer for packets sent and received */
static void orchestra_packet_received(void);
static void orchestra_packet_sent(int mac_status);
RIME_SNIFFER(orhcestra_sniffer, orchestra_packet_received, orchestra_packet_sent);

/* The current RPL preferred parent's link-layer address */
linkaddr_t curr_parent_linkaddr;
/* Set to one only after getting an ACK from our preferred parent */
int curr_parent_confirmed = 0;

/*---------------------------------------------------------------------------*/
static int
neighbor_has_uc_link(const linkaddr_t *linkaddr)
{
  if(curr_parent_confirmed && linkaddr_cmp(&curr_parent_linkaddr, linkaddr)) {
    return 1;
  }
#if UIP_DS6_ROUTE_NB > 0
  if(nbr_table_get_from_lladdr(nbr_routes, (linkaddr_t *)linkaddr) != NULL) {
    return 1;
  }
#endif
  return 0;
}
/*---------------------------------------------------------------------------*/
void
orchestra_callback_ready_to_send()
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  int has_uc_link = neighbor_has_uc_link(dest);
#if WITH_TSCH_SLOTFRAME_SELECTOR
  if(has_uc_link) {
    packetbuf_set_attr(PACKETBUF_ATTR_TSCH_SLOTFRAME, 2);
  } else {
    packetbuf_set_attr(PACKETBUF_ATTR_TSCH_SLOTFRAME, 1);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
orchestra_packet_received(void)
{
}
/*---------------------------------------------------------------------------*/
static void
orchestra_packet_sent(int mac_status)
{
#ifdef WITH_ATTR_RPL_DAO_LIFETIME
  if(mac_status == MAC_TX_OK
      && packetbuf_attr(PACKETBUF_ATTR_PROTO) == UIP_PROTO_ICMP6
      && packetbuf_attr(PACKETBUF_ATTR_RPL_DAO_LIFETIME) > 0
      ) {
    if(curr_parent_confirmed == 0
        && !linkaddr_cmp(&curr_parent_linkaddr, &linkaddr_null)
        && linkaddr_cmp(&curr_parent_linkaddr, packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) {
      curr_parent_confirmed = 1;
      PRINTF("Orchestra: preferred parent confirmed %d\n", LOG_NODEID_FROM_LINKADDR(&curr_parent_linkaddr));
    }
  }
#endif
}
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot(linkaddr_t *addr) {
  return orchestra_linkaddr_hash(addr) % ORCHESTRA_SB_UNICAST_PERIOD;
}
/*---------------------------------------------------------------------------*/
static void
rx_neighbor_add(linkaddr_t *linkaddr) {
  uint16_t timeslot = get_node_timeslot(linkaddr);
  tsch_schedule_add_link(sf_sb,
      LINK_OPTION_RX,
      LINK_TYPE_NORMAL, &tsch_broadcast_address,
      timeslot, 2);
  PRINTF("Orchestra: adding Rx link at %u\n", timeslot);
}
/*---------------------------------------------------------------------------*/
static void
rx_neighbor_rm(linkaddr_t *linkaddr) {
  uint16_t timeslot = get_node_timeslot(linkaddr);
  struct tsch_link *l = tsch_schedule_get_link_from_timeslot(sf_sb, timeslot);
  if(l == NULL) {
    return;
  }
  /* Does our current parent need this timeslot? */
  if(timeslot == get_node_timeslot(&curr_parent_linkaddr)) {
    /* Yes, this timeslot is being used, return */
    return;
  }
  /* Does any other child need this timeslot?
   * (lookup all route next hops) */
  nbr_table_item_t *item = nbr_table_head(nbr_routes);
  while(item != NULL) {
    linkaddr_t *addr = nbr_table_get_lladdr(nbr_routes, item);
    if(timeslot == get_node_timeslot(addr)) {
      /* Yes, this timeslot is being used, return */
      return;
    }
    item = nbr_table_next(nbr_routes, item);
  }
  tsch_schedule_remove_link(sf_sb, l);
  PRINTF("Orchestra: removing Rx link at %u\n", timeslot);
}
/*---------------------------------------------------------------------------*/
static void
update_parent(linkaddr_t *linkaddr) {
  if(!linkaddr_cmp(&curr_parent_linkaddr, linkaddr)) {
    if(linkaddr != NULL) {
      linkaddr_copy(&curr_parent_linkaddr, linkaddr);
    } else {
      linkaddr_copy(&curr_parent_linkaddr, &linkaddr_null);
    }
    curr_parent_confirmed = 0;
    PRINTF("Orchestra: new preferred parent %d\n", LOG_NODEID_FROM_LINKADDR(&curr_parent_linkaddr));
    rx_neighbor_add(linkaddr);
  }
}
/*---------------------------------------------------------------------------*/
void
orchestra_sf_sb_unicast_new_time_source(struct tsch_neighbor *old, struct tsch_neighbor *new)
{
  if(new != old) {
    update_parent(new != NULL ? &new->addr : NULL);
  }
}
/*---------------------------------------------------------------------------*/
void
orchestra_callback_routing_neighbor_added(linkaddr_t *linkaddr)
{
  PRINTF("Orchestra: new child %d\n", LOG_NODEID_FROM_LINKADDR(linkaddr));
  rx_neighbor_add(linkaddr);
}
/*---------------------------------------------------------------------------*/
void
orchestra_callback_routing_neighbor_removed(linkaddr_t *linkaddr)
{
  PRINTF("Orchestra: lost child %d\n", LOG_NODEID_FROM_LINKADDR(linkaddr));
  rx_neighbor_rm(linkaddr);
}
/*---------------------------------------------------------------------------*/
void
orchestra_sf_sb_unicast_init(uint16_t slotframe_handle)
{
  linkaddr_copy(&curr_parent_linkaddr, &linkaddr_null);
  /* Snoop on packet transmission to know if our parent is confirmed
   * (i.e. has ACKed at least one of our packets since we decided to use it as a parent) */
  rime_sniffer_add(&orhcestra_sniffer);
  /* Slotframe for unicast transmissions */
  sf_sb = tsch_schedule_add_slotframe(slotframe_handle, ORCHESTRA_SB_UNICAST_PERIOD);
  uint16_t timeslot = get_node_timeslot(&linkaddr_node_addr);
  tsch_schedule_add_link(sf_sb,
            LINK_OPTION_TX | (UNICAST_SLOT_IS_SHARED ? LINK_OPTION_SHARED : 0),
            LINK_TYPE_NORMAL, &tsch_broadcast_address,
            timeslot, 2);
  PRINTF("Orchestra: adding Tx link at %u\n", timeslot);
}
