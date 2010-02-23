/**
 * \addtogroup rimepolite
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
 * $Id: polite.c,v 1.10 2010/02/23 18:38:05 adamdunkels Exp $
 */

/**
 * \file
 *         Polite Anonymous best effort local area BroadCast (polite)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "net/rime/polite.h"
#include "lib/random.h"

#include <string.h>

#ifndef MAX
#define MAX(a,b) ((a) > (b)? (a) : (b))
#endif /* MAX */

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */


/*---------------------------------------------------------------------------*/
static void
recv(struct abc_conn *abc)
{
  struct polite_conn *c = (struct polite_conn *)abc;
  if(c->q != NULL &&
     packetbuf_datalen() == queuebuf_datalen(c->q) &&
     memcmp(packetbuf_dataptr(), queuebuf_dataptr(c->q),
	    MIN(c->hdrsize, packetbuf_datalen())) == 0) {
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
    c->cb->recv(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
sent(struct abc_conn *c, int status, int num_tx)
{

}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct polite_conn *c = ptr;

  if(c->q != NULL) {
    queuebuf_to_packetbuf(c->q);
    queuebuf_free(c->q);
    c->q = NULL;
    abc_send(&c->c);
    if(c->cb->sent) {
	c->cb->sent(c);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct abc_callbacks abc = { recv, sent };
/*---------------------------------------------------------------------------*/
void
polite_open(struct polite_conn *c, uint16_t channel,
	  const struct polite_callbacks *cb)
{
  abc_open(&c->c, channel, &abc);
  c->cb = cb;
}
/*---------------------------------------------------------------------------*/
void
polite_close(struct polite_conn *c)
{
  abc_close(&c->c);
  ctimer_stop(&c->t);
  if(c->q != NULL) {
    queuebuf_free(c->q);
    c->q = NULL;
  }
}
/*---------------------------------------------------------------------------*/
int
polite_send(struct polite_conn *c, clock_time_t interval, uint8_t hdrsize)
{
  if(c->q != NULL) {
    /* If we are already about to send a packet, we cancel the old one. */
    queuebuf_free(c->q);
  }
  c->hdrsize = hdrsize;
  c->q = queuebuf_new_from_packetbuf();
  if(c->q != NULL) {
    ctimer_set(&c->t, interval / 2 + (random_rand() % (interval / 2)), send, c);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
polite_cancel(struct polite_conn *c)
{
  ctimer_stop(&c->t);
  if(c->q != NULL) {
    queuebuf_free(c->q);
    c->q = NULL;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
