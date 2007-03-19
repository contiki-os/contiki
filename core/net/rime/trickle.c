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
 * $Id: trickle.c,v 1.1 2007/03/19 23:26:18 adamdunkels Exp $
 */

/**
 * \file
 *         Trickle (reliable single source flooding) for Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/trickle.h"

struct trickle_hdr {
  u8_t seqno;
  u8_t pad;
};

#define K 1

#define DEFAULT_INTERVAL (CLOCK_SECOND / 2)
#define INTERVAL_MIN 1
#define INTERVAL_MAX 10

#define SEQNO_LT(a, b) ((signed char)((a) - (b)) < 0)

static int trickle_pt(struct trickle_conn *c);

/*---------------------------------------------------------------------------*/
static void
send(struct trickle_conn *c)
{
  struct trickle_hdr *hdr;

  if(c->q != NULL) {
    queuebuf_to_rimebuf(c->q);
    rimebuf_hdrextend(sizeof(struct trickle_hdr));
    hdr = rimebuf_hdrptr();
    hdr->seqno = c->seqno;
    abc_send(&c->c);
  }
}
/*---------------------------------------------------------------------------*/
static void
set_intervaltimer(struct trickle_conn *c)
{
  ctimer_set(&c->intervaltimer, DEFAULT_INTERVAL << c->interval,
	     (void (*)(void *))trickle_pt, c);
}
/*---------------------------------------------------------------------------*/
static void
set_listentimer(struct trickle_conn *c)
{
  ctimer_set(&c->timer, (DEFAULT_INTERVAL << c->interval) / 2,
	     (void (*)(void *))trickle_pt, c);
}
/*---------------------------------------------------------------------------*/
static void
set_transmittimer(struct trickle_conn *c)
{
  clock_time_t tval = (DEFAULT_INTERVAL << c->interval) / 2;

  ctimer_set(&c->timer, random_rand() & tval,
	     (void (*)(void *))trickle_pt, c);
}
/*---------------------------------------------------------------------------*/
static void
reset_interval(struct trickle_conn *c)
{
  PT_INIT(&c->pt);
  trickle_pt(c);
}
/*---------------------------------------------------------------------------*/
static void
recv(struct abc_conn *abc)
{
  struct trickle_conn *c = (struct trickle_conn *)abc;
  struct trickle_hdr *hdr = rimebuf_dataptr();

  rimebuf_hdrreduce(sizeof(struct trickle_hdr));

  if(hdr->seqno == c->seqno) {
    c->count++;
  } else if(SEQNO_LT(hdr->seqno, c->seqno)) {
    send(c);
  } else { /* hdr->seqno > c->seqno */
    c->seqno = hdr->seqno;
    /* Store the incoming data in the queuebuf */
    if(c->q != NULL) {
      queuebuf_free(c->q);
    }
    c->q = queuebuf_new_from_rimebuf();
    reset_interval(c);

    c->cb->recv(c);
  }
}
/*---------------------------------------------------------------------------*/
static int
trickle_pt(struct trickle_conn *c)
{
  PT_BEGIN(&c->pt);

  c->interval = INTERVAL_MIN;

  while(1) {
    c->count = 0;
    set_intervaltimer(c);
    set_listentimer(c);
    PT_YIELD(&c->pt); /* Wait for listen timer to expire. */
    set_transmittimer(c);
    PT_YIELD(&c->pt); /* Wait for transmit timer to expire. */
    if(c->count < K) {
      send(c);
    }
    PT_YIELD(&c->pt); /* Wait for interval timer to expire. */
    c->interval++;
    if(c->interval > INTERVAL_MAX) {
      c->interval = INTERVAL_MAX;
    }
  }
  PT_END(&c->pt);
}
/*---------------------------------------------------------------------------*/
static const struct abc_callbacks abc = {recv};
/*---------------------------------------------------------------------------*/
void
trickle_open(struct trickle_conn *c, u16_t channel,
	     const struct trickle_callbacks *cb)
{
  abc_open(&c->c, channel, &abc);
  c->cb = cb;
  reset_interval(c);
  c->q = NULL;
  c->count = 0;
}
/*---------------------------------------------------------------------------*/
void
trickle_close(struct trickle_conn *c)
{
  abc_close(&c->c);
  ctimer_stop(&c->intervaltimer);
  ctimer_stop(&c->timer);
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
  reset_interval(c);
}
/*---------------------------------------------------------------------------*/
