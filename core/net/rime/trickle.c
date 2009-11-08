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
 * $Id: trickle.c,v 1.18 2009/11/08 19:40:18 adamdunkels Exp $
 */

/**
 * \file
 *         Trickle (reliable single source flooding) for Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/trickle.h"
#include "lib/random.h"

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif

#define INTERVAL_MIN 1
#define INTERVAL_MAX 4

#define DUPLICATE_THRESHOLD 1

#define SEQNO_LT(a, b) ((signed char)((a) - (b)) < 0)

static const struct packetbuf_attrlist attributes[] =
  {
    TRICKLE_ATTRIBUTES PACKETBUF_ATTR_LAST
  };


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static int run_trickle(struct trickle_conn *c);
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct trickle_conn *c = ptr;

  if(c->q != NULL) {
    queuebuf_to_packetbuf(c->q);
    broadcast_send(&c->c);
  } else {
    PRINTF("%d.%d: trickle send but c->q == NULL\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  }
}
/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  struct trickle_conn *c = ptr;
  run_trickle(c);
}
/*---------------------------------------------------------------------------*/
static void
reset_interval(struct trickle_conn *c)
{
  PT_INIT(&c->pt);
  run_trickle(c);
}
/*---------------------------------------------------------------------------*/
static void
set_timer(struct trickle_conn *c, struct ctimer *t, clock_time_t i)
{
  ctimer_set(t, i, timer_callback, c);
}
/*---------------------------------------------------------------------------*/
static int
run_trickle(struct trickle_conn *c)
{
  clock_time_t interval;
  PT_BEGIN(&c->pt);

  while(1) {
    interval = c->interval << c->interval_scaling;
    set_timer(c, &c->interval_timer, interval);
    set_timer(c, &c->t, interval / 2 + (random_rand() % (interval / 2)));

    c->duplicates = 0;
    PT_YIELD(&c->pt); /* Wait until listen timeout */
    if(c->duplicates < DUPLICATE_THRESHOLD) {
      send(c);
    }
    PT_YIELD(&c->pt); /* Wait until interval timer expired. */
    if(c->interval_scaling < INTERVAL_MAX) {
      c->interval_scaling++;
    }
  }
  
  PT_END(&c->pt);
}
/*---------------------------------------------------------------------------*/
static void
recv(struct broadcast_conn *bc, const rimeaddr_t *from)
{
  struct trickle_conn *c = (struct trickle_conn *)bc;
  uint16_t seqno = packetbuf_attr(PACKETBUF_ATTR_EPACKET_ID);

  PRINTF("%d.%d: trickle recv seqno %d from %d.%d our %d data len %d channel %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 seqno,
	 from->u8[0], from->u8[1],
	 c->seqno,
	 packetbuf_datalen(),
	 packetbuf_attr(PACKETBUF_ATTR_CHANNEL));

  if(seqno == c->seqno) {
    /*    c->cb->recv(c);*/
    ++c->duplicates;
  } else if(SEQNO_LT(seqno, c->seqno)) {
    c->interval_scaling = 0;
    send(c);
  } else { /* hdr->seqno > c->seqno */
#if CONTIKI_TARGET_NETSIM
    /*    ether_set_line(from->u8[0], from->u8[1]);*/
#endif /* CONTIKI_TARGET_NETSIM */
    c->seqno = seqno;
    /* Store the incoming data in the queuebuf */
    if(c->q != NULL) {
      queuebuf_free(c->q);
    }
    c->q = queuebuf_new_from_packetbuf();
    c->interval_scaling = 0;
    reset_interval(c);
    ctimer_set(&c->first_transmission_timer, random_rand() % c->interval,
	       send, c);
    c->cb->recv(c);
  }
}
/*---------------------------------------------------------------------------*/
static CC_CONST_FUNCTION struct broadcast_callbacks bc = { recv };
/*---------------------------------------------------------------------------*/
void
trickle_open(struct trickle_conn *c, clock_time_t interval,
	     uint16_t channel, const struct trickle_callbacks *cb)
{
  broadcast_open(&c->c, channel, &bc);
  c->cb = cb;
  c->q = NULL;
  c->interval = interval;
  c->interval_scaling = 0;
  channel_set_attributes(channel, attributes);
}
/*---------------------------------------------------------------------------*/
void
trickle_close(struct trickle_conn *c)
{
  broadcast_close(&c->c);
  ctimer_stop(&c->t);
  ctimer_stop(&c->interval_timer);
}
/*---------------------------------------------------------------------------*/
void
trickle_send(struct trickle_conn *c)
{
  if(c->q != NULL) {
    queuebuf_free(c->q);
  }
  c->seqno++;
  packetbuf_set_attr(PACKETBUF_ATTR_EPACKET_ID, c->seqno);
  c->q = queuebuf_new_from_packetbuf();
  PRINTF("%d.%d: trickle send seqno %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 c->seqno);
  reset_interval(c);
  send(c);
}
/*---------------------------------------------------------------------------*/
/** @} */
