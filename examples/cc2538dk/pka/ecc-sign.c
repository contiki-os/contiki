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
 * \defgroup cc2538-ecdsa-sign-test cc2538dk ECDSA-Sign Test Project
 *
 *   ECDSA-Sign example for CC2538 on SmartRF06EB.
 *
 *   This example shows how ECDSA-Sign should be used. The example also verifies
 *   the ECDSA-Sign functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating ECDSA-Sign on the cc2538dk platform
 */
#include "contiki.h"
#include "dev/ecc-algorithm.h"
#include "dev/ecc-curve.h"
#include "sys/rtimer.h"
#include "sys/pt.h"

#include <string.h>
#include <stdio.h>

PROCESS(ecdsa_sign_test, "ecdsa sign test");
AUTOSTART_PROCESSES(&ecdsa_sign_test);

PROCESS_THREAD(ecdsa_sign_test, ev, data) {
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
   * Setup Variables
   */
  static ecc_compare_state_t comp_state = {
    .process = &ecdsa_sign_test,
    .size    = 8,
  };
  static ecc_dsa_sign_state_t state = {
    .process = &ecdsa_sign_test,
    .curve_info = &nist_p_256,
    .secret  = { 0x94A949FA, 0x401455A1, 0xAD7294CA, 0x896A33BB,
                 0x7A80E714, 0x4321435B, 0x51247A14, 0x41C1CB6B },
    .k_e     = { 0x1D1E1F20, 0x191A1B1C, 0x15161718, 0x11121314,
                 0x0D0E0F10, 0x090A0B0C, 0x05060708, 0x01020304 },
    .hash    = { 0x65637572, 0x20612073, 0x68206F66, 0x20686173,
                 0x69732061, 0x68697320, 0x6F2C2054, 0x48616C6C },
  };

  /*
   * Sign
   */
  time = RTIMER_NOW();
  PT_SPAWN(&(ecdsa_sign_test.pt), &(state.pt), ecc_dsa_sign(&state));
  time = RTIMER_NOW() - time;
  printf("ecc_dsa_sign(), %lu ms\n",
         (uint32_t)((uint64_t)time * 1000 / RTIMER_SECOND));

  /*
   * Check Result
   */
  static uint32_t ecdsaTestresultR1[] = { 0xC3B4035F, 0x515AD0A6, 0xBF375DCA, 0x0CC1E997,
                                          0x7F54FDCD, 0x04D3FECA, 0xB9E396B9, 0x515C3D6E };
  static uint32_t ecdsaTestresultS1[] = { 0x5366B1AB, 0x0F1DBF46, 0xB0C8D3C4, 0xDB755B6F,
                                          0xB9BF9243, 0xE644A8BE, 0x55159A59, 0x6F9E52A6 };

  memcpy(comp_state.a, state.point_r.x, sizeof(uint32_t) * 8);
  memcpy(comp_state.b, ecdsaTestresultR1, sizeof(uint32_t) * 8);
  PT_SPAWN(&(ecdsa_sign_test.pt), &(comp_state.pt), ecc_compare(&comp_state));
  if(comp_state.result) {
    puts("r1 of signature does not match");
  } else {
    puts("r1 of signature OK");
  }

  memcpy(comp_state.a, state.signature_s, sizeof(uint32_t) * 8);
  memcpy(comp_state.b, ecdsaTestresultS1, sizeof(uint32_t) * 8);
  PT_SPAWN(&(ecdsa_sign_test.pt), &(comp_state.pt), ecc_compare(&comp_state));
  if(comp_state.result) {
    puts("s1 of signature does not match");
  } else {
    puts("s1 of signature OK");
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
