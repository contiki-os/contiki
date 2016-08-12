/*
 * Copyright (c) 2015, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
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
 * \addtogroup cc2538-ccm-star
 * @{
 *
 * \file
 * Implementation of the AES-CCM* driver for the CC2538 SoC
 */
#include "contiki.h"
#include "dev/ccm.h"
#include "dev/cc2538-aes-128.h"
#include "dev/cc2538-ccm-star.h"

#include <stdint.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define MODULE_NAME     "cc2538-ccm-star"

#define CCM_STAR_LEN_LEN        (CCM_NONCE_LEN_LEN - CCM_STAR_NONCE_LENGTH)

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t
enable_crypto(void)
{
  uint8_t enabled = CRYPTO_IS_ENABLED();
  if(!enabled) {
    crypto_enable();
  }
  return enabled;
}
/*---------------------------------------------------------------------------*/
static void
restore_crypto(uint8_t enabled)
{
  if(!enabled) {
    crypto_disable();
  }
}
/*---------------------------------------------------------------------------*/
static void
set_key(const uint8_t *key)
{
  cc2538_aes_128_driver.set_key(key);
}
/*---------------------------------------------------------------------------*/
static void
aead(const uint8_t *nonce, uint8_t *m, uint8_t m_len, const uint8_t *a,
     uint8_t a_len, uint8_t *result, uint8_t mic_len, int forward)
{
  uint16_t cdata_len;
  uint8_t crypto_enabled, ret;

  crypto_enabled = enable_crypto();

  if(forward) {
    ret = ccm_auth_encrypt_start(CCM_STAR_LEN_LEN, CC2538_AES_128_KEY_AREA,
                                 nonce, a, a_len, m, m_len, m, mic_len, NULL);
    if(ret != CRYPTO_SUCCESS) {
      PRINTF("%s: ccm_auth_encrypt_start() error %u\n", MODULE_NAME, ret);
      restore_crypto(crypto_enabled);
      return;
    }

    while(!ccm_auth_encrypt_check_status());
    ret = ccm_auth_encrypt_get_result(result, mic_len);
    if(ret != CRYPTO_SUCCESS) {
      PRINTF("%s: ccm_auth_encrypt_get_result() error %u\n", MODULE_NAME, ret);
    }
  } else {
    cdata_len = m_len + mic_len;
    ret = ccm_auth_decrypt_start(CCM_STAR_LEN_LEN, CC2538_AES_128_KEY_AREA,
                                 nonce, a, a_len, m, cdata_len, m, mic_len,
                                 NULL);
    if(ret != CRYPTO_SUCCESS) {
      PRINTF("%s: ccm_auth_decrypt_start() error %u\n", MODULE_NAME, ret);
      restore_crypto(crypto_enabled);
      return;
    }

    while(!ccm_auth_decrypt_check_status());
    ret = ccm_auth_decrypt_get_result(m, cdata_len, result, mic_len);
    if(ret != CRYPTO_SUCCESS) {
      PRINTF("%s: ccm_auth_decrypt_get_result() error %u\n", MODULE_NAME, ret);
    }
  }

  restore_crypto(crypto_enabled);
}
/*---------------------------------------------------------------------------*/
const struct ccm_star_driver cc2538_ccm_star_driver = {
  set_key,
  aead
};

/** @} */
