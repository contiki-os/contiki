/**
 * \addtogroup rime_uibc
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
 * $Id: uibc.c,v 1.3 2007/03/25 12:03:12 adamdunkels Exp $
 */

/**
 * \file
 *         Unique Identified best effort local area BroadCast (uibc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "net/rime/uibc.h"
#include "lib/rand.h"

#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
recv(struct ibc_conn *ibc, rimeaddr_t *from)
{
  struct uibc_conn *c = (struct uibc_conn *)ibc;

  if(c->q != NULL) {
    PRINTF("%d.%d: recv queued packet, rimebuf len %d queuebuf %d, memcmp %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   rimebuf_datalen(), queuebuf_datalen(c->q),
	   memcmp(rimebuf_dataptr(), queuebuf_dataptr(c->q),
		  rimebuf_datalen()));
    
    if(rimebuf_datalen() == queuebuf_datalen(c->q) &&
       memcmp(rimebuf_dataptr(), queuebuf_dataptr(c->q),
	      rimebuf_datalen()) == 0) {
      /* We received a copy of our own packet, so we do not send out
	 packet. */
      queuebuf_free(c->q);
      c->q = NULL;
      ctimer_stop(&c->t);
      if(c->cb->dropped) {
	c->cb->dropped(c);
      }
    }
  }
  if(c->cb->recv) {
    PRINTF("%d.%d: uibc calling recv\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    c->cb->recv(c, from);
  }
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct uibc_conn *c = ptr;

  if(c->q != NULL) {
    queuebuf_to_rimebuf(c->q);
    queuebuf_free(c->q);
    c->q = NULL;
    PRINTF("%d.%d: uibc sending with ibc_send\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    ibc_send(&c->c);
    if(c->cb->sent) {
	c->cb->sent(c);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct ibc_callbacks ibc = { recv };
/*---------------------------------------------------------------------------*/
void
uibc_open(struct uibc_conn *c, u16_t channel,
	  const struct uibc_callbacks *cb)
{
  ibc_open(&c->c, channel, &ibc);
  c->cb = cb;
}
/*---------------------------------------------------------------------------*/
void
uibc_close(struct uibc_conn *c)
{
  ibc_close(&c->c);
  ctimer_stop(&c->t);
  if(c->q != NULL) {
    queuebuf_free(c->q);
  }
}
/*---------------------------------------------------------------------------*/
int
uibc_send(struct uibc_conn *c, clock_time_t interval)
{
  if(c->q != NULL) {
    /* If we are already about to send a packet, we cancel the old one. */
    queuebuf_free(c->q);
  }
  c->q = queuebuf_new_from_rimebuf();
  if(c->q != NULL) {
    ctimer_set(&c->t, interval / 2 + (rand() % (interval / 2)), send, c);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
