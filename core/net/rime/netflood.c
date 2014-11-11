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
 * \file
 *         Best-effort network flooding (netflood)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rimenetflood
 * @{
 */

#include "net/rime/netflood.h"

#include <string.h>

#define HOPS_MAX 16

struct netflood_hdr {
  uint16_t originator_seqno;
  linkaddr_t originator;
  uint16_t hops;
};

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static int
send(struct netflood_conn *c)
{
  PRINTF("%d.%d: netflood send to ipolite\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
  return ipolite_send(&c->c, c->queue_time, 4);
}
/*---------------------------------------------------------------------------*/
static void
recv_from_ipolite(struct ipolite_conn *ipolite, const linkaddr_t *from)
{
  struct netflood_conn *c = (struct netflood_conn *)ipolite;
  struct netflood_hdr hdr;
  uint8_t hops;
  struct queuebuf *queuebuf;

  memcpy(&hdr, packetbuf_dataptr(), sizeof(struct netflood_hdr));
  hops = hdr.hops;

  /* Remember packet if we need to forward it. */
  queuebuf = queuebuf_new_from_packetbuf();

  packetbuf_hdrreduce(sizeof(struct netflood_hdr));
  if(c->u->recv != NULL) {
    if(!(linkaddr_cmp(&hdr.originator, &c->last_originator) &&
	 hdr.originator_seqno <= c->last_originator_seqno)) {

      if(c->u->recv(c, from, &hdr.originator, hdr.originator_seqno,
		    hops)) {
	
	if(queuebuf != NULL) {
	  queuebuf_to_packetbuf(queuebuf);
	  queuebuf_free(queuebuf);
	  queuebuf = NULL;
	  memcpy(&hdr, packetbuf_dataptr(), sizeof(struct netflood_hdr));
	  
	  /* Rebroadcast received packet. */
	  if(hops < HOPS_MAX) {
	    PRINTF("%d.%d: netflood rebroadcasting %d.%d/%d (%d.%d/%d) hops %d\n",
		   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
		   hdr.originator.u8[0], hdr.originator.u8[1],
		   hdr.originator_seqno,
		   c->last_originator.u8[0], c->last_originator.u8[1],
		   c->last_originator_seqno,
		  hops);
	    hdr.hops++;
	    memcpy(packetbuf_dataptr(), &hdr, sizeof(struct netflood_hdr));
	    send(c);
	    linkaddr_copy(&c->last_originator, &hdr.originator);
	    c->last_originator_seqno = hdr.originator_seqno;
	  }
	}
      }
    }
  }
  if(queuebuf != NULL) {
    queuebuf_free(queuebuf);
  }
}
/*---------------------------------------------------------------------------*/
static void
sent(struct ipolite_conn *ipolite)
{
  struct netflood_conn *c = (struct netflood_conn *)ipolite;
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
dropped(struct ipolite_conn *ipolite)
{
  struct netflood_conn *c = (struct netflood_conn *)ipolite;
  if(c->u->dropped != NULL) {
    c->u->dropped(c);
  }
}
/*---------------------------------------------------------------------------*/
static const struct ipolite_callbacks netflood = {recv_from_ipolite, sent, dropped};
/*---------------------------------------------------------------------------*/
void
netflood_open(struct netflood_conn *c, clock_time_t queue_time,
	uint16_t channel, const struct netflood_callbacks *u)
{
  ipolite_open(&c->c, channel, 1, &netflood);
  c->u = u;
  c->queue_time = queue_time;
}
/*---------------------------------------------------------------------------*/
void
netflood_close(struct netflood_conn *c)
{
  ipolite_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
netflood_send(struct netflood_conn *c, uint8_t seqno)
{
  if(packetbuf_hdralloc(sizeof(struct netflood_hdr))) {
    struct netflood_hdr *hdr = packetbuf_hdrptr();
    linkaddr_copy(&hdr->originator, &linkaddr_node_addr);
    linkaddr_copy(&c->last_originator, &hdr->originator);
    c->last_originator_seqno = hdr->originator_seqno = seqno;
    hdr->hops = 0;
    PRINTF("%d.%d: netflood sending '%s'\n",
	   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	   (char *)packetbuf_dataptr());
    return ipolite_send(&c->c, 0, 4);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
netflood_cancel(struct netflood_conn *c)
{
  ipolite_cancel(&c->c);
}
/*---------------------------------------------------------------------------*/
/** @} */
