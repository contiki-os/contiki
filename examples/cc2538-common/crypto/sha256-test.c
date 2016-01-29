/*
 * Original file:
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-sha256-test cc2538 SHA-256 Test Project
 *
 *   SHA-256 access example for CC2538-based platforms
 *
 *   This example shows how SHA-256 should be used. The example also verifies
 *   the SHA-256 functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating SHA-256
 */
#include "contiki.h"
#include "sys/rtimer.h"
#include "dev/rom-util.h"
#include "dev/sha256.h"

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
PROCESS(sha256_test_process, "sha256 test process");
AUTOSTART_PROCESSES(&sha256_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sha256_test_process, ev, data)
{
  static const char *const str_res[] = {
    "success",
    "invalid param",
    "NULL error",
    "resource in use",
    "DMA bus error"
  };
  static const struct {
    const char *data[3];
    uint8_t sha256[32];
  } vectors[] = {
    { /* Simple */
      {
        "abc",
        NULL,
        NULL
      }, {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
      }
    }, { /* Simple */
      {
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        NULL,
        NULL,
      }, {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
        0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
        0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
        0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
      }
    }, { /* Message of length 130 */
      {
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklabcd"
        "efghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmn",
        NULL,
        NULL
      }, {
        0x15, 0xd2, 0x3e, 0xea, 0x57, 0xb3, 0xd4, 0x61,
        0xbf, 0x38, 0x91, 0x12, 0xab, 0x4c, 0x43, 0xce,
        0x85, 0xe1, 0x68, 0x23, 0x8a, 0xaa, 0x54, 0x8e,
        0xc8, 0x6f, 0x0c, 0x9d, 0x65, 0xf9, 0xb9, 0x23
      }
    }, { /* Message of length 128 */
      {
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklabcd"
        "efghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl",
        NULL,
        NULL
      }, {
        0xf8, 0xa3, 0xf2, 0x26, 0xfc, 0x42, 0x10, 0xe9,
        0x0d, 0x13, 0x0c, 0x7f, 0x41, 0xf2, 0xbe, 0x66,
        0x45, 0x53, 0x85, 0xd2, 0x92, 0x0a, 0xda, 0x78,
        0x15, 0xf8, 0xf7, 0x95, 0xd9, 0x44, 0x90, 0x5f
      }
    }, { /* Message of length 64 */
      {
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl",
        NULL,
        NULL
      }, {
        0x2f, 0xcd, 0x5a, 0x0d, 0x60, 0xe4, 0xc9, 0x41,
        0x38, 0x1f, 0xcc, 0x4e, 0x00, 0xa4, 0xbf, 0x8b,
        0xe4, 0x22, 0xc3, 0xdd, 0xfa, 0xfb, 0x93, 0xc8,
        0x09, 0xe8, 0xd1, 0xe2, 0xbf, 0xff, 0xae, 0x8e
      }
    }, { /* Message of length 66 */
      {
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmn",
        NULL,
        NULL
      }, {
        0x92, 0x90, 0x1c, 0x85, 0x82, 0xe3, 0x1c, 0x05,
        0x69, 0xb5, 0x36, 0x26, 0x9c, 0xe2, 0x2c, 0xc8,
        0x30, 0x8b, 0xa4, 0x17, 0xab, 0x36, 0xc1, 0xbb,
        0xaf, 0x08, 0x4f, 0xf5, 0x8b, 0x18, 0xdc, 0x6a
      }
    }, {
      {
        "abcdbcdecdefde",
        "fgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        NULL
      }, {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
        0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
        0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
        0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
      }
    }, {
      {
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl",
        NULL
      }, {
        0xf8, 0xa3, 0xf2, 0x26, 0xfc, 0x42, 0x10, 0xe9,
        0x0d, 0x13, 0x0c, 0x7f, 0x41, 0xf2, 0xbe, 0x66,
        0x45, 0x53, 0x85, 0xd2, 0x92, 0x0a, 0xda, 0x78,
        0x15, 0xf8, 0xf7, 0x95, 0xd9, 0x44, 0x90, 0x5f
      }
    }, {
      {
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh",
        "ijkl",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl"
      }, {
        0xf8, 0xa3, 0xf2, 0x26, 0xfc, 0x42, 0x10, 0xe9,
        0x0d, 0x13, 0x0c, 0x7f, 0x41, 0xf2, 0xbe, 0x66,
        0x45, 0x53, 0x85, 0xd2, 0x92, 0x0a, 0xda, 0x78,
        0x15, 0xf8, 0xf7, 0x95, 0xd9, 0x44, 0x90, 0x5f
      }
    }
  };
  static sha256_state_t state;
  static uint8_t sha256[32];
  static int i, j;
  static uint8_t ret;
  static rtimer_clock_t total_time;
  rtimer_clock_t time;
  size_t len;

  PROCESS_BEGIN();

  puts("-----------------------------------------\n"
       "Initializing cryptoprocessor...");
  crypto_init();

  for(i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
    printf("-----------------------------------------\n"
           "Test vector #%d:\n", i);

    time = RTIMER_NOW();
    ret = sha256_init(&state);
    time = RTIMER_NOW() - time;
    total_time = time;
    printf("sha256_init(): %s, %lu us\n", str_res[ret],
           (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
    PROCESS_PAUSE();
    if(ret != CRYPTO_SUCCESS) {
      continue;
    }

    for(j = 0; j < sizeof(vectors[i].data) / sizeof(vectors[i].data[0]) &&
               vectors[i].data[j] != NULL; j++) {
      len = strlen(vectors[i].data[j]);
      printf("Buffer #%d (length: %u):\n", j, len);
      time = RTIMER_NOW();
      ret = sha256_process(&state, vectors[i].data[j], len);
      time = RTIMER_NOW() - time;
      total_time += time;
      printf("sha256_process(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      PROCESS_PAUSE();
      if(ret != CRYPTO_SUCCESS) {
        break;
      }
    }
    if(ret != CRYPTO_SUCCESS) {
      continue;
    }

    time = RTIMER_NOW();
    ret = sha256_done(&state, sha256);
    time = RTIMER_NOW() - time;
    total_time += time;
    printf("sha256_done(): %s, %lu us\n", str_res[ret],
           (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
    PROCESS_PAUSE();
    if(ret != CRYPTO_SUCCESS) {
      continue;
    }

    if(rom_util_memcmp(sha256, vectors[i].sha256, sizeof(sha256))) {
      puts("Computed SHA-256 hash does not match expected hash");
    } else {
      puts("Computed SHA-256 hash OK");
    }
    printf("Total duration: %lu us\n",
           (uint32_t)((uint64_t)total_time * 1000000 / RTIMER_SECOND));
  }

  puts("-----------------------------------------\n"
       "Disabling cryptoprocessor...");
  crypto_disable();

  puts("Done!");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
