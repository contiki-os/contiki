/**
 * \addtogroup rimerunicast
 * @{
 */


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
 * $Id: runicast.c,v 1.2 2008/07/07 23:27:57 adamdunkels Exp $
 */

/**
 * \file
 *         Reliable unicast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/runicast.h"
#include "net/rime/neighbor.h"
#include "net/rime.h"
#include <string.h>

#define RUNICAST_PACKET_ID_BITS 2

#define REXMIT_TIME CLOCK_SECOND

static const struct rimebuf_attrlist attributes[] =
  {
    RUNICAST_ATTRIBUTES
    RIMEBUF_ATTR_LAST
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
sent_by_stunicast(struct stunicast_conn *stunicast)
{
  struct runicast_conn *c = (struct runicast_conn *)stunicast;

  if(c->rxmit != 0) {
    RIMESTATS_ADD(rexmit);
    PRINTF("%d.%d: runicast: packet %u resent %u\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   rimebuf_attr(RIMEBUF_ATTR_PACKET_ID), c->rxmit);
  }

  c->rxmit++;
  if(c->rxmit >= c->max_rxmit) {
    RIMESTATS_ADD(timedout);
    stunicast_cancel(&c->c);
    if(c->u->timedout) {
      c->u->timedout(c, stunicast_receiver(&c->c), c->rxmit);
    }
    PRINTF("%d.%d: runicast: packet %d timed out\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	   c->sndnxt);
  } else {
    int shift;

    shift = c->rxmit > 4? 4: c->rxmit;
    stunicast_set_timer(&c->c, (REXMIT_TIME) << shift);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv_from_stunicast(struct stunicast_conn *stunicast, rimeaddr_t *from)
{
  struct runicast_conn *c = (struct runicast_conn *)stunicast;
  /*  struct runicast_hdr *hdr = rimebuf_dataptr();*/

  PRINTF("%d.%d: runicast: recv_from_stunicast from %d.%d type %d seqno %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1],
	 rimebuf_attr(RIMEBUF_ATTR_PACKET_TYPE),
	 rimebuf_attr(RIMEBUF_ATTR_PACKET_ID));

  if(rimebuf_attr(RIMEBUF_ATTR_PACKET_TYPE) ==
     RIMEBUF_ATTR_PACKET_TYPE_ACK) {
    if(rimebuf_attr(RIMEBUF_ATTR_PACKET_ID) == c->sndnxt) {
      RIMESTATS_ADD(ackrx);
      PRINTF("%d.%d: runicast: ACKed %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     rimebuf_attr(RIMEBUF_ATTR_PACKET_ID));
      c->sndnxt = (c->sndnxt + 1) % (1 << RUNICAST_PACKET_ID_BITS);
      stunicast_cancel(&c->c);
      if(c->u->sent != NULL) {
	c->u->sent(c, stunicast_receiver(&c->c), c->rxmit);
      }
    } else {
      PRINTF("%d.%d: runicast: received bad ACK %d for %d\n",
	     rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	     rimebuf_attr(RIMEBUF_ATTR_PACKET_ID),
	     c->sndnxt);
      RIMESTATS_ADD(badackrx);
    }
  } else if(rimebuf_attr(RIMEBUF_ATTR_PACKET_TYPE) ==
	    RIMEBUF_ATTR_PACKET_TYPE_DATA) {
    /*    int send_ack = 1;*/
    uint16_t packet_seqno;
    struct queuebuf *q;

    RIMESTATS_ADD(reliablerx);

    PRINTF("%d.%d: runicast: got packet %d\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	   rimebuf_attr(RIMEBUF_ATTR_PACKET_ID));

    packet_seqno = rimebuf_attr(RIMEBUF_ATTR_PACKET_ID);

    /*    rimebuf_hdrreduce(sizeof(struct runicast_hdr));*/

    q = queuebuf_new_from_rimebuf();
    if(q != NULL) {
      PRINTF("%d.%d: runicast: Sending ACK to %d.%d for %d\n",
	     rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	     from->u8[0], from->u8[1],
	     packet_seqno);
      rimebuf_clear();
      /*    rimebuf_hdralloc(sizeof(struct runicast_hdr));
	    hdr = rimebuf_hdrptr();
	    hdr->type = TYPE_ACK;
	    hdr->seqno = packet_seqno;*/
      rimebuf_set_attr(RIMEBUF_ATTR_PACKET_TYPE, RIMEBUF_ATTR_PACKET_TYPE_ACK);
      rimebuf_set_attr(RIMEBUF_ATTR_PACKET_ID, packet_seqno);
      stunicast_send(&c->c, from);
      RIMESTATS_ADD(acktx);
      
      queuebuf_to_rimebuf(q);
      queuebuf_free(q);
    }      
    if(c->u->recv != NULL) {
      c->u->recv(c, from, packet_seqno);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct stunicast_callbacks runicast = {recv_from_stunicast, sent_by_stunicast};
/*---------------------------------------------------------------------------*/
void
runicast_open(struct runicast_conn *c, uint16_t channel,
	  const struct runicast_callbacks *u)
{
  stunicast_open(&c->c, channel, &runicast);
  channel_set_attributes(channel, attributes);
  c->u = u;
  c->rxmit = 0;
  c->sndnxt = 0;
}
/*---------------------------------------------------------------------------*/
void
runicast_close(struct runicast_conn *c)
{
  stunicast_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
runicast_send(struct runicast_conn *c, rimeaddr_t *receiver, uint8_t max_retransmissions)
{
  rimebuf_set_attr(RIMEBUF_ATTR_RELIABLE, 1);
  rimebuf_set_attr(RIMEBUF_ATTR_PACKET_TYPE, RIMEBUF_ATTR_PACKET_TYPE_DATA);
  rimebuf_set_attr(RIMEBUF_ATTR_PACKET_ID, c->sndnxt);
  c->max_rxmit = max_retransmissions;
  c->rxmit = 0;
  RIMESTATS_ADD(reliabletx);
  PRINTF("%d.%d: runicast: sending packet %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 c->sndnxt);
  return stunicast_send_stubborn(&c->c, receiver, REXMIT_TIME);
}
/*---------------------------------------------------------------------------*/
/** @} */
