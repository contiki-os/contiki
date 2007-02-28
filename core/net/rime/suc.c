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
 * $Id: suc.c,v 1.1 2007/02/28 16:38:52 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/suc.h"
#include "net/rime.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
static void
recv_from_uc(struct uc_conn *uc, node_id_t from_id)
{
  register struct suc_conn *c = (struct suc_conn *)uc;
  DEBUGF(3, "%d: suc: recv_from_uc from %d %p\n", node_id, from_id, c);
  if(c->u->recv != NULL) {
    c->u->recv(c, from_id);
  }
}
/*---------------------------------------------------------------------------*/
static const struct uc_ulayer suc = {recv_from_uc};
/*---------------------------------------------------------------------------*/
void
suc_setup(struct suc_conn *c, u16_t channel,
	  const struct suc_ulayer *u)
{
  uc_setup(&c->c, channel, &suc);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct suc_conn *c = ptr;

  DEBUGF(3, "%d: suc: resend to %d\n", node_id, c->receiver_id);
  queuebuf_to_rimebuf(c->buf);
  uc_send(&c->c, c->receiver_id);
  suc_set_timer(c, CLOCK_SECOND);
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
}
/*---------------------------------------------------------------------------*/
void
suc_set_timer(struct suc_conn *c, clock_time_t t)
{
  ctimer_set(&c->t, t, send, c);
}
/*---------------------------------------------------------------------------*/
int
suc_send_stubborn(struct suc_conn *c, node_id_t receiver_id)
{
  if(c->buf != NULL) {
    queuebuf_free(c->buf);
  }
  c->buf = queuebuf_new_from_rimebuf();
  if(c->buf == NULL) {
    return 0;
  }
  c->receiver_id = receiver_id;
  ctimer_set(&c->t, CLOCK_SECOND, send, c);

  DEBUGF(3, "%d: suc_send_stubborn to %d\n", node_id, c->receiver_id);
  uc_send(&c->c, c->receiver_id);
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
  
  return 1;
  
}
/*---------------------------------------------------------------------------*/
int
suc_send_uc(struct suc_conn *c, node_id_t receiver_id)
{
  return uc_send(&c->c, receiver_id);
}
/*---------------------------------------------------------------------------*/
void
suc_cancel(struct suc_conn *c)
{
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
