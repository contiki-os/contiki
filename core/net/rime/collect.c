/* XXX: send explicit congestion notification if already forwarding
   packet. */

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
 * $Id: collect.c,v 1.32 2010/02/08 21:59:49 adamdunkels Exp $
 */

/**
 * \file
 *         Tree-based hop-by-hop reliable data collection
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime.h"
#include "net/rime/neighbor.h"
#include "net/rime/collect.h"

#include "net/rime/packetqueue.h"

#include "dev/radio-sensor.h"

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

static const struct packetbuf_attrlist attributes[] =
  {
    COLLECT_ATTRIBUTES
    PACKETBUF_ATTR_LAST
  };

#define NUM_RECENT_PACKETS 2

struct recent_packet {
  rimeaddr_t originator;
  uint8_t seqno;
};

static struct recent_packet recent_packets[NUM_RECENT_PACKETS];
static uint8_t recent_packet_ptr;

#define FORWARD_PACKET_LIFETIME (CLOCK_SECOND * 16)
#define MAX_FORWARDING_QUEUE 6
PACKETQUEUE(forwarding_queue, MAX_FORWARDING_QUEUE);

#define SINK 0
#define RTMETRIC_MAX COLLECT_MAX_DEPTH

#define MAX_HOPLIM 10

#ifndef COLLECT_CONF_ANNOUNCEMENTS
#define COLLECT_ANNOUNCEMENTS 1
#else
#define COLLECT_ANNOUNCEMENTS COLLECT_CONF_ANNOUNCEMENTS
#endif /* COLLECT_CONF_ANNOUNCEMENTS */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_queued_packet(void)
{
  struct queuebuf *q;
  struct neighbor *n;
  struct packetqueue_item *i;
  struct collect_conn *c;

  i = packetqueue_first(&forwarding_queue);
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
  
  if(c->forwarding) {
    /* If we are currently forwarding a packet, we wait until the
       packet is forwarded and try again then. */
    PRINTF("%d.%d: queue, c is forwarding\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    return;
  }

  q = packetqueue_queuebuf(i);
  if(q != NULL) {
    PRINTF("%d.%d: queue, q is on queue\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    queuebuf_to_packetbuf(q);
    
    n = neighbor_best();
    
    /* Don't send to the neighbor if it is the same neighbor that sent
       us the packet. */
    if(n != NULL && !rimeaddr_cmp(&n->addr, packetbuf_addr(PACKETBUF_ADDR_SENDER))) {
#if CONTIKI_TARGET_NETSIM
      ether_set_line(n->addr.u8[0], n->addr.u8[1]);
#endif /* CONTIKI_TARGET_NETSIM */
      PRINTF("%d.%d: sending packet to %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     n->addr.u8[0], n->addr.u8[1]);
      
      c->forwarding = 1;
      runicast_send(&c->runicast_conn, &n->addr, packetbuf_attr(PACKETBUF_ATTR_MAX_REXMIT));
    } else {
      PRINTF("%d.%d: did not find any neighbor to forward to\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
update_rtmetric(struct collect_conn *tc)
{
  struct neighbor *n;

  /* We should only update the rtmetric if we are not the sink. */
  if(tc->rtmetric != SINK) {

    /* Find the neighbor with the lowest rtmetric. */
    n = neighbor_best();

    /* If n is NULL, we have no best neighbor. */
    if(n == NULL) {

      /* If we have don't have any neighbors, we set our rtmetric to
	 the maximum value to indicate that we do not have a route. */
      
      if(tc->rtmetric != RTMETRIC_MAX) {
	PRINTF("%d.%d: didn't find a best neighbor, setting rtmetric to max\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
      tc->rtmetric = RTMETRIC_MAX;
      announcement_set_value(&tc->announcement, tc->rtmetric);
    } else {

      /* We set our rtmetric to the rtmetric of our best neighbor plus
	 the expected transmissions to reach that neighbor. */
      if(n->rtmetric + neighbor_etx(n) != tc->rtmetric) {
	uint16_t old_rtmetric = tc->rtmetric;
	
	tc->rtmetric = n->rtmetric + neighbor_etx(n);

#if !COLLECT_ANNOUNCEMENTS
	neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
#else
	announcement_set_value(&tc->announcement, tc->rtmetric);
#endif /* !COLLECT_ANNOUNCEMENTS */

	PRINTF("%d.%d: new rtmetric %d\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	       tc->rtmetric);
	
	/* We got a new, working, route we send any queued packets we may have. */
	if(old_rtmetric == RTMETRIC_MAX) {
	  send_queued_packet();
	}
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
      sprintf(buf, "%.1f", (float)tc->rtmetric / NEIGHBOR_ETX_SCALE);
    }
    ether_set_text(buf);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
node_packet_received(struct runicast_conn *c, const rimeaddr_t *from,
		     uint8_t seqno)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, runicast_conn));
  int i;

  /* To protect against forwarding duplicate packets, we keep a list
     of recently forwarded packet seqnos. If the seqno of the current
     packet exists in the list, we drop the packet. */

  for(i = 0; i < NUM_RECENT_PACKETS; i++) {
    if(recent_packets[i].seqno == packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID) &&
       rimeaddr_cmp(&recent_packets[i].originator,
		    packetbuf_addr(PACKETBUF_ADDR_ESENDER))) {
      PRINTF("%d.%d: dropping duplicate packet from %d.%d with seqno %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     recent_packets[i].originator.u8[0], recent_packets[i].originator.u8[1],
	     packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID));
      /* Drop the packet. */
      return;
    }
  }
  recent_packets[recent_packet_ptr].seqno = packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID);
  rimeaddr_copy(&recent_packets[recent_packet_ptr].originator,
		packetbuf_addr(PACKETBUF_ADDR_ESENDER));
  recent_packet_ptr = (recent_packet_ptr + 1) % NUM_RECENT_PACKETS;
  
  if(tc->rtmetric == SINK) {

    /* If we are the sink, we call the receive function. */
    
    PRINTF("%d.%d: sink received packet from %d.%d via %d.%d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
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

        
    PRINTF("%d.%d: packet received from %d.%d via %d.%d, forwarding %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[0],
	   packetbuf_addr(PACKETBUF_ADDR_ESENDER)->u8[1],
	   from->u8[0], from->u8[1], tc->forwarding);

    if(packetqueue_enqueue_packetbuf(&forwarding_queue, FORWARD_PACKET_LIFETIME,
				     tc)) {
      send_queued_packet();
    } else {
      PRINTF("%d.%d: packet dropped: no queue buffer available\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    }
  }
  
  return;
}
/*---------------------------------------------------------------------------*/
static void
node_packet_sent(struct runicast_conn *c, const rimeaddr_t *to,
		 uint8_t transmissions)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, runicast_conn));

  PRINTF("%d.%d: sent to %d.%d after %d retransmissions\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 to->u8[0], to->u8[1],
	 transmissions);

  
  tc->forwarding = 0;
  neighbor_update_etx(neighbor_find(to), transmissions);
  update_rtmetric(tc);

  /* Remove the first packet on the queue, the packet that was just sent. */
  packetqueue_dequeue(&forwarding_queue);

  /* Send the next packet in the queue, if any. */
  send_queued_packet();
}
/*---------------------------------------------------------------------------*/
static void
node_packet_timedout(struct runicast_conn *c, const rimeaddr_t *to,
		     uint8_t transmissions)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, runicast_conn));

  PRINTF("%d.%d: timedout after %d retransmissions: packet dropped\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1], transmissions);
  
  tc->forwarding = 0;
  neighbor_timedout_etx(neighbor_find(to), transmissions);
  update_rtmetric(tc);

  /* Remove the first packet on the queue, the packet that just timed out. */
  packetqueue_dequeue(&forwarding_queue);

  /* Send the next packet in the queue, if any. */
  send_queued_packet();
}
/*---------------------------------------------------------------------------*/
#if !COLLECT_ANNOUNCEMENTS
static void
adv_received(struct neighbor_discovery_conn *c, const rimeaddr_t *from,
	     uint16_t rtmetric)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, neighbor_discovery_conn));
  struct neighbor *n;
  
  n = neighbor_find(from);

  if(n == NULL) {
    neighbor_add(from, rtmetric, 1);
  } else {
    neighbor_update(n, rtmetric);
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
  struct neighbor *n;
  
  n = neighbor_find(from);

  if(n == NULL) {
    neighbor_add(from, value, 1);
    PRINTF("%d.%d: new neighbor %d.%d, etx %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   from->u8[0], from->u8[1], value);
  } else {
    neighbor_update(n, value);
    PRINTF("%d.%d: updating neighbor %d.%d, etx %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   n->addr.u8[0], n->addr.u8[1], value);
  }

  update_rtmetric(tc);  
}
#endif /* !COLLECT_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
static const struct runicast_callbacks runicast_callbacks = {node_packet_received,
							     node_packet_sent,
							     node_packet_timedout};
#if !COLLECT_ANNOUNCEMENTS
static const struct neighbor_discovery_callbacks neighbor_discovery_callbacks =
  { adv_received, NULL};
#endif /* !COLLECT_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
void
collect_open(struct collect_conn *tc, uint16_t channels,
	     const struct collect_callbacks *cb)
{
#if !COLLECT_ANNOUNCEMENTS
  neighbor_discovery_open(&tc->neighbor_discovery_conn, channels,
			  CLOCK_SECOND * 2,
			  CLOCK_SECOND * 10,
			  CLOCK_SECOND * 60,
			  &neighbor_discovery_callbacks);
#endif /* !COLLECT_ANNOUNCEMENTS */
  runicast_open(&tc->runicast_conn, channels + 1, &runicast_callbacks);
  channel_set_attributes(channels + 1, attributes);
  tc->rtmetric = RTMETRIC_MAX;
  tc->cb = cb;
#if COLLECT_ANNOUNCEMENTS
  announcement_register(&tc->announcement, channels, tc->rtmetric,
			received_announcement);
  announcement_listen(2);
#else
  neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
#endif /* COLLECT_ANNOUNCEMENTS */
  neighbor_init();
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
  runicast_close(&tc->runicast_conn);
}
/*---------------------------------------------------------------------------*/
void
collect_set_sink(struct collect_conn *tc, int should_be_sink)
{
  if(should_be_sink) {
    tc->rtmetric = SINK;
#if !COLLECT_ANNOUNCEMENTS
    neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
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
  struct neighbor *n;
  
  packetbuf_set_attr(PACKETBUF_ATTR_EPACKET_ID, tc->seqno++);
  packetbuf_set_addr(PACKETBUF_ADDR_ESENDER, &rimeaddr_node_addr);
  packetbuf_set_attr(PACKETBUF_ATTR_HOPS, 1);
  packetbuf_set_attr(PACKETBUF_ATTR_TTL, MAX_HOPLIM);
  packetbuf_set_attr(PACKETBUF_ATTR_MAX_REXMIT, rexmits);

  if(tc->rtmetric == SINK) {
    packetbuf_set_attr(PACKETBUF_ATTR_HOPS, 0);
    if(tc->cb->recv != NULL) {
      tc->cb->recv(packetbuf_addr(PACKETBUF_ADDR_ESENDER),
		   packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID),
		   packetbuf_attr(PACKETBUF_ATTR_HOPS));
    }
    return 1;
  } else {
    n = neighbor_best();
    if(n != NULL) {
#if CONTIKI_TARGET_NETSIM
      ether_set_line(n->addr.u8[0], n->addr.u8[1]);
#endif /* CONTIKI_TARGET_NETSIM */
      PRINTF("%d.%d: sending to %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     n->addr.u8[0], n->addr.u8[1]);
      if(packetqueue_enqueue_packetbuf(&forwarding_queue, FORWARD_PACKET_LIFETIME,
				       tc)) {
	send_queued_packet();
	return 1;
      } else {
        PRINTF("%d.%d: drop originated packet: no queuebuf\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
    } else {
      /*      printf("Didn't find any neighbor\n");*/
      PRINTF("%d.%d: did not find any neighbor to send to\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      announcement_listen(1);
      if(packetqueue_enqueue_packetbuf(&forwarding_queue, FORWARD_PACKET_LIFETIME,
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
/** @} */
