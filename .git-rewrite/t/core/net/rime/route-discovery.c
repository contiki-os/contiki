/**
 * \addtogroup routediscovery
 * @{
 */

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
 * $Id: route-discovery.c,v 1.7 2007/08/30 14:39:17 matsutsuka Exp $
 */

/**
 * \file
 *         Route discovery protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/route.h"
#include "net/rime/route-discovery.h"

#include <stddef.h> /* For offsetof */

struct route_msg {
  rimeaddr_t dest;
  u8_t rreq_id;
  u8_t pad;
};

struct rrep_hdr {
  u8_t rreq_id;
  u8_t hops;
  rimeaddr_t dest;
  rimeaddr_t originator;
};

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_rreq(struct route_discovery_conn *c, rimeaddr_t *dest)
{
  struct route_msg *msg;

  rimebuf_clear();
  msg = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct route_msg));

  msg->pad = 0;
  msg->rreq_id = c->rreq_id;
  rimeaddr_copy(&msg->dest, dest);

  nf_send(&c->rreqconn, c->rreq_id);
  c->rreq_id++;
}
/*---------------------------------------------------------------------------*/
static void
send_rrep(struct route_discovery_conn *c, rimeaddr_t *dest)
{
  struct rrep_hdr *rrepmsg;
  struct route_entry *rt;
  
  rimebuf_clear();
  rrepmsg = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct rrep_hdr));
  rrepmsg->hops = 0;
  rimeaddr_copy(&rrepmsg->dest, dest);
  rimeaddr_copy(&rrepmsg->originator, &rimeaddr_node_addr);
  rt = route_lookup(dest);
  if(rt != NULL) {
    PRINTF("%d.%d: send_rrep to %d.%d via %d.%d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   dest->u8[0],dest->u8[1],
	   rt->nexthop.u8[0],rt->nexthop.u8[1]);
    uc_send(&c->rrepconn, &rt->nexthop);
  }
}
/*---------------------------------------------------------------------------*/
static void
insert_route(rimeaddr_t *originator, rimeaddr_t *last_hop, u8_t hops)
{
  struct route_entry *rt;
  
  rt = route_lookup(originator);
  if(rt == NULL || hops < rt->hop_count) {
    PRINTF("%d.%d: Inserting %d.%d into routing table, next hop %d.%d, hop count %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   originator->u8[0], originator->u8[1],
	   last_hop->u8[0], last_hop->u8[1],
	   hops);
    route_add(originator, last_hop, hops, 0);
  }
}
/*---------------------------------------------------------------------------*/
static void
rrep_packet_received(struct uc_conn *uc, rimeaddr_t *from)
{
  struct rrep_hdr *msg = rimebuf_dataptr();
  struct route_entry *rt;
  rimeaddr_t dest;
  struct route_discovery_conn *c = (struct route_discovery_conn *)
    ((char *)uc - offsetof(struct route_discovery_conn, rrepconn));

  PRINTF("%d.%d: rrep_packet_received from %d.%d towards %d.%d len %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0],from->u8[1],
	 msg->dest.u8[0],msg->dest.u8[1],
	 rimebuf_datalen());

  insert_route(&msg->originator, from, msg->hops);

  if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
    PRINTF("rrep for us!\n");
    if(c->cb->new_route) {
      c->cb->new_route(c, &msg->originator);
    }

  } else {
    rimeaddr_copy(&dest, &msg->dest);

    rt = route_lookup(&msg->dest);
    if(rt != NULL) {
      PRINTF("forwarding to %d\n", rt->nexthop.u16[0]);
      msg->hops++;
      uc_send(&c->rrepconn, &rt->nexthop);
    } else {
      PRINTF("%d: no route to %d\n", rimeaddr_node_addr.u16[0], msg->dest.u16[0]);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
rreq_packet_received(struct nf_conn *nf, rimeaddr_t *from,
		      rimeaddr_t *originator, u8_t seqno, u8_t hops)
{
  struct route_msg *msg = rimebuf_dataptr();
  struct route_discovery_conn *c = (struct route_discovery_conn *)
    ((char *)nf - offsetof(struct route_discovery_conn, rreqconn));

  PRINTF("%d.%d: rreq_packet_received from %d.%d hops %d rreq_id %d last %d/%d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1],
	 hops, msg->rreq_id,
	 c->last_rreq_originator.u16[0],
	 c->last_rreq_id);

  if(!(rimeaddr_cmp(&c->last_rreq_originator, originator) &&
       c->last_rreq_id == msg->rreq_id)) {

    PRINTF("%d.%d: rreq_packet_received: request for %d.%d originator %d.%d / %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   msg->dest.u8[0], msg->dest.u8[1],
	   originator->u8[0], originator->u8[1],
	   msg->rreq_id);

    rimeaddr_copy(&c->last_rreq_originator, originator);
    c->last_rreq_id = msg->rreq_id;

    if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
      PRINTF("%d.%d: route_packet_received: route request for our address\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
      insert_route(originator, from, hops);
      
      /* Send route reply back to source. */
      send_rrep(c, originator);
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
static const struct uc_callbacks rrep_callbacks = {rrep_packet_received};
static const struct nf_callbacks rreq_callbacks = {rreq_packet_received, NULL, NULL};
/*---------------------------------------------------------------------------*/
void
route_discovery_open(struct route_discovery_conn *c,
		     clock_time_t time,
		     u16_t channels,
		     const struct route_discovery_callbacks *callbacks)
{
  nf_open(&c->rreqconn, time, channels + 0, &rreq_callbacks);
  uc_open(&c->rrepconn, channels + 1, &rrep_callbacks);
  c->cb = callbacks;
}
/*---------------------------------------------------------------------------*/
void
route_discovery_close(struct route_discovery_conn *c)
{
  uc_close(&c->rrepconn);
  nf_close(&c->rreqconn);
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void *ptr)
{
  struct route_discovery_conn *c = ptr;
  PRINTF("route_discovery: timeout, timed out\n");
  if(c->cb->timedout) {
    c->cb->timedout(c);
  }
}
/*---------------------------------------------------------------------------*/
void
route_discovery_discover(struct route_discovery_conn *c, rimeaddr_t *addr,
			 clock_time_t timeout)
{
  PRINTF("route_discovery_send: sending route request\n");
  ctimer_set(&c->t, timeout, timeout_handler, c);
  send_rreq(c, addr);
}
/*---------------------------------------------------------------------------*/
/** @} */
