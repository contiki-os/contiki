/* XXX: send explicit congestion notification if already forwarding
   packet + add queue for keeping packets to forward. */

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
 * $Id: collect.c,v 1.4 2008/01/08 07:54:40 adamdunkels Exp $
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
#include "net/rime/nf.h"
#include "net/rime/collect.h"

#include "dev/radio-sensor.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

struct hdr {
  rimeaddr_t originator;
  u8_t originator_seqno;
  u8_t hops;
  u8_t hoplim;
  u8_t rexmits;
};

#define NUM_RECENT_PACKETS 4

struct recent_packet {
  rimeaddr_t originator;
  uint8_t seqno;
};

static struct recent_packet recent_packets[NUM_RECENT_PACKETS];
static uint8_t recent_packet_ptr;

#define SINK 0
#define RTMETRIC_MAX COLLECT_MAX_DEPTH

#define MAX_HOPLIM 10

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

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
    } else {

      /* We set our rtmetric to the rtmetric of our best neighbor plus
	 the expected transmissions to reach that neighbor. */
      if(n->rtmetric + neighbor_etx(n) != tc->rtmetric) {
	tc->rtmetric = n->rtmetric + neighbor_etx(n);
	neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
	PRINTF("%d.%d: new rtmetric %d\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	       tc->rtmetric);
      }
    }
  }

  /*  DEBUG_PRINTF("%d: new rtmetric %d\n", node_id, rtmetric);*/
#if NETSIM
  {
    char buf[8];
    if(tc->rtmetric == RTMETRIC_MAX) {
      strcpy(buf, " ");
    } else {
      sprintf(buf, "%d", tc->rtmetric);
    }
    ether_set_text(buf);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
node_packet_received(struct ruc_conn *c, rimeaddr_t *from, u8_t seqno)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, ruc_conn));
  struct hdr *hdr = rimebuf_dataptr();
  struct neighbor *n;
  int i;

  /* To protect against forwarding duplicate packets, we keep a list
     of recently forwarded packet seqnos. If the seqno of the current
     packet exists in the list, we drop the packet. */

  for(i = 0; i < NUM_RECENT_PACKETS; i++) {
    if(recent_packets[i].seqno == hdr->originator_seqno &&
       rimeaddr_cmp(&recent_packets[i].originator, &hdr->originator)) {
      /* Drop the packet. */
      return;
    }
  }
  recent_packets[recent_packet_ptr].seqno = hdr->originator_seqno;
  rimeaddr_copy(&recent_packets[recent_packet_ptr].originator, &hdr->originator);
  recent_packet_ptr = (recent_packet_ptr + 1) % NUM_RECENT_PACKETS;
  
  if(tc->rtmetric == SINK) {

    /* If we are the sink, we call the receive function. */
    
    rimebuf_hdrreduce(sizeof(struct hdr));
    
    PRINTF("%d.%d: sink received packet from %d.%d via %d.%d with rtmetric %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   hdr->originator.u8[0], hdr->originator.u8[1],
	   from->u8[0], from->u8[1], hdr->rtmetric);
    
    if(tc->cb->recv != NULL) {
      tc->cb->recv(&hdr->originator, hdr->originator_seqno,
		   hdr->hops);
    }
    return;
  } else if(hdr->hoplim > 1 && tc->rtmetric != RTMETRIC_MAX) {

    /* If we are not the sink, we forward the packet to the best
       neighbor. */
    hdr->hops++;
    hdr->hoplim--;

        
    PRINTF("%d.%d: packet received from %d.%d via %d.%d with rtmetric %d, best neighbor %p, forwarding %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   hdr->originator.u8[0], hdr->originator.u8[1],
	   from->u8[0], from->u8[1], hdr->rtmetric,
	   neighbor_best(), tc->forwarding);

    if(!tc->forwarding) {
      tc->forwarding = 1;
      n = neighbor_best();
      if(n != NULL) {
	ruc_send(c, &n->addr, hdr->rexmits);
      }
      return;
    } else {

      PRINTF("%d.%d: still forwarding another packet\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      return;
    }
  }
  return;
}
/*---------------------------------------------------------------------------*/
static void
node_packet_sent(struct ruc_conn *c, rimeaddr_t *to, u8_t retransmissions)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, ruc_conn));

  tc->forwarding = 0;
  neighbor_update_etx(neighbor_find(to), retransmissions);
  update_rtmetric(tc);
}
/*---------------------------------------------------------------------------*/
static void
node_packet_timedout(struct ruc_conn *c, rimeaddr_t *to, u8_t retransmissions)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, ruc_conn));

  tc->forwarding = 0;
  neighbor_timedout_etx(neighbor_find(to), retransmissions);
  update_rtmetric(tc);
}
/*---------------------------------------------------------------------------*/
static void
adv_received(struct neighbor_discovery_conn *c, rimeaddr_t *from, uint16_t rtmetric)
{
  struct collect_conn *tc = (struct collect_conn *)
    ((char *)c - offsetof(struct collect_conn, neighbor_discovery_conn));
  struct neighbor *n;
  
  n = neighbor_find(from);

  if(n == NULL) {
    neighbor_add(from, rtmetric, 1);
  } else {
    neighbor_update(n, rtmetric);
    PRINTF("%d.%d: updating neighbor %d.%d, etx %d, hops %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   n->addr.u8[0], n->addr.u8[1],
	   1, rtmetric);
  }

  update_rtmetric(tc);
}
/*---------------------------------------------------------------------------*/
static const struct ruc_callbacks ruc_callbacks = {node_packet_received,
						   node_packet_sent,
						   node_packet_timedout};
static const struct neighbor_discovery_callbacks neighbor_discovery_callbacks =
  { adv_received, NULL};
/*---------------------------------------------------------------------------*/
void
collect_open(struct collect_conn *tc, u16_t channels,
	     const struct collect_callbacks *cb)
{
  neighbor_discovery_open(&tc->neighbor_discovery_conn, channels,
			  &neighbor_discovery_callbacks);
  ruc_open(&tc->ruc_conn, channels + 1, &ruc_callbacks);
  tc->rtmetric = RTMETRIC_MAX;
  tc->cb = cb;
  neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
}
/*---------------------------------------------------------------------------*/
void
collect_close(struct collect_conn *tc)
{
  neighbor_discovery_close(&tc->neighbor_discovery_conn);
  ruc_close(&tc->ruc_conn);
}
/*---------------------------------------------------------------------------*/
void
collect_set_sink(struct collect_conn *tc, int should_be_sink)
{
  if(should_be_sink) {
    tc->rtmetric = SINK;
    neighbor_discovery_start(&tc->neighbor_discovery_conn, tc->rtmetric);
  } else {
    tc->rtmetric = RTMETRIC_MAX;
  }
  update_rtmetric(tc);
}
/*---------------------------------------------------------------------------*/
void
collect_send(struct collect_conn *tc, int rexmits)
{
  struct neighbor *n;
  struct hdr *hdr;

  
  if(rimebuf_hdralloc(sizeof(struct hdr))) {
    hdr = rimebuf_hdrptr();
    hdr->originator_seqno = tc->seqno++;
    rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
    hdr->hops = 1;
    hdr->hoplim = MAX_HOPLIM;
    hdr->rexmits = rexmits;
    if(tc->rtmetric == 0) {
      hdr->hops = 0;
      if(tc->cb->recv != NULL) {
	tc->cb->recv(&hdr->originator, hdr->originator_seqno,
		     hdr->hops);
      }
    } else {
      n = neighbor_best();
      if(n != NULL) {
	/*      printf("Sending to best neighbor\n");*/
	ruc_send(&tc->ruc_conn, &n->addr, rexmits);
      } else {
	/*      printf("Didn't find any neighbor\n");*/
	PRINTF("%d.%d: did not find any neighbor to send to\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
int
collect_depth(struct collect_conn *tc)
{
  return tc->rtmetric;
}
/*---------------------------------------------------------------------------*/
/** @} */
