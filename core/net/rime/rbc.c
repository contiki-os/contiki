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
 * $Id: rbc.c,v 1.2 2007/03/19 19:24:37 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/rbc.h"

struct rbc_hdr {
  u16_t seqno;
  u16_t ackno;
};

PROCESS(rbc_process, "Reliable local broadcast");
PROCESS_THREAD(rbc_process, ev, data)
{
  struct rbc_conn *c = (struct rbc_conn *)data;
  u8_t *packet;
  u16_t packetlen;
    
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_COM &&
		       llbc_received(&c->c, &packet, &packetlen));
    printf("rbc: received %p %d\n", packet, packetlen);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
rbc_open(struct rbc_conn *c, u16_t id)
{
  PROCESS_CONTEXT_BEGIN(&rbc_process);
  llbc_open(&c->c, id);
  PROCESS_CONTEXT_END(&rbc_process);
}
/*---------------------------------------------------------------------------*/
static int
send(struct rbc_conn *c, struct lcpbuf *buf)
{
  int r;
  PROCESS_CONTEXT_BEGIN(&rbc_process);
  r = llbc_send(&c->c, buf);
  PROCESS_CONTEXT_END(&rbc_process);
  return r;
}
/*---------------------------------------------------------------------------*/
static int
all_acked(struct rbc_conn *c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
rbc_send(struct rbc_conn *c, struct lcpbuf *buf)
{
  PT_BEGIN(&c->pt);
  
  send(c, buf);

  PT_WAIT_UNTIL(&c->pt, all_acked(c));

  PT_END(&c->pt);
}
/*---------------------------------------------------------------------------*/
int
rbc_received(struct rbc_conn *c, u8_t **data, u16_t *datalen)
{
  return llbc_received(c, data, datalen);
}
/*---------------------------------------------------------------------------*/
