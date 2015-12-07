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
 *         Header file for the ringbufindex library
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __RINGBUFINDEX_H__
#define __RINGBUFINDEX_H__

#include "contiki-conf.h"

struct ringbufindex {
  uint8_t mask;
  /* These must be 8-bit quantities to avoid race conditions. */
  uint8_t put_ptr, get_ptr;
};

/* Initialize a ring buffer. The size must be a power of two */
void ringbufindex_init(struct ringbufindex *r, uint8_t size);
/* Put one element to the ring buffer */
int ringbufindex_put(struct ringbufindex *r);
/* Check if there is space to put an element.
 * Return the index where the next element is to be added */
int ringbufindex_peek_put(const struct ringbufindex *r);
/* Remove the first element and return its index */
int ringbufindex_get(struct ringbufindex *r);
/* Return the index of the first element
 * (which will be removed if calling ringbufindex_peek) */
int ringbufindex_peek_get(const struct ringbufindex *r);
/* Return the ring buffer size */
int ringbufindex_size(const struct ringbufindex *r);
/* Return the number of elements currently in the ring buffer */
int ringbufindex_elements(const struct ringbufindex *r);
/* Is the ring buffer full? */
int ringbufindex_full(const struct ringbufindex *r);
/* Is the ring buffer empty? */
int ringbufindex_empty(const struct ringbufindex *r);

#endif /* __RINGBUFINDEX_H__ */
