/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
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
 * \addtogroup cc2538-cbc-mac
 * @{
 *
 * \file
 * Implementation of the cc2538 AES-CBC-MAC driver
 */
#include "contiki.h"
#include "dev/rom-util.h"
#include "dev/cbc-mac.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
uint8_t
cbc_mac_auth_start(uint8_t key_area, const void *mdata, uint16_t mdata_len,
                   struct process *process)
{
  uint32_t ctrl;
  uint32_t iv[AES_IV_LEN / sizeof(uint32_t)];

  /* Program AES-CBC-MAC authentication operation */
  ctrl = AES_AES_CTRL_SAVE_CONTEXT | /* Save context */
    AES_AES_CTRL_CBC_MAC |           /* CBC-MAC */
    AES_AES_CTRL_DIRECTION_ENCRYPT;  /* Encryption */

  /* Prepare the crypto initialization vector
   * Set initialization vector to 0 */
  rom_util_memset(iv, 0, AES_IV_LEN);

  return aes_auth_crypt_start(ctrl, key_area, iv, NULL, 0,
                              mdata, NULL, mdata_len, process);
}
/*---------------------------------------------------------------------------*/
uint8_t
cbc_mac_auth_get_result(const void *mac_in, void *mac_out)
{
  uint32_t tag[AES_TAG_LEN / sizeof(uint32_t)];
  uint8_t ret;

  ret = aes_auth_crypt_get_result(NULL, tag);
  if(ret != CRYPTO_SUCCESS) {
    return ret;
  }

  if(mac_in != NULL) {
    /* Check MAC */
    if(rom_util_memcmp(tag, mac_in, CBC_MAC_MAC_LEN)) {
      ret = AES_AUTHENTICATION_FAILED;
    }
  }

  if(mac_out != NULL) {
    /* Copy tag to MAC */
    rom_util_memcpy(mac_out, tag, CBC_MAC_MAC_LEN);
  }

  return ret;
}

/** @} */
