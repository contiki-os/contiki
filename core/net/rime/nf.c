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
 * $Id: nf.c,v 1.8 2007/03/22 18:53:38 adamdunkels Exp $
 */

/**
 * \file
 *         Best-effort network flooding (nf)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/nf.h"
#include "net/rime.h"
#include "lib/rand.h"
#include <string.h>

#define QUEUE_TIME CLOCK_SECOND / 4

#define HOPS_MAX 16

struct nf_hdr {
  u8_t hops;
  u8_t originator_seqno;
  rimeaddr_t originator;
};

static u8_t seqno;

static void send(void *ptr);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
static void
set_timer(struct nf_conn *c)
{
  ctimer_set(&c->t, QUEUE_TIME, send, c);
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct nf_conn *c = ptr;

  if(c->packets_received > 0) {
    c->packets_received = 0;
    set_timer(c);
  } else {
    /*  DEBUGF(3, "nf: send()\n");*/
    queuebuf_to_rimebuf(c->buf);
    queuebuf_free(c->buf);
    c->buf = NULL;
    PRINTF("%d.%d: nf send to uibc\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    uibc_send(&c->c, QUEUE_TIME);
    if(c->u->sent != NULL) {
      c->u->sent(c);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
queue_for_send(struct nf_conn *c)
{
  if(c->buf == NULL) {
    c->buf = queuebuf_new_from_rimebuf();
  }
  if(c->buf == NULL) {
    return 0;
  }
  c->packets_received = 0;
  set_timer(c);
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
recv_from_uibc(struct uibc_conn *uibc, rimeaddr_t *from)
{
  register struct nf_conn *c = (struct nf_conn *)uibc;
  struct nf_hdr *hdr = rimebuf_dataptr();
  u8_t hops;
  struct queuebuf *queuebuf;

  c->packets_received++;

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
	    queue_for_send(c);
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
static const struct uibc_callbacks nf = {recv_from_uibc, NULL, NULL};
/*---------------------------------------------------------------------------*/
void
nf_open(struct nf_conn *c, u16_t channel,
	  const struct nf_callbacks *u)
{
  uibc_open(&c->c, channel, &nf);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
void
nf_close(struct nf_conn *c)
{
  uibc_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
nf_send(struct nf_conn *c)
{
  if(c->buf != NULL) {
    queuebuf_free(c->buf);
    c->buf = NULL;
  }

  if(rimebuf_hdrextend(sizeof(struct nf_hdr))) {
    struct nf_hdr *hdr = rimebuf_hdrptr();
    rimeaddr_copy(&hdr->originator, &rimeaddr_node_addr);
    rimeaddr_copy(&c->last_originator, &hdr->originator);
    c->last_originator_seqno = hdr->originator_seqno = ++seqno;
    hdr->hops = 0;
    return queue_for_send(c);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
nf_cancel(struct nf_conn *c)
{
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
