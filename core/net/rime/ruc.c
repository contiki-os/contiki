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
 * $Id: ruc.c,v 1.1 2007/02/28 16:38:52 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/ruc.h"
#include "net/rime/neighbor.h"
#include "net/rime.h"
#include <string.h>

#define TYPE_DATA 0
#define TYPE_ACK  1

struct ruc_hdr {
  u8_t type;
  u8_t seqno;
};

enum {
  STATE_READY,
  STATE_SENDING
};

static u8_t seqno;

/*---------------------------------------------------------------------------*/
static void
sent_by_suc(struct suc_conn *suc)
{
  
}
/*---------------------------------------------------------------------------*/
static void
recv_from_suc(struct suc_conn *suc, node_id_t from_id)
{
  struct ruc_conn *c = (struct ruc_conn *)suc;
  struct ruc_hdr *hdr = rimebuf_dataptr();

  DEBUGF(4, "%d: ruc: recv_from_suc type %d seqno %d\n", node_id,
	 hdr->type, hdr->seqno);
  
  if(hdr->type == TYPE_ACK) {
    if(hdr->seqno == seqno) {
      DEBUGF(4, "%d: ruc: ACKed\n", node_id);
      ++seqno;
      suc_cancel(&c->c);
      if(c->u->sent != NULL) {
	c->u->sent(c);
      }
    }
  } else if(hdr->type == TYPE_DATA) {
    int send_ack = 1;
    u16_t packet_seqno;

    packet_seqno = hdr->seqno;
    

    rimebuf_hdrreduce(sizeof(struct ruc_hdr));
    if(c->u->recv != NULL) {
      send_ack = c->u->recv(c, from_id, packet_seqno);
    }
    
    if(send_ack) {
      DEBUGF(4, "%d: ruc: Sending ACK to %d for %d\n", node_id, from_id,
	     packet_seqno);
      rimebuf_clear();
      rimebuf_hdrextend(sizeof(struct ruc_hdr));
      hdr = rimebuf_hdrptr();
      hdr->type = TYPE_ACK;
      hdr->seqno = packet_seqno;
      suc_send_uc(&c->c, from_id);
    } else {
      DEBUGF(4, "%d: Not sending ACK\n", node_id);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct suc_ulayer ruc = {recv_from_suc, sent_by_suc};
/*---------------------------------------------------------------------------*/
void
ruc_setup(struct ruc_conn *c, u16_t channel,
	  const struct ruc_ulayer *u)
{
  suc_setup(&c->c, channel, &ruc);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
int
ruc_send(struct ruc_conn *c, node_id_t receiver_id)
{
  if(rimebuf_hdrextend(sizeof(struct ruc_hdr))) {
    struct ruc_hdr *hdr = rimebuf_hdrptr();
    hdr->type = TYPE_DATA;
    hdr->seqno = seqno;
    return suc_send_stubborn(&c->c, receiver_id);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
