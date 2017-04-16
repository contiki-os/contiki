/*
 * Copyright (c) 2012, Alex Barclay.
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
 *
 *
 * Author: Alex Barclay <alex@planet-barclay.com>
 *
 */

#include "freebuf.h"

/* Freebuf library
 * Each element must be larger than a void*
 * When an element is on the freelist it will have a void* stored at the beginning of the buffer that points to the next buffer
 * There is no overhead
 */
void
freebuf_init(void **freelist, void *memAddr, size_t elemSize, int numElem)
{
  *freelist = memAddr;
  void **t = (void **)memAddr;
  while(--numElem) {
    *t = (uint8_t *)t + elemSize; /* Will be where the next element starts */
    t = *t;
  }
  /* Last element, zero the pointer to indicate the end of the list */
  *t = 0;
}
void *
freebuf_pop(void **freelist)
{
  void *r = *freelist;
  if(!r) {
    return 0; /* Shortcut a null return if the list is empty */
  }
  *freelist = *(void **)r;
  return r;
}
void
freebuf_push(void **freelist, void *e)
{
  void **t = (void **)e;
  *t = *freelist;
  *freelist = e;
}
