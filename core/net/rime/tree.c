/**
 * \addtogroup rimetree
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
 * $Id: tree.c,v 1.10 2007/03/31 18:33:04 adamdunkels Exp $
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
#include "net/rime/tree.h"

#include "dev/radio-sensor.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

struct adv_msg {
  u8_t hopcount;
  u8_t pad;
};

struct hdr {
  rimeaddr_t originator;
  u8_t originator_seqno;
  u8_t hopcount;
  u8_t hoplim;
};

#define SINK 0
#define HOPCOUNT_MAX TREE_MAX_DEPTH

#define MAX_HOPLIM 10

#define MAX_INTERVAL CLOCK_SECOND * 10
#define MIN_INTERVAL CLOCK_SECOND * 2

#define MAX_RETRANSMISSIONS 3

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_adv(struct tree_conn *tc, clock_time_t interval)
{
  struct adv_msg *hdr;

  rimebuf_clear();
  rimebuf_set_datalen(sizeof(struct adv_msg));
  hdr = rimebuf_dataptr();
  hdr->hopcount = tc->hops_from_sink;
  uibc_send(&tc->uibc_conn, interval);
}
/*---------------------------------------------------------------------------*/
static void
update_hopcount(struct tree_conn *tc)
{
  struct neighbor *n;
  
  if(tc->hops_from_sink != SINK) {
    n = neighbor_best();
    if(n == NULL) {
      /*      if(hopcount != HOPCOUNT_MAX) {
	printf("%d: didn't find a best neighbor, setting hopcount to max\n", node_id);
	}*/
      tc->hops_from_sink = HOPCOUNT_MAX;
    } else {
      if(n->hopcount + 1 != tc->hops_from_sink) {
	tc->hops_from_sink = n->hopcount + 1;
	send_adv(tc, MIN_INTERVAL);
      }
    }
  }

  /*  DEBUG_PRINTF("%d: new hopcount %d\n", node_id, hopcount);*/
#if NETSIM
  {
    char buf[8];
    if(tc->hops_from_sink == HOPCOUNT_MAX) {
      strcpy(buf, " ");
    } else {
      snprintf(buf, sizeof(buf), "%d", tc->hops_from_sink);
    }
    ether_set_text(buf);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
adv_packet_received(struct uibc_conn *c, rimeaddr_t *from)
{
  struct tree_conn *tc = (struct tree_conn *)
    ((char *)c - offsetof(struct tree_conn, uibc_conn));
  struct adv_msg *msg = rimebuf_dataptr();
  struct neighbor *n;

  /*  PRINTF("%d.%d: adv_packet_received from %d.%d with hopcount %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], msg->hopcount);*/

  n = neighbor_find(from);

  if(n == NULL) {
    neighbor_add(from, msg->hopcount, radio_sensor.value(1));
  } else {
    neighbor_update(n, msg->hopcount, radio_sensor.value(1));
  }

  update_hopcount(tc);

}
/*---------------------------------------------------------------------------*/
static void
adv_packet_sent(struct uibc_conn *c)
{
  struct tree_conn *tc = (struct tree_conn *)
    ((char *)c - offsetof(struct tree_conn, uibc_conn));
  send_adv(tc, MAX_INTERVAL);
}
/*---------------------------------------------------------------------------*/
static void
adv_packet_dropped(struct uibc_conn *c)
{
  struct tree_conn *tc = (struct tree_conn *)
    ((char *)c - offsetof(struct tree_conn, uibc_conn));
  send_adv(tc, MAX_INTERVAL);
}
/*---------------------------------------------------------------------------*/
static int
node_packet_received(struct ruc_conn *c, rimeaddr_t *from, u8_t seqno)
{
  struct tree_conn *tc = (struct tree_conn *)
    ((char *)c - offsetof(struct tree_conn, ruc_conn));
  struct hdr *hdr = rimebuf_dataptr();
  struct neighbor *n;

  if(tc->hops_from_sink == SINK) {

    rimebuf_hdrreduce(sizeof(struct hdr));
    
    PRINTF("%d.%d: sink received packet from %d.%d via %d.%d with hopcount %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   hdr->originator.u8[0], hdr->originator.u8[1],
	   from->u8[0], from->u8[1], hdr->hopcount);
    
    if(tc->cb->recv != NULL) {
      tc->cb->recv(&hdr->originator, hdr->originator_seqno,
		   hdr->hopcount);
    }
    return 1;
  } else if(hdr->hoplim > 1 && tc->hops_from_sink != HOPCOUNT_MAX) {
    hdr->hoplim--;

        
    PRINTF("%d.%d: packet received from %d.%d via %d.%d with hopcount %d, best neighbor %p, forwarding %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   hdr->originator.u8[0], hdr->originator.u8[1],
	   from->u8[0], from->u8[1], hdr->hopcount,
	   neighbor_best(), tc->forwarding);

    if(!tc->forwarding) {
      tc->forwarding = 1;
      n = neighbor_best();
      if(n != NULL) {
	ruc_send(c, &n->addr, MAX_RETRANSMISSIONS);
      }
      return 1;
    } else {

      PRINTF("%d.%d: still forwarding another packet, not sending ACK\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
node_packet_sent(struct ruc_conn *c)
{
  struct tree_conn *tc = (struct tree_conn *)
    ((char *)c - offsetof(struct tree_conn, ruc_conn));

  tc->forwarding = 0;
}
/*---------------------------------------------------------------------------*/
static const struct uibc_callbacks uibc_callbacks =
  {adv_packet_received, adv_packet_sent, adv_packet_dropped};
static const struct ruc_callbacks ruc_callbacks = {node_packet_received,
					     node_packet_sent};
/*---------------------------------------------------------------------------*/
void
tree_open(struct tree_conn *tc, u16_t channels,
	  const struct tree_callbacks *cb)
{
  uibc_open(&tc->uibc_conn, channels, &uibc_callbacks);
  ruc_open(&tc->ruc_conn, channels + 1, &ruc_callbacks);
  tc->hops_from_sink = HOPCOUNT_MAX;
  /*  rimebuf_clear();
  rimebuf_reference(&tc.hello, sizeof(tc.hello));
  sibc_send_stubborn(&sibc_conn, CLOCK_SECOND * 8);*/
  tc->cb = cb;
  send_adv(tc, MAX_INTERVAL);
}
/*---------------------------------------------------------------------------*/
void
tree_close(struct tree_conn *tc)
{
  uibc_close(&tc->uibc_conn);
  ruc_close(&tc->ruc_conn);
}
/*---------------------------------------------------------------------------*/
void
tree_set_sink(struct tree_conn *tc, int should_be_sink)
{
  if(should_be_sink) {
    tc->hops_from_sink = SINK;
    send_adv(tc, MIN_INTERVAL);
  } else {
    tc->hops_from_sink = HOPCOUNT_MAX;
  }
  update_hopcount(tc);
}
/*---------------------------------------------------------------------------*/
void
tree_send(struct tree_conn *tc)
{
  struct neighbor *n;
  struct hdr *hdr;

  if(rimebuf_hdralloc(sizeof(struct hdr))) {
    hdr = rimebuf_hdrptr();
    hdr->originator_seqno = tc->seqno++;
    rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
    hdr->hopcount = tc->hops_from_sink;
    hdr->hoplim = MAX_HOPLIM;
    n = neighbor_best();
    if(n != NULL) {
      /*      printf("Sending to best neighbor\n");*/
      ruc_send(&tc->ruc_conn, &n->addr, MAX_RETRANSMISSIONS);
    } else {
      /*      printf("Didn't find any neighbor\n");*/
    }
  }
}
/*---------------------------------------------------------------------------*/
int
tree_depth(struct tree_conn *tc)
{
  return tc->hops_from_sink;
}
/*---------------------------------------------------------------------------*/
/** @} */
