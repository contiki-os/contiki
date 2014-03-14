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
 *         A Chameleon module that produces non-optimized headers
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <string.h>

#include "net/rime/chameleon.h"
#include "net/rime/rime.h"

/* This option enables an optimization where the link addresses are
   left to the MAC RDC and not encoded in the Chameleon header.
   Note: this requires that the underlying MAC layer to add link
   addresses and will not work together with for example nullrdc.
 */
#ifdef CHAMELEON_CONF_WITH_MAC_LINK_ADDRESSES
#define CHAMELEON_WITH_MAC_LINK_ADDRESSES CHAMELEON_CONF_WITH_MAC_LINK_ADDRESSES
#else /* !CHAMELEON_CONF_WITH_MAC_LINK_ADDRESSES */
#define CHAMELEON_WITH_MAC_LINK_ADDRESSES 0
#endif /* !CHAMELEON_CONF_WITH_MAC_LINK_ADDRESSES */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

struct raw_hdr {
  uint8_t channel[2];
};

/*---------------------------------------------------------------------------*/
static struct channel *
input(void)
{
  const struct packetbuf_attrlist *a;
  int byteptr, bitptr, len;
  uint8_t *hdrptr;
  struct raw_hdr *hdr;
  struct channel *c;

  /* The packet has a header that tells us what channel the packet is
     for. */
  hdr = (struct raw_hdr *)packetbuf_dataptr();
  if(packetbuf_hdrreduce(sizeof(struct raw_hdr)) == 0) {
    PRINTF("chameleon-raw: too short packet\n");
    return NULL;
  }
  c = channel_lookup((hdr->channel[1] << 8) + hdr->channel[0]);
  if(c == NULL) {
    PRINTF("chameleon-raw: input: channel %u not found\n",
           (hdr->channel[1] << 8) + hdr->channel[0]);
    return NULL;
  }

  hdrptr = packetbuf_dataptr();
  if(packetbuf_hdrreduce(c->hdrsize) == 0) {
    PRINTF("chameleon-raw: too short packet\n");
    return NULL;
  }
  byteptr = bitptr = 0;
  for(a = c->attrlist; a->type != PACKETBUF_ATTR_NONE; ++a) {
#if CHAMELEON_WITH_MAC_LINK_ADDRESSES
    if(a->type == PACKETBUF_ADDR_SENDER ||
       a->type == PACKETBUF_ADDR_RECEIVER) {
      /* Let the link layer handle sender and receiver */
      continue;
    }
#endif /* CHAMELEON_WITH_MAC_LINK_ADDRESSES */
    PRINTF("%d.%d: unpack_header type %d, len %d\n",
	   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	   a->type, a->len);
    len = (a->len & 0xf8) + ((a->len & 7) ? 8: 0);
    if(PACKETBUF_IS_ADDR(a->type)) {
      const linkaddr_t addr;
      memcpy((uint8_t *)&addr, &hdrptr[byteptr], len / 8);
      PRINTF("%d.%d: unpack_header type %d, addr %d.%d\n",
	     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	     a->type, addr.u8[0], addr.u8[1]);
      packetbuf_set_addr(a->type, &addr);
    } else {
      packetbuf_attr_t val = 0;
      memcpy((uint8_t *)&val, &hdrptr[byteptr], len / 8);

      packetbuf_set_attr(a->type, val);
      PRINTF("%d.%d: unpack_header type %d, val %d\n",
	     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	     a->type, val);
    }
    byteptr += len / 8;
  }
  return c;
}
/*---------------------------------------------------------------------------*/
static int
output(struct channel *c)
{
  const struct packetbuf_attrlist *a;
  int byteptr, len;
  uint8_t *hdrptr;
  struct raw_hdr *hdr;
  
  /* Compute the total size of the final header by summing the size of
     all attributes that are used on this channel. */
  if(packetbuf_hdralloc(c->hdrsize + sizeof(struct raw_hdr)) == 0) {
    PRINTF("chameleon-raw: insufficient space for headers\n");
    return 0;
  }
  hdr = (struct raw_hdr *)packetbuf_hdrptr();
  hdr->channel[0] = c->channelno & 0xff;
  hdr->channel[1] = (c->channelno >> 8) & 0xff;

  hdrptr = ((uint8_t *)packetbuf_hdrptr()) + sizeof(struct raw_hdr);
  byteptr = 0;
  for(a = c->attrlist; a->type != PACKETBUF_ATTR_NONE; ++a) {
#if CHAMELEON_WITH_MAC_LINK_ADDRESSES
    if(a->type == PACKETBUF_ADDR_SENDER ||
       a->type == PACKETBUF_ADDR_RECEIVER) {
      /* Let the link layer handle sender and receiver */
      PRINTF("%d.%d: pack_header leaving sender/receiver to link layer\n");
      continue;
    }
#endif /* CHAMELEON_WITH_MAC_LINK_ADDRESSES */
    PRINTF("%d.%d: pack_header type %d, len %d\n",
	   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	   a->type, a->len);
    len = (a->len & 0xf8) + ((a->len & 7) ? 8: 0);
    if(PACKETBUF_IS_ADDR(a->type)) {
      const linkaddr_t *linkaddr;
      /*      memcpy(&hdrptr[byteptr], (uint8_t *)packetbuf_attr_aget(a->type), len / 8);*/
      linkaddr = packetbuf_addr(a->type);
      hdrptr[byteptr] = linkaddr->u8[0];
      hdrptr[byteptr + 1] = linkaddr->u8[1];
      
      PRINTF("%d.%d: address %d.%d\n",
	    linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	    ((uint8_t *)packetbuf_addr(a->type))[0],
	    ((uint8_t *)packetbuf_addr(a->type))[1]);
    } else {
      packetbuf_attr_t val;
      val = packetbuf_attr(a->type);
      memcpy(&hdrptr[byteptr], &val, len / 8);
      PRINTF("%d.%d: value %d\n",
	    linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	    val);
    }
    byteptr += len / 8;
  }

  return 1; /* Send out packet */
}
/*---------------------------------------------------------------------------*/
static int
hdrsize(const struct packetbuf_attrlist *a)
{
  int size, len;
  
  /* Compute the total size of the final header by summing the size of
     all attributes that are used on this channel. */
  
  size = 0;
  for(; a->type != PACKETBUF_ATTR_NONE; ++a) {
    /*    PRINTF("chameleon header_size: header type %d len %d\n",
	   a->type, a->len);*/
#if CHAMELEON_WITH_MAC_LINK_ADDRESSES
    if(a->type == PACKETBUF_ADDR_SENDER ||
       a->type == PACKETBUF_ADDR_RECEIVER) {
      /* Let the mac layer handle the sender and receiver */
      continue;
    }
#endif /* CHAMELEON_WITH_MAC_LINK_ADDRESSES */
    len = a->len;
    if(len < 8) {
      len = 8;
    }
    size += len;
  }
  return size / 8;
}
/*---------------------------------------------------------------------------*/
CC_CONST_FUNCTION struct chameleon_module chameleon_raw = { input, output,
							    hdrsize };
