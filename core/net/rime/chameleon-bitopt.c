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
 */

/**
 * \file
 *         A Chameleon module that produces bit-optimized headers
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/chameleon.h"

#include "net/rime/rime.h"

#include <string.h>

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

struct bitopt_hdr {
  uint8_t channel[2];
};

#define BITOPT_HDR_SIZE 2

static const uint8_t bitmask[9] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0,
				 0xf8, 0xfc, 0xfe, 0xff };

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
/* For get_bits/set_bits functions in this file to work correctly,
 * the values contained in packetbuf_attr_t variables (uint16_t internally)
 * must be in little endian byte order.
 */
/* Write little endian 16 bit value */
static void CC_INLINE
le16_write(void *ptr, uint16_t v)
{
  uint8_t *p = (uint8_t *)ptr;
  p[0] = v & 0xff;
  p[1] = v >> 8;
}
/*---------------------------------------------------------------------------*/
/* Read little endian 16 bit value */
static uint16_t CC_INLINE
le16_read(const void *ptr)
{
  const uint8_t *p = (const uint8_t *)ptr;
  return ((uint16_t)p[1] << 8) | p[0];
}
/*---------------------------------------------------------------------------*/
static uint8_t CC_INLINE
get_bits_in_byte(uint8_t *from, int bitpos, int vallen)
{
  uint16_t shifted_val;

  shifted_val = (from[0] << 8) | from[1];

  /*  PRINTF("get_bits_in_byte: from[0] 0x%02x from[1] 0x%02x shifted_val 0x%04x, return 0x%02x vallen %d\n",
	from[0], from[1], shifted_val,
	(((shifted_val << bitpos) >> 8) & bitmask[vallen]) >> (8 - vallen),
	vallen
	);*/
  
  return (((shifted_val << bitpos) >> 8) & bitmask[vallen]) >> (8 - vallen);
}
/*---------------------------------------------------------------------------*/
void
get_bits(uint8_t *to, uint8_t *from, int bitpos, int vallen)
{
  int i, bits;
  
  
  if(vallen < 8) {
    *to = get_bits_in_byte(from, bitpos, vallen);
  } else {
    if(bitpos == 0) {
      for(i = 0; i < vallen / 8; ++i) {
	/*	PRINTF("get_bits i %d val 0x%02x\n",
		i, from[i]);*/
	to[i] = from[i];
      }
      bits = vallen & 7;
      if(bits) {
	to[i] = get_bits_in_byte(&from[i], 0, bits);
      }
    } else {
      for(i = 0; i < vallen / 8; ++i) {
	/*	PRINTF("get_bits i %d val 0x%02x bitpos %d\n",
		i, from[i], bitpos);*/
	to[i] = get_bits_in_byte(&from[i], bitpos, 8);
      }
      bits = vallen & 7;
      if(bits) {
	to[i] = get_bits_in_byte(&from[i], bitpos, bits);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
header_size(const struct packetbuf_attrlist *a)
{
  int size, len;
  
  /* Compute the total size of the final header by summing the size of
     all attributes that are used on this channel. */
  
  size = 0;
  for(; a->type != PACKETBUF_ATTR_NONE; ++a) {
#if CHAMELEON_WITH_MAC_LINK_ADDRESSES
    if(a->type == PACKETBUF_ADDR_SENDER ||
       a->type == PACKETBUF_ADDR_RECEIVER) {
      /* Let the link layer handle sender and receiver */
      continue;
    }
#endif /* CHAMELEON_WITH_MAC_LINK_ADDRESSES */
    /*    PRINTF("chameleon header_size: header type %d len %d\n",
	   a->type, a->len);*/
    len = a->len;
    /*    if(len < 8) {
      len = 8;
      }*/
    size += len;
  }
  return size;
}
/*---------------------------------------------------------------------------*/
static void CC_INLINE
set_bits_in_byte(uint8_t *target, int bitpos, uint8_t val, int vallen)
{
  unsigned short shifted_val;
  shifted_val = val << (8 - bitpos + 8 - vallen);
  /*  printf("set_bits_in_byte before target[0] 0x%02x target[1] 0x%02x shifted_val 0x%04x val 0x%02x vallen %d\n",
      target[0], target[1], shifted_val, val, vallen);*/
  target[0] |= shifted_val >> 8;
  target[1] |= shifted_val & 0xff;
}
/*---------------------------------------------------------------------------*/
void
set_bits(uint8_t *ptr, int bitpos, uint8_t *val, int vallen)
{
  int i, bits;

  /*  PRINTF("set_bits %p bitpos %d, val %p len %d\n",
      ptr, bitpos, val, vallen);*/

  if(vallen < 8) {
    set_bits_in_byte(ptr, bitpos, *val /*>> (8 - vallen)*/, vallen);
  } else {
    if(bitpos == 0) {
      for(i = 0; i < vallen / 8; ++i) {
	/*      PRINTF("set_bits i %d val %d\n",
		i, val[i]);*/
	ptr[i] = val[i];
      }
      bits = vallen & 7;
      if(bits) {
	set_bits_in_byte(&ptr[i], 0, val[i] >> (8 - bits), bits);
      }
    } else {
      for(i = 0; i < vallen / 8; ++i) {
	/*      PRINTF("set_bits i %d val %d\n",
		i, val[i]);*/
	set_bits_in_byte(&ptr[i], bitpos, val[i], 8);
      }
      bits = vallen & 7;
      if(bits) {
	set_bits_in_byte(&ptr[i], 0, val[i] >> (8 - bits + bitpos), bits);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
#if 0
static void
printbin(int n, int digits)
{
  int i;
  char output[128];
  
  for(i = 0; i < digits; ++i) {
    output[digits - i - 1] = (n & 1) + '0';
    n >>= 1;
  }
  output[i] = 0;
  
  printf(output);
}

static void
printhdr(uint8_t *hdr, int len)
{
  int i, j;

  j = 0;
  for(i = 0; i < len; ++i) {
    printbin(hdr[i], 8);
    printf(", ");
    ++j;
    if(j == 10) {
      printf("\n");
      j = 0;
    }
  }

  if(j != 0) {
    printf("\n");
  }
}
#endif
/*---------------------------------------------------------------------------*/
static int
pack_header(struct channel *c)
{
  const struct packetbuf_attrlist *a;
  int hdrbytesize;
  int byteptr, bitptr, len;
  uint8_t *hdrptr;
  struct bitopt_hdr *hdr;
  
  /* Compute the total size of the final header by summing the size of
     all attributes that are used on this channel. */

  hdrbytesize = c->hdrsize / 8 + ((c->hdrsize & 7) == 0? 0: 1);
  if(packetbuf_hdralloc(hdrbytesize + BITOPT_HDR_SIZE) == 0) {
    PRINTF("chameleon-bitopt: insufficient space for headers\n");
    return 0;
  }
  hdr = (struct bitopt_hdr *)packetbuf_hdrptr();
  hdr->channel[0] = c->channelno & 0xff;
  hdr->channel[1] = (c->channelno >> 8) & 0xff;

  hdrptr = ((uint8_t *)packetbuf_hdrptr()) + BITOPT_HDR_SIZE;
  memset(hdrptr, 0, hdrbytesize);
  
  byteptr = bitptr = 0;
  
  for(a = c->attrlist; a->type != PACKETBUF_ATTR_NONE; ++a) {
#if CHAMELEON_WITH_MAC_LINK_ADDRESSES
    if(a->type == PACKETBUF_ADDR_SENDER ||
       a->type == PACKETBUF_ADDR_RECEIVER) {
      /* Let the link layer handle sender and receiver */
      PRINTF("%d.%d: pack_header leaving sender/receiver to link layer\n");
      continue;
    }
#endif /* CHAMELEON_WITH_MAC_LINK_ADDRESSES */
    PRINTF("%d.%d: pack_header type %d, len %d, bitptr %d, ",
	   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	   a->type, a->len, bitptr);
    /*    len = (a->len & 0xf8) + ((a->len & 7) ? 8: 0);*/
    len = a->len;
    byteptr = bitptr / 8;
    if(PACKETBUF_IS_ADDR(a->type)) {
      set_bits(&hdrptr[byteptr], bitptr & 7,
	       (uint8_t *)packetbuf_addr(a->type), len);
      PRINTF("address %d.%d\n",
	    /*	    linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],*/
	    ((uint8_t *)packetbuf_addr(a->type))[0],
	    ((uint8_t *)packetbuf_addr(a->type))[1]);
    } else {
      uint8_t buffer[2];
      packetbuf_attr_t val = packetbuf_attr(a->type);
      le16_write(buffer, val);
      set_bits(&hdrptr[byteptr], bitptr & 7, buffer, len);
      PRINTF("value %d\n",
	    /*linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],*/
	    val);
    }
    /*    printhdr(hdrptr, hdrbytesize);*/
    bitptr += len;
  }
  /*  printhdr(hdrptr, hdrbytesize);*/

  return 1; /* Send out packet */
}
/*---------------------------------------------------------------------------*/
static struct channel *
unpack_header(void)
{
  const struct packetbuf_attrlist *a;
  int byteptr, bitptr, len;
  int hdrbytesize;
  uint8_t *hdrptr;
  struct bitopt_hdr *hdr;
  struct channel *c;
  

  /* The packet has a header that tells us what channel the packet is
     for. */
  hdr = (struct bitopt_hdr *)packetbuf_dataptr();
  if(packetbuf_hdrreduce(BITOPT_HDR_SIZE) == 0) {
    PRINTF("chameleon-bitopt: too short packet\n");
    return NULL;
  }
  c = channel_lookup((hdr->channel[1] << 8) + hdr->channel[0]);
  if(c == NULL) {
    PRINTF("chameleon-bitopt: input: channel %u not found\n",
           (hdr->channel[1] << 8) + hdr->channel[0]);
    return NULL;
  }

  hdrptr = packetbuf_dataptr();
  hdrbytesize = c->hdrsize / 8 + ((c->hdrsize & 7) == 0? 0: 1);
  if(packetbuf_hdrreduce(hdrbytesize) == 0) {
    PRINTF("chameleon-bitopt: too short packet\n");
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
    PRINTF("%d.%d: unpack_header type %d, len %d, bitptr %d\n",
	   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	   a->type, a->len, bitptr);
    /*    len = (a->len & 0xf8) + ((a->len & 7) ? 8: 0);*/
    len = a->len;
    byteptr = bitptr / 8;
    if(PACKETBUF_IS_ADDR(a->type)) {
      linkaddr_t addr;
      get_bits((uint8_t *)&addr, &hdrptr[byteptr], bitptr & 7, len);
      PRINTF("%d.%d: unpack_header type %d, addr %d.%d\n",
	     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	     a->type, addr.u8[0], addr.u8[1]);
      packetbuf_set_addr(a->type, &addr);
    } else {
      packetbuf_attr_t val;
      uint8_t buffer[2] = {0};
      get_bits(buffer, &hdrptr[byteptr], bitptr & 7, len);
      val = le16_read(buffer);
      packetbuf_set_attr(a->type, val);
      PRINTF("%d.%d: unpack_header type %d, val %d\n",
	     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	     a->type, val);
    }
    /*    byteptr += len / 8;*/
    bitptr += len;
  }
  return c;
}
/*---------------------------------------------------------------------------*/
CC_CONST_FUNCTION struct chameleon_module chameleon_bitopt = {
  unpack_header,
  pack_header,
  header_size
};
/*---------------------------------------------------------------------------*/
