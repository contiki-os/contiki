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
 * $Id: rimebuf.c,v 1.10 2007/11/17 18:05:21 adamdunkels Exp $
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


static u16_t buflen, bufptr;
static u8_t hdrptr;
static u8_t rimebuf[RIMEBUF_SIZE + RIMEBUF_HDR_SIZE];

static u8_t *rimebufptr;

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
}
/*---------------------------------------------------------------------------*/
int
rimebuf_copyfrom(const void *from, u16_t len)
{
  u16_t l;

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
  if(rimebuf_is_reference()) {
    memcpy(&rimebuf[RIMEBUF_HDR_SIZE], rimebuf_reference_ptr(),
	   rimebuf_datalen());
  } else {
    memcpy(&rimebuf[RIMEBUF_HDR_SIZE], &rimebuf[bufptr + RIMEBUF_HDR_SIZE],
	   rimebuf_datalen());
    bufptr = 0;
  }
}
/*---------------------------------------------------------------------------*/
int
rimebuf_copyto_hdr(u8_t *to)
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
  memcpy((u8_t *)to + RIMEBUF_HDR_SIZE - hdrptr, rimebufptr + bufptr,
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
rimebuf_set_datalen(u16_t len)
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
rimebuf_reference(void *ptr, u16_t len)
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
u16_t
rimebuf_datalen(void)
{
  return buflen;
}
/*---------------------------------------------------------------------------*/
u8_t
rimebuf_hdrlen(void)
{
  return RIMEBUF_HDR_SIZE - hdrptr;
}
/*---------------------------------------------------------------------------*/
u16_t
rimebuf_totlen(void)
{
  return rimebuf_hdrlen() + rimebuf_datalen();
}
/*---------------------------------------------------------------------------*/
/** @} */
