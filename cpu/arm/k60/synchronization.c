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
 *         Implementation of synchronization primitives for Cortex-M3/M4 processors.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "K60.h"
#include "synchronization.h"

/*
 * See also:
 *  ARM Application Note 321: ARM Cortex-M Programming Guide to Memory Barrier Instructions
 *   4.19. Semaphores and Mutual Exclusives (Mutex) - unicore and multicore
 */

/** Blocking access to lock variable */
void lock_acquire(lock_t *Lock_Variable)
{
  int status = 0;
  do {
    /* Wait until Lock_Variable is free */
    while (__LDREXB(Lock_Variable) != 0);

    /* Try to set Lock_Variable */
    status = __STREXB(1, Lock_Variable);
  } while (status != 0); /* retry until lock successfully */

  /* Do not start any other memory access until memory barrier is completed */
  __DMB();

  return;
}

/** Non-blocking access to lock variable */
int lock_try_acquire(lock_t *Lock_Variable)
{
  int status = 0;
  if (__LDREXB(Lock_Variable) != 0) {
    /* Lock_Variable is busy */
    return -1;
  }
  status = __STREXB(1, Lock_Variable); /* Try to set Lock_Variable */
  if (status != 0) {
    /* Locking failed, someone else modified the Lock_Variable before we could. */
    return -2;
  }

  /* Do not start any other memory access until memory barrier is completed */
  __DMB();

  /* Lock successful */
  return 0;
}

/** Release a lock after having acquired it using lock_acquire or lock_try_acquire. */
void lock_release(lock_t *Lock_Variable)
{
  __DMB(); /* Ensure memory operations completed before releasing lock */
  (*Lock_Variable) = 0;
  return;
}

/** \todo Implement counting semaphores for Cortex-M */
