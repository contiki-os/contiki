/**
 * \addtogroup rimebuf
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
 * $Id: rimebuf.c,v 1.16 2008/06/30 19:57:10 adamdunkels Exp $
 */

/**
 * \file
 *         Rime buffer (rimebuf) management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <string.h>

#include "contiki-net.h"
#include "net/rime/rimebuf.h"
#include "net/rime.h"

struct rimebuf_attr rimebuf_attrs[RIMEBUF_NUM_ATTRS];
struct rimebuf_addr rimebuf_addrs[RIMEBUF_NUM_ADDRS];

const char *rimebuf_attr_strings[] =
  {
    "RIMEBUF_ATTR_NONE",
    "RIMEBUF_ATTR_CHANNEL",
    "RIMEBUF_ATTR_PACKET_ID",
    "RIMEBUF_ATTR_PACKET_TYPE",
    "RIMEBUF_ATTR_EPACKET_ID",
    "RIMEBUF_ATTR_EPACKET_TYPE",
    "RIMEBUF_ATTR_HOPS",
    "RIMEBUF_ATTR_TTL",
    "RIMEBUF_ATTR_REXMIT",
    "RIMEBUF_ATTR_MAX_REXMIT",
    "RIMEBUF_ATTR_NUM_REXMIT",
    "RIMEBUF_ATTR_LINK_QUALITY",
    "RIMEBUF_ATTR_RSSI",
    "RIMEBUF_ATTR_TIMESTAMP",
    "RIMEBUF_ATTR_NETWORK_ID",

    "RIMEBUF_ATTR_RELIABLE",
    "RIMEBUF_ATTR_ERELIABLE",

    "RIMEBUF_ADDR_SENDER",
    "RIMEBUF_ADDR_RECEIVER",
    "RIMEBUF_ADDR_ESENDER",
    "RIMEBUF_ADDR_ERECEIVER",

    "RIMEBUF_ATTR_MAX",
  };

static uint16_t buflen, bufptr;
static uint8_t hdrptr;
static uint8_t rimebuf[RIMEBUF_SIZE + RIMEBUF_HDR_SIZE];

static uint8_t *rimebufptr;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
rimebuf_clear(void)
{
  buflen = bufptr = 0;
  hdrptr = RIMEBUF_HDR_SIZE;

  rimebufptr = &rimebuf[RIMEBUF_HDR_SIZE];
  rimebuf_attr_clear();
}
/*---------------------------------------------------------------------------*/
int
rimebuf_copyfrom(const void *from, uint16_t len)
{
  uint16_t l;

  rimebuf_clear();
  l = len > RIMEBUF_SIZE? RIMEBUF_SIZE: len;
  memcpy(rimebufptr, from, l);
  buflen = l;
  return l;
}
/*---------------------------------------------------------------------------*/
void
rimebuf_compact(void)
{
  int i, len;

  if(rimebuf_is_reference()) {
    memcpy(&rimebuf[RIMEBUF_HDR_SIZE], rimebuf_reference_ptr(),
	   rimebuf_datalen());
  } else if (bufptr > 0) {
    len = rimebuf_datalen() + RIMEBUF_HDR_SIZE;
    for (i = RIMEBUF_HDR_SIZE; i < len; i++) {
      rimebuf[i] = rimebuf[bufptr + i];
    }

    bufptr = 0;
  }
}
/*---------------------------------------------------------------------------*/
int
rimebuf_copyto_hdr(uint8_t *to)
{
#if DEBUG_LEVEL > 0
  {
    int i;
    PRINTF("rimebuf_write_hdr: header:\n");
    for(i = hdrptr; i < RIMEBUF_HDR_SIZE; ++i) {
      PRINTF("0x%02x, ", rimebuf[i]);
    }
    PRINTF("\n");
  }
#endif /* DEBUG_LEVEL */
  memcpy(to, rimebuf + hdrptr, RIMEBUF_HDR_SIZE - hdrptr);
  return RIMEBUF_HDR_SIZE - hdrptr;
}
/*---------------------------------------------------------------------------*/
int
rimebuf_copyto(void *to)
{
#if DEBUG_LEVEL > 0
  {
    int i;
    char buffer[1000];
    char *bufferptr = buffer;
    
    bufferptr[0] = 0;
    for(i = hdrptr; i < RIMEBUF_HDR_SIZE; ++i) {
      bufferptr += sprintf(bufferptr, "0x%02x, ", rimebuf[i]);
    }
    PRINTF("rimebuf_write: header: %s\n", buffer);
    bufferptr = buffer;
    bufferptr[0] = 0;
    for(i = bufptr; i < buflen + bufptr; ++i) {
      bufferptr += sprintf(bufferptr, "0x%02x, ", rimebufptr[i]);
    }
    PRINTF("rimebuf_write: data: %s\n", buffer);
  }
#endif /* DEBUG_LEVEL */
  memcpy(to, rimebuf + hdrptr, RIMEBUF_HDR_SIZE - hdrptr);
  memcpy((uint8_t *)to + RIMEBUF_HDR_SIZE - hdrptr, rimebufptr + bufptr,
	 buflen);
  return RIMEBUF_HDR_SIZE - hdrptr + buflen;
}
/*---------------------------------------------------------------------------*/
int
rimebuf_hdralloc(int size)
{
  if(hdrptr > size) {
    hdrptr -= size;
    return 1;
  }
  hdrptr = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
int
rimebuf_hdrreduce(int size)
{
  if(buflen < size) {
    return 0;
  }
  
  bufptr += size;
  buflen -= size;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
rimebuf_set_datalen(uint16_t len)
{
  PRINTF("rimebuf_set_len: len %d\n", len);
  buflen = len;
}
/*---------------------------------------------------------------------------*/
void *
rimebuf_dataptr(void)
{
  return (void *)(&rimebuf[bufptr + RIMEBUF_HDR_SIZE]);
}
/*---------------------------------------------------------------------------*/
void *
rimebuf_hdrptr(void)
{
  return (void *)(&rimebuf[hdrptr]);
}
/*---------------------------------------------------------------------------*/
void
rimebuf_reference(void *ptr, uint16_t len)
{
  rimebuf_clear();
  rimebufptr = ptr;
  buflen = len;
}
/*---------------------------------------------------------------------------*/
int
rimebuf_is_reference(void)
{
  return rimebufptr != &rimebuf[RIMEBUF_HDR_SIZE];
}
/*---------------------------------------------------------------------------*/
void *
rimebuf_reference_ptr(void)
{
  return rimebufptr;
}
/*---------------------------------------------------------------------------*/
uint16_t
rimebuf_datalen(void)
{
  return buflen;
}
/*---------------------------------------------------------------------------*/
uint8_t
rimebuf_hdrlen(void)
{
  return RIMEBUF_HDR_SIZE - hdrptr;
}
/*---------------------------------------------------------------------------*/
uint16_t
rimebuf_totlen(void)
{
  return rimebuf_hdrlen() + rimebuf_datalen();
}
/*---------------------------------------------------------------------------*/



void
rimebuf_attr_clear(void)
{
  int i;
  for(i = 0; i < RIMEBUF_NUM_ATTRS; ++i) {
/*     rimebuf_attrs[i].type = RIMEBUF_ATTR_NONE; */
    rimebuf_attrs[i].val = 0;
  }
  for(i = 0; i < RIMEBUF_NUM_ADDRS; ++i) {
/*     rimebuf_addrs[i].type = RIMEBUF_ATTR_NONE; */
    rimeaddr_copy(&rimebuf_addrs[i].addr, &rimeaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
void
rimebuf_attr_copyto(struct rimebuf_attr *attrs,
		    struct rimebuf_addr *addrs)
{
  memcpy(attrs, rimebuf_attrs, sizeof(rimebuf_attrs));
  memcpy(addrs, rimebuf_addrs, sizeof(rimebuf_addrs));
}
/*---------------------------------------------------------------------------*/
void
rimebuf_attr_copyfrom(struct rimebuf_attr *attrs,
		      struct rimebuf_addr *addrs)
{
  memcpy(rimebuf_attrs, attrs, sizeof(rimebuf_attrs));
  memcpy(rimebuf_addrs, addrs, sizeof(rimebuf_addrs));
}
/*---------------------------------------------------------------------------*/
#if !RIMEBUF_CONF_ATTRS_INLINE
int
rimebuf_set_attr(uint8_t type, const rimebuf_attr_t val)
{
/*   rimebuf_attrs[type].type = type; */
  rimebuf_attrs[type].val = val;
  return 1;
}
/*---------------------------------------------------------------------------*/
rimebuf_attr_t
rimebuf_attr(uint8_t type)
{
  return rimebuf_attrs[type].val;
}
/*---------------------------------------------------------------------------*/
int
rimebuf_set_addr(uint8_t type, const rimeaddr_t *addr)
{
/*   rimebuf_addrs[type - RIMEBUF_ADDR_FIRST].type = type; */
  rimeaddr_copy(&rimebuf_addrs[type - RIMEBUF_ADDR_FIRST].addr, addr);
  return 1;
}
/*---------------------------------------------------------------------------*/
const rimeaddr_t *
rimebuf_addr(uint8_t type)
{
  return &rimebuf_addrs[type - RIMEBUF_ADDR_FIRST].addr;
}
/*---------------------------------------------------------------------------*/
#endif /* RIMEBUF_CONF_ATTRS_INLINE */
/** @} */
