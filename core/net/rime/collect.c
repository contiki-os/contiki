/**
 * \addtogroup rimecollect
 * @{
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: collect.c,v 1.50 2010/09/08 19:21:45 adamdunkels Exp $
 */

/**
 * \file
 *         Tree-based hop-by-hop reliable data collection
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime.h"
#include "net/rime/collect.h"
#include "net/rime/collect-neighbor.h"
#include "net/rime/collect-link-estimate.h"

#include "net/packetqueue.h"

#include "dev/radio-sensor.h"

#include "lib/random.h"

#include <string.h>
#include <stdio.h>
#include <stddef.h>

static const struct packetbuf_attrlist attributes[] =
  {
    COLLECT_ATTRIBUTES
    PACKETBUF_ATTR_LAST
  };


/* The recent_packets list holds the sequence number, the originator,
   and the connection for packets that have been recently
   forwarded. This list is maintained to avoid forwarding duplicate
   packets. */
#define NUM_RECENT_PACKETS 8

struct recent_packet {
  struct collect_conn *conn;
  rimeaddr_t originator;
  uint8_t seqno;
};

static struct recent_packet recent_packets[NUM_RECENT_PACKETS];
static uint8_t recent_packet_ptr;


/* This is the header of data packets. The header comtains the routing
   metric of the last hop sender. This is used to avoid routing loops:
   if a node receives a packet with a lower routing metric than its
   own, it drops the packet. */
struct data_msg_hdr {
  uint8_t flags, dummy;
  uint16_t rtmetric;
};


/* This is the header of ACK packets. It contains a flags field that
   indicates if the node is congested (ACK_FLAGS_CONGESTED), if the
   packet was dropped (ACK_FLAGS_DROPPED), and if a packet was dropped
   due to its lifetime was exceeded (ACK_FLAGS_LIFETIME_EXCEEDED). The
   flags can contain any combination of the flags. The ACK header also
   contains the routing metric of the node that sends tha ACK. This is
   used to keep an up-to-date routing state in the network. */
struct ack_msg {
  uint8_t flags, dummy;
  uint16_t rtmetric;
};

#define ACK_FLAGS_CONGESTED         0x80
#define ACK_FLAGS_DROPPED           0x40
#define ACK_FLAGS_LIFETIME_EXCEEDED 0x20


/* These are configuration knobs that normally should not be
   tweaked. MAX_MAC_REXMITS defines how many times the underlying CSMA
   MAC layer should attempt to resend a data packet before giving
   up. The MAX_ACK_MAC_REXMITS defines how many times the MAC layer
   should resend ACK packets. The REXMIT_TIME is the lowest
   retransmission timeout at the network layer. It is exponentially
   increased for every new network layer retransmission. The
   FORWARD_PACKET_LIFETIME is the maximum time a packet is held in the
   forwarding queue before it is removed. The MAX_SENDING_QUEUE
   specifies the maximum length of the output queue. If the queue is
   full, incoming packets are dropped instead of being forwarded. */
#define MAX_MAC_REXMITS            3
#define MAX_ACK_MAC_REXMITS        3
#define REXMIT_TIME                CLOCK_SECOND * 2
#define FORWARD_PACKET_LIFETIME    (6 * (REXMIT_TIME) << 3)
#define MAX_SENDING_QUEUE          6
PACKETQUEUE(sending_queue, MAX_SENDING_QUEUE);


/* These specifiy the sink's routing metric (0) and the maximum
   routing metric. If a node has routing metric zero, it is the
   sink. If a node has the maximum routing metric, it has no route to
   a sink. */
#define RTMETRIC_SINK              0
#define RTMETRIC_MAX               COLLECT_MAX_DEPTH

/* Here we define what we mean with a significantly improved
   rtmetric. This is used to determine when a new parent should be
   chosen over an old parent and when to begin more rapidly advertise
   a new rtmetric. */
#define SIGNIFICANT_RTMETRIC_IMPROVEMENT (COLLECT_LINK_ESTIMATE_UNIT +  \
                                          COLLECT_LINK_ESTIMATE_UNIT / 1)

/* This defines the maximum hops that a packet can take before it is
   dropped. */
#define MAX_HOPLIM                 15


/* COLLECT_CONF_ANNOUNCEMENTS defines if the Collect implementation
   should use Contiki's announcement primitive to announce its routes
   or if it should use periodic broadcasts. */
#ifndef COLLECT_CONF_ANNOUNCEMENTS
#define COLLECT_ANNOUNCEMENTS 0
#else
#define COLLECT_ANNOUNCEMENTS COLLECT_CONF_ANNOUNCEMENTS
#endif /* COLLECT_CONF_ANNOUNCEMENTS */

/* The ANNOUNCEMENT_SCAN_TIME defines for how long the Collect
   implementation should listen for announcements from other nodes
   when it requires a route. */
#ifdef ANNOUNCEMENT_CONF_PERIOD
#define ANNOUNCEMENT_SCAN_TIME ANNOUNCEMENT_CONF_PERIOD
#else /* ANNOUNCEMENT_CONF_PERIOD */
#define ANNOUNCEMENT_SCAN_TIME CLOCK_SECOND
#endif /* ANNOUNCEMENT_CONF_PERIOD */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static void send_queued_packet(void);
static void retransmit_callback(void *ptr);
/*---------------------------------------------------------------------------*/
/**
 * This function computes the current rtmetric by adding the last
 * known rtmetric from our parent with the link estimate to the
 * parent.
 *
 */
static uint8_t
rtmetric_compute(struct collect_conn *tc)
{
  struct collect_neighbor *n;
  uint8_t rtmetric = RTMETRIC_MAX;

  /* This function computes the current rtmetric for this node. It
     uses the rtmetric of the parent node in the tree and adds the
     current link estimate from us to the parent node. */

  /* The collect connection structure stores the address of its
     current parent. We look up the neighbor identification struct in
     the collect-neighbor list. */
  n = collect_neighbor_find(&tc->parent);

  /* If n is NULL, we have no best neighbor. Thus our rtmetric is
     then COLLECT_RTMETRIC_MAX. */
  if(n == NULL) {
    rtmetric = RTMETRIC_MAX;
  } else {
    /* Our rtmetric is the rtmetric of our parent neighbor plus
       the expected transmissions to reach that neighbor. */
    rtmetric = collect_neighbor_rtmetric(n);
  }

  return rtmetric;
}
/*---------------------------------------------------------------------------*/
/**
 * This function is called to update the current parent node. The
 * parent may change if new routing information has been found, for
 * example if a new node with a lower rtmetric and link estimate has
 * appeared.
 *
 */
static void
update_parent(struct collect_conn *tc)
{
  struct collect_neighbor *current;
  struct collect_neighbor *best;

  /* We grab the collect_neighbor struct of our current parent. */
  current = collect_neighbor_find(&tc->parent);

  /* We call the collect_neighbor module to find the current best
     parent. */
  best = collect_neighbor_best();

  /* We check if we need to switch parent. Switching parent is done in
     the following situations:

     * We do not have a current parent.
     * The best parent is significantly better than the current parent.

     If we do not have a current parent, and have found a best parent,
     we simply use the new best parent.

     If we already have a current parent, but have found a new parent
     that is better, we employ a heuristic to avoid switching parents
     too often. The new parent must be significantly better than the
     current parent. Being "significantly better" is defined as having
     an rtmetric that is has a difference of at least 1.5 times the
     COLLECT_LINK_ESTIMATE_UNIT. This is derived from the experience
     by Gnawali et al (SenSys 2009). */

  if(best != NULL) {
    if(current == NULL) {
      /* New parent. */
      PRINTF("update_parent: new parent %d.%d\n", best->addr.u8[0], best->addr.u8[1]);
      rimeaddr_copy(&tc->parent, &best->addr);
    } else {
      PRINTF("#L %d 0\n", tc->parent.u8[0]);
      if(collect_neighbor_rtmetric(best) + SIGNIFICANT_RTMETRIC_IMPROVEMENT <
         collect_neighbor_rtmetric(current)) {
        /* We switch parent. */
        PRINTF("update_parent: new parent %d.%d old parent %d.%d\n",
               best->addr.u8[0], best->addr.u8[1],
               tc->parent.u8[0], tc->parent.u8[1]);
        rimeaddr_copy(&tc->parent, &best->addr);
      }
    }
    PRINTF("#L %d 1\n", tc->parent.u8[0]);
  } else {
    /* No parent. */
    PRINTF("#L %d 0\n", tc->parent.u8[0]);
    rimeaddr_copy(&tc->parent, &rimeaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * This function is called whenever there is a chance that the routing
 * metric has changed. The function goes through the list of neighbors
 * to compute the new routing metric. If the metric has changed, it
 * notifies neighbors.
 *
 *
 */
static void
update_rtmetric(struct collect_conn *tc)
{
  PRINTF("update_rtmetric: tc->rtmetric %d\n", tc->rtmetric);

  /* We should only update the rtmetric if we are not the sink. */
  if(tc->rtmetric != RTMETRIC_SINK) {
    uint8_t old_rtmetric, new_rtmetric;

    /* We remember the current (old) rtmetric for later. */
    old_rtmetric = tc->rtmetric;

    /* We may need to update our parent node so we do that now. */
    update_parent(tc);

    /* We compute the new rtmetric. */
    new_rtmetric = rtmetric_compute(tc);

    /* We sanity check our new rtmetric. */
    if(new_rtmetric == RTMETRIC_SINK) {
      /* Defensive programming: if the new rtmetric somehow got to be
         the rtmetric of the sink, there is a bug somewhere. To avoid
         destroying the network, we simply will not assume this new
         rtmetric. Instead, we set our rtmetric to maximum, to
         indicate that we have no sane route. */
      new_rtmetric = RTMETRIC_MAX;
    }

    /* We set our new rtmetric in the collect conn structure. Then we
       decide how we should announce this new rtmetric. */
    tc->rtmetric = new_rtmetric;

    if(tc->is_router) {
      /* If we are a router, we update our advertised rtmetric. */
      
#if COLLECT_ANNOUNCEMENTS
      announcement_set_value(&tc->announcement, tc->rtmetric);
#else /* COLLECT_ANNOUNCEMENTS */
      neighbor_discovery_set_val(&tc->neighbor_discovery_conn, tc->rtmetric);
#endif /* COLLECT_ANNOUNCEMENTS */

      /* If we now have a significantly better rtmetric than we had
         before, what we need to make sure that our neighbors find out
         about this quickly. */
      if(new_rtmetric + SIGNIFICANT_RTMETRIC_IMPROVEMENT < old_rtmetric) {
#if ! COLLECT_ANNOUNCEMENTS
        neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
#else /* ! COLLECT_ANNOUNCEMENTS */
        announcement_bump(&tc->announcement);
#endif /* ! COLLECT_ANNOUNCEMENTS */
      }
    }
    
    PRINTF("%d.%d: new rtmetric %d\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           tc->rtmetric);

    /* We got a new, working, route we send any queued packets we may have. */
    if(old_rtmetric == RTMETRIC_MAX && new_rtmetric != RTMETRIC_MAX) {
      PRINTF("Sending queued packet because rtmetric was max\n");
      send_queued_packet();
    }
  }

}
/*---------------------------------------------------------------------------*/
/**
 * This function is called when a queued packet should be sent
 * out. The function takes the first packet on the output queue, adds
 * the necessary packet attributes, and sends the packet to the
 * next-hop neighbor.
 *
 */
static void
send_queued_packet(void)
{
  struct queuebuf *q;
  struct collect_neighbor *n;
  struct packetqueue_item *i;
  struct collect_conn *c;

  /* Grab the first packet on the send queue. */
  i = packetqueue_first(&sending_queue);
  if(i == NULL) {
      PRINTF("%d.%d: nothing on queue\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    /* No packet on the queue, so there is nothing for us to send. */
    return;
  }

  /* Obtain the Collect connection on which this packet should be
     sent. */
  c = packetqueue_ptr(i);
  if(c == NULL) {
    /* c should not be NULL, but we check it just to be sure. */
    PRINTF("%d.%d: queue, c == NULL!\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    return;
  }

  if(c->sending) {
    /* If we are currently sending a packet, we wait until the
       packet is forwarded and try again then. */
    PRINTF("%d.%d: queue, c is sending\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    return;
  }

  /* We should send the first packet from the queue. */
  q = packetqueue_queuebuf(i);
  if(q != NULL) {
    /* Place the queued packet into the packetbuf. */
    queuebuf_to_packetbuf(q);

    /* Pick the neighbor to which to send the packet. We use the
       parent in the n->parent. */
    n = collect_neighbor_find(&c->parent);

    if(n != NULL) {
      clock_time_t time;
      uint8_t rexmit_time_scaling;

      /* If the connection had a neighbor, we construct the packet
         buffer attributes and set the appropriate flags in the
         Collect connection structure and send the packet. */
      
      PRINTF("%d.%d: sending packet to %d.%d with eseqno %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     n->addr.u8[0], n->addr.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID));

      /* Mark that we are currently sending a packet. */
      c->sending = 1;

      /* This is the first time we transmit this packet, so set
         transmissions to zero. */
      c->transmissions = 0;

      /* Remember that maximum amount of retransmissions we should
         make. This is stored inside a packet attribute in the packet
         on the send queue. */
      c->max_rexmits = packetbuf_attr(PACKETBUF_ATTR_MAX_REXMIT);

      /* Set the packet attributes: this packet wants an ACK, so we
         sent the PACKETBUF_ATTR_RELIABLE flag; the MAC should retry
         MAX_MAC_REXMITS times; and the PACKETBUF_ATTR_PACKET_ID is
         set to the current sequence number on the connection. */
      packetbuf_set_attr(PACKETBUF_ATTR_RELIABLE, 1);
      packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS, MAX_MAC_REXMITS);
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, c->seqno);

      /* Send the packet. */
      unicast_send(&c->unicast_conn, &n->addr);

      /* Compute the retransmission timeout and set up the
         retransmission timer. */
      rexmit_time_scaling = c->transmissions;
      if(rexmit_time_scaling > 3) {
        rexmit_time_scaling = 3;
      }
      time = REXMIT_TIME << rexmit_time_scaling;
      time = time / 2 + random_rand() % (time / 2);
      PRINTF("retransmission time %lu scaling %d\n", time, rexmit_time_scaling);
      ctimer_set(&c->retransmission_timer, time,
                 retransmit_callback, c);
    } else {
#if COLLECT_ANNOUNCEMENTS
#if COLLECT_CONF_WITH_LISTEN
      PRINTF("listen\n");
      announcement_listen(1);
      ctimer_set(&c->transmit_after_scan_timer, ANNOUNCEMENT_SCAN_TIME,
                 send_queued_packet, NULL);
#else /* COLLECT_CONF_WITH_LISTEN */
      announcement_set_value(&c->announcement, RTMETRIC_MAX);
      announcement_bump(&c->announcement);
#endif /* COLLECT_CONF_WITH_LISTEN */
#endif /* COLLECT_ANNOUNCEMENTS */
    }
  }
}
/*---------------------------------------------------------------------------*/
/**
 * This function is called 
 *
 */
static void
send_next_packet(struct collect_conn *tc)
{
  /* Cancel retransmission timer. */
  ctimer_stop(&tc->retransmission_timer);

  /* Remove the first packet on the queue, the packet that was just sent. */
  packetqueue_dequeue(&sending_queue);
  tc->seqno = (tc->seqno + 1) % (1 << COLLECT_PACKET_ID_BITS);
  tc->sending = 0;
  tc->transmissions = 0;

  PRINTF("sending next packet, seqno %d, queue len %d\n",
         tc->seqno, packetqueue_len(&sending_queue));

  /* Send the next packet in the queue, if any. */
  send_queued_packet();
}
/*---------------------------------------------------------------------------*/
static void
handle_ack(struct collect_conn *tc)
{
  struct ack_msg *msg;
  uint16_t rtmetric;
  struct collect_neighbor *n;

  PRINTF("handle_ack: sender %d.%d parent %d.%d, id %d seqno %d\n",
         packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
         packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1],
         tc->parent.u8[0], tc->parent.u8[1],
         packetbuf_attr(PACKETBUF_ATTR_PACKET_ID), tc->seqno);
  if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
                  &tc->parent) &&
     packetbuf_attr(PACKETBUF_ATTR_PACKET_ID) == tc->seqno) {

    msg = packetbuf_dataptr();
    memcpy(&rtmetric, &msg->rtmetric, sizeof(uint16_t));
    n = collect_neighbor_find(packetbuf_addr(PACKETBUF_ADDR_SENDER));
    if(n != NULL) {
      collect_neighbor_update_rtmetric(n, rtmetric);
      update_rtmetric(tc);
    }

    PRINTF("%d.%d: ACK from %d.%d after %d transmissions, flags %02x\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           tc->parent.u8[0], tc->parent.u8[1],
           tc->transmissions,
           msg->flags);

    if(!(msg->flags & ACK_FLAGS_DROPPED)) {
      send_next_packet(tc);
    }
  } 
}
/*---------------------------------------------------------------------------*/
static void
send_ack(struct collect_conn *tc, const rimeaddr_t *to, int flags)
{ struct ack_msg *ack;
  struct queuebuf *q;
  uint16_t packet_seqno, packet_eseqno;

  packet_seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);
  packet_eseqno = packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID);

  q = queuebuf_new_from_packetbuf();
  if(q != NULL) {

    packetbuf_clear();
    packetbuf_set_datalen(sizeof(struct ack_msg));
    ack = packetbuf_dataptr();
    memset(ack, 0, sizeof(struct ack_msg));
    ack->rtmetric = tc->rtmetric;
    ack->flags = flags;

    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, to);
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE, PACKETBUF_ATTR_PACKET_TYPE_ACK);
    packetbuf_set_attr(PACKETBUF_ATTR_RELIABLE, 0);
    packetbuf_set_attr(PACKETBUF_ATTR_ERELIABLE, 0);
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, packet_seqno);
    packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS, MAX_ACK_MAC_REXMITS);
    unicast_send(&tc->unicast_conn, to);

    PRINTF("%d.%d: collect: Sending ACK to %d.%d for %d (epacket_id %d)\n",
           rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
           to->u8[0],
           to->u8[1],
           packet_seqno, packet_eseqno);

    RIMESTATS_ADD(acktx);

    queuebuf_to_packetbuf(q);
    queuebuf_free(q);
  } else {
    PRINTF("%d.%d: collect: could not send ACK to %d.%d for %d: no queued buffers\n",
           rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
           to->u8[0], to->u8[1],
           packet_seqno);
  }
}
/*---------------------------------------------------------------------------*/
static void
node_packet_received(struct unicast_conn *c, const rimeaddr_t *from)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, unicast_conn));
  int i;

  /* To protect against sending duplicate packets, we keep a list of
     recently forwarded packet seqnos. If the seqno of the current
     packet exists in the list, we immediately send an ACK and drop
     the packet. */
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
     PACKETBUF_ATTR_PACKET_TYPE_DATA) {
    rimeaddr_t ack_to;
    uint8_t packet_seqno;

    /* Remember to whom we should send the ACK, since we reuse the
       packet buffer and its attributes when sending the ACK. */
    rimeaddr_copy(&ack_to, packetbuf_addr(PACKETBUF_ADDR_SENDER));
    packet_seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);

    for(i = 0; i < NUM_RECENT_PACKETS; i++) {
      if(recent_packets[i].conn == tc &&
         recent_packets[i].seqno == packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID) &&
         rimeaddr_cmp(&recent_packets[i].originator,
                      packetbuf_addr(PACKETBUF_ADDR_ESENDER))) {
        /* This is a duplicate of a packet we recently received, so we
           just send an ACK. */
        PRINTF("%d.%d: found duplicate packet from %d.%d with seqno %d, via %d.%d\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
               recent_packets[i].originator.u8[0], recent_packets[i].originator.u8[1],
               packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
               packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
               packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
        send_ack(tc, &ack_to, 0);
        return;
      }
    }

    /* Remember that we have seen this packet for later. */
    recent_packets[recent_packet_ptr].seqno = packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID);
    rimeaddr_copy(&recent_packets[recent_packet_ptr].originator,
                  packetbuf_addr(PACKETBUF_ADDR_ESENDER));
    recent_packets[recent_packet_ptr].conn = tc;

    recent_packet_ptr = (recent_packet_ptr + 1) % NUM_RECENT_PACKETS;

    /* If we are the sink, the packet has reached its final
       destination and we call the receive function. */
    if(tc->rtmetric == RTMETRIC_SINK) {

      /* We first send the ACK. */
      send_ack(tc, &ack_to, 0);

      PRINTF("%d.%d: sink received packet %d from %d.%d via %d.%d\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[0],
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[1],
             from->u8[0], from->u8[1]);

      /* Call receive function. */
      if(tc->cb->recv != NULL) {
        tc->cb->recv(packetbuf_addr(PACKETBUF_ADDR_ESENDER),
                     packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
                     packetbuf_attr(PACKETBUF_ATTR_HOPS));
      }
      return;
    } else if(packetbuf_attr(PACKETBUF_ATTR_TTL) > 1 &&
              tc->rtmetric != RTMETRIC_MAX) {

      /* If we are not the sink, we forward the packet to our best
         neighbor. First, we update the hop count and ttl. */
      packetbuf_set_attr(PACKETBUF_ATTR_HOPS,
                         packetbuf_attr(PACKETBUF_ATTR_HOPS) + 1);
      packetbuf_set_attr(PACKETBUF_ATTR_TTL,
                         packetbuf_attr(PACKETBUF_ATTR_TTL) - 1);

      PRINTF("%d.%d: packet received from %d.%d via %d.%d, sending %d, max_rexmits %d\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[0],
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[1],
             from->u8[0], from->u8[1], tc->sending,
             packetbuf_attr(PACKETBUF_ATTR_MAX_REXMIT));

      /* We try to enqueue the packet on the outgoing packet queue. If
         we are able to enqueue the packet, we send a positive ACK. If
         we are unable to enqueue the packet, we send a negative ACK
         to inform the sender that the packet was dropped due to
         memory problems. */
      if(packetqueue_enqueue_packetbuf(&sending_queue,
                                       FORWARD_PACKET_LIFETIME, tc)) {
        send_ack(tc, &ack_to, 0);
        send_queued_packet();
      } else {
        send_ack(tc, &ack_to, ACK_FLAGS_DROPPED | ACK_FLAGS_CONGESTED);
        PRINTF("%d.%d: packet dropped: no queue buffer available\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
    } else if(packetbuf_attr(PACKETBUF_ATTR_TTL) <= 1) {
      PRINTF("%d.%d: packet dropped: ttl %d\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_TTL));
      send_ack(tc, &ack_to, ACK_FLAGS_DROPPED | ACK_FLAGS_LIFETIME_EXCEEDED);
    }
  } else if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
            PACKETBUF_ATTR_PACKET_TYPE_ACK) {
    PRINTF("Collect: incoming ack %d from %d.%d (%d.%d) seqno %d (%d)\n",
           packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE),
           packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
           packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1],
           tc->parent.u8[0],
           tc->parent.u8[1],
           packetbuf_attr(PACKETBUF_ATTR_PACKET_ID),
           tc->seqno);
    handle_ack(tc);
  }
  return;
}
/*---------------------------------------------------------------------------*/
static void
node_packet_sent(struct unicast_conn *c, int status, int transmissions)
{
  uint16_t tx;
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, unicast_conn));

  /* For data packets, we record the number of transmissions */
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
     PACKETBUF_ATTR_PACKET_TYPE_DATA && transmissions > 0) {
    PRINTF("%d.%d: sent to %d.%d after %d transmissions\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           tc->parent.u8[0], tc->parent.u8[1],
           transmissions);

    tc->transmissions += transmissions;

    tx = tc->transmissions;

    /* Update neighgor with the number of transmissions. */
    PRINTF("Updating link estimate with %d (%d) transmissions\n",
           tc->transmissions, tx);
    collect_neighbor_tx(collect_neighbor_find(&tc->parent), tx);

    update_rtmetric(tc);
  }
}
/*---------------------------------------------------------------------------*/
static void
timedout(struct collect_conn *tc)
{
  PRINTF("%d.%d: timedout after %d retransmissions (max retransmissions %d): packet dropped\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1], tc->transmissions,
         tc->max_rexmits);

  tc->sending = 0;
  collect_neighbor_timedout(collect_neighbor_find(&tc->parent), tc->transmissions);
  update_rtmetric(tc);

  send_next_packet(tc);
}
/*---------------------------------------------------------------------------*/
static void
retransmit_callback(void *ptr)
{
  struct collect_conn *c = ptr;

  PRINTF("retransmit\n");
  if(c->transmissions >= c->max_rexmits) {
    timedout(c);
  } else {
    c->sending = 0;
    send_queued_packet();
  }
}
/*---------------------------------------------------------------------------*/
#if !COLLECT_ANNOUNCEMENTS
static void
adv_received(struct neighbor_discovery_conn *c, const rimeaddr_t *from,
	     uint16_t rtmetric)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, neighbor_discovery_conn));
  struct collect_neighbor *n;

  n = collect_neighbor_find(from);

  if(n == NULL) {
    collect_neighbor_add(from, rtmetric);
  } else {
    collect_neighbor_update_rtmetric(n, rtmetric);
    PRINTF("%d.%d: updating neighbor %d.%d, etx %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   n->addr.u8[0], n->addr.u8[1], rtmetric);
  }

  update_rtmetric(tc);
}
#else
static void
received_announcement(struct announcement *a, const rimeaddr_t *from,
		      uint16_t id, uint16_t value)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)a - offsetof(struct collect_conn, announcement));
  struct collect_neighbor *n;

  n = collect_neighbor_find(from);

  if(n == NULL) {
    collect_neighbor_add(from, value);
    PRINTF("%d.%d: new neighbor %d.%d, etx %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   from->u8[0], from->u8[1], value);
  } else {
    collect_neighbor_update_rtmetric(n, value);
    PRINTF("%d.%d: updating neighbor %d.%d, etx %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   n->addr.u8[0], n->addr.u8[1], value);
  }

  update_rtmetric(tc);

#if ! COLLECT_CONF_WITH_LISTEN
  if(value == RTMETRIC_MAX &&
     tc->rtmetric != RTMETRIC_MAX) {
    announcement_bump(&tc->announcement);
  }
#endif /* COLLECT_CONF_WITH_LISTEN */
}
#endif /* !COLLECT_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {node_packet_received,
                                                           node_packet_sent};
#if !COLLECT_ANNOUNCEMENTS
static const struct neighbor_discovery_callbacks neighbor_discovery_callbacks =
  { adv_received, NULL};
#endif /* !COLLECT_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
void
collect_open(struct collect_conn *tc, uint16_t channels,
             uint8_t is_router,
	     const struct collect_callbacks *cb)
{
  unicast_open(&tc->unicast_conn, channels + 1, &unicast_callbacks);
  channel_set_attributes(channels + 1, attributes);
  tc->rtmetric = RTMETRIC_MAX;
  tc->cb = cb;
  tc->is_router = is_router;
  tc->seqno = 10;
  collect_neighbor_init();
  packetqueue_init(&sending_queue);

#if !COLLECT_ANNOUNCEMENTS
  neighbor_discovery_open(&tc->neighbor_discovery_conn, channels,
			  CLOCK_SECOND * 4,
			  CLOCK_SECOND * 60,
                          CLOCK_SECOND * 600UL,
			  &neighbor_discovery_callbacks);
  neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
#else /* !COLLECT_ANNOUNCEMENTS */
  announcement_register(&tc->announcement, channels,
			received_announcement);
#if ! COLLECT_CONF_WITH_LISTEN
  announcement_set_value(&tc->announcement, RTMETRIC_MAX);
#endif /* COLLECT_CONF_WITH_LISTEN */
#endif /* !COLLECT_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
void
collect_close(struct collect_conn *tc)
{
#if COLLECT_ANNOUNCEMENTS
  announcement_remove(&tc->announcement);
#else
  neighbor_discovery_close(&tc->neighbor_discovery_conn);
#endif /* COLLECT_ANNOUNCEMENTS */
  unicast_close(&tc->unicast_conn);
}
/*---------------------------------------------------------------------------*/
void
collect_set_sink(struct collect_conn *tc, int should_be_sink)
{
  if(should_be_sink) {
    tc->is_router = 1;
    tc->rtmetric = RTMETRIC_SINK;
    PRINTF("collect_set_sink: tc->rtmetric %d\n", tc->rtmetric);
#if !COLLECT_ANNOUNCEMENTS
    neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
#else
    announcement_bump(&tc->announcement);
#endif /* !COLLECT_ANNOUNCEMENTS */
  } else {
    tc->rtmetric = RTMETRIC_MAX;
  }
#if COLLECT_ANNOUNCEMENTS
  announcement_set_value(&tc->announcement, tc->rtmetric);
#endif /* COLLECT_ANNOUNCEMENTS */
  update_rtmetric(tc);
}
/*---------------------------------------------------------------------------*/
int
collect_send(struct collect_conn *tc, int rexmits)
{
  struct collect_neighbor *n;

  packetbuf_set_attr(PACKETBUF_ATTR_EPACKET_ID, tc->eseqno++);
  packetbuf_set_addr(PACKETBUF_ADDR_ESENDER, &rimeaddr_node_addr);
  packetbuf_set_attr(PACKETBUF_ATTR_HOPS, 1);
  packetbuf_set_attr(PACKETBUF_ATTR_TTL, MAX_HOPLIM);
  packetbuf_set_attr(PACKETBUF_ATTR_MAX_REXMIT, rexmits);

  PRINTF("%d.%d: originating packet %d, max_rexmits %d\n",
         rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
         packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
         packetbuf_attr(PACKETBUF_ATTR_MAX_REXMIT));

  //  PRINTF("rexmit %d\n", rexmits);

  if(tc->rtmetric == RTMETRIC_SINK) {
    packetbuf_set_attr(PACKETBUF_ATTR_HOPS, 0);
    if(tc->cb->recv != NULL) {
      tc->cb->recv(packetbuf_addr(PACKETBUF_ADDR_ESENDER),
		   packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
		   packetbuf_attr(PACKETBUF_ATTR_HOPS));
    }
    return 1;
  } else {
    //    update_rtmetric(tc);
    n = collect_neighbor_best();
    if(n != NULL) {
      PRINTF("%d.%d: sending to %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     n->addr.u8[0], n->addr.u8[1]);
      
      if(packetqueue_enqueue_packetbuf(&sending_queue, FORWARD_PACKET_LIFETIME,
                                       tc)) {
        send_queued_packet();
        return 1;
      } else {
        PRINTF("%d.%d: drop originated packet: no queuebuf\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }

    } else {
      PRINTF("%d.%d: did not find any neighbor to send to\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
#if COLLECT_ANNOUNCEMENTS
#if COLLECT_CONF_WITH_LISTEN
      PRINTF("listen\n");
      announcement_listen(1);
      ctimer_set(&tc->transmit_after_scan_timer, ANNOUNCEMENT_SCAN_TIME,
                 send_queued_packet, NULL);
#else /* COLLECT_CONF_WITH_LISTEN */
      announcement_set_value(&tc->announcement, RTMETRIC_MAX);
      announcement_bump(&tc->announcement);
#endif /* COLLECT_CONF_WITH_LISTEN */
#endif /* COLLECT_ANNOUNCEMENTS */

      if(packetqueue_enqueue_packetbuf(&sending_queue, FORWARD_PACKET_LIFETIME,
                                       tc)) {
	return 1;
      } else {
        PRINTF("%d.%d: drop originated packet: no queuebuf\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
collect_depth(struct collect_conn *tc)
{
  return tc->rtmetric;
}
/*---------------------------------------------------------------------------*/
void
collect_purge(struct collect_conn *tc)
{
  collect_neighbor_purge();
  update_rtmetric(tc);
  PRINTF("#L %d 0\n", tc->parent.u8[0]);
  rimeaddr_copy(&tc->parent, &rimeaddr_null);
}
/*---------------------------------------------------------------------------*/
/** @} */
