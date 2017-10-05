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
 *         Header file for the ringbuf16index library
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __RINGBUF16INDEX_H__
#define __RINGBUF16INDEX_H__

#include "contiki-conf.h"

#if (PACKETBUF_CONF_ATTRS_INLINE) || defined(__GNUC__)
#define LIB_INLINES     1
#else
#define LIB_INLINES     0
#endif

struct ringbuf16index {
  uint16_t mask;
  /* These must be 8-bit quantities to avoid race conditions. */
  uint16_t put_ptr, get_ptr;
};

/**
 * \brief Initialize a ring buffer. The size must be a power of two
 * \param r Pointer to ringbuf16index
 * \param size Size of ring buffer
 */
void ringbuf16index_init(struct ringbuf16index *r, uint16_t size);

/**
 * \brief Put one element to the ring buffer
 * \param r Pointer to ringbuf16index
 * \retval 0 Failure; the ring buffer is full
 * \retval 1 Success; an element is added
 */
int ringbuf16index_put(struct ringbuf16index *r);

/**
 * \brief Check if there is space to put an element.
 * \param r Pinter to ringbuf16index
 * \retval >= 0 The index where the next element is to be added.
 * \retval -1 Failure; the ring buffer is full
 */
int ringbuf16index_peek_put(const struct ringbuf16index *r);

/**
 * \brief Check solid space size to put an element.
 * \param r Pointer to ringbuf16index
 * \retval size of solid space at put position, avail for fill
 */
unsigned ringbuf16index_put_free(const struct ringbuf16index *r);

/**
 * \brief Put size elements to the ring buffer
 * \param r Pointer to ringbuf16index
 * \param size Amount of placed items
 */
void ringbuf16index_putn(struct ringbuf16index *r, uint16_t size);

/**
 * \brief Remove the first element and return its index
 * \param r Pinter to ringbuf16index
 * \retval >= 0 The index of the first element
 * \retval -1 No element in the ring buffer
 */
int ringbuf16index_get(struct ringbuf16index *r);

/**
 * \brief Return the index of the first element which will be removed if calling
 *        ringbuf16index_get.
 * \param r Pinter to ringbuf16index
 * \retval >= 0 The index of the first element
 * \retval -1 No element in the ring buffer
 */
int ringbuf16index_peek_get(const struct ringbuf16index *r);

/**
 * \brief Return the ring buffer size
 * \param r Pinter to ringbuf16index
 * \return The size of the ring buffer
 */
#if LIB_INLINES
static inline
int ringbuf16index_size(const struct ringbuf16index *r)
{
  return r->mask + 1;
};
#else
int ringbuf16index_size(const struct ringbuf16index *r);
#endif

/**
 * \brief Return the number of elements currently in the ring buffer.
 * \param r Pinter to ringbuf16index
 * \return The number of elements in the ring buffer
 */
int ringbuf16index_elements(const struct ringbuf16index *r);

/**
 * \brief Is the ring buffer full?
 * \retval 0 Not full
 * \retval 1 Full
 */
int ringbuf16index_full(const struct ringbuf16index *r);

/**
 * \brief Is the ring buffer empty?
 * \retval 0 Not empty
 * \retval 1 Empty
 */
#if LIB_INLINES
static inline
int ringbuf16index_empty(const struct ringbuf16index *r){
    return r->get_ptr == r->put_ptr;
};
#else
int ringbuf16index_empty(const struct ringbuf16index *r);
#endif

#endif /* __RINGBUFINDEX_H__ */
