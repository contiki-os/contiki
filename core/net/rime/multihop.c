/**
 * \addtogroup rimemh
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
 * $Id: multihop.c,v 1.1 2008/07/03 22:36:03 adamdunkels Exp $
 */

/**
 * \file
 *         Multihop forwarding
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/multihop.h"
#include "net/rime/route.h"

#include <string.h>

struct data_hdr {
  rimeaddr_t dest;
  rimeaddr_t originator;
  uint8_t hops;
  uint8_t pad;
};

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
data_packet_received(struct unicast_conn *uc, rimeaddr_t *from)
{
  struct multihop_conn *c = (struct multihop_conn *)uc;
  struct data_hdr msg;
  rimeaddr_t *nexthop;

  memcpy(&msg, rimebuf_dataptr(), sizeof(struct data_hdr));
  
  PRINTF("data_packet_received from %d.%d towards %d.%d len %d\n",
	 from->u8[0], from->u8[1],
	 msg.dest.u8[0], msg.dest.u8[1],
	 rimebuf_datalen());

  if(rimeaddr_cmp(&msg.dest, &rimeaddr_node_addr)) {
    PRINTF("for us!\n");
    rimebuf_hdrreduce(sizeof(struct data_hdr));
    if(c->cb->recv) {
      c->cb->recv(c, &msg.originator, from, msg.hops);
    }
  } else {
    nexthop = NULL;
    if(c->cb->forward) {
      rimebuf_hdrreduce(sizeof(struct data_hdr));
      nexthop = c->cb->forward(c, &msg.originator,
			       &msg.dest, from, msg.hops);
      rimebuf_hdralloc(sizeof(struct data_hdr));
      msg.hops++;
      memcpy(rimebuf_hdrptr(), &msg, sizeof(struct data_hdr));
    }
    if(nexthop) {
      PRINTF("forwarding to %d.%d\n", nexthop->u8[0], nexthop->u8[1]);
      unicast_send(&c->c, nexthop);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks data_callbacks = { data_packet_received };
/*---------------------------------------------------------------------------*/
void
multihop_open(struct multihop_conn *c, uint16_t channel,
	const struct multihop_callbacks *callbacks)
{
  unicast_open(&c->c, channel, &data_callbacks);
  c->cb = callbacks;
}
/*---------------------------------------------------------------------------*/
void
multihop_close(struct multihop_conn *c)
{
  unicast_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
multihop_send(struct multihop_conn *c, rimeaddr_t *to)
{
  rimeaddr_t *nexthop;
  struct data_hdr *hdr;

  if(c->cb->forward == NULL) {
    return 0;
  }
  rimebuf_compact();
  nexthop = c->cb->forward(c, &rimeaddr_node_addr, to, NULL, 0);
  
  if(nexthop == NULL) {
    PRINTF("multihop_send: no route\n");
    return 0;
  } else {
    PRINTF("multihop_send: sending data towards %d.%d\n",
	   nexthop->u8[0], nexthop->u8[1]);
    if(rimebuf_hdralloc(sizeof(struct data_hdr))) {
      hdr = rimebuf_hdrptr();
      rimeaddr_copy(&hdr->dest, to);
      rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
      hdr->hops = 1;
      unicast_send(&c->c, nexthop);
    }
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
