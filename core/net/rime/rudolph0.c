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
 * $Id: rudolph0.c,v 1.1 2007/03/25 11:56:59 adamdunkels Exp $
 */

/**
 * \file
 *         Rudolph0: a simple block data flooding protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "rudolph0.h"

#include <stddef.h> /* for offsetof */

#define SENDING_TIME CLOCK_SECOND / 2
#define STEADY_TIME CLOCK_SECOND * 2

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
  rimebuf_clear();
  rimebuf_hdralloc(sizeof(struct rudolph0_hdr));
  hdr = rimebuf_hdrptr();

  hdr->type = TYPE_NACK;
  hdr->version = c->current.h.version;
  hdr->chunk = c->current.h.chunk;

  PRINTF("Sending nack for %d:%d\n", hdr->version, hdr->chunk);
  uabc_send(&c->nackc, CLOCK_SECOND / 4);
}
/*---------------------------------------------------------------------------*/
static void
sent(struct sabc_conn *sabc)
{
  struct rudolph0_conn *c = (struct rudolph0_conn *)sabc;

  if(c->current.datalen == RUDOLPH0_DATASIZE) {
    c->current.h.chunk++;
    PRINTF("Sending data chunk %d next time\n", c->current.h.chunk);
    read_new_datapacket(c);
  } else {
    sabc_set_timer(&c->c, STEADY_TIME);
    PRINTF("Steady: Sending the same data chunk next time datalen %d, %d\n",
	   c->current.datalen, RUDOLPH0_DATASIZE);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv(struct sabc_conn *sabc)
{
  struct rudolph0_conn *c = (struct rudolph0_conn *)sabc;
  struct rudolph0_datapacket *p = rimebuf_dataptr();

  /*  if(rand() & 1) {
    return;
    }*/
  
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
recv_nack(struct uabc_conn *uabc)
{
  struct rudolph0_conn *c = (struct rudolph0_conn *)
    ((char *)uabc - offsetof(struct rudolph0_conn,
			     nackc));
  struct rudolph0_datapacket *p = rimebuf_dataptr();

  if(p->h.type == TYPE_NACK && c->state == STATE_SENDER) {
    if(p->h.version == c->current.h.version) {
      PRINTF("Reseting chunk to %d\n", p->h.chunk);
      c->current.h.chunk = p->h.chunk;
    } else {
      PRINTF("Wrong version, reseting chunk to 0\n");
      c->current.h.chunk = 0;
    }
    read_new_datapacket(c);
    sabc_set_timer(&c->c, SENDING_TIME);
  }
}
/*---------------------------------------------------------------------------*/
static const struct uabc_callbacks uabc = { recv_nack, NULL, NULL };
static const struct sabc_callbacks sabc = { recv, sent };
/*---------------------------------------------------------------------------*/
void
rudolph0_open(struct rudolph0_conn *c, u16_t channel,
	      const struct rudolph0_callbacks *cb)
{
  sabc_open(&c->c, channel, &sabc);
  uabc_open(&c->nackc, channel + 1, &uabc);
  c->cb = cb;
  c->current.h.version = 0;
  c->state = STATE_RECEIVER;
}
/*---------------------------------------------------------------------------*/
void
rudolph0_close(struct rudolph0_conn *c)
{
  sabc_close(&c->c);
  uabc_close(&c->nackc);
}
/*---------------------------------------------------------------------------*/
void
rudolph0_send(struct rudolph0_conn *c)
{
  c->state = STATE_SENDER;
  c->current.h.version++;
  c->current.h.chunk = 0;
  c->current.h.type = TYPE_DATA;
  read_new_datapacket(c);
  rimebuf_reference(&c->current, sizeof(struct rudolph0_datapacket));
  sabc_send_stubborn(&c->c, SENDING_TIME);
}
/*---------------------------------------------------------------------------*/
void
rudolph0_stop(struct rudolph0_conn *c)
{
  sabc_cancel(&c->c);
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
