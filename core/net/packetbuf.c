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
 */

/**
 * \file
 *         Rime buffer (packetbuf) management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup packetbuf
 * @{
 */

#include <string.h>

#include "contiki-net.h"
#include "net/packetbuf.h"
#include "net/rime/rime.h"
#include "sys/cc.h"

struct packetbuf_attr packetbuf_attrs[PACKETBUF_NUM_ATTRS];
struct packetbuf_addr packetbuf_addrs[PACKETBUF_NUM_ADDRS];


static uint16_t buflen, bufptr;
static uint8_t hdrlen;

/* The declarations below ensure that the packet buffer is aligned on
   an even 32-bit boundary. On some platforms (most notably the
   msp430 or OpenRISC), having a potentially misaligned packet buffer may lead to
   problems when accessing words. */
static uint32_t packetbuf_aligned[(PACKETBUF_SIZE + 3) / 4];
static uint8_t *packetbuf = (uint8_t *)packetbuf_aligned;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
packetbuf_clear(void)
{
  buflen = bufptr = 0;
  hdrlen = 0;

  packetbuf_attr_clear();
}
/*---------------------------------------------------------------------------*/
int
packetbuf_copyfrom(const void *from, uint16_t len)
{
  uint16_t l;

  packetbuf_clear();
  l = MIN(PACKETBUF_SIZE, len);
  memcpy(packetbuf, from, l);
  buflen = l;
  return l;
}
/*---------------------------------------------------------------------------*/
void
packetbuf_compact(void)
{
  int16_t i;

  if(bufptr) {
    /* shift data to the left */
    for(i = 0; i < buflen; i++) {
      packetbuf[hdrlen + i] = packetbuf[packetbuf_hdrlen() + i];
    }
    bufptr = 0;
  }
}
/*---------------------------------------------------------------------------*/
int
packetbuf_copyto(void *to)
{
  if(hdrlen + buflen > PACKETBUF_SIZE) {
    return 0;
  }
  memcpy(to, packetbuf_hdrptr(), hdrlen);
  memcpy((uint8_t *)to + hdrlen, packetbuf_dataptr(), buflen);
  return hdrlen + buflen;
}
/*---------------------------------------------------------------------------*/
int
packetbuf_hdralloc(int size)
{
  int16_t i;

  if(size + packetbuf_totlen() > PACKETBUF_SIZE) {
    return 0;
  }

  /* shift data to the right */
  for(i = packetbuf_totlen() - 1; i >= 0; i--) {
    packetbuf[i + size] = packetbuf[i];
  }
  hdrlen += size;
  return 1;
}
/*---------------------------------------------------------------------------*/
int
packetbuf_hdrreduce(int size)
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
packetbuf_set_datalen(uint16_t len)
{
  PRINTF("packetbuf_set_len: len %d\n", len);
  buflen = len;
}
/*---------------------------------------------------------------------------*/
void *
packetbuf_dataptr(void)
{
  return packetbuf + packetbuf_hdrlen();
}
/*---------------------------------------------------------------------------*/
void *
packetbuf_hdrptr(void)
{
  return packetbuf;
}
/*---------------------------------------------------------------------------*/
uint16_t
packetbuf_datalen(void)
{
  return buflen;
}
/*---------------------------------------------------------------------------*/
uint8_t
packetbuf_hdrlen(void)
{
  return bufptr + hdrlen;
}
/*---------------------------------------------------------------------------*/
uint16_t
packetbuf_totlen(void)
{
  return packetbuf_hdrlen() + packetbuf_datalen();
}
/*---------------------------------------------------------------------------*/
void
packetbuf_attr_clear(void)
{
  int i;
  memset(packetbuf_attrs, 0, sizeof(packetbuf_attrs));
  for(i = 0; i < PACKETBUF_NUM_ADDRS; ++i) {
    linkaddr_copy(&packetbuf_addrs[i].addr, &linkaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
void
packetbuf_attr_copyto(struct packetbuf_attr *attrs,
		    struct packetbuf_addr *addrs)
{
  memcpy(attrs, packetbuf_attrs, sizeof(packetbuf_attrs));
  memcpy(addrs, packetbuf_addrs, sizeof(packetbuf_addrs));
}
/*---------------------------------------------------------------------------*/
void
packetbuf_attr_copyfrom(struct packetbuf_attr *attrs,
		      struct packetbuf_addr *addrs)
{
  memcpy(packetbuf_attrs, attrs, sizeof(packetbuf_attrs));
  memcpy(packetbuf_addrs, addrs, sizeof(packetbuf_addrs));
}
/*---------------------------------------------------------------------------*/
#if !PACKETBUF_CONF_ATTRS_INLINE
int
packetbuf_set_attr(uint8_t type, const packetbuf_attr_t val)
{
  packetbuf_attrs[type].val = val;
  return 1;
}
/*---------------------------------------------------------------------------*/
packetbuf_attr_t
packetbuf_attr(uint8_t type)
{
  return packetbuf_attrs[type].val;
}
/*---------------------------------------------------------------------------*/
int
packetbuf_set_addr(uint8_t type, const linkaddr_t *addr)
{
  linkaddr_copy(&packetbuf_addrs[type - PACKETBUF_ADDR_FIRST].addr, addr);
  return 1;
}
/*---------------------------------------------------------------------------*/
const linkaddr_t *
packetbuf_addr(uint8_t type)
{
  return &packetbuf_addrs[type - PACKETBUF_ADDR_FIRST].addr;
}
/*---------------------------------------------------------------------------*/
#endif /* PACKETBUF_CONF_ATTRS_INLINE */
int
packetbuf_holds_broadcast(void)
{
  return linkaddr_cmp(&packetbuf_addrs[PACKETBUF_ADDR_RECEIVER - PACKETBUF_ADDR_FIRST].addr, &linkaddr_null);
}
/*---------------------------------------------------------------------------*/

/** @} */
