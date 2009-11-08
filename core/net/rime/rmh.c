/**
 * \addtogroup rimermh
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
 * $Id: rmh.c,v 1.10 2009/11/08 19:40:17 adamdunkels Exp $
 */

/**
 * \file
 *         Multihop forwarding
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/rmh.h"

struct data_hdr {
  rimeaddr_t dest;
  rimeaddr_t originator;
  uint8_t hops;
  uint8_t max_rexmits;
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
received(struct runicast_conn *uc, const rimeaddr_t *from, uint8_t seqno)
{
  struct rmh_conn *c = (struct rmh_conn *)uc;
  struct data_hdr *msg = packetbuf_dataptr();
  rimeaddr_t *nexthop;

  PRINTF("data_packet_received from %d.%d towards %d.%d len %d\n", 
         from->u8[0], from->u8[1],
	 msg->dest.u8[0], msg->dest.u8[1],
	 packetbuf_datalen());

  if(rimeaddr_cmp(&msg->dest, &rimeaddr_node_addr)) {
    PRINTF("for us!\n");
    packetbuf_hdrreduce(sizeof(struct data_hdr));
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
      runicast_send(&c->c, nexthop, c->num_rexmit);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
sent(struct runicast_conn *c, const rimeaddr_t *to, uint8_t retransmissions)
{

}
/*---------------------------------------------------------------------------*/
static void
timedout(struct runicast_conn *c, const rimeaddr_t *to, uint8_t retransmissions)
{

}
/*---------------------------------------------------------------------------*/
static const struct runicast_callbacks data_callbacks = { received ,
						     sent,
						     timedout};
/*---------------------------------------------------------------------------*/
void
rmh_open(struct rmh_conn *c, uint16_t channel,
	const struct rmh_callbacks *callbacks)
{
  runicast_open(&c->c, channel, &data_callbacks);
  c->cb = callbacks;
}
/*---------------------------------------------------------------------------*/
void
rmh_close(struct rmh_conn *c)
{
  runicast_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
rmh_send(struct rmh_conn *c, rimeaddr_t *to, uint8_t num_rexmit, uint8_t max_hops)
{
  rimeaddr_t *nexthop;
  struct data_hdr *hdr;
  
  c->num_rexmit = num_rexmit;
  
  if(c->cb->forward == NULL) {
    return 0;
  }
  
  nexthop = c->cb->forward(c, &rimeaddr_node_addr, to, NULL, 0);
  if(nexthop == NULL) {
    PRINTF("rmh_send: no route\n");
    return 0;
  } else {
    PRINTF("rmh_send: sending data\n");

    
    if(packetbuf_hdralloc(sizeof(struct data_hdr))) {
      hdr = packetbuf_hdrptr();
      rimeaddr_copy(&hdr->dest, to);
      rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
      hdr->hops = 1;
      hdr->max_rexmits = num_rexmit;
      runicast_send(&c->c, nexthop, num_rexmit);
    }
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
