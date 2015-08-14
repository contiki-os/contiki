/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         Header file for the ring buffer index library
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

/** \addtogroup lib
 * @{ */

/**
 * \defgroup ringbuf Ring buffer index library
 * @{
 *
 */

#ifndef __RINGBUFINDEX_H__
#define __RINGBUFINDEX_H__

#include "contiki-conf.h"

/**
 * \brief      Structure that holds the state of a ring buffer.
 *
 *             This structure holds the state of a ring buffer. The
 *             actual buffer needs to be defined separately. This
 *             struct is an opaque structure with no user-visible
 *             elements.
 *
 */
struct ringbufindex {
  uint8_t mask;
  
  /* XXX these must be 8-bit quantities to avoid race conditions. */
  uint8_t put_ptr, get_ptr;
};

/**
 * \brief      Initialize a ring buffer
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \param size_power_of_two The size of the ring buffer, which must be a power of two
 *
 *
 */
void    ringbufindex_init(struct ringbufindex *r,
		     uint8_t size_power_of_two);

/**
 * \brief      Insert a byte into the ring buffer
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     Non-zero if there data could be written, or zero if the buffer was full.
 *
 *             This function inserts a byte into the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int     ringbufindex_put(struct ringbufindex *r);

/**
 * \brief      Get the index of the next put
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     Non-zero if there data could be written, or zero if the buffer was full.
 *
 *             This function inserts a byte into the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int16_t     ringbufindex_peek_put(const struct ringbufindex *r);

/**
 * \brief      Remove and get an element from the ring buffer
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     The index in the buffer, or -1 if the buffer was empty
 *
 *             This function removes a byte from the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int16_t     ringbufindex_get(struct ringbufindex *r);

/**
 * \brief      Get the next index
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     The index in the buffer, or -1 if the buffer was empty
 *
 *             It is safe to call this function from an interrupt
 *             handler.
 *
 */
int16_t     ringbufindex_peek_get(const struct ringbufindex *r);

/**
 * \brief      Get the size of a ring buffer
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     The size of the buffer.
 */
int     ringbufindex_size(const struct ringbufindex *r);

/**
 * \brief      Get the number of elements currently in the ring buffer
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     The number of elements in the buffer.
 */
int     ringbufindex_elements(const struct ringbufindex *r);

/**
 * \brief      Is the ring buffer full?
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     1 if full 0 otherwise
 */
int     ringbufindex_full(const struct ringbufindex *r);

/**
 * \brief      Is the ring buffer empty?
 * \param r    A pointer to a struct ringbufindex to hold the state of the ring buffer
 * \return     1 if empty 0 otherwise
 */
int     ringbufindex_empty(const struct ringbufindex *r);

#endif /* __RINGBUFINDEX_H__ */

/** @}*/
/** @}*/
