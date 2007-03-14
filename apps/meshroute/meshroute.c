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
 * $Id: meshroute.c,v 1.1 2007/03/14 01:06:42 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"

#include "route.h"

struct route_msg {
  node_id_t dest_id;
  u8_t rreq_id;
  u8_t pad;
};

struct data_hdr {
  u8_t type;
  u8_t rreq_id;
  u8_t hops;
  u8_t pad;
  node_id_t dest;
  node_id_t originator;
};

#define TYPE_RREP 1
#define TYPE_DATA 2

struct meshroute_conn {
  struct uc_conn dataconn;
  struct nf_conn rreqconn;
  node_id_t last_rreq_originator;
  u16_t last_rreq_id;
  u16_t rreq_id;
  struct queuebuf *queued_data;
  node_id_t queued_data_dest;
  const struct meshroute_ulayer *u;
};

static struct meshroute_conn mc;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_rreq(node_id_t dest_id)
{
  struct route_msg *msg;

  rimebuf_clear();
  msg = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct route_msg));

  msg->pad = 0;
  msg->rreq_id = mc.rreq_id;
  msg->dest_id = dest_id;

  nf_send(&mc.rreqconn);
}
/*---------------------------------------------------------------------------*/
static void
send_data(node_id_t to, struct route_entry *next)
{
  struct data_hdr *hdr;

  if(rimebuf_hdrextend(sizeof(struct data_hdr))) {
    hdr = rimebuf_hdrptr();
    hdr->hops = 0;
    hdr->pad = 0;
    hdr->dest = to;
    hdr->type = TYPE_DATA;
    hdr->originator = node_id;
    uc_send(&mc.dataconn, next->nexthop);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_rrep(node_id_t dest)
{
  struct data_hdr *rrepmsg;
  struct route_entry *rt;
  
  rimebuf_clear();
  rrepmsg = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct data_hdr));
  rrepmsg->type = TYPE_RREP;
  rrepmsg->hops = 0;
  rrepmsg->pad = 0;
  rrepmsg->dest = dest;
  rrepmsg->originator = node_id;
  rt = route_lookup(&dest);
  if(rt != NULL) {
    PRINTF("send_rrep to %d via %d\n", dest, rt->nexthop);
    uc_send(&mc.dataconn, rt->nexthop);
  }
}
/*---------------------------------------------------------------------------*/
static void
insert_route(node_id_t originator, node_id_t last_hop, u8_t hops)
{
  struct route_entry *rt;
  
  rt = route_lookup(&originator);
  if(rt == NULL || hops < rt->hop_count) {
    PRINTF("%d: Inserting %d into routing table, next hop %d, hop count %d\n",
	   node_id, originator, last_hop, hops);
    route_add(&originator, &last_hop, hops, 0);
  }
}
/*---------------------------------------------------------------------------*/
static void
data_packet_received(struct uc_conn *c, node_id_t from)
{
  struct data_hdr *msg = rimebuf_dataptr();
  struct route_entry *rt;
  node_id_t dest;
  
  PRINTF("data_packet_received from %d towards %d len %d\n", from, msg->dest,
	 rimebuf_datalen());

  insert_route(msg->originator, from, msg->hops);
  
  if(msg->dest == node_id) {
    PRINTF("for us!\n");
    if(msg->type == TYPE_RREP &&
       msg->originator == mc.queued_data_dest &&
       mc.queued_data != NULL) {
      PRINTF("Got RREP for us from %d. queued packet to %d\n",
	     msg->originator, mc.queued_data_dest);

      queuebuf_to_rimebuf(mc.queued_data);
      mc.queued_data = NULL;
      rt = route_lookup(&mc.queued_data_dest);
      if(rt != NULL) {
	PRINTF("Sending data to %d through %d\n", mc.queued_data_dest,
	       rt->nexthop);
	send_data(mc.queued_data_dest, rt);
      }
    } else if(msg->type == TYPE_DATA) {
      rimebuf_hdrreduce(sizeof(struct data_hdr));
      mc.u->recv(msg->originator);
    }
  } else {
    dest = msg->dest;
    
    rt = route_lookup(&msg->dest);
    if(rt != NULL) {
      PRINTF("forwarding to %d\n", rt->nexthop);
      msg->hops++;
      uc_send(c, rt->nexthop);
    } else {
      PRINTF("%d: no route to %d\n", node_id, msg->dest);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
route_packet_received(struct nf_conn *c, node_id_t from,
		      node_id_t originator, u8_t hops)
{
  struct route_msg *msg = rimebuf_dataptr();

  /*  PRINTF("route_packet_received from %d hops %d\n", from, hops);*/

  if(!(mc.last_rreq_originator == originator &&
       mc.last_rreq_id <= msg->rreq_id)) {

    mc.last_rreq_originator = originator;
    mc.last_rreq_id = msg->rreq_id;

    if(msg->dest_id == node_id) {
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
static const struct uc_ulayer data_callbacks = {data_packet_received};
static const struct nf_ulayer route_callbacks = {route_packet_received, NULL};
/*---------------------------------------------------------------------------*/
void
meshroute_init(const struct meshroute_ulayer *u)
{
  uc_setup(&mc.dataconn, CHANNEL_MESHROUTE_DATA, &data_callbacks);
  nf_setup(&mc.rreqconn, CHANNEL_MESHROUTE_RREQ, &route_callbacks);
  mc.u = u;
}
/*---------------------------------------------------------------------------*/
int
meshroute_send(node_id_t to)
{
  struct route_entry *rt;

  rt = route_lookup(&to);
  if(rt == NULL) {
    if(mc.queued_data == NULL) {
      PRINTF("meshroute_send: queueing data, sending rreq\n");
      mc.queued_data = queuebuf_new_from_rimebuf();
      mc.queued_data_dest = to;
      send_rreq(to);
      return 1;
    }
    return 0;
  } else {
    send_data(to, rt);
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
