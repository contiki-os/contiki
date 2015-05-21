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
 */

/**
 * \addtogroup rimeibc
 * @{
 */

/**
 * \file
 *         Identified best-effort local area broadcast (broadcast)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki-net.h"
#include <string.h>

static const struct packetbuf_attrlist attributes[] =
  {
    BROADCAST_ATTRIBUTES PACKETBUF_ATTR_LAST
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
recv_from_abc(struct abc_conn *bc)
{
  linkaddr_t sender;
  struct broadcast_conn *c = (struct broadcast_conn *)bc;

  linkaddr_copy(&sender, packetbuf_addr(PACKETBUF_ADDR_SENDER));
  
  PRINTF("%d.%d: broadcast: from %d.%d\n",
	 linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1],
	 sender.u8[0], sender.u8[1]);
  if(c->u->recv) {
    c->u->recv(c, &sender);
  }
}
/*---------------------------------------------------------------------------*/
static void
sent_by_abc(struct abc_conn *bc, int status, int num_tx)
{
  struct broadcast_conn *c = (struct broadcast_conn *)bc;

  PRINTF("%d.%d: sent to %d.%d status %d num_tx %d\n",
	 linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1],
	 packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
         packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1],
         status, num_tx);
  if(c->u->sent) {
    c->u->sent(c, status, num_tx);
  }
}
/*---------------------------------------------------------------------------*/
static const struct abc_callbacks broadcast = {recv_from_abc, sent_by_abc};
/*---------------------------------------------------------------------------*/
void
broadcast_open(struct broadcast_conn *c, uint16_t channel,
	  const struct broadcast_callbacks *u)
{
  abc_open(&c->c, channel, &broadcast);
  c->u = u;
  channel_set_attributes(channel, attributes);
}
/*---------------------------------------------------------------------------*/
void
broadcast_close(struct broadcast_conn *c)
{
  abc_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
broadcast_send(struct broadcast_conn *c)
{
  PRINTF("%d.%d: broadcast_send\n",
	 linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1]);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  return abc_send(&c->c);
}
/*---------------------------------------------------------------------------*/
/** @} */
