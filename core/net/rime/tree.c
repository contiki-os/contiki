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
 * $Id: tree.c,v 1.3 2007/03/19 19:24:37 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime.h"
#include "net/rime/neighbor.h"

#include "dev/radio-sensor.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>

static struct ruc_conn ruc_conn;
static struct sibc_conn sibc_conn;

struct hdr {
  rimeaddr_t originator;
  u8_t originator_seqno;
  u8_t hopcount;
  u8_t hoplim;
  u8_t retransmissions;
  u8_t datalen;
  u8_t data[1];
};

#define SINK 0
#define HOPCOUNT_MAX TREE_MAX_DEPTH

#define MAX_HOPLIM 10

struct tree_conn {
  int forwarding;
  struct hdr hello;
  u8_t seqno;
  const struct tree_callbacks *cb;
};

static struct tree_conn tc;

/*---------------------------------------------------------------------------*/
static void
update_hopcount(void)
{
  struct neighbor *n;
  
  if(tc.hello.hopcount != SINK) {
    n = neighbor_best();
    if(n == NULL) {
      /*      if(hopcount != HOPCOUNT_MAX) {
	printf("%d: didn't find a best neighbor, setting hopcount to max\n", node_id);
	}*/
      tc.hello.hopcount = HOPCOUNT_MAX;
    } else {
      if(n->hopcount + 1 != tc.hello.hopcount) {
	tc.hello.hopcount = n->hopcount + 1;
      }
    }
  }

  /*  DEBUG_PRINTF("%d: new hopcount %d\n", node_id, hopcount);*/
#if NETSIM
  {
    char buf[8];
    if(tc.hello.hopcount == HOPCOUNT_MAX) {
      strcpy(buf, " ");
    } else {
      snprintf(buf, sizeof(buf), "%d", tc.hello.hopcount);
    }
    ether_set_text(buf);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
neighbor_packet_received(struct sibc_conn *c, rimeaddr_t *from)
{
  struct hdr *hdr = rimebuf_dataptr();
  struct neighbor *n;

  /*  printf("%d: neighbor_packet_received from %d with hopcount %d\n",
	 node_id, from, hdr->hopcount
	 );*/
  
  n = neighbor_find(from);

  if(n == NULL) {
    neighbor_add(from, hdr->hopcount, radio_sensor.value(1));
  } else {
    neighbor_update(n, hdr->hopcount, radio_sensor.value(1));
  }

  update_hopcount();

}
/*---------------------------------------------------------------------------*/
static int
node_packet_received(struct ruc_conn *c, rimeaddr_t *from, u8_t seqno)
{
  struct hdr *hdr = rimebuf_dataptr();
  struct neighbor *n;

  if(tc.hello.hopcount == SINK) {
    printf("Sink packet received\n");
    if(tc.cb->recv != NULL) {
      /*      receiver(hdr->data, hdr->datalen,
	       &hdr->originator, hdr->originator_seqno,
	       from, MAX_HOPLIM - hdr->hoplim,
	       hdr->retransmissions);*/
    }
    return 1;
  } else if(hdr->hoplim > 1 && tc.hello.hopcount != HOPCOUNT_MAX) {
    printf("%d: packet received from %d, forwarding %d, best neighbor %p\n",
	   rimeaddr_node_addr.u16, from->u16, tc.forwarding, neighbor_best());
    if(!tc.forwarding) {
      tc.forwarding = 1;
      n = neighbor_best();
      if(n != NULL) {
	ruc_send(c, &n->addr);
      }
      return 1;
    } else {

      printf("%d: still forwarding another packet, not sending ACK\n",
	     rimeaddr_node_addr.u16);
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
node_packet_sent(struct ruc_conn *c)
{
  tc.forwarding = 0;
}
/*---------------------------------------------------------------------------*/
static const struct sibc_callbacks sibc_callbacks = {neighbor_packet_received, NULL};
static const struct ruc_callbacks ruc_callbacks = {node_packet_received,
					     node_packet_sent};
/*---------------------------------------------------------------------------*/
void
tree_open(const struct tree_callbacks *cb)
{
  sibc_open(&sibc_conn, CHANNEL_TREE_META, &sibc_callbacks);
  ruc_open(&ruc_conn, CHANNEL_TREE_DATA, &ruc_callbacks);
  tc.hello.hopcount = HOPCOUNT_MAX;
  rimebuf_clear();
  rimebuf_reference(&tc.hello, sizeof(tc.hello));
  sibc_send_stubborn(&sibc_conn, CLOCK_SECOND * 8);
  tc.cb = cb;
}
/*---------------------------------------------------------------------------*/
void
tree_set_sink(int should_be_sink)
{
  if(should_be_sink) {
    tc.hello.hopcount = SINK;
  } else {
    tc.hello.hopcount = HOPCOUNT_MAX;
  }
}
/*---------------------------------------------------------------------------*/
void
tree_send(void)
{
  struct neighbor *n;
  struct hdr *hdr;

  if(rimebuf_hdrextend(sizeof(struct hdr))) {
    hdr = rimebuf_hdrptr();
    hdr->originator_seqno = tc.seqno++;
    rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
    hdr->hopcount = tc.hello.hopcount;
    hdr->hoplim = MAX_HOPLIM;
    hdr->retransmissions = 0;
    hdr->datalen = 0;
    n = neighbor_best();
    if(n != NULL) {
      printf("Sending to best neighbor\n");
      ruc_send(&ruc_conn, &n->addr);
    } else {
      printf("Didn't find any neighbor\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
int
tree_depth(void)
{
  return tc.hello.hopcount;
}
/*---------------------------------------------------------------------------*/
