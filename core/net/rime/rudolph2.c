/* XXX todo: add timeout so that hops_from_sink is reset to MAX
   after a while. */

/* XXX todo: use a ctimer to drive peridodic transmission: the current
   way does not work if a queuebuf cannot be allocated. */

/**
 * \addtogroup rudolph2
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
 */

/**
 * \file
 *         Rudolph2: a simple block data flooding protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <stdio.h>
#include <stddef.h> /* for offsetof */

#include "net/rime.h"
#include "net/rime/polite.h"
#include "net/rime/rudolph2.h"
#include "cfs/cfs.h"

#define SEND_INTERVAL CLOCK_SECOND / 2
#define STEADY_INTERVAL CLOCK_SECOND * 16
#define RESEND_INTERVAL SEND_INTERVAL * 4
#define NACK_TIMEOUT CLOCK_SECOND / 4

struct rudolph2_hdr {
  uint8_t type;
  uint8_t hops_from_base;
  uint16_t version;
  uint16_t chunk;
};

#define POLITE_HEADER 1

#define HOPS_MAX 64

enum {
  TYPE_DATA,
  TYPE_NACK,
};

#define FLAG_LAST_SENT     0x01
#define FLAG_LAST_RECEIVED 0x02
#define FLAG_IS_STOPPED    0x04

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define LT(a, b) ((signed short)((a) - (b)) < 0)

/*---------------------------------------------------------------------------*/
static int
read_data(struct rudolph2_conn *c, uint8_t *dataptr, int chunk)
{
  int len = 0;

  if(c->cb->read_chunk) {
    len = c->cb->read_chunk(c, chunk * RUDOLPH2_DATASIZE,
			    dataptr, RUDOLPH2_DATASIZE);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static int
format_data(struct rudolph2_conn *c, int chunk)
{
  struct rudolph2_hdr *hdr;
  int len;
  
  packetbuf_clear();
  hdr = packetbuf_dataptr();
  hdr->type = TYPE_DATA;
  hdr->hops_from_base = c->hops_from_base;
  hdr->version = c->version;
  hdr->chunk = chunk;
  len = read_data(c, (uint8_t *)hdr + sizeof(struct rudolph2_hdr), chunk);
  packetbuf_set_datalen(sizeof(struct rudolph2_hdr) + len);

  return len;
}
/*---------------------------------------------------------------------------*/
static void
write_data(struct rudolph2_conn *c, int chunk, uint8_t *data, int datalen)
{
  /* xxx Don't write any data if the application has been stopped. */
  if(c->flags & FLAG_IS_STOPPED) {
    return;
  }
  
  if(chunk == 0) {
    c->cb->write_chunk(c, 0, RUDOLPH2_FLAG_NEWFILE, data, 0);
  }
  
  PRINTF("%d.%d: get %d bytes\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 datalen);

  
  if(datalen < RUDOLPH2_DATASIZE) {
    PRINTF("%d.%d: get %d bytes, file complete\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   datalen);
    c->cb->write_chunk(c, chunk * RUDOLPH2_DATASIZE,
		       RUDOLPH2_FLAG_LASTCHUNK, data, datalen);
  } else {
    c->cb->write_chunk(c, chunk * RUDOLPH2_DATASIZE,
		       RUDOLPH2_FLAG_NONE, data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
static int
send_data(struct rudolph2_conn *c, clock_time_t interval)
{
  int len;

  len = format_data(c, c->snd_nxt);
  polite_send(&c->c, interval, POLITE_HEADER);
  PRINTF("%d.%d: send_data chunk %d, rcv_nxt %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 c->snd_nxt, c->rcv_nxt);

  return len;
}
/*---------------------------------------------------------------------------*/
static void
send_nack(struct rudolph2_conn *c)
{
  struct rudolph2_hdr *hdr;
  packetbuf_clear();
  packetbuf_hdralloc(sizeof(struct rudolph2_hdr));
  hdr = packetbuf_hdrptr();

  hdr->hops_from_base = c->hops_from_base;
  hdr->type = TYPE_NACK;
  hdr->version = c->version;
  hdr->chunk = c->rcv_nxt;

  PRINTF("%d.%d: Sending nack for %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 hdr->chunk);
  polite_send(&c->c, NACK_TIMEOUT, POLITE_HEADER);
}
/*---------------------------------------------------------------------------*/
#if 0 /* Function below not currently used in the code */
static void
send_next(struct rudolph2_conn *c)
{
  int len;
  clock_time_t interval;

  if(c->flags & FLAG_LAST_SENT) {
    interval = STEADY_INTERVAL;
  } else {
    interval = SEND_INTERVAL;
  }
  
  len = send_data(c, interval);

  if(len < RUDOLPH2_DATASIZE) {
    c->flags |= FLAG_LAST_SENT;
  } else {
    c->flags &= ~FLAG_LAST_SENT;
  }
  
  if(c->nacks == 0 &&
     len == RUDOLPH2_DATASIZE &&
     c->snd_nxt + 1 < c->rcv_nxt) {
    c->snd_nxt++;
  }
  c->nacks = 0;
}
#endif /* 0 */
/*---------------------------------------------------------------------------*/
static void
sent(struct polite_conn *polite)
{
  /*  struct rudolph2_conn *c = (struct rudolph2_conn *)polite;

  if((c->flags & FLAG_IS_STOPPED) == 0 &&
     (c->flags & FLAG_LAST_RECEIVED)) {
    if(c->snd_nxt < c->rcv_nxt) {
      send_next(c);
    } else {
      send_data(c, STEADY_INTERVAL);
    }
    }*/
  
}
/*---------------------------------------------------------------------------*/
static void
dropped(struct polite_conn *polite)
{
  /*  struct rudolph2_conn *c = (struct rudolph2_conn *)polite;
  if((c->flags & FLAG_IS_STOPPED) == 0 &&
     (c->flags & FLAG_LAST_RECEIVED)) {
    if(c->snd_nxt + 1 < c->rcv_nxt) {
      send_data(c, SEND_INTERVAL);
    } else {
      send_data(c, STEADY_INTERVAL);
    }
    }*/
}
/*---------------------------------------------------------------------------*/
static void
timed_send(void *ptr)
{
  struct rudolph2_conn *c = (struct rudolph2_conn *)ptr;
  clock_time_t interval;
  int len;
  
  if((c->flags & FLAG_IS_STOPPED) == 0 &&
     (c->flags & FLAG_LAST_RECEIVED)) {
    /*    if(c->snd_nxt + 1 < c->rcv_nxt) {
      interval = SEND_INTERVAL;
    } else {
      interval = STEADY_INTERVAL;
      }*/
    /*    send_data(c, interval);*/

    if(c->flags & FLAG_LAST_SENT) {
      interval = STEADY_INTERVAL;
    } else {
      interval = SEND_INTERVAL;
    }
  

    len = send_data(c, interval);
    
    if(len < RUDOLPH2_DATASIZE) {
      c->flags |= FLAG_LAST_SENT;
    } else {
      c->flags &= ~FLAG_LAST_SENT;
    }
    
    if(c->nacks == 0 &&
       len == RUDOLPH2_DATASIZE &&
     c->snd_nxt + 1 < c->rcv_nxt) {
      c->snd_nxt++;
    }
    c->nacks = 0;
    ctimer_set(&c->t, interval, timed_send, c);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv(struct polite_conn *polite)
{
  struct rudolph2_conn *c = (struct rudolph2_conn *)polite;
  struct rudolph2_hdr *hdr = packetbuf_dataptr();

  /* Only accept NACKs from nodes that are farther away from the base
     than us. */

  if(hdr->type == TYPE_NACK && hdr->hops_from_base > c->hops_from_base) {
    c->nacks++;
    PRINTF("%d.%d: Got NACK for %d:%d (%d:%d)\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   hdr->version, hdr->chunk,
	   c->version, c->rcv_nxt);
    if(hdr->version == c->version) {
      if(hdr->chunk < c->rcv_nxt) {
	c->snd_nxt = hdr->chunk;
	send_data(c, SEND_INTERVAL);
      }
    } else if(LT(hdr->version, c->version)) {
      c->snd_nxt = 0;
      send_data(c, SEND_INTERVAL);
    }
  } else if(hdr->type == TYPE_DATA) {
    if(hdr->hops_from_base < c->hops_from_base) {
      /* Only accept data from nodes that are closer to the base than
	 us. */
      c->hops_from_base = hdr->hops_from_base + 1;
      if(LT(c->version, hdr->version)) {
	PRINTF("%d.%d: rudolph2 new version %d, chunk %d\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	       hdr->version, hdr->chunk);
	c->version = hdr->version;
	c->snd_nxt = c->rcv_nxt = 0;
	c->flags &= ~FLAG_LAST_RECEIVED;
	c->flags &= ~FLAG_LAST_SENT;
	if(hdr->chunk != 0) {
	  send_nack(c);
	} else {
	  packetbuf_hdrreduce(sizeof(struct rudolph2_hdr));
	  write_data(c, 0, packetbuf_dataptr(), packetbuf_totlen());
	}
      } else if(hdr->version == c->version) {
	PRINTF("%d.%d: got chunk %d snd_nxt %d rcv_nxt %d\n",
	       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	       hdr->chunk, c->snd_nxt, c->rcv_nxt);
	
	if(hdr->chunk == c->rcv_nxt) {
	  int len;
	  packetbuf_hdrreduce(sizeof(struct rudolph2_hdr));
	  PRINTF("%d.%d: received chunk %d len %d\n",
		 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		 hdr->chunk, packetbuf_totlen());
	  len = packetbuf_totlen();
	  write_data(c, hdr->chunk, packetbuf_dataptr(), packetbuf_totlen());
	  c->rcv_nxt++;
	  if(len < RUDOLPH2_DATASIZE) {
	    c->flags |= FLAG_LAST_RECEIVED;
	    send_data(c, RESEND_INTERVAL);
	    ctimer_set(&c->t, RESEND_INTERVAL, timed_send, c);
	  }
	} else if(hdr->chunk > c->rcv_nxt) {
	  PRINTF("%d.%d: received chunk %d > %d, sending NACK\n",
		 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		 hdr->chunk, c->rcv_nxt);
	  send_nack(c);
	} else if(hdr->chunk < c->rcv_nxt) {
	  /* Ignore packets with a lower chunk number */
	}
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct polite_callbacks polite = { recv, sent, dropped };
/*---------------------------------------------------------------------------*/
void
rudolph2_open(struct rudolph2_conn *c, uint16_t channel,
	      const struct rudolph2_callbacks *cb)
{
  polite_open(&c->c, channel, &polite);
  c->cb = cb;
  c->version = 0;
  c->hops_from_base = HOPS_MAX;
}
/*---------------------------------------------------------------------------*/
void
rudolph2_close(struct rudolph2_conn *c)
{
  polite_close(&c->c);
}
/*---------------------------------------------------------------------------*/
void
rudolph2_send(struct rudolph2_conn *c, clock_time_t send_interval)
{
  int len;

  c->hops_from_base = 0;
  c->version++;
  c->snd_nxt = 0;
  len = RUDOLPH2_DATASIZE;
  packetbuf_clear();
  for(c->rcv_nxt = 0; len == RUDOLPH2_DATASIZE; c->rcv_nxt++) {
    len = read_data(c, packetbuf_dataptr(), c->rcv_nxt);
  }
  c->flags = FLAG_LAST_RECEIVED;
  /*  printf("Highest chunk %d\n", c->rcv_nxt);*/
  send_data(c, SEND_INTERVAL);
  ctimer_set(&c->t, SEND_INTERVAL, timed_send, c);
}
/*---------------------------------------------------------------------------*/
void
rudolph2_stop(struct rudolph2_conn *c)
{
  polite_cancel(&c->c);
  c->flags |= FLAG_IS_STOPPED;
}
/*---------------------------------------------------------------------------*/
/** @} */
