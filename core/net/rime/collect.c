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
 * $Id: collect.c,v 1.47 2010/04/30 07:33:51 adamdunkels Exp $
 */

/**
 * \file
 *         Tree-based hop-by-hop reliable data collection
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime.h"
#include "net/rime/collect-neighbor.h"
#include "net/rime/collect.h"

#include "net/rime/packetqueue.h"

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

#define NUM_RECENT_PACKETS 8

struct recent_packet {
  rimeaddr_t originator;
  rimeaddr_t sent_to;
  uint8_t seqno;
};

struct ack_msg {
  uint8_t flags, dummy;
  uint16_t rtmetric;
};

#define ACK_FLAGS_CONGESTED         0x80
#define ACK_FLAGS_DROPPED           0x40
#define ACK_FLAGS_LIFETIME_EXCEEDED 0x20

static struct recent_packet recent_packets[NUM_RECENT_PACKETS];
static uint8_t recent_packet_ptr;

#define MAX_MAC_REXMITS            3
#define MAX_ACK_MAC_REXMITS        3
#define REXMIT_TIME                CLOCK_SECOND * 2
#define FORWARD_PACKET_LIFETIME    (6 * (REXMIT_TIME) << 3)
#define MAX_SENDING_QUEUE          6
PACKETQUEUE(sending_queue, MAX_SENDING_QUEUE);

#define SINK 0
#define RTMETRIC_MAX COLLECT_MAX_DEPTH

#define MAX_HOPLIM 15

#ifndef COLLECT_CONF_ANNOUNCEMENTS
#define COLLECT_ANNOUNCEMENTS 0
#else
#define COLLECT_ANNOUNCEMENTS COLLECT_CONF_ANNOUNCEMENTS
#endif /* COLLECT_CONF_ANNOUNCEMENTS */

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

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif /* CONTIKI_TARGET_NETSIM */

static void send_queued_packet(void);
static void retransmit_callback(void *ptr);
/*---------------------------------------------------------------------------*/
static void
update_rtmetric(struct collect_conn *tc)
{
  struct collect_neighbor *n;

  PRINTF("update_rtmetric: tc->rtmetric %d\n", tc->rtmetric);

  /* We should only update the rtmetric if we are not the sink. */
  if(tc->rtmetric != SINK) {
    struct collect_neighbor *best;

    /* Pick the neighbor to use as a parent. We normally use
       the parent in the n->parent. */
    n = collect_neighbor_find(&tc->parent);
    /* Find the neighbor with the lowest rtmetric. */
    best = collect_neighbor_best();

    /* If we do not have a parent in n->parent, we use the best
       neighbor that we have as a new parent. Also, if the best
       neighbor is better than our parent (which is defined as having
       an ETX that is 1 ETX lower than the current parent), we
       choose that neighbor as the new parent. */
    best = collect_neighbor_best();

    if(best != NULL && (n == NULL ||
                        collect_neighbor_etx(best) <
                        collect_neighbor_etx(n) - COLLECT_NEIGHBOR_ETX_SCALE)) {
      PRINTF("Switched parent from %d.%d to %d.%d\n",
             tc->parent.u8[0], tc->parent.u8[1],
             best->addr.u8[0], best->addr.u8[1]);
      PRINTF("#L %d 0\n", tc->parent.u8[0]);
      PRINTF("#L %d 1\n", best->addr.u8[0]);
      rimeaddr_copy(&tc->parent, &best->addr);
    }

    /* If n is NULL, we have no best neighbor. */
    if(n == NULL) {

      /* If we have don't have any neighbors, we set our rtmetric to
	 the maximum value to indicate that we do not have a route. */

      if(tc->rtmetric != RTMETRIC_MAX) {
        PRINTF("%d.%d: didn't find a best neighbor, setting rtmetric to max\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
      tc->rtmetric = RTMETRIC_MAX;
#if COLLECT_ANNOUNCEMENTS
#if ! COLLECT_CONF_WITH_LISTEN
      announcement_set_value(&tc->announcement, tc->rtmetric);
#endif /* COLLECT_CONF_WITH_LISTEN */
#else /* COLLECT_ANNOUNCEMENTS */
      neighbor_discovery_set_val(&tc->neighbor_discovery_conn, tc->rtmetric);
#endif /* COLLECT_ANNOUNCEMENTS */
    } else {
      /* We set our rtmetric to the rtmetric of our best neighbor plus
         the expected transmissions to reach that neighbor. */
      if(n->rtmetric + collect_neighbor_etx(n) != tc->rtmetric) {
	uint16_t old_rtmetric = tc->rtmetric;

	tc->rtmetric = n->rtmetric + collect_neighbor_etx(n);
        if(tc->rtmetric == SINK) {
          /* Something strange happened - ETX to this neighbors is zero! */
          printf("Error: n->rtmetric %d, collect_neighbor_etx(n) %d\n",
                 n->rtmetric, collect_neighbor_etx(n));

          /* Fix the problem by setting ETX to one. */
          tc->rtmetric = COLLECT_NEIGHBOR_ETX_SCALE;
        }

#if ! COLLECT_ANNOUNCEMENTS

        /* If we get a significantly better rtmetric than we had
           before, we call neighbor_discovery_start to start a new
           period. */
        if(old_rtmetric >= tc->rtmetric + COLLECT_NEIGHBOR_ETX_SCALE + COLLECT_NEIGHBOR_ETX_SCALE / 2 ||
           old_rtmetric == RTMETRIC_MAX) {
          neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
        } else {
          neighbor_discovery_set_val(&tc->neighbor_discovery_conn, tc->rtmetric);
        }
#else /* ! COLLECT_ANNOUNCEMENTS */
        if(tc->is_router) {
          announcement_set_value(&tc->announcement, tc->rtmetric);
        } else {
          announcement_remove_value(&tc->announcement);
        }
        if(old_rtmetric >= tc->rtmetric + COLLECT_NEIGHBOR_ETX_SCALE + COLLECT_NEIGHBOR_ETX_SCALE / 2 ||
           old_rtmetric == RTMETRIC_MAX) {
          announcement_bump(&tc->announcement);
        }
#endif /* ! COLLECT_ANNOUNCEMENTS */

        PRINTF("%d.%d: new rtmetric %d\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
               tc->rtmetric);
#if ! COLLECT_CONF_WITH_LISTEN
	/* We got a new, working, route we send any queued packets we may have. */
	if(old_rtmetric == RTMETRIC_MAX) {
          PRINTF("Sending queued packet because rtmetric was max\n");
	  send_queued_packet();
	}
#endif /* COLLECT_CONF_WITH_LISTEN */
      }
    }
  }

  /*  DEBUG_PRINTF("%d: new rtmetric %d\n", node_id, rtmetric);*/
#if CONTIKI_TARGET_NETSIM
  {
    char buf[8];
    if(tc->rtmetric == RTMETRIC_MAX) {
      strcpy(buf, " ");
    } else {
      PRINTF(buf, "%.1f", (float)tc->rtmetric / COLLECT_NEIGHBOR_ETX_SCALE);
    }
    ether_set_text(buf);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
send_queued_packet(void)
{
  struct queuebuf *q;
  struct collect_neighbor *n;
  struct packetqueue_item *i;
  struct collect_conn *c;

  //  PRINTF("%d.%d: send_queued_packet queue len %d\n",
  //         rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
  //         packetqueue_len(&sending_queue));

  i = packetqueue_first(&sending_queue);
  if(i == NULL) {
      PRINTF("%d.%d: nothing on queue\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    /* No packet on the queue, so there is nothing for us to send. */
    return;
  }
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
    //    PRINTF("%d.%d: queue, q is on queue\n",
    //	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);

    /* Place the queued packet into the packetbuf. */
    queuebuf_to_packetbuf(q);

    /* Pick the neighbor to which to send the packet. We use the
       parent in the n->parent. */
    n = collect_neighbor_find(&c->parent);

    if(n != NULL) {
      clock_time_t time;
      uint8_t rexmit_time_scaling;
#if CONTIKI_TARGET_NETSIM
      ether_set_line(n->addr.u8[0], n->addr.u8[1]);
#endif /* CONTIKI_TARGET_NETSIM */
      PRINTF("%d.%d: sending packet to %d.%d with eseqno %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     n->addr.u8[0], n->addr.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID));

      c->sending = 1;
      c->transmissions = 0;
      c->max_rexmits = packetbuf_attr(PACKETBUF_ATTR_MAX_REXMIT);
      packetbuf_set_attr(PACKETBUF_ATTR_RELIABLE, 1);
      packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS, MAX_MAC_REXMITS);
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, c->seqno);
      unicast_send(&c->unicast_conn, &n->addr);
      rexmit_time_scaling = c->transmissions;
      if(rexmit_time_scaling > 3) {
        rexmit_time_scaling = 3;
      }
      time = REXMIT_TIME << rexmit_time_scaling;
      time = time / 2 + random_rand() % (time / 2);
      //      PRINTF("retransmission time %lu\n", time);
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
      collect_neighbor_update(n, rtmetric);
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
send_ack(struct collect_conn *tc, const rimeaddr_t *to, int congestion, int dropped, int ttl)
{
  struct ack_msg *ack;
  struct queuebuf *q;
  uint16_t packet_seqno, packet_eseqno;


  //  PRINTF("send_ack\n");

  packet_seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);
  packet_eseqno = packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID);

  q = queuebuf_new_from_packetbuf();
  if(q != NULL) {

    packetbuf_clear();
    packetbuf_set_datalen(sizeof(struct ack_msg));
    ack = packetbuf_dataptr();
    memset(ack, 0, sizeof(struct ack_msg));
    ack->rtmetric = tc->rtmetric;
    ack->flags = (congestion? ACK_FLAGS_CONGESTED: 0) |
      (dropped? ACK_FLAGS_DROPPED: 0) |
      (ttl? ACK_FLAGS_LIFETIME_EXCEEDED: 0);
    /* XXX: send explicit congestion notification in ACK queue full; add rtmetric to ACK. */
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
  struct collect_neighbor *n;

  /* To protect against sending duplicate packets, we keep a list
     of recently forwarded packet seqnos. If the seqno of the current
     packet exists in the list, we increase the
     ETX of the neighbor we sent it to in the first place. */
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
     PACKETBUF_ATTR_PACKET_TYPE_DATA) {
    rimeaddr_t ack_to;
    uint8_t packet_seqno;

    rimeaddr_copy(&ack_to, packetbuf_addr(PACKETBUF_ADDR_SENDER));
    packet_seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);

    if(rimeaddr_cmp(&tc->last_received_addr, packetbuf_addr(PACKETBUF_ADDR_SENDER)) &&
       tc->last_received_seqno == packetbuf_attr(PACKETBUF_ATTR_PACKET_ID)) {
      /* This is a duplicate of the packet we last received, so we just send an ACK. */
      PRINTF("%d.%d: received same packet again from %d.%d with seqno %d, via %d.%d, acking\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             ack_to.u8[0], ack_to.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
             packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
             packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
      send_ack(tc, &ack_to, 0, 0, 0);
      return;
    }
    rimeaddr_copy(&tc->last_received_addr, packetbuf_addr(PACKETBUF_ADDR_SENDER));
    tc->last_received_seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);

    for(i = 0; i < NUM_RECENT_PACKETS; i++) {
      if(recent_packets[i].seqno == packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID) &&
         rimeaddr_cmp(&recent_packets[i].originator,
                      packetbuf_addr(PACKETBUF_ADDR_ESENDER))) {
        PRINTF("%d.%d: found duplicate packet from %d.%d with seqno %d, via %d.%d\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
               recent_packets[i].originator.u8[0], recent_packets[i].originator.u8[1],
               packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
               packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
               packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
        n = collect_neighbor_find(&recent_packets[i].sent_to);
        if(n != NULL) {
          collect_neighbor_update_etx(n, collect_neighbor_etx(n) / COLLECT_NEIGHBOR_ETX_SCALE + 4);
          update_rtmetric(tc);
        }
        break;
      }
    }
    recent_packets[recent_packet_ptr].seqno = packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID);
    rimeaddr_copy(&recent_packets[recent_packet_ptr].originator,
                  packetbuf_addr(PACKETBUF_ADDR_ESENDER));
    /*    n = collect_neighbor_best();*/

    if(tc->rtmetric != SINK) {
      n = collect_neighbor_best();
      rimeaddr_copy(&recent_packets[recent_packet_ptr].sent_to,
                    &n->addr);
    } else {
      rimeaddr_copy(&recent_packets[recent_packet_ptr].sent_to,
                    &rimeaddr_null);
    }

    recent_packet_ptr = (recent_packet_ptr + 1) % NUM_RECENT_PACKETS;

    if(tc->rtmetric == SINK) {

      /* If we are the sink, we call the receive function. */

      send_ack(tc, &ack_to, 0, 0, 0);

      PRINTF("%d.%d: sink received packet %d from %d.%d via %d.%d\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[0],
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[1],
             from->u8[0], from->u8[1]);

      if(tc->cb->recv != NULL) {
        tc->cb->recv(packetbuf_addr(PACKETBUF_ADDR_ESENDER),
                     packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
                     packetbuf_attr(PACKETBUF_ATTR_HOPS));
      }
      return;
    } else if(packetbuf_attr(PACKETBUF_ATTR_TTL) > 1 &&
              tc->rtmetric != RTMETRIC_MAX) {

      /* If we are not the sink, we forward the packet to the best
         neighbor. */
      packetbuf_set_attr(PACKETBUF_ATTR_HOPS, packetbuf_attr(PACKETBUF_ATTR_HOPS) + 1);
      packetbuf_set_attr(PACKETBUF_ATTR_TTL, packetbuf_attr(PACKETBUF_ATTR_TTL) - 1);


      PRINTF("%d.%d: packet received from %d.%d via %d.%d, sending %d, max_rexmits %d\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[0],
             packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[1],
             from->u8[0], from->u8[1], tc->sending,
             packetbuf_attr(PACKETBUF_ATTR_MAX_REXMIT));

      if(packetqueue_enqueue_packetbuf(&sending_queue, FORWARD_PACKET_LIFETIME,
                                       tc)) {
        send_ack(tc, &ack_to, 0, 0, 0);
        send_queued_packet();
      } else {
        send_ack(tc, &ack_to, 0, 1, 0);
        PRINTF("%d.%d: packet dropped: no queue buffer available\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
    } else if(packetbuf_attr(PACKETBUF_ATTR_TTL) <= 1) {
      PRINTF("%d.%d: packet dropped: ttl %d\n",
             rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
             packetbuf_attr(PACKETBUF_ATTR_TTL));
      send_ack(tc, &ack_to, 0, 1, 1);
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

    /*    neighbor_update_etx(neighbor_find(to), transmissions);
          update_rtmetric(tc);*/
    tc->transmissions += transmissions;

    tx = tc->transmissions;

#if 0
    /* Punish neighbors that don't hear us by increasing the expeted
       transmissions by four times the actual amount of transmissions
       that we tried. This is due to the ETX calculation is done with
       a moving average. */
    if(status == MAC_TX_NOACK) {
      tx *= 8;
    }
    /* Punish collisions too, but not as much. */
    if(status == MAC_TX_COLLISION) {
      //      tx *= 2;
    }
#endif /* 0 */
    /* Update ETX with the number of transmissions. */
    //    PRINTF("Updating ETX with %d transmissions (punished %d)\n", tc->transmissions,
    //           tx);
    collect_neighbor_update_etx(collect_neighbor_find(&tc->parent), tx);
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
  collect_neighbor_timedout_etx(collect_neighbor_find(&tc->parent), tc->transmissions);
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
    collect_neighbor_add(from, rtmetric, 1);
  } else {
    collect_neighbor_update(n, rtmetric);
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
    collect_neighbor_add(from, value, 1);
    PRINTF("%d.%d: new neighbor %d.%d, etx %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   from->u8[0], from->u8[1], value);
  } else {
    collect_neighbor_update(n, value);
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
			  CLOCK_SECOND * 8,
			  CLOCK_SECOND * 32,
                          CLOCK_SECOND * 300UL,
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
    tc->rtmetric = SINK;
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

  if(tc->rtmetric == SINK) {
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
#if CONTIKI_TARGET_NETSIM
      ether_set_line(n->addr.u8[0], n->addr.u8[1]);
#endif /* CONTIKI_TARGET_NETSIM */
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
