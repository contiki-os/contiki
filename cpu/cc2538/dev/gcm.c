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
 * \addtogroup cc2538-gcm
 * @{
 *
 * \file
 * Implementation of the cc2538 AES-GCM driver
 */
#include "contiki.h"
#include "dev/rom-util.h"
#include "dev/gcm.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
static uint8_t
gcm_auth_crypt_start(uint8_t encrypt, uint8_t key_area, const void *iv,
                     const void *adata, uint16_t adata_len,
                     const void *data_in, void *data_out, uint16_t data_len,
                     struct process *process)
{
  uint32_t ctrl;
  uint32_t aes_iv[AES_IV_LEN / sizeof(uint32_t)];

  /* Program AES-GCM authentication/crypto operation */
  ctrl = AES_AES_CTRL_SAVE_CONTEXT |                /* Save context */
    AES_AES_CTRL_GCM |                              /* GCM */
    AES_AES_CTRL_CTR_WIDTH_32 |                     /* CTR width 32 */
    AES_AES_CTRL_CTR |                              /* CTR */
    (encrypt ? AES_AES_CTRL_DIRECTION_ENCRYPT : 0); /* En/decryption */

  /* Prepare the crypto initialization vector
   * Initialization vector */
  rom_util_memcpy(aes_iv, iv, GCM_IV_LEN);
  /* Initialize counter to 1 */
  aes_iv[GCM_IV_LEN / sizeof(aes_iv[0])] = 0x01000000;

  return aes_auth_crypt_start(ctrl, key_area, aes_iv, adata, adata_len,
                              data_in, data_out, data_len, process);
}
/*---------------------------------------------------------------------------*/
static uint8_t
gcm_auth_crypt_get_result(const void *tag_in, void *tag_out)
{
  uint32_t tag[AES_TAG_LEN / sizeof(uint32_t)];
  uint8_t ret;

  ret = aes_auth_crypt_get_result(NULL, tag);
  if(ret != CRYPTO_SUCCESS) {
    return ret;
  }

  if(tag_in != NULL) {
    /* Check tag */
    if(rom_util_memcmp(tag, tag_in, GCM_TAG_LEN)) {
      ret = AES_AUTHENTICATION_FAILED;
    }
  }

  if(tag_out != NULL) {
    /* Copy tag */
    rom_util_memcpy(tag_out, tag, GCM_TAG_LEN);
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
uint8_t
gcm_auth_encrypt_start(uint8_t key_area, const void *iv, const void *adata,
                       uint16_t adata_len, const void *pdata,
                       uint16_t pdata_len, void *cdata, struct process *process)
{
  return gcm_auth_crypt_start(true, key_area, iv, adata, adata_len,
                              pdata, cdata, pdata_len, process);
}
/*---------------------------------------------------------------------------*/
uint8_t
gcm_auth_encrypt_get_result(void *tag)
{
  return gcm_auth_crypt_get_result(NULL, tag);
}
/*---------------------------------------------------------------------------*/
uint8_t
gcm_auth_decrypt_start(uint8_t key_area, const void *iv, const void *adata,
                       uint16_t adata_len, const void *cdata,
                       uint16_t cdata_len, void *pdata, struct process *process)
{
  return gcm_auth_crypt_start(false, key_area, iv, adata, adata_len,
                              cdata, pdata, cdata_len, process);
}
/*---------------------------------------------------------------------------*/
uint8_t
gcm_auth_decrypt_get_result(const void *tag_in, void *tag_out)
__attribute__ ((alias("gcm_auth_crypt_get_result")));

/** @} */
