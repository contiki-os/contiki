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
 * $Id: ccsabc.c,v 1.3 2007/03/19 19:24:37 adamdunkels Exp $
 */

/**
 * \file
 *         Congestion-Controlled Stubborn Anonymous BroadCast (ccsabc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/ccsabc.h"

#define STATE_LISTENPERIOD 0
#define STATE_SENDINGPERIOD 1

/*---------------------------------------------------------------------------*/
static void
recv_from_sabc(struct sabc_conn *sabc)
{
  struct ccsabc_conn *c = (struct ccsabc_conn *)sabc;
  if(c->u->recv != NULL) {
    c->u->recv(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
sent_from_sabc(struct sabc_conn *sabc)
{
  struct ccsabc_conn *c = (struct ccsabc_conn *)sabc;
  if(c->u->sent != NULL) {
    c->u->sent(c);
  }
}
/*---------------------------------------------------------------------------*/
static const struct sabc_callbacks ccsabc = {recv_from_sabc, sent_from_sabc};
/*---------------------------------------------------------------------------*/
void
ccsabc_open(struct ccsabc_conn *c, u16_t channel,
	     const struct ccsabc_callbacks *u)
{
  sabc_open(&c->c, channel, &ccsabc);
}
/*---------------------------------------------------------------------------*/
int
ccsabc_send_stubborn(struct ccsabc_conn *c, clock_time_t t)
{
  return sabc_send_stubborn(&c->c, t);
}
/*---------------------------------------------------------------------------*/
void
ccsabc_cancel(struct ccsabc_conn *c)
{
  sabc_cancel(&c->c);
}
/*---------------------------------------------------------------------------*/
void
ccsabc_set_timer(struct ccsabc_conn *c, clock_time_t t)
{
  sabc_set_timer(&c->c, t);
}
/*---------------------------------------------------------------------------*/
