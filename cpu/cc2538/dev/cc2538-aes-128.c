/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
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
 *
 * This file is part of the Contiki operating system.
 */
/**
 * \addtogroup cc2538-aes-128
 * @{
 *
 * \file
 *         Implementation of the AES-128 driver for the CC2538 SoC
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */
#include "contiki.h"
#include "dev/ecb.h"
#include "dev/cc2538-aes-128.h"
#include "dev/sys-ctrl.h"

#include <stdint.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define MODULE_NAME     "cc2538-aes-128"

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
  uint8_t crypto_enabled, ret;

  crypto_enabled = enable_crypto();

  ret = aes_load_keys(key, AES_KEY_STORE_SIZE_KEY_SIZE_128, 1,
                      CC2538_AES_128_KEY_AREA);
  if(ret != CRYPTO_SUCCESS) {
    PRINTF("%s: aes_load_keys() error %u\n", MODULE_NAME, ret);
    sys_ctrl_reset();
  }

  restore_crypto(crypto_enabled);
}
/*---------------------------------------------------------------------------*/
static void
encrypt(uint8_t *plaintext_and_result)
{
  uint8_t crypto_enabled, ret;
  int8_t res;

  crypto_enabled = enable_crypto();

  ret = ecb_crypt_start(true, CC2538_AES_128_KEY_AREA, plaintext_and_result,
                        plaintext_and_result, AES_128_BLOCK_SIZE, NULL);
  if(ret != CRYPTO_SUCCESS) {
    PRINTF("%s: ecb_crypt_start() error %u\n", MODULE_NAME, ret);
    sys_ctrl_reset();
  }

  while((res = ecb_crypt_check_status()) == CRYPTO_PENDING);
  if(res != CRYPTO_SUCCESS) {
    PRINTF("%s: ecb_crypt_check_status() error %d\n", MODULE_NAME, res);
    sys_ctrl_reset();
  }

  restore_crypto(crypto_enabled);
}
/*---------------------------------------------------------------------------*/
const struct aes_128_driver cc2538_aes_128_driver = {
  set_key,
  encrypt
};

/** @} */
