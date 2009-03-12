/**
 * \addtogroup rudolph0
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
 * $Id: rudolph0.c,v 1.11 2009/03/12 21:58:21 adamdunkels Exp $
 */

/**
 * \file
 *         Rudolph0: a simple block data flooding protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <stddef.h> /* for offsetof */

#include "net/rime.h"
#include "net/rime/rudolph0.h"

#define STEADY_TIME CLOCK_SECOND * 2

#define DEFAULT_SEND_INTERVAL CLOCK_SECOND / 2
enum {
  TYPE_DATA,
  TYPE_NACK,
};

enum {
  STATE_RECEIVER,
  STATE_SENDER,
};

#define VERSION_LT(a, b) ((signed char)((a) - (b)) < 0)

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
read_new_datapacket(struct rudolph0_conn *c)
{
  int len = 0;

  if(c->cb->read_chunk) {
    len = c->cb->read_chunk(c, c->current.h.chunk * RUDOLPH0_DATASIZE,
			    c->current.data, RUDOLPH0_DATASIZE);
  }
  c->current.datalen = len;

  PRINTF("read_new_datapacket len %d\n", len);
}
/*---------------------------------------------------------------------------*/
static void
send_nack(struct rudolph0_conn *c)
{
  struct rudolph0_hdr *hdr;
  packetbuf_clear();
  packetbuf_hdralloc(sizeof(struct rudolph0_hdr));
  hdr = packetbuf_hdrptr();

  hdr->type = TYPE_NACK;
  hdr->version = c->current.h.version;
  hdr->chunk = c->current.h.chunk;

  PRINTF("Sending nack for %d:%d\n", hdr->version, hdr->chunk);
  polite_send(&c->nackc, c->send_interval / 2, sizeof(struct rudolph0_hdr));
}
/*---------------------------------------------------------------------------*/
static void
sent(struct stbroadcast_conn *stbroadcast)
{
  struct rudolph0_conn *c = (struct rudolph0_conn *)stbroadcast;

  if(c->current.datalen == RUDOLPH0_DATASIZE) {
    c->current.h.chunk++;
    PRINTF("Sending data chunk %d next time\n", c->current.h.chunk);
    read_new_datapacket(c);
  } else {
    stbroadcast_set_timer(&c->c, STEADY_TIME);
    PRINTF("Steady: Sending the same data chunk next time datalen %d, %d\n",
	   c->current.datalen, RUDOLPH0_DATASIZE);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv(struct stbroadcast_conn *stbroadcast)
{
  struct rudolph0_conn *c = (struct rudolph0_conn *)stbroadcast;
  struct rudolph0_datapacket *p = packetbuf_dataptr();

  if(p->h.type == TYPE_DATA) {
    if(c->current.h.version != p->h.version) {
      PRINTF("rudolph0 new version %d\n", p->h.version);
      c->current.h.version = p->h.version;
      c->current.h.chunk = 0;
      c->cb->write_chunk(c, 0, RUDOLPH0_FLAG_NEWFILE, p->data, 0);
      if(p->h.chunk != 0) {
	send_nack(c);
      } else {
	c->cb->write_chunk(c, 0, RUDOLPH0_FLAG_NONE, p->data, p->datalen);
	c->current.h.chunk++;
      }
    } else if(p->h.version == c->current.h.version) {
      if(p->h.chunk == c->current.h.chunk) {
	PRINTF("received chunk %d\n", p->h.chunk);
	if(p->datalen < RUDOLPH0_DATASIZE) {
	  c->cb->write_chunk(c, c->current.h.chunk * RUDOLPH0_DATASIZE,
			     RUDOLPH0_FLAG_LASTCHUNK, p->data, p->datalen);
	} else {
	  c->cb->write_chunk(c, c->current.h.chunk * RUDOLPH0_DATASIZE,
			     RUDOLPH0_FLAG_NONE, p->data, p->datalen);
	}
	c->current.h.chunk++;
	
      } else if(p->h.chunk > c->current.h.chunk) {
	PRINTF("received chunk %d > %d, sending NACK\n", p->h.chunk, c->current.h.chunk);
	send_nack(c);
      }
    } else { /* p->h.version < c->current.h.version */
      /* Ignore packets with old version */
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
recv_nack(struct polite_conn *polite)
{
  struct rudolph0_conn *c = (struct rudolph0_conn *)
    ((char *)polite - offsetof(struct rudolph0_conn,
			     nackc));
  struct rudolph0_datapacket *p = packetbuf_dataptr();

  if(p->h.type == TYPE_NACK && c->state == STATE_SENDER) {
    if(p->h.version == c->current.h.version) {
      PRINTF("Reseting chunk to %d\n", p->h.chunk);
      c->current.h.chunk = p->h.chunk;
    } else {
      PRINTF("Wrong version, reseting chunk to 0\n");
      c->current.h.chunk = 0;
    }
    read_new_datapacket(c);
    stbroadcast_set_timer(&c->c, c->send_interval);
  }
}
/*---------------------------------------------------------------------------*/
static const struct polite_callbacks polite = { recv_nack, 0, 0 };
static const struct stbroadcast_callbacks stbroadcast = { recv, sent };
/*---------------------------------------------------------------------------*/
void
rudolph0_open(struct rudolph0_conn *c, uint16_t channel,
	      const struct rudolph0_callbacks *cb)
{
  stbroadcast_open(&c->c, channel, &stbroadcast);
  polite_open(&c->nackc, channel + 1, &polite);
  c->cb = cb;
  c->current.h.version = 0;
  c->state = STATE_RECEIVER;
  c->send_interval = DEFAULT_SEND_INTERVAL;
}
/*---------------------------------------------------------------------------*/
void
rudolph0_close(struct rudolph0_conn *c)
{
  stbroadcast_close(&c->c);
  polite_close(&c->nackc);
}
/*---------------------------------------------------------------------------*/
void
rudolph0_send(struct rudolph0_conn *c, clock_time_t send_interval)
{
  c->state = STATE_SENDER;
  c->current.h.version++;
  c->current.h.version++;
  c->current.h.chunk = 0;
  c->current.h.type = TYPE_DATA;
  read_new_datapacket(c);
  packetbuf_reference(&c->current, sizeof(struct rudolph0_datapacket));
  c->send_interval = send_interval;
  stbroadcast_send_stubborn(&c->c, c->send_interval);
}
/*---------------------------------------------------------------------------*/
void
rudolph0_force_restart(struct rudolph0_conn *c)
{
  c->current.h.chunk = 0;
  send_nack(c);
}
/*---------------------------------------------------------------------------*/
void
rudolph0_stop(struct rudolph0_conn *c)
{
  stbroadcast_cancel(&c->c);
}
/*---------------------------------------------------------------------------*/
int
rudolph0_version(struct rudolph0_conn *c)
{
  return c->current.h.version;
}
/*---------------------------------------------------------------------------*/
void
rudolph0_set_version(struct rudolph0_conn *c, int version)
{
  c->current.h.version = version;
}
/*---------------------------------------------------------------------------*/
/** @} */
