/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: mesh.c,v 1.4 2007/03/19 22:10:17 adamdunkels Exp $
 */

/**
 * \file
 *         A mesh routing protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/route.h"

struct route_msg {
  rimeaddr_t dest;
  u8_t rreq_id;
  u8_t pad;
};

struct data_hdr {
  rimeaddr_t dest;
  rimeaddr_t originator;
  u16_t seqno;
};

struct rrep_hdr {
  u8_t rreq_id;
  u8_t hops;
  rimeaddr_t dest;
  rimeaddr_t originator;
};

struct mesh_conn {
  struct uc_conn dataconn;
  struct uc_conn rrepconn;
  struct nf_conn rreqconn;
  rimeaddr_t last_rreq_originator;
  u16_t last_rreq_id;
  u16_t rreq_id;
  struct queuebuf *queued_data;
  rimeaddr_t queued_data_dest;
  const struct mesh_callbacks *cb;
};

static struct mesh_conn mc;

static void (* send_datapacket_handler)(rimeaddr_t *next) = NULL;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_rreq(rimeaddr_t *dest)
{
  struct route_msg *msg;

  rimebuf_clear();
  msg = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct route_msg));

  msg->pad = 0;
  msg->rreq_id = mc.rreq_id;
  rimeaddr_copy(&msg->dest, dest);

  nf_send(&mc.rreqconn);
}
/*---------------------------------------------------------------------------*/
static void
send_data(rimeaddr_t *to, struct route_entry *next)
{
  struct data_hdr *hdr;

  if(rimebuf_hdrextend(sizeof(struct data_hdr))) {
    hdr = rimebuf_hdrptr();
    rimeaddr_copy(&hdr->dest, to);
    rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
    uc_send(&mc.dataconn, &next->nexthop);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_rrep(rimeaddr_t *dest)
{
  struct rrep_hdr *rrepmsg;
  struct route_entry *rt;
  
  rimebuf_clear();
  rrepmsg = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct data_hdr));
  rrepmsg->hops = 0;
  rimeaddr_copy(&rrepmsg->dest, dest);
  rimeaddr_copy(&rrepmsg->originator, &rimeaddr_node_addr);
  rt = route_lookup(dest);
  if(rt != NULL) {
    PRINTF("send_rrep to %d via %d\n", dest->u16, rt->nexthop.u16);
    uc_send(&mc.rrepconn, &rt->nexthop);
  }
}
/*---------------------------------------------------------------------------*/
static void
insert_route(rimeaddr_t *originator, rimeaddr_t *last_hop, u8_t hops)
{
  struct route_entry *rt;
  
  rt = route_lookup(originator);
  if(rt == NULL || hops < rt->hop_count) {
    PRINTF("%d: Inserting %d into routing table, next hop %d, hop count %d\n",
	   rimeaddr_node_addr.u16, originator->u16, last_hop->u16, hops);
    route_add(originator, last_hop, hops, 0);
  }
}
/*---------------------------------------------------------------------------*/
static rimeaddr_t *
handle_datapacket(rimeaddr_t *from)
{
  struct data_hdr *msg = rimebuf_dataptr();
  struct route_entry *rt;
  rimeaddr_t dest;
  
  PRINTF("data_packet_received from %d towards %d len %d\n", from->u16,
	 msg->dest.u16,
	 rimebuf_datalen());

  /*  insert_route(msg->originator, from, msg->hops);*/
  
  if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
    PRINTF("for us!\n");
    rimebuf_hdrreduce(sizeof(struct data_hdr));
    mc.cb->recv(&msg->originator);
  } else {
    rimeaddr_copy(&dest, &msg->dest);

    rt = route_lookup(&msg->dest);
    if(rt != NULL) {
      PRINTF("forwarding to %d\n", rt->nexthop.u16);
      /*      msg->hops++;*/
      return &rt->nexthop;
      /*      uc_send(c, &rt->nexthop);*/
    } else {
      PRINTF("%d: no route to %d\n", rimeaddr_node_addr.u16, msg->dest.u16);
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static void
data_packet_received(struct uc_conn *c, rimeaddr_t *from)
{
  rimeaddr_t *forward;
  forward = handle_datapacket(from);

  if(forward) {
    uc_send(c, forward);
  }
}
/*---------------------------------------------------------------------------*/
void
mesh_data_received(rimeaddr_t *from)
{
  rimeaddr_t *nexthop_addr;
  nexthop_addr = handle_datapacket(from);

  if(nexthop_addr && send_datapacket_handler) {
    send_datapacket_handler(nexthop_addr);
  }
}
/*---------------------------------------------------------------------------*/
static void
rrep_packet_received(struct uc_conn *c, rimeaddr_t *from)
{
  struct rrep_hdr *msg = rimebuf_dataptr();
  struct route_entry *rt;
  rimeaddr_t dest;
  
  PRINTF("rrep_packet_received from %d towards %d len %d\n", from->u16,
	 msg->dest.u16,
	 rimebuf_datalen());

  insert_route(&msg->originator, from, msg->hops);
  
  if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
    PRINTF("for us!\n");
    if(rimeaddr_cmp(&msg->originator, &mc.queued_data_dest) &&
       mc.queued_data != NULL) {
      PRINTF("Got RREP for us from %d. queued packet to %d\n",
	     msg->originator.u16, mc.queued_data_dest.u16);

      queuebuf_to_rimebuf(mc.queued_data);
      mc.queued_data = NULL;
      rt = route_lookup(&mc.queued_data_dest);
      if(rt != NULL) {
	PRINTF("Sending data to %d through %d\n", mc.queued_data_dest.u16,
	       rt->nexthop.u16);
	send_data(&mc.queued_data_dest, rt);
      }
    }
  } else {
    rimeaddr_copy(&dest, &msg->dest);

    rt = route_lookup(&msg->dest);
    if(rt != NULL) {
      PRINTF("forwarding to %d\n", rt->nexthop.u16);
      msg->hops++;
      uc_send(c, &rt->nexthop);
    } else {
      PRINTF("%d: no route to %d\n", rimeaddr_node_addr.u16, msg->dest.u16);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
rreq_packet_received(struct nf_conn *c, rimeaddr_t *from,
		      rimeaddr_t *originator, u8_t seqno, u8_t hops)
{
  struct route_msg *msg = rimebuf_dataptr();

  /*  PRINTF("route_packet_received from %d hops %d\n", from, hops);*/

  if(!(rimeaddr_cmp(&mc.last_rreq_originator, originator) &&
       mc.last_rreq_id <= msg->rreq_id)) {

    rimeaddr_copy(&mc.last_rreq_originator, originator);
    mc.last_rreq_id = msg->rreq_id;

    if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
      PRINTF("route_packet_received: route request for our ID\n");
      insert_route(originator, from, hops);
      
      /* Send route reply back to source. */
      send_rrep(originator);
      return 0; /* Don't continue to flood the rreq packet. */
    } else {
      /*      PRINTF("route request for %d\n", msg->dest_id);*/
      insert_route(originator, from, hops);
    }
    
    return 1;
  }
  return 0; /* Don't forward packet. */
}
/*---------------------------------------------------------------------------*/
static const struct uc_callbacks data_callbacks = {data_packet_received};
static const struct uc_callbacks rrep_callbacks = {rrep_packet_received};
static const struct nf_callbacks rreq_callbacks = {rreq_packet_received, NULL};
/*---------------------------------------------------------------------------*/
void
mesh_open(const struct mesh_callbacks *callbacks,
	   void (* send_datapacket)(rimeaddr_t *next))
{
  uc_open(&mc.dataconn, CHANNEL_MESH_DATA, &data_callbacks);
  uc_open(&mc.rrepconn, CHANNEL_MESH_RREP, &rrep_callbacks);
  nf_open(&mc.rreqconn, CHANNEL_MESH_RREQ, &rreq_callbacks);
  mc.cb = callbacks;

  send_datapacket_handler = send_datapacket;
}
/*---------------------------------------------------------------------------*/
void
mesh_close(void)
{
  uc_close(&mc.dataconn);
  uc_close(&mc.rrepconn);
  nf_close(&mc.rreqconn);
}
/*---------------------------------------------------------------------------*/
int
mesh_send(rimeaddr_t *to)
{
  struct route_entry *rt;

  rt = route_lookup(to);
  if(rt == NULL) {
    if(mc.queued_data == NULL) {
      PRINTF("mesh_send: queueing data, sending rreq\n");
      mc.queued_data = queuebuf_new_from_rimebuf();
      rimeaddr_copy(&mc.queued_data_dest, to);
      send_rreq(to);
      return 1;
    }
    return 0;
  } else {
    PRINTF("mesh_send: sending data\n");
    send_data(to, rt);
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
