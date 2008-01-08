/**
 * \addtogroup rimenf
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
 * $Id: nf.c,v 1.15 2008/01/08 08:26:48 adamdunkels Exp $
 */

/**
 * \file
 *         Best-effort network flooding (nf)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/nf.h"
#include "lib/rand.h"
#include <string.h>

#define HOPS_MAX 16

struct nf_hdr {
  u16_t originator_seqno;
  rimeaddr_t originator;
  u16_t hops;
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
send(struct nf_conn *c)
{
  PRINTF("%d.%d: nf send to ipolite\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  return ipolite_send(&c->c, c->queue_time, 4);
}
/*---------------------------------------------------------------------------*/
static void
recv_from_ipolite(struct ipolite_conn *ipolite, rimeaddr_t *from)
{
  struct nf_conn *c = (struct nf_conn *)ipolite;
  struct nf_hdr *hdr = rimebuf_dataptr();
  u8_t hops;
  struct queuebuf *queuebuf;

  hops = hdr->hops;

  /* Remember packet if we need to forward it. */
  queuebuf = queuebuf_new_from_rimebuf();

  rimebuf_hdrreduce(sizeof(struct nf_hdr));
  if(c->u->recv != NULL) {
    if(!(rimeaddr_cmp(&hdr->originator, &c->last_originator) &&
	 hdr->originator_seqno <= c->last_originator_seqno)) {

      if(c->u->recv(c, from, &hdr->originator, hdr->originator_seqno,
		    hops)) {
	
	if(queuebuf != NULL) {
	  queuebuf_to_rimebuf(queuebuf);
	  queuebuf_free(queuebuf);
	  queuebuf = NULL;
	  hdr = rimebuf_dataptr();
	  
	  /* Rebroadcast received packet. */
	  if(hops < HOPS_MAX) {
	    PRINTF("%d.%d: nf rebroadcasting %d.%d/%d (%d.%d/%d) hops %d\n",
		   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		   hdr->originator.u8[0], hdr->originator.u8[1],
		   hdr->originator_seqno,
		   c->last_originator.u8[0], c->last_originator.u8[1],
		   c->last_originator_seqno,
		  hops);
	    hdr->hops++;
	    send(c);
	    rimeaddr_copy(&c->last_originator, &hdr->originator);
	    c->last_originator_seqno = hdr->originator_seqno;
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
  struct nf_conn *c = (struct nf_conn *)ipolite;
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
dropped(struct ipolite_conn *ipolite)
{
  struct nf_conn *c = (struct nf_conn *)ipolite;
  if(c->u->dropped != NULL) {
    c->u->dropped(c);
  }
}
/*---------------------------------------------------------------------------*/
static const struct ipolite_callbacks nf = {recv_from_ipolite, sent, dropped};
/*---------------------------------------------------------------------------*/
void
nf_open(struct nf_conn *c, clock_time_t queue_time,
	u16_t channel, const struct nf_callbacks *u)
{
  ipolite_open(&c->c, channel, &nf);
  c->u = u;
  c->queue_time = queue_time;
}
/*---------------------------------------------------------------------------*/
void
nf_close(struct nf_conn *c)
{
  ipolite_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
nf_send(struct nf_conn *c, u8_t seqno)
{
  if(rimebuf_hdralloc(sizeof(struct nf_hdr))) {
    struct nf_hdr *hdr = rimebuf_hdrptr();
    rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
    rimeaddr_copy(&c->last_originator, &hdr->originator);
    c->last_originator_seqno = hdr->originator_seqno = seqno;
    hdr->hops = 0;
    PRINTF("%d.%d: nf sending '%s'\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   (char *)rimebuf_dataptr());
    return ipolite_send(&c->c, 0, 4);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
nf_cancel(struct nf_conn *c)
{
  ipolite_cancel(&c->c);
}
/*---------------------------------------------------------------------------*/
/** @} */
