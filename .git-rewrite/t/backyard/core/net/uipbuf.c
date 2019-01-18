/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: uipbuf.c,v 1.1 2007/11/18 01:18:50 oliverschmidt Exp $
 */

#if 0 /* This whole file is #ifdef'd out - the contents are to be removed */

#include "net/uipbuf.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
void
uipbuf_setup(struct uipbuf_buffer *buf,
	     u8_t *bufptr, u16_t bufsize)
{
  buf->buffer = buf->ptr = bufptr;
  buf->bufsize = buf->left = bufsize;
}
/*---------------------------------------------------------------------------*/
u8_t
uipbuf_bufdata(struct uipbuf_buffer *buf, u16_t len,
	       u8_t **dataptr, u16_t *datalen)
{
  if(*datalen < buf->left) {
    memcpy(buf->ptr, *dataptr, *datalen);
    buf->ptr += *datalen;
    buf->left -= *datalen;
    *dataptr += *datalen;
    *datalen = 0;
    return UIPBUF_NOT_FULL;
  } else if(*datalen == buf->left) {
    memcpy(buf->ptr, *dataptr, *datalen);
    buf->ptr += *datalen;
    buf->left = 0;
    *dataptr += *datalen;
    *datalen = 0;
    return UIPBUF_FULL;
  } else {
    memcpy(buf->ptr, *dataptr, buf->left);
    buf->ptr += buf->left;
    *datalen -= buf->left;
    *dataptr += buf->left;
    buf->left = 0;
    return UIPBUF_FULL;
  }

}
/*---------------------------------------------------------------------------*/
u8_t
uipbuf_bufto(CC_REGISTER_ARG struct uipbuf_buffer *buf, u8_t endmarker,
	     CC_REGISTER_ARG u8_t **dataptr, CC_REGISTER_ARG u16_t *datalen)
{
  u8_t c;
  /*
  int len;

  ptr = memchr(*dataptr, endmarker, *datalen);
  if(ptr != NULL) {
    len = ptr - *dataptr;
  } else {
    len = *datalen;
  }
  memcpy(buf->ptr, *dataptr, len);
  *dataptr += len;
  *datalen -= len;
  buf->ptr += len;
  */
  while(buf->left > 0 && *datalen > 0) {
    c = *buf->ptr = **dataptr;
    ++*dataptr;
    ++buf->ptr;
    --*datalen;
    --buf->left;
    
    if(c == endmarker) {
      return UIPBUF_FOUND;
    }
  }

  if(*datalen == 0) {
    return UIPBUF_NOT_FOUND;
  }

  while(*datalen > 0) {
    c = **dataptr;
    --*datalen;
    ++*dataptr;
    
    if(c == endmarker) {
      return UIPBUF_FOUND | UIPBUF_FULL;
    }
  }
  
  return UIPBUF_FULL;
}
/*----------------------------------------------------------------------------*/
u16_t
uipbuf_len(struct uipbuf_buffer *buf)
{
  return buf->bufsize - buf->left;
}
/*----------------------------------------------------------------------------*/
#endif /* 0 */
