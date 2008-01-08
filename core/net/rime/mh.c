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
 * $Id: mh.c,v 1.7 2008/01/08 07:55:56 adamdunkels Exp $
 */

/**
 * \file
 *         Multihop forwarding
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/mh.h"
#include "net/rime/route.h"

struct data_hdr {
  rimeaddr_t dest;
  rimeaddr_t originator;
  u8_t hops;
  u8_t pad;
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
data_packet_received(struct uc_conn *uc, rimeaddr_t *from)
{
  struct mh_conn *c = (struct mh_conn *)uc;
  struct data_hdr *msg = rimebuf_dataptr();
  rimeaddr_t *nexthop;

  PRINTF("data_packet_received from %d towards %d len %d\n", from->u16[0],
	 msg->dest.u16[0],
	 rimebuf_datalen());

  if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
    PRINTF("for us!\n");
    rimebuf_hdrreduce(sizeof(struct data_hdr));
    if(c->cb->recv) {
      c->cb->recv(c, &msg->originator, msg->hops);
    }
  } else {
    nexthop = NULL;
    if(c->cb->forward) {
      nexthop = c->cb->forward(c, &msg->originator,
			       &msg->dest, from, msg->hops);
    }
    if(nexthop) {
      PRINTF("forwarding to %d.%d\n", nexthop->u8[0], nexthop->u8[1]);
      msg->hops++;
      uc_send(&c->c, nexthop);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct uc_callbacks data_callbacks = { data_packet_received };
/*---------------------------------------------------------------------------*/
void
mh_open(struct mh_conn *c, u16_t channel,
	const struct mh_callbacks *callbacks)
{
  uc_open(&c->c, channel, &data_callbacks);
  c->cb = callbacks;
}
/*---------------------------------------------------------------------------*/
void
mh_close(struct mh_conn *c)
{
  uc_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
mh_send(struct mh_conn *c, rimeaddr_t *to)
{
  rimeaddr_t *nexthop;
  struct data_hdr *hdr;

  if(c->cb->forward == NULL) {
    return 0;
  }
  nexthop = c->cb->forward(c, &rimeaddr_node_addr, to, NULL, 0);
  
  if(nexthop == NULL) {
    PRINTF("mh_send: no route\n");
    return 0;
  } else {
    PRINTF("mh_send: sending data towards %d.%d\n",
	   nexthop->u8[0], nexthop->u8[1]);
    if(rimebuf_hdralloc(sizeof(struct data_hdr))) {
      hdr = rimebuf_hdrptr();
      rimeaddr_copy(&hdr->dest, to);
      rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
      hdr->hops = 1;
      uc_send(&c->c, nexthop);
    }
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
