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
 * $Id: rucb.c,v 1.5 2008/01/14 09:36:35 adamdunkels Exp $
 */

/**
 * \file
 *         Reliable unicast bulk transfer
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/rucb.h"
#include "net/rime.h"
#include <string.h>

#define MAX_TRANSMISSIONS 8

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static int
read_data(struct rucb_conn *c)
{
  int len = 0;

  rimebuf_clear();
  if(c->u->read_chunk) {
    len = c->u->read_chunk(c, c->chunk * RUCB_DATASIZE,
			    rimebuf_dataptr(), RUCB_DATASIZE);
  }
  rimebuf_set_datalen(len);
  return len;
}
/*---------------------------------------------------------------------------*/
static void
acked(struct ruc_conn *ruc, rimeaddr_t *to, u8_t retransmissions)
{
  struct rucb_conn *c = (struct rucb_conn *)ruc;
  PRINTF("%d.%d: rucb acked\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
  c->chunk++;
  if(read_data(c) > 0) {
    ruc_send(&c->c, &c->receiver, MAX_TRANSMISSIONS);
  }
}
/*---------------------------------------------------------------------------*/
static void
timedout(struct ruc_conn *ruc, rimeaddr_t *to, u8_t retransmissions)
{
  struct rucb_conn *c = (struct rucb_conn *)ruc;
  PRINTF("%d.%d: rucb timedout\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
  if(c->u->timedout) {
    c->u->timedout(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv(struct ruc_conn *ruc, rimeaddr_t *from, u8_t seqno)
{
  struct rucb_conn *c = (struct rucb_conn *)ruc;

  PRINTF("%d.%d: rucb: recv from %d.%d len %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], rimebuf_totlen());

  if(seqno == c->last_seqno) {
    return;
  }
  c->last_seqno = seqno;

  if(rimeaddr_cmp(&c->sender, &rimeaddr_null)) {
    rimeaddr_copy(&c->sender, from);
    c->u->write_chunk(c, 0, RUCB_FLAG_NEWFILE, rimebuf_dataptr(), 0);
    c->chunk = 0;
  }

  
  if(rimeaddr_cmp(&c->sender, from)) {
    int datalen = rimebuf_datalen();
    
    if(datalen < RUCB_DATASIZE) {
      PRINTF("%d.%d: get %d bytes, file complete\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     datalen);
      c->u->write_chunk(c, c->chunk * RUCB_DATASIZE,
			 RUCB_FLAG_LASTCHUNK, rimebuf_dataptr(), datalen);
    } else {
      c->u->write_chunk(c, c->chunk * RUCB_DATASIZE,
			RUCB_FLAG_NONE, rimebuf_dataptr(), datalen);
    }
    c->chunk++;
  }

  if(rimebuf_datalen() < RUCB_DATASIZE) {
    rimeaddr_copy(&c->sender, &rimeaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
static const struct ruc_callbacks ruc = {recv, acked, timedout};
/*---------------------------------------------------------------------------*/
void
rucb_open(struct rucb_conn *c, u16_t channel,
	  const struct rucb_callbacks *u)
{
  ruc_open(&c->c, channel, &ruc);
  c->u = u;
  c->last_seqno = -1;
}
/*---------------------------------------------------------------------------*/
void
rucb_close(struct rucb_conn *c)
{
  ruc_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
rucb_send(struct rucb_conn *c, rimeaddr_t *receiver)
{
  read_data(c);
  rimeaddr_copy(&c->receiver, receiver);
  rimeaddr_copy(&c->sender, &rimeaddr_node_addr);
  ruc_send(&c->c, receiver, MAX_TRANSMISSIONS);
  return 0;
}
/*---------------------------------------------------------------------------*/
