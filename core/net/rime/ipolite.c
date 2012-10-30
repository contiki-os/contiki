/**
 * \addtogroup rimeipolite
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
 */

/**
 * \file
 *         Ipolite Anonymous best effort local area BroadCast (ipolite)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "net/rime/ipolite.h"
#include "lib/random.h"

#include <string.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b)? (a) : (b))
#endif /* MAX */

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
recv(struct broadcast_conn *broadcast, const rimeaddr_t *from)
{
  struct ipolite_conn *c = (struct ipolite_conn *)broadcast;
  if(c->q != NULL &&
     packetbuf_datalen() == queuebuf_datalen(c->q) &&
     memcmp(packetbuf_dataptr(), queuebuf_dataptr(c->q),
	    MIN(c->hdrsize, packetbuf_datalen())) == 0) {
    /* We received a copy of our own packet, so we increase the
       duplicate counter. If it reaches its maximum, do not send out
       our packet. */
    c->dups++;
    if(c->dups == c->maxdups) {
      queuebuf_free(c->q);
      c->q = NULL;
      ctimer_stop(&c->t);
      if(c->cb->dropped) {
        c->cb->dropped(c);
      }
    }
  }
  if(c->cb->recv) {
    c->cb->recv(c, from);
  }
}
/*---------------------------------------------------------------------------*/
static void
sent(struct broadcast_conn *bc, int status, int num_tx)
{

}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct ipolite_conn *c = ptr;
  
  PRINTF("%d.%d: ipolite: send queuebuf %p\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 c->q);
  
  if(c->q != NULL) {
    queuebuf_to_packetbuf(c->q);
    queuebuf_free(c->q);
    c->q = NULL;
    broadcast_send(&c->c);
    if(c->cb->sent) {
      c->cb->sent(c);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast = { recv, sent };
/*---------------------------------------------------------------------------*/
void
ipolite_open(struct ipolite_conn *c, uint16_t channel, uint8_t dups,
	  const struct ipolite_callbacks *cb)
{
  broadcast_open(&c->c, channel, &broadcast);
  c->cb = cb;
  c->maxdups = dups;
  PRINTF("ipolite open channel %d\n", channel);
}
/*---------------------------------------------------------------------------*/
void
ipolite_close(struct ipolite_conn *c)
{
  broadcast_close(&c->c);
  ctimer_stop(&c->t);
  if(c->q != NULL) {
    queuebuf_free(c->q);
    c->q = NULL;
  }
}
/*---------------------------------------------------------------------------*/
int
ipolite_send(struct ipolite_conn *c, clock_time_t interval, uint8_t hdrsize)
{
  if(c->q != NULL) {
    /* If we are already about to send a packet, we cancel the old one. */
    PRINTF("%d.%d: ipolite_send: cancel old send\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    queuebuf_free(c->q);
  }
  c->dups = 0;
  c->hdrsize = hdrsize;
  if(interval == 0) {
    PRINTF("%d.%d: ipolite_send: interval 0\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    if(broadcast_send(&c->c)) {
      if(c->cb->sent) {
	c->cb->sent(c);
      }
      return 1;
    }

  } else {
    c->q = queuebuf_new_from_packetbuf();
    if(c->q != NULL) {
      ctimer_set(&c->t,
		 interval / 2 + (random_rand() % (interval / 2)),
		 send, c);
      return 1;
    }
    PRINTF("%d.%d: ipolite_send: could not allocate queue buffer\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
ipolite_cancel(struct ipolite_conn *c)
{
  ctimer_stop(&c->t);
  if(c->q != NULL) {
    queuebuf_free(c->q);
    c->q = NULL;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
