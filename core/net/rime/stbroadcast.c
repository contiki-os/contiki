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
 */

/**
 * \file
 *         Implementation of the Rime module Stubborn Anonymous
 *         BroadCast (stbroadcast)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rimestbroadcast
 * @{
 */
    
#include "net/rime/stbroadcast.h"
#include "net/rime/rime.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
static void
recv_from_broadcast(struct broadcast_conn *broadcast, const linkaddr_t *sender)
{
  register struct stbroadcast_conn *c = (struct stbroadcast_conn *)broadcast;
  /*  DEBUGF(3, "stbroadcast: recv_from_broadcast from %d\n", from_id);*/
  if(c->u->recv != NULL) {
    c->u->recv(c);
  }
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks stbroadcast = {recv_from_broadcast};
/*---------------------------------------------------------------------------*/
void
stbroadcast_open(struct stbroadcast_conn *c, uint16_t channel,
	  const struct stbroadcast_callbacks *u)
{
  broadcast_open(&c->c, channel, &stbroadcast);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
void
stbroadcast_close(struct stbroadcast_conn *c)
{
  broadcast_close(&c->c);
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct stbroadcast_conn *c = ptr;

  /*  DEBUGF(3, "stbroadcast: send()\n");*/
  queuebuf_to_packetbuf(c->buf);
  broadcast_send(&c->c);
  ctimer_reset(&c->t);
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
}
/*---------------------------------------------------------------------------*/
void
stbroadcast_set_timer(struct stbroadcast_conn *c, clock_time_t t)
{
  ctimer_set(&c->t, t, send, c);
}
/*---------------------------------------------------------------------------*/
int
stbroadcast_send_stubborn(struct stbroadcast_conn *c, clock_time_t t)
{
  if(c->buf != NULL) {
    queuebuf_free(c->buf);
  }
  c->buf = queuebuf_new_from_packetbuf();
  if(c->buf == NULL) {
    return 0;
  }
  send(c);
  stbroadcast_set_timer(c, t);
  return 1;
  
}
/*---------------------------------------------------------------------------*/
void
stbroadcast_cancel(struct stbroadcast_conn *c)
{
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
/** @} */
