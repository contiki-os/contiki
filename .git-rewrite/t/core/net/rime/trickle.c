/**
 * \addtogroup trickle
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
 * $Id: trickle.c,v 1.6 2008/01/08 07:59:51 adamdunkels Exp $
 */

/**
 * \file
 *         Trickle (reliable single source flooding) for Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/trickle.h"

#define INTERVAL_MIN 1
#define INTERVAL_MAX 4

#define SEQNO_LT(a, b) ((signed char)((a) - (b)) < 0)

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct trickle_conn *c = ptr;

  if(c->q != NULL) {
    queuebuf_to_rimebuf(c->q);
    nf_send(&c->c, c->seqno);
    ctimer_set(&c->t, c->interval << c->interval_scaling,
	       send, c);
  } else {
    PRINTF("%d.%d: trickle send but c->q == NULL\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  }
}
/*---------------------------------------------------------------------------*/
static int
recv(struct nf_conn *nf, rimeaddr_t *from,
     rimeaddr_t *originator, u8_t seqno, u8_t hops)
{
  struct trickle_conn *c = (struct trickle_conn *)nf;

  PRINTF("%d.%d: trickle recv seqno %d our %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 seqno, c->seqno);

  if(seqno == c->seqno) {
    /*    c->cb->recv(c);*/
  } else if(SEQNO_LT(seqno, c->seqno)) {
    c->interval_scaling = 0;
    send(c);
  } else { /* hdr->seqno > c->seqno */
    c->seqno = seqno;
    /* Store the incoming data in the queuebuf */
    if(c->q != NULL) {
      queuebuf_free(c->q);
    }
    c->q = queuebuf_new_from_rimebuf();
    c->interval_scaling = 0;
    send(c);
    c->cb->recv(c);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
sent_or_dropped(struct nf_conn *nf)
{
  struct trickle_conn *c = (struct trickle_conn *)nf;
  
  c->interval_scaling++;
  if(c->interval_scaling > INTERVAL_MAX) {
    c->interval_scaling = INTERVAL_MAX;
  }
}
/*---------------------------------------------------------------------------*/
static const struct nf_callbacks nf = {recv,
				       sent_or_dropped,
				       sent_or_dropped};
/*---------------------------------------------------------------------------*/
void
trickle_open(struct trickle_conn *c, clock_time_t interval,
	     u16_t channel, const struct trickle_callbacks *cb)
{
  nf_open(&c->c, interval, channel, &nf);
  c->cb = cb;
  c->q = NULL;
  c->interval = interval;
}
/*---------------------------------------------------------------------------*/
void
trickle_close(struct trickle_conn *c)
{
  nf_close(&c->c);
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
void
trickle_send(struct trickle_conn *c)
{
  if(c->q != NULL) {
    queuebuf_free(c->q);
  }
  c->q = queuebuf_new_from_rimebuf();
  c->seqno++;
  PRINTF("%d.%d: trickle send seqno %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 c->seqno);
  send(c);
}
/*---------------------------------------------------------------------------*/
/** @} */
