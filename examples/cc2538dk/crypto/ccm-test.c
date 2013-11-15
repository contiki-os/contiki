/*
 * Original file:
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 * \defgroup cc2538-ccm-test cc2538dk AES-CCM Test Project
 *
 *   AES-CCM access example for CC2538 on SmartRF06EB.
 *
 *   This example shows how AES-CCM should be used. The example also verifies
 *   the AES-CCM functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating AES-CCM on the cc2538dk platform
 */
#include "contiki.h"
#include "sys/rtimer.h"
#include "dev/crypto.h"
#include "dev/ccm.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
PROCESS(ccm_test_process, "ccm test process");
AUTOSTART_PROCESSES(&ccm_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ccm_test_process, ev, data)
{
  static const char *const str_res[] = {
    "success",
    "resource in use",
    "keystore read error",
    "keystore write error",
    "DMA bus error",
    "authentication failed",
    "invalid param",
    "NULL error"
  };
  static struct {
    bool encrypt;
    uint8_t len_len;
    uint8_t key[16];
    uint8_t key_area;
    uint8_t nonce[13];
    uint8_t adata[26];
    uint16_t adata_len;
    uint8_t mdata[24];
    uint16_t mdata_len;
    uint8_t mic[8];
    uint8_t mic_len;
    uint8_t expected[24];
  } vectors[] = {
    {
      true, /* encrypt */
      2, /* len_len */
      { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
      0, /* key_area */
      { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0,
        0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
      {}, /* adata */
      0, /* adata_len */
      { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f }, /* mdata */
      20, /* mdata_len */
      {}, /* mic */
      0, /* mic_len */
      { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0,
        0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6,
        0x2c, 0x61, 0x01, 0x4e } /* expected */
    }, {
      true, /* encrypt */
      2, /* len_len */
      { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
        0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf }, /* key */
      0, /* key_area */
      { 0xac, 0xde, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x05, 0x02 }, /* nonce */
      { 0x08, 0xd0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x48, 0xde, 0xac, 0x02, 0x05, 0x00,
        0x00, 0x00, 0x55, 0xcf, 0x00, 0x00, 0x51, 0x52,
        0x53, 0x54 }, /* adata */
      26, /* adata_len */
      {}, /* mdata */
      0, /* mdata_len */
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* mic */
      8, /* mic_len */
      { 0x22, 0x3b, 0xc1, 0xec, 0x84, 0x1a, 0xb5, 0x53 } /* expected */
    }, {
      true, /* encrypt */
      2, /* len_len */
      { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
      0, /* key_area */
      { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0,
        0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
      { 0x69, 0x98, 0x03, 0x33, 0x63, 0xbb, 0xaa, 0x01,
        0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03 }, /* adata */
      15, /* adata_len */
      { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f }, /* mdata */
      20, /* mdata_len */
      { 0x00, 0x00, 0x00, 0x00 }, /* mic */
      4, /* mic_len */
      { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0,
        0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6,
        0x2c, 0x61, 0x01, 0x4e, 0x7b, 0x34, 0x4f, 0x09 } /* expected */
    }, {
      false, /* decrypt */
      2, /* len_len */
      { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
      0, /* key_area */
      { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0,
        0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
      {}, /* adata */
      0, /* adata_len */
      { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0,
        0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6,
        0x2c, 0x61, 0x01, 0x4e }, /* mdata */
      20, /* mdata_len */
      {}, /* mic */
      0, /* mic_len */
      { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f } /* expected */
    }, {
      false, /* decrypt */
      2, /* len_len */
      { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
        0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf }, /* key */
      0, /* key_area */
      { 0xac, 0xde, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x05, 0x02 }, /* nonce */
      { 0x08, 0xd0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x48, 0xde, 0xac, 0x02, 0x05, 0x00,
        0x00, 0x00, 0x55, 0xcf, 0x00, 0x00, 0x51, 0x52,
        0x53, 0x54 }, /* adata */
      26, /* adata_len */
      { 0x22, 0x3b, 0xc1, 0xec, 0x84, 0x1a, 0xb5, 0x53 }, /* mdata */
      8, /* mdata_len */
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* mic */
      8, /* mic_len */
      {} /* expected */
    }, {
      false, /* decrypt */
      2, /* len_len */
      { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
      0, /* key_area */
      { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0,
        0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
      { 0x69, 0x98, 0x03, 0x33, 0x63, 0xbb, 0xaa, 0x01,
        0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03 }, /* adata */
      15, /* adata_len */
      { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0,
        0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6,
        0x2c, 0x61, 0x01, 0x4e, 0x7b, 0x34, 0x4f, 0x09 }, /* mdata */
      24, /* mdata_len */
      { 0x00, 0x00, 0x00, 0x00 }, /* mic */
      4, /* mic_len */
      { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f } /* expected */
    }
  };
  static int i;
  static uint8_t ret;
  static rtimer_clock_t time, time2, total_time;

  PROCESS_BEGIN();

  puts("-----------------------------------------\n"
       "Initializing cryptoprocessor...");
  crypto_init();

  for(i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
    printf("-----------------------------------------\n"
           "Test vector #%d: %s\n"
           "len_len=%d key_area=%d\n"
           "adata_len=%d mdata_len=%d mic_len=%d\n",
           i, vectors[i].encrypt ? "encrypt" : "decrypt",
           vectors[i].len_len, vectors[i].key_area,
           vectors[i].adata_len, vectors[i].mdata_len, vectors[i].mic_len);

    time = RTIMER_NOW();
    ret = aes_load_key(vectors[i].key, vectors[i].key_area);
    time = RTIMER_NOW() - time;
    total_time = time;
    printf("aes_load_key(): %s, %lu us\n", str_res[ret],
           (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
    PROCESS_PAUSE();
    if(ret != AES_SUCCESS) {
      continue;
    }

    time = RTIMER_NOW();
    if(vectors[i].encrypt) {
      ret = ccm_auth_encrypt_start(vectors[i].len_len, vectors[i].key_area,
                                   vectors[i].nonce, vectors[i].adata,
                                   vectors[i].adata_len, vectors[i].mdata,
                                   vectors[i].mdata_len, vectors[i].mic_len,
                                   &ccm_test_process);
      time2 = RTIMER_NOW();
      time = time2 - time;
      total_time += time;
      if(ret == AES_SUCCESS) {
        PROCESS_WAIT_EVENT_UNTIL(ccm_auth_encrypt_check_status());
        time2 = RTIMER_NOW() - time2;
        total_time += time2;
      }
      printf("ccm_auth_encrypt_start(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      if(ret != AES_SUCCESS) {
        PROCESS_PAUSE();
        continue;
      }
      printf("ccm_auth_encrypt_check_status() wait: %lu us\n",
             (uint32_t)((uint64_t)time2 * 1000000 / RTIMER_SECOND));

      time = RTIMER_NOW();
      ret = ccm_auth_encrypt_get_result(vectors[i].mic, vectors[i].mic_len);
      time = RTIMER_NOW() - time;
      total_time += time;
      printf("ccm_auth_encrypt_get_result(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      PROCESS_PAUSE();
      if(ret != AES_SUCCESS) {
        continue;
      }

      if(memcmp(vectors[i].mdata, vectors[i].expected, vectors[i].mdata_len)) {
        puts("Encrypted message does not match expected one");
      } else {
        puts("Encrypted message OK");
      }

      if(memcmp(vectors[i].mic, vectors[i].expected + vectors[i].mdata_len,
                vectors[i].mic_len)) {
        puts("MIC does not match expected one");
      } else {
        puts("MIC OK");
      }
    } else {
      ret = ccm_auth_decrypt_start(vectors[i].len_len, vectors[i].key_area,
                                   vectors[i].nonce, vectors[i].adata,
                                   vectors[i].adata_len, vectors[i].mdata,
                                   vectors[i].mdata_len, vectors[i].mic_len,
                                   &ccm_test_process);
      time2 = RTIMER_NOW();
      time = time2 - time;
      total_time += time;
      if(ret == AES_SUCCESS) {
        PROCESS_WAIT_EVENT_UNTIL(ccm_auth_decrypt_check_status());
        time2 = RTIMER_NOW() - time2;
        total_time += time2;
      }
      printf("ccm_auth_decrypt_start(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      if(ret != AES_SUCCESS) {
        PROCESS_PAUSE();
        continue;
      }
      printf("ccm_auth_decrypt_check_status() wait: %lu us\n",
             (uint32_t)((uint64_t)time2 * 1000000 / RTIMER_SECOND));

      time = RTIMER_NOW();
      ret = ccm_auth_decrypt_get_result(vectors[i].mdata, vectors[i].mdata_len,
                                        vectors[i].mic, vectors[i].mic_len);
      time = RTIMER_NOW() - time;
      total_time += time;
      printf("ccm_auth_decrypt_get_result(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      PROCESS_PAUSE();
      if(ret != AES_SUCCESS) {
        continue;
      }

      if(memcmp(vectors[i].mdata, vectors[i].expected,
                vectors[i].mdata_len - vectors[i].mic_len)) {
        puts("Decrypted message does not match expected one");
      } else {
        puts("Decrypted message OK");
      }
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
