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
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-ecdh-test cc2538 ECDH Test Project
 *
 *   ECDH example for CC2538-based platforms
 *
 *   This example shows how ECDH should be used. The example also verifies
 *   the ECDH functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating ECDH
 */
#include "contiki.h"
#include "dev/ecc-algorithm.h"
#include "dev/ecc-curve.h"
#include "lib/random.h"
#include "sys/rtimer.h"
#include "sys/pt.h"

#include <string.h>
#include <stdio.h>

static void
ecc_set_random(uint32_t *secret)
{
  int i;
  for(i = 0; i < 8; ++i) {
    secret[i] = (uint32_t)random_rand() | (uint32_t)random_rand() << 16;
  }
}

PROCESS(ecdh_test, "ecdh test");
AUTOSTART_PROCESSES(&ecdh_test);

PROCESS_THREAD(ecdh_test, ev, data) {
  PROCESS_BEGIN();

  /*
   * Variable for Time Measurement
   */
  static rtimer_clock_t time;

  /*
   * Activate Engine
   */
  puts("-----------------------------------------\n"
       "Initializing pka...");
  pka_init();

  /*
   * Generate secrets make sure they are valid (smaller as order)
   */
  static ecc_compare_state_t state = {
    .process = &ecdh_test,
    .size    = 8,
  };
  memcpy(state.b, nist_p_256.n, sizeof(uint32_t) * 8);
  static uint32_t secret_a[8];
  do {
    ecc_set_random(secret_a);
    memcpy(state.a, secret_a, sizeof(uint32_t) * 8);
    PT_SPAWN(&(ecdh_test.pt), &(state.pt), ecc_compare(&state));
  } while(state.result != PKA_STATUS_A_LT_B);

  static uint32_t secret_b[8];
  ecc_set_random(secret_b);
  do {
    ecc_set_random(secret_b);
    memcpy(state.a, secret_b, sizeof(uint32_t) * 8);
    PT_SPAWN(&(ecdh_test.pt), &(state.pt), ecc_compare(&state));
  } while(state.result != PKA_STATUS_A_LT_B);

  /*
   * Prepare Points
   */
  static ecc_multiply_state_t side_a = {
    .process    = &ecdh_test,
    .curve_info = &nist_p_256,
  };
  memcpy(side_a.point_in.x, nist_p_256.x, sizeof(uint32_t) * 8);
  memcpy(side_a.point_in.y, nist_p_256.y, sizeof(uint32_t) * 8);
  memcpy(side_a.secret, secret_a, sizeof(secret_a));

  static ecc_multiply_state_t side_b = {
    .process    = &ecdh_test,
    .curve_info = &nist_p_256,
  };
  memcpy(side_b.point_in.x, nist_p_256.x, sizeof(uint32_t) * 8);
  memcpy(side_b.point_in.y, nist_p_256.y, sizeof(uint32_t) * 8);
  memcpy(side_b.secret, secret_b, sizeof(secret_b));

  /*
   * Round 1
   */
  time = RTIMER_NOW();
  PT_SPAWN(&(ecdh_test.pt), &(side_a.pt), ecc_multiply(&side_a));
  time = RTIMER_NOW() - time;
  printf("Round 1, Side a: %i, %lu ms\n", (unsigned)side_a.result,
         (uint32_t)((uint64_t)time * 1000 / RTIMER_SECOND));

  time = RTIMER_NOW();
  PT_SPAWN(&(ecdh_test.pt), &(side_b.pt), ecc_multiply(&side_b));
  time = RTIMER_NOW() - time;
  printf("Round 1, Side b: %i, %lu ms\n", (unsigned)side_b.result,
         (uint32_t)((uint64_t)time * 1000 / RTIMER_SECOND));

  /*
   * Key Exchange
   */
  memcpy(side_a.point_in.x, side_b.point_out.x, sizeof(uint32_t) * 8);
  memcpy(side_a.point_in.y, side_b.point_out.y, sizeof(uint32_t) * 8);
  memcpy(side_b.point_in.x, side_a.point_out.x, sizeof(uint32_t) * 8);
  memcpy(side_b.point_in.y, side_a.point_out.y, sizeof(uint32_t) * 8);

  /*
   * Round 2
   */
  time = RTIMER_NOW();
  PT_SPAWN(&(ecdh_test.pt), &(side_a.pt), ecc_multiply(&side_a));
  time = RTIMER_NOW() - time;
  printf("Round 2, Side a: %i, %lu ms\n", (unsigned)side_a.result,
         (uint32_t)((uint64_t)time * 1000 / RTIMER_SECOND));
  time = RTIMER_NOW();
  PT_SPAWN(&(ecdh_test.pt), &(side_b.pt), ecc_multiply(&side_b));
  time = RTIMER_NOW() - time;
  printf("Round 2, Side b: %i, %lu ms\n", (unsigned)side_b.result,
         (uint32_t)((uint64_t)time * 1000 / RTIMER_SECOND));

  /*
   * Check Result
   */
  memcpy(state.a, side_a.point_out.x, sizeof(uint32_t) * 8);
  memcpy(state.b, side_b.point_out.x, sizeof(uint32_t) * 8);

  PT_SPAWN(&(ecdh_test.pt), &(state.pt), ecc_compare(&state));
  if(state.result) {
    puts("shared secrets do not match");
  } else {
    puts("shared secrets MATCH");
  }

  puts("-----------------------------------------\n"
       "Disabling pka...");
  pka_disable();

  puts("Done!");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
