/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 *         ringbufindex library. Implements basic support for ring buffers
 *         of any type, as opposed to the core/lib/ringbuf module which
 *         is only for byte arrays. Simply returns index in the ringbuf
 *         rather than actual elements. The ringbuf size must be power of two.
 *         Like the original ringbuf, this module implements atomic put and get.
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *         based on Contiki's core/lib/ringbuf library by Adam Dunkels
 */

#include <string.h>
#include "lib/ringbufindex.h"

/* Initialize a ring buffer. The size must be a power of two */
void
ringbufindex_init(struct ringbufindex *r, uint8_t size)
{
  r->mask = size - 1;
  r->put_ptr = 0;
  r->get_ptr = 0;
}
/* Put one element to the ring buffer */
int
ringbufindex_put(struct ringbufindex *r)
{
  /* Check if buffer is full. If it is full, return 0 to indicate that
     the element was not inserted.

     XXX: there is a potential risk for a race condition here, because
     the ->get_ptr field may be written concurrently by the
     ringbufindex_get() function. To avoid this, access to ->get_ptr must
     be atomic. We use an uint8_t type, which makes access atomic on
     most platforms, but C does not guarantee this.
   */
  if(((r->put_ptr - r->get_ptr) & r->mask) == r->mask) {
    return 0;
  }
  r->put_ptr = (r->put_ptr + 1) & r->mask;
  return 1;
}
/* Check if there is space to put an element.
 * Return the index where the next element is to be added */
int
ringbufindex_peek_put(const struct ringbufindex *r)
{
  /* Check if there are bytes in the buffer. If so, we return the
     first one. If there are no bytes left, we return -1.
   */
  if(((r->put_ptr - r->get_ptr) & r->mask) == r->mask) {
    return -1;
  }
  return (r->put_ptr + 1) & r->mask;
}
/* Remove the first element and return its index */
int
ringbufindex_get(struct ringbufindex *r)
{
  int get_ptr;

  /* Check if there are bytes in the buffer. If so, we return the
     first one and increase the pointer. If there are no bytes left, we
     return -1.

     XXX: there is a potential risk for a race condition here, because
     the ->put_ptr field may be written concurrently by the
     ringbufindex_put() function. To avoid this, access to ->get_ptr must
     be atomic. We use an uint8_t type, which makes access atomic on
     most platforms, but C does not guarantee this.
   */
  if(((r->put_ptr - r->get_ptr) & r->mask) > 0) {
    get_ptr = r->get_ptr;
    r->get_ptr = (r->get_ptr + 1) & r->mask;
    return get_ptr;
  } else {
    return -1;
  }
}
/* Return the index of the first element
 * (which will be removed if calling ringbufindex_peek) */
int
ringbufindex_peek_get(const struct ringbufindex *r)
{
  /* Check if there are bytes in the buffer. If so, we return the
     first one. If there are no bytes left, we return -1.
   */
  if(((r->put_ptr - r->get_ptr) & r->mask) > 0) {
    return (r->get_ptr + 1) & r->mask;
  } else {
    return -1;
  }
}
/* Return the ring buffer size */
int
ringbufindex_size(const struct ringbufindex *r)
{
  return r->mask + 1;
}
/* Return the number of elements currently in the ring buffer */
int
ringbufindex_elements(const struct ringbufindex *r)
{
  return (r->put_ptr - r->get_ptr) & r->mask;
}
/* Is the ring buffer full? */
int
ringbufindex_full(const struct ringbufindex *r)
{
  return ((r->put_ptr - r->get_ptr) & r->mask) == r->mask;
}
/* Is the ring buffer empty? */
int
ringbufindex_empty(const struct ringbufindex *r)
{
  return ringbufindex_elements(r) == 0;
}
