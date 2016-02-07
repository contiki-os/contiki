/*
 * Copyright (c) 2014, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author: Andreas Dröscher <contiki@anticat.ch>
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS "AS IS" AND
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
 */
/**
 * \addtogroup lib
 * @{
 *
 * \defgroup newlib Generic Newlib customizations
 *
 * Library providing generic implementations of Newlib features for Contiki
 * @{
 *
 * \file
 * System calls
 */
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/**
 * \brief Enlarges the allocated heap space
 * \param incr Number of bytes by which to increase the heap space
 * \return The previous end of heap on success (which is also a pointer to the
 *         start of the newly allocated memory if \p incr is positive), or
 *         <tt>(caddr_t)-1</tt> with \c errno set to \c ENOMEM on error
 */
caddr_t
_sbrk(int incr)
{
  /*
   * Newlib's _sbrk_r() assumes that this global errno variable is used here,
   * which is different from the errno definition provided by <errno.h>.
   */
#undef errno
  extern int errno;

  /* Heap boundaries from linker script. */
  extern uint8_t _heap;
  extern uint8_t _eheap;

  static uint8_t *heap_end = &_heap;
  uint8_t *prev_heap_end = heap_end;

  if(heap_end + incr > &_eheap) {
    PRINTF("Out of heap space!\n");
    errno = ENOMEM;
    return (caddr_t)-1;
  }

  heap_end += incr;
  return (caddr_t)prev_heap_end;
}

/**
 * @}
 * @}
 */
