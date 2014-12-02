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
 *         Simple linear-congruential generator generic C implementation.
 *
 * \authors
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include <stdint.h>
#include "random.h"

/* Constants must be chosen carefully to generate a full cycle sequence. */
/* These specific numeric values are sometimes referred to as BCPL in literature. */
#define RAND_MULTIPLIER 2147001325u
#define RAND_INCREMENT 715136305u

/* Defines to match API definitions in Contiki */
#define RAND_STATE_TYPE uint32_t
#define RAND_SEED_TYPE unsigned short
#define RAND_RETURN_TYPE unsigned short

/** Saved state between invocations of random_rand() */
static RAND_STATE_TYPE rand_state;

/** Initialize the PRNG.
 *
 * \note seed is only a short (likely 16 bits), but the saved state is actually 32 bits.
 */
void
random_init(RAND_SEED_TYPE seed)
{
  rand_state = seed;
  /** \todo Use CPU UUID for additional random seeding */
}

/**
 * Generate the next state and return the upper part of it.
 */
RAND_RETURN_TYPE
random_rand(void)
{
  /* LCG iteration, the compiler will know to optimize this into a Fused
   * Multiply-Add (FMA) instruction or similar.
   * GCC 4.8.3 -mcpu=cortex-m4 yields:
   *  ldr  r1, [r0, #0]
   *  movw r2, #7473  ; 0x1d31
   *  movt r2, #10912 ; 0x2aa0
   *  movw r3, #41965 ; 0xa3ed
   *  movt r3, #32760 ; 0x7ff8
   *  mla  r1, r1, r3, r2
   *  str  r1, [r0, #0]
   * where r1 contains the rand_state variable.
   */
  rand_state = rand_state * RAND_MULTIPLIER + RAND_INCREMENT;
  /* Return topmost bits, this also gets optimized by the compiler into a read
   * with offset instead of a separate shift instruction since the shift is an
   * even number of bytes. */
  /* With a power of two modulo, the top bits have a better distribution than
   * the lower bits, e.g. Bit 0 will be inverted in every iteration. */

  return (rand_state >> ((sizeof(RAND_STATE_TYPE) - sizeof(RAND_RETURN_TYPE)) * 8));
}
