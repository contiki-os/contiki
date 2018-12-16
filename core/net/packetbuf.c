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

struct packetbuf temp;
struct packetbuf *packetbuf = &temp;

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
  packetbuf->datalen = 0;
  packetbuf->bufptr = 0;
  packetbuf->hdrlen = 0;
  packetbuf_attr_clear();
}
/*---------------------------------------------------------------------------*/
int
packetbuf_copyfrom(const void *from, uint16_t len)
{
  uint16_t l;

  packetbuf_clear();
  l = MIN(PACKETBUF_SIZE, len);
  memcpy(packetbuf->data, from, l);
  packetbuf->datalen = l;
  return l;
}
/*---------------------------------------------------------------------------*/
void
packetbuf_compact(void)
{
  int16_t i;

  if(packetbuf->bufptr) {
    /* shift data to the left */
    for(i = 0; i < packetbuf->datalen; i++) {
      packetbuf->data[packetbuf->hdrlen + i] = packetbuf->data[packetbuf_hdrlen() + i];
    }
    packetbuf->bufptr = 0;
  }
}
/*---------------------------------------------------------------------------*/
int
packetbuf_copyto(void *to)
{
  if(packetbuf->hdrlen + packetbuf->datalen > PACKETBUF_SIZE) {
    return 0;
  }
  memcpy(to, packetbuf_hdrptr(), packetbuf->hdrlen);
  memcpy((uint8_t *)to + packetbuf->hdrlen, packetbuf_dataptr(), packetbuf->datalen);
  return packetbuf->hdrlen + packetbuf->datalen;
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
    packetbuf->data[i + size] = packetbuf->data[i];
  }
  packetbuf->hdrlen += size;
  return 1;
}
/*---------------------------------------------------------------------------*/
int
packetbuf_hdrreduce(int size)
{
  if(size > packetbuf->datalen) {
    return 0;
  }

  packetbuf->bufptr += size;
  packetbuf->datalen -= size;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
packetbuf_set_datalen(uint16_t len)
{
  PRINTF("packetbuf_set_len: len %d\n", len);
  packetbuf->datalen = len;
}
/*---------------------------------------------------------------------------*/
void *
packetbuf_dataptr(void)
{
  return packetbuf->data + packetbuf_hdrlen();
}
/*---------------------------------------------------------------------------*/
void *
packetbuf_hdrptr(void)
{
  return packetbuf->data;
}
/*---------------------------------------------------------------------------*/
uint16_t
packetbuf_datalen(void)
{
  return packetbuf->datalen;
}
/*---------------------------------------------------------------------------*/
uint8_t
packetbuf_hdrlen(void)
{
  return packetbuf->bufptr + packetbuf->hdrlen;
}
/*---------------------------------------------------------------------------*/
uint16_t
packetbuf_totlen(void)
{
  return packetbuf_hdrlen() + packetbuf_datalen();
}
/*---------------------------------------------------------------------------*/
uint16_t
packetbuf_remaininglen(void)
{
  return PACKETBUF_SIZE - packetbuf_totlen();
}
/*---------------------------------------------------------------------------*/
void
packetbuf_attr_clear(void)
{
  int i;
  memset(packetbuf->attrs, 0, sizeof(packetbuf->attrs));
  for(i = 0; i < PACKETBUF_NUM_ADDRS; ++i) {
    linkaddr_copy(&packetbuf->addrs[i].addr, &linkaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
void
packetbuf_attr_copyto(struct packetbuf_attr *attrs,
                      struct packetbuf_addr *addrs)
{
  memcpy(attrs, packetbuf->attrs, sizeof(packetbuf->attrs));
  memcpy(addrs, packetbuf->addrs, sizeof(packetbuf->addrs));
}
/*---------------------------------------------------------------------------*/
void
packetbuf_attr_copyfrom(struct packetbuf_attr *attrs,
                        struct packetbuf_addr *addrs)
{
  memcpy(packetbuf->attrs, attrs, sizeof(packetbuf->attrs));
  memcpy(packetbuf->addrs, addrs, sizeof(packetbuf->addrs));
}
/*---------------------------------------------------------------------------*/
#if !PACKETBUF_CONF_ATTRS_INLINE
int
packetbuf_set_attr(uint8_t type, const packetbuf_attr_t val)
{
  packetbuf->attrs[type].val = val;
  return 1;
}
/*---------------------------------------------------------------------------*/
packetbuf_attr_t
packetbuf_attr(uint8_t type)
{
  return packetbuf->attrs[type].val;
}
/*---------------------------------------------------------------------------*/
int
packetbuf_set_addr(uint8_t type, const linkaddr_t *addr)
{
  linkaddr_copy(&packetbuf->addrs[type - PACKETBUF_ADDR_FIRST].addr, addr);
  return 1;
}
/*---------------------------------------------------------------------------*/
const linkaddr_t *
packetbuf_addr(uint8_t type)
{
  return &packetbuf->addrs[type - PACKETBUF_ADDR_FIRST].addr;
}
/*---------------------------------------------------------------------------*/
#endif /* PACKETBUF_CONF_ATTRS_INLINE */
int
packetbuf_holds_broadcast(void)
{
  return linkaddr_cmp(&packetbuf->addrs[PACKETBUF_ADDR_RECEIVER - PACKETBUF_ADDR_FIRST].addr, &linkaddr_null);
}
/*---------------------------------------------------------------------------*/

/** @} */
