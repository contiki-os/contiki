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
 * \defgroup cc2538-ecdsa-verify-test cc2538dk ECDSA-Verify Test Project
 *
 *   ECDSA-Verify example for CC2538 on SmartRF06EB.
 *
 *   This example shows how ECDSA-Verify should be used. The example also verifies
 *   the ECDSA-Verify functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating ECDSA-Verify on the cc2538dk platform
 */
#include "contiki.h"
#include "dev/ecc-algorithm.h"
#include "dev/ecc-curve.h"
#include "sys/rtimer.h"
#include "sys/pt.h"

#include <string.h>
#include <stdio.h>

PROCESS(ecdsa_verify_test, "ecdsa verify test");
AUTOSTART_PROCESSES(&ecdsa_verify_test);

PROCESS_THREAD(ecdsa_verify_test, ev, data) {
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
  static ecc_dsa_verify_state_t state = {
    .process     = &ecdsa_verify_test,
    .curve_info  = &nist_p_256,
    .signature_r = { 0xC3B4035F, 0x515AD0A6, 0xBF375DCA, 0x0CC1E997,
                     0x7F54FDCD, 0x04D3FECA, 0xB9E396B9, 0x515C3D6E },
    .signature_s = { 0x5366B1AB, 0x0F1DBF46, 0xB0C8D3C4, 0xDB755B6F,
                     0xB9BF9243, 0xE644A8BE, 0x55159A59, 0x6F9E52A6 },
    .hash        = { 0x65637572, 0x20612073, 0x68206F66, 0x20686173,
                     0x69732061, 0x68697320, 0x6F2C2054, 0x48616C6C },
  };
  static uint32_t public_x[8] = { 0x5fa58f52, 0xe47cfbf2, 0x300c28c5, 0x6375ba10,
                                  0x62684e91, 0xda0a9a8f, 0xf9f2ed29, 0x36dfe2c6 };
  static uint32_t public_y[8] = { 0xc772f829, 0x4fabc36f, 0x09daed0b, 0xe93f9872,
                                  0x35a7cfab, 0x5a3c7869, 0xde1ab878, 0x71a0d4fc };

  memcpy(state.public.x, public_x, sizeof(public_x));
  memcpy(state.public.y, public_y, sizeof(public_y));

  /*
   * Verify
   */
  time = RTIMER_NOW();
  PT_SPAWN(&(ecdsa_verify_test.pt), &(state.pt), ecc_dsa_verify(&state));
  time = RTIMER_NOW() - time;
  printf("ecc_dsa_verify(), %lu ms\n",
         (uint32_t)((uint64_t)time * 1000 / RTIMER_SECOND));

  if(state.result) {
    puts("signature verification failed");
  } else {
    puts("signature verification OK");
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
