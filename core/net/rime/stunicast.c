/**
 * \addtogroup rimestunicast
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
 * $Id: stunicast.c,v 1.6 2010/04/30 07:29:31 adamdunkels Exp $
 */

/**
 * \file
 *         Stubborn unicast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/stunicast.h"
#include "net/rime.h"
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
recv_from_uc(struct unicast_conn *uc, const rimeaddr_t *from)
{
  register struct stunicast_conn *c = (struct stunicast_conn *)uc;
  PRINTF("%d.%d: stunicast: recv_from_uc from %d.%d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	from->u8[0], from->u8[1]);
  if(c->u->recv != NULL) {
    c->u->recv(c, from);
  }
}
/*---------------------------------------------------------------------------*/
static void
sent_by_uc(struct unicast_conn *uc, int status, int num_tx)
{
  register struct stunicast_conn *c = (struct stunicast_conn *)uc;
  PRINTF("%d.%d: stunicast: recv_from_uc from %d.%d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
         packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],
         packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
  if(c->u->sent != NULL) {
    c->u->sent(c, status, num_tx);
  }
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks stunicast = {recv_from_uc,
                                                   sent_by_uc};
/*---------------------------------------------------------------------------*/
void
stunicast_open(struct stunicast_conn *c, uint16_t channel,
	  const struct stunicast_callbacks *u)
{
  unicast_open(&c->c, channel, &stunicast);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
void
stunicast_close(struct stunicast_conn *c)
{
  unicast_close(&c->c);
  stunicast_cancel(c);
}
/*---------------------------------------------------------------------------*/
rimeaddr_t *
stunicast_receiver(struct stunicast_conn *c)
{
  return &c->receiver;
}
/*---------------------------------------------------------------------------*/
static void
send(void *ptr)
{
  struct stunicast_conn *c = ptr;

  PRINTF("%d.%d: stunicast: resend to %d.%d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 c->receiver.u8[0], c->receiver.u8[1]);
	 if(c->buf) {
  	queuebuf_to_packetbuf(c->buf);
  	unicast_send(&c->c, &c->receiver);
  	stunicast_set_timer(c, CLOCK_SECOND);
  }
  /*  if(c->u->sent != NULL) {
    c->u->sent(c);
    }*/
}
/*---------------------------------------------------------------------------*/
void
stunicast_set_timer(struct stunicast_conn *c, clock_time_t t)
{
  ctimer_set(&c->t, t, send, c);
}
/*---------------------------------------------------------------------------*/
int
stunicast_send_stubborn(struct stunicast_conn *c, const rimeaddr_t *receiver,
		  clock_time_t rxmittime)
{
  if(c->buf != NULL) {
    queuebuf_free(c->buf);
  }
  c->buf = queuebuf_new_from_packetbuf();
  if(c->buf == NULL) {
    return 0;
  }
  rimeaddr_copy(&c->receiver, receiver);
  ctimer_set(&c->t, rxmittime, send, c);

  PRINTF("%d.%d: stunicast_send_stubborn to %d.%d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 c->receiver.u8[0],c->receiver.u8[1]);
  unicast_send(&c->c, &c->receiver);
  /*  if(c->u->sent != NULL) {
    c->u->sent(c);
    }*/
  
  return 1;
  
}
/*---------------------------------------------------------------------------*/
int
stunicast_send(struct stunicast_conn *c, const rimeaddr_t *receiver)
{
  PRINTF("%d.%d: stunicast_send to %d.%d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 receiver->u8[0], receiver->u8[1]);
  return unicast_send(&c->c, receiver);
}
/*---------------------------------------------------------------------------*/
void
stunicast_cancel(struct stunicast_conn *c)
{
  ctimer_stop(&c->t);
  if(c->buf != NULL) {
    queuebuf_free(c->buf);
    c->buf = NULL;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
