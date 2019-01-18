/**
 * \addtogroup rimesibc
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
 * $Id: sibc.c,v 1.6 2007/03/31 18:31:28 adamdunkels Exp $
 */

/**
 * \file
 *         Implementation of the Rime module Stubborn Identified BroadCast (sibc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/sibc.h"
#include "net/rime.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
static void
recv_from_ibc(struct ibc_conn *ibc, rimeaddr_t *from)
{
  register struct sibc_conn *c = (struct sibc_conn *)ibc;
  /*  DEBUGF(3, "sibc: recv_from_ibc from %d\n", from_id);*/
  if(c->u->recv != NULL) {
    c->u->recv(c, from);
  }
}
/*---------------------------------------------------------------------------*/
static const struct ibc_callbacks sibc = {recv_from_ibc};
/*---------------------------------------------------------------------------*/
void
sibc_open(struct sibc_conn *c, u16_t channel,
	  const struct sibc_callbacks *u)
{
  ibc_open(&c->c, channel, &sibc);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
void
sibc_close(struct sibc_conn *c)
{
  ibc_close(&c->c);
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct sibc_conn *c = ptr;

  /*  DEBUGF(3, "sibc: send()\n");*/
  queuebuf_to_rimebuf(c->buf);
  ibc_send(&c->c);
  ctimer_reset(&c->t);
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
}
/*---------------------------------------------------------------------------*/
void
sibc_set_timer(struct sibc_conn *c, clock_time_t t)
{
  ctimer_set(&c->t, t, send, c);
}
/*---------------------------------------------------------------------------*/
int
sibc_send_stubborn(struct sibc_conn *c, clock_time_t t)
{
  if(c->buf != NULL) {
    queuebuf_free(c->buf);
  }
  c->buf = queuebuf_new_from_rimebuf();
  if(c->buf == NULL) {
    return 0;
  }
  ctimer_set(&c->t, t, send, c);
  return 1;
  
}
/*---------------------------------------------------------------------------*/
void
sibc_cancel(struct sibc_conn *c)
{
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
/** @} */
