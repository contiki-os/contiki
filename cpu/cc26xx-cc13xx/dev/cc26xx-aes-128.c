/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
 * All rights reserved.
 *
 * Port to cc26xx:
 * Copyright (c) 2017, Alexander Lityagin   alexraynepe196@gmail.com
 *                                          alexraynepe196@hotbox.ru
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
 * This file is port of cc2538/aes driver API for cc26xx
 */
/**
 * \addtogroup cc26xx-aes-128
 * @{
 *
 * \file
 *         Implementation of the AES-128 driver for the CC26xx SoC
 * \author Alexander Lityagin   alexraynepe196@gmail.com
 *                              alexraynepe196@hotbox.ru
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */
#include <stdint.h>
#include "contiki.h"

#include <dev/crypto.h>
#include <dev/aes.h>
#include <dev/cc26xx-aes-128.h>
#include "ti-lib.h"

/*---------------------------------------------------------------------------*/
#define MODULE_NAME     "cc26xx-aes-128"

#undef  DEBUG
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
                      CC26XX_AES_128_KEY_AREA);
  if(ret != CRYPTO_SUCCESS) {
    PRINTF("%s: aes_load_keys() error %u\n", MODULE_NAME, ret);
    ASSERT(false);
    ti_lib_sys_ctrl_system_reset();
  }

  restore_crypto(crypto_enabled);
}
/*---------------------------------------------------------------------------*/
static void
encrypt(uint8_t *plaintext_and_result)
{
    // dummy not impemented function. do not call it.
    // TODO - implement this code.
    ASSERT(false);
}
/*---------------------------------------------------------------------------*/
const struct aes_128_driver cc26xx_aes_128_driver = {
  set_key,
  encrypt
};

/** @} */
