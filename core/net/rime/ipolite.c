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
 * $Id: ipolite.c,v 1.4 2008/01/07 14:53:04 adamdunkels Exp $
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
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif /* MAX */

/*---------------------------------------------------------------------------*/
static void
recv(struct ibc_conn *ibc, rimeaddr_t *from)
{
  struct ipolite_conn *c = (struct ipolite_conn *)ibc;
  if(c->q != NULL &&
     rimebuf_datalen() == queuebuf_datalen(c->q) &&
     memcmp(rimebuf_dataptr(), queuebuf_dataptr(c->q),
	    MAX(c->hdrsize, rimebuf_datalen())) == 0) {
    /* We received a copy of our own packet, so we do not send out
       packet. */
    queuebuf_free(c->q);
    c->q = NULL;
    ctimer_stop(&c->t);
    if(c->cb->dropped) {
      c->cb->dropped(c);
    }
  }
  if(c->cb->recv) {
    c->cb->recv(c, from);
  }
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct ipolite_conn *c = ptr;

  if(c->q != NULL) {
    queuebuf_to_rimebuf(c->q);
    queuebuf_free(c->q);
    c->q = NULL;
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
ipolite_open(struct ipolite_conn *c, u16_t channel,
	  const struct ipolite_callbacks *cb)
{
  ibc_open(&c->c, channel, &ibc);
  c->cb = cb;
}
/*---------------------------------------------------------------------------*/
void
ipolite_close(struct ipolite_conn *c)
{
  ibc_close(&c->c);
  ctimer_stop(&c->t);
  if(c->q != NULL) {
    queuebuf_free(c->q);
    c->q = NULL;
  }
}
/*---------------------------------------------------------------------------*/
int
ipolite_send(struct ipolite_conn *c, clock_time_t interval, u8_t hdrsize)
{
  if(c->q != NULL) {
    /* If we are already about to send a packet, we cancel the old one. */
    queuebuf_free(c->q);
  }
  c->hdrsize = hdrsize;
  c->q = queuebuf_new_from_rimebuf();
  if(c->q != NULL) {
    if(interval == 0) {
      ctimer_set(&c->t, 0, send, c);
    } else {
      ctimer_set(&c->t,
		 interval / 2 + (random_rand() % (interval / 2)),
		 send, c);
    }
    return 1;
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
