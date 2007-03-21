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
 * $Id: rudolph1.c,v 1.1 2007/03/21 23:14:40 adamdunkels Exp $
 */

/**
 * \file
 *         Rudolph1: a simple block data flooding protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "rudolph1.h"
#include "cfs/cfs.h"

#include <stddef.h> /* for offsetof */

#define DATA_INTERVAL CLOCK_SECOND * 2
#define TRICKLE_INTERVAL TRICKLE_SECOND
#define NACK_TIMEOUT CLOCK_SECOND

struct rudolph1_hdr {
  u8_t type;
  u8_t version;
  u16_t chunk;
};

#define RUDOLPH1_DATASIZE 32

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
  int len;

  cfs_seek(c->cfs_fd, chunk * RUDOLPH1_DATASIZE);
  len = cfs_read(c->cfs_fd, dataptr, RUDOLPH1_DATASIZE);
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
  cfs_seek(c->cfs_fd, chunk * RUDOLPH1_DATASIZE);
  cfs_write(c->cfs_fd, data, datalen);
  if(datalen < RUDOLPH1_DATASIZE) {
    PRINTF("%d: get %d bytes, file complete\n",
	   rimeaddr_node_addr.u16, datalen);
    c->cb->received_file(c, c->cfs_fd);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_nack(struct rudolph1_conn *c)
{
  struct rudolph1_hdr *hdr;
  rimebuf_clear();
  rimebuf_hdrextend(sizeof(struct rudolph1_hdr));
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
    PRINTF("rudolph1 new version %d\n", p->h.version);
    c->cfs_fd = c->cb->new_file(c);
    c->version = p->h.version;
    c->chunk = 1; /* Next chunk is 1. */
    if(c->cfs_fd != -1) {
      if(p->h.chunk != 0) {
	send_nack(c);
      } else {
	write_data(c, 0, p->data, p->datalen);
      }
    }
  } else if(p->h.version == c->version) {
    if(c->cfs_fd != -1) {
      if(p->h.chunk == c->chunk) {
	PRINTF("%d: received chunk %d\n",
	       rimeaddr_node_addr.u16, p->h.chunk);
	cfs_seek(c->cfs_fd, c->chunk * RUDOLPH1_DATASIZE);
	cfs_write(c->cfs_fd, p->data, p->datalen);
	c->chunk++;
	if(p->datalen < RUDOLPH1_DATASIZE) {
	  c->cb->received_file(c, c->cfs_fd);
	}
      } else if(p->h.chunk > c->chunk) {
	PRINTF("%d: received chunk %d > %d, sending NACK\n",
	       rimeaddr_node_addr.u16,
	       p->h.chunk, c->chunk);
	send_nack(c);
      }
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

  /*  if(rand() & 1) {
    return;
    }*/

  if(p->h.type == TYPE_DATA) {
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

  /*  if(rand() & 1) {
    return;
    }*/

  c->nacks++;

  if(p->h.type == TYPE_NACK) {
    if(p->h.version == c->version) {
      if(p->h.chunk < c->chunk) {
	format_data(c, p->h.chunk);
	uabc_send(&c->uabc, NACK_TIMEOUT);
      }
    } else if(LT(p->h.version, c->version)) {
      format_data(c, 0);
      uabc_send(&c->uabc, NACK_TIMEOUT);
    }
  } else if(p->h.type == TYPE_DATA) {
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
    c->chunk++;
    len = format_data(c, c->chunk);
    trickle_send(&c->trickle, c->trickle_interval);
    if(len == RUDOLPH1_DATASIZE) {
      ctimer_set(&c->t, DATA_INTERVAL, send_next_packet, c);
    }
  } else {
    ctimer_set(&c->t, DATA_INTERVAL, send_next_packet, c);
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
rudolph1_send(struct rudolph1_conn *c, int cfs_fd)
{
  c->cfs_fd = cfs_fd;
  c->version++;
  c->chunk = 0;
  c->trickle_interval = TRICKLE_INTERVAL;
  format_data(c, 0);
  trickle_send(&c->trickle, c->trickle_interval);
  ctimer_set(&c->t, DATA_INTERVAL, send_next_packet, c);
}
/*---------------------------------------------------------------------------*/
