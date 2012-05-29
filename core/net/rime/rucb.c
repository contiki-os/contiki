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
 * $Id: rucb.c,v 1.11 2009/11/08 19:40:18 adamdunkels Exp $
 */

/**
 * \file
 *         Reliable unicast bulk transfer
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/rucb.h"
#include "net/rime.h"
#include <string.h>

#define MAX_TRANSMISSIONS 8

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "sys/timetable.h"
/*---------------------------------------------------------------------------*/
static int
read_data(struct rucb_conn *c)
{
  int len = 0;
  packetbuf_clear();
  if(c->u->read_chunk) {
    len = c->u->read_chunk(c, c->chunk * RUCB_DATASIZE,
			    packetbuf_dataptr(), RUCB_DATASIZE);
  }
  packetbuf_set_datalen(len);
  return len;
}
/*---------------------------------------------------------------------------*/
static void
acked(struct runicast_conn *ruc, const rimeaddr_t *to, uint8_t retransmissions)
{
  struct rucb_conn *c = (struct rucb_conn *)ruc;
  int len;
  PRINTF("%d.%d: rucb acked\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
  c->chunk++;
  len = read_data(c);
  if(len == 0 && c->last_size == 0) {
    /* Nothing more to do */
    return;
  }

  if(len >= 0) {
    runicast_send(&c->c, &c->receiver, MAX_TRANSMISSIONS);
    c->last_size = len;

    /*    {
      extern struct timetable cc2420_timetable;
      timetable_print(&cc2420_timetable);
      }*/
  }
}
/*---------------------------------------------------------------------------*/
static void
timedout(struct runicast_conn *ruc, const rimeaddr_t *to, uint8_t retransmissions)
{
  struct rucb_conn *c = (struct rucb_conn *)ruc;
  PRINTF("%d.%d: rucb timedout\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
  if(c->u->timedout) {
    c->u->timedout(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv(struct runicast_conn *ruc, const rimeaddr_t *from, uint8_t seqno)
{
  struct rucb_conn *c = (struct rucb_conn *)ruc;

  PRINTF("%d.%d: rucb: recv from %d.%d len %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], packetbuf_totlen());

  if(seqno == c->last_seqno) {
    return;
  }
  c->last_seqno = seqno;

  if(rimeaddr_cmp(&c->sender, &rimeaddr_null)) {
    rimeaddr_copy(&c->sender, from);
    c->u->write_chunk(c, 0, RUCB_FLAG_NEWFILE, packetbuf_dataptr(), 0);
    c->chunk = 0;
  }


  if(rimeaddr_cmp(&c->sender, from)) {
    int datalen = packetbuf_datalen();

    if(datalen < RUCB_DATASIZE) {
      PRINTF("%d.%d: get %d bytes, file complete\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     datalen);
      c->u->write_chunk(c, c->chunk * RUCB_DATASIZE,
			 RUCB_FLAG_LASTCHUNK, packetbuf_dataptr(), datalen);
    } else {
      c->u->write_chunk(c, c->chunk * RUCB_DATASIZE,
			RUCB_FLAG_NONE, packetbuf_dataptr(), datalen);
    }
    c->chunk++;
  }

  if(packetbuf_datalen() < RUCB_DATASIZE) {
    rimeaddr_copy(&c->sender, &rimeaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
static const struct runicast_callbacks ruc = {recv, acked, timedout};
/*---------------------------------------------------------------------------*/
void
rucb_open(struct rucb_conn *c, uint16_t channel,
	  const struct rucb_callbacks *u)
{
  rimeaddr_copy(&c->sender, &rimeaddr_null);
  runicast_open(&c->c, channel, &ruc);
  c->u = u;
  c->last_seqno = -1;
  c->last_size = -1;
}
/*---------------------------------------------------------------------------*/
void
rucb_close(struct rucb_conn *c)
{
  runicast_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
rucb_send(struct rucb_conn *c, const rimeaddr_t *receiver)
{
  c->chunk = 0;
  read_data(c);
  rimeaddr_copy(&c->receiver, receiver);
  rimeaddr_copy(&c->sender, &rimeaddr_node_addr);
  runicast_send(&c->c, receiver, MAX_TRANSMISSIONS);
  return 0;
}
/*---------------------------------------------------------------------------*/
