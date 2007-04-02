/**
 * \addtogroup rudolph1
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
 * $Id: rudolph1.c,v 1.8 2007/04/02 19:12:37 adamdunkels Exp $
 */

/**
 * \file
 *         Rudolph1: a simple block data flooding protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <stdio.h>
#include <stddef.h> /* for offsetof */

#include "net/rime.h"
#include "net/rime/rudolph1.h"
#include "cfs/cfs.h"

#define DEFAULT_SEND_INTERVAL CLOCK_SECOND * 2
#define TRICKLE_INTERVAL TRICKLE_SECOND / 2
#define NACK_TIMEOUT CLOCK_SECOND / 4

struct rudolph1_hdr {
  u8_t type;
  u8_t version;
  u16_t chunk;
};

#define RUDOLPH1_DATASIZE 64

struct rudolph1_datapacket {
  struct rudolph1_hdr h;
  u8_t datalen;
  u8_t data[RUDOLPH1_DATASIZE];
};

enum {
  TYPE_DATA,
  TYPE_NACK,
};

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define LT(a, b) ((signed char)((a) - (b)) < 0)

/*---------------------------------------------------------------------------*/
static int
read_data(struct rudolph1_conn *c, char *dataptr, int chunk)
{
  int len = 0;

  if(c->cb->read_chunk) {
    len = c->cb->read_chunk(c, chunk * RUDOLPH1_DATASIZE,
			    dataptr, RUDOLPH1_DATASIZE);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static int
format_data(struct rudolph1_conn *c, int chunk)
{
  struct rudolph1_datapacket *p;
  
  rimebuf_clear();
  p = rimebuf_dataptr();
  p->h.type = TYPE_DATA;
  p->h.version = c->version;
  p->h.chunk = chunk;
  p->datalen = read_data(c, p->data, chunk);
  rimebuf_set_datalen(sizeof(struct rudolph1_datapacket) -
		      (RUDOLPH1_DATASIZE - p->datalen));

  return p->datalen;
}
/*---------------------------------------------------------------------------*/
static void
write_data(struct rudolph1_conn *c, int chunk, u8_t *data, int datalen)
{
  if(chunk == 0) {
    c->cb->write_chunk(c, 0, RUDOLPH1_FLAG_NEWFILE, data, 0);
  }

  if(datalen < RUDOLPH1_DATASIZE) {
    PRINTF("%d: get %d bytes, file complete\n",
	   rimeaddr_node_addr.u16, datalen);
    c->cb->write_chunk(c, chunk * RUDOLPH1_DATASIZE,
		       RUDOLPH1_FLAG_LASTCHUNK, data, datalen);
  } else {
    c->cb->write_chunk(c, chunk * RUDOLPH1_DATASIZE,
		       RUDOLPH1_FLAG_NONE, data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_nack(struct rudolph1_conn *c)
{
  struct rudolph1_hdr *hdr;
  rimebuf_clear();
  rimebuf_hdralloc(sizeof(struct rudolph1_hdr));
  hdr = rimebuf_hdrptr();

  hdr->type = TYPE_NACK;
  hdr->version = c->version;
  hdr->chunk = c->chunk;

  PRINTF("Sending nack for %d:%d\n", hdr->version, hdr->chunk);
  uabc_send(&c->uabc, CLOCK_SECOND / 4);
}
/*---------------------------------------------------------------------------*/
static void
handle_data(struct rudolph1_conn *c, struct rudolph1_datapacket *p)
{
  if(LT(c->version, p->h.version)) {
    PRINTF("%d.%d: rudolph1 new version %d, chunk %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   p->h.version, p->h.chunk);
    c->version = p->h.version;
    c->highest_chunk_heard = c->chunk = 0;
      if(p->h.chunk != 0) {
	send_nack(c);
      } else {
	write_data(c, 0, p->data, p->datalen);
	c->chunk = 1; /* Next chunk is 1. */
      }
      /*    }*/
  } else if(p->h.version == c->version) {
    if(p->h.chunk == c->chunk) {
      PRINTF("%d.%d: received chunk %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     p->h.chunk);
      write_data(c, p->h.chunk, p->data, p->datalen);
      c->highest_chunk_heard = c->chunk;
      c->chunk++;
    } else if(p->h.chunk > c->chunk) {
      PRINTF("%d: received chunk %d > %d, sending NACK\n",
	     rimeaddr_node_addr.u16,
	     p->h.chunk, c->chunk);
      send_nack(c);
      c->highest_chunk_heard = p->h.chunk;
    } else if(p->h.chunk < c->chunk) {
      /* Ignore packets with a lower chunk number */
    }

    /* If we have heard a higher chunk number, we send a NACK so that
       we get a repair for the next packet. */
    
    if(c->highest_chunk_heard < p->h.chunk) {
      send_nack(c);
    }
  } else { /* p->h.version < c->current.h.version */
    /* Ignore packets with old version */
  }

}
/*---------------------------------------------------------------------------*/
static void
recv_trickle(struct trickle_conn *trickle)
{
  struct rudolph1_conn *c = (struct rudolph1_conn *)trickle;
  struct rudolph1_datapacket *p = rimebuf_dataptr();

  if(p->h.type == TYPE_DATA) {
    PRINTF("%d.%d: received trickle with chunk %d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   p->h.chunk);
    handle_data(c, p);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv_uabc(struct uabc_conn *uabc)
{
  struct rudolph1_conn *c = (struct rudolph1_conn *)
    ((char *)uabc - offsetof(struct rudolph1_conn, uabc));
  struct rudolph1_datapacket *p = rimebuf_dataptr();

  c->nacks++;

  if(p->h.type == TYPE_NACK) {
    PRINTF("Got NACK for %d:%d\n", p->h.version, p->h.chunk);
    if(p->h.version == c->version) {
      if(p->h.chunk < c->chunk) {
	/* Format and send a repair packet */
	PRINTF("%d.%d: sending repair for chunk %d\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	       p->h.chunk);
	format_data(c, p->h.chunk);
	uabc_send(&c->uabc, c->send_interval / 2);
      }
    } else if(LT(p->h.version, c->version)) {
      format_data(c, 0);
      uabc_send(&c->uabc, c->send_interval / 2);
    }
  } else if(p->h.type == TYPE_DATA) {
    /* This is a repair packet from someone else. */
    handle_data(c, p);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_next_packet(void *ptr)
{
  struct rudolph1_conn *c = ptr;
  int len;
  if(c->nacks == 0) {
    len = format_data(c, c->chunk);
    trickle_send(&c->trickle, c->trickle_interval);
    if(len == RUDOLPH1_DATASIZE) {
      ctimer_set(&c->t, c->send_interval, send_next_packet, c);
    }
    c->chunk++;
  } else {
    ctimer_set(&c->t, c->send_interval, send_next_packet, c);
  }
  c->nacks = 0;
}
/*---------------------------------------------------------------------------*/
static const struct uabc_callbacks uabc = { recv_uabc, NULL, NULL };
static const struct trickle_callbacks trickle = { recv_trickle };
/*---------------------------------------------------------------------------*/
void
rudolph1_open(struct rudolph1_conn *c, u16_t channel,
	      const struct rudolph1_callbacks *cb)
{
  trickle_open(&c->trickle, channel, &trickle);
  uabc_open(&c->uabc, channel + 1, &uabc);
  c->cb = cb;
  c->version = 0;
  c->send_interval = DEFAULT_SEND_INTERVAL;
}
/*---------------------------------------------------------------------------*/
void
rudolph1_close(struct rudolph1_conn *c)
{
  trickle_close(&c->trickle);
  uabc_close(&c->uabc);
}
/*---------------------------------------------------------------------------*/
void
rudolph1_send(struct rudolph1_conn *c, clock_time_t send_interval)
{
  c->version++;
  c->chunk = c->highest_chunk_heard = 0;
  c->trickle_interval = TRICKLE_INTERVAL;
  format_data(c, 0);
  trickle_send(&c->trickle, c->trickle_interval);
  c->chunk++;
  c->send_interval = send_interval;
  ctimer_set(&c->t, send_interval, send_next_packet, c);
}
/*---------------------------------------------------------------------------*/
void
rudolph1_stop(struct rudolph1_conn *c)
{
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
/** @} */
