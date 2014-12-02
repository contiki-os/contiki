/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Synchronization primitives for Cortex-M3/M4 processors.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef CORTEX_M_SYNCHRONIZATION_H_
#define CORTEX_M_SYNCHRONIZATION_H_

#include <stdint.h>
#include "core_cmInstr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * See also:
 *  ARM Application Note 321: ARM Cortex-M Programming Guide to Memory Barrier Instructions
 *   4.19. Semaphores and Mutual Exclusives (Mutex) - unicore and multicore
 */

/** Lock variable typedef */
typedef volatile uint8_t lock_t;

/** Blocking access to lock variable */
void lock_acquire(lock_t *Lock_Variable);

/** Non-blocking access to lock variable */
int lock_try_acquire(lock_t *Lock_Variable);

/** Release a lock after having acquired it using lock_acquire or lock_try_acquire. */
void lock_release(lock_t *Lock_Variable);

/** Safely increment a counter variable, without race condition issues regarding
 * the read-modify-write sequence. */
static inline void exclusive_increment(volatile uint32_t* value) {
  int status;
  int tmp;
  do {
    /* Load exclusive */
    tmp = __LDREXW(value);

    /* increment counter */
    ++tmp;

    /* Try to write the new value */
    status = __STREXW(tmp, value);
  } while (status != 0); /* retry until load-store cycle was exclusive. */
}

/** Safely decrement a counter variable, without race condition issues regarding
 * the read-modify-write sequence. */
static inline void exclusive_decrement(volatile uint32_t* value) {
  int status;
  int tmp;
  do {
    /* Load exclusive */
    tmp = __LDREXW(value);

    /* decrement counter */
    --tmp;

    /* Try to write the new value */
    status = __STREXW(tmp, value);
  } while (status != 0); /* retry until load-store cycle was exclusive. */
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !defined(CORTEX_M_SYNCHRONIZATION_H_) */
