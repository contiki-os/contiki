/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538-ccm
 * @{
 *
 * \file
 * Implementation of the cc2538 AES-CCM driver
 */
#include "contiki.h"
#include "sys/cc.h"
#include "dev/rom-util.h"
#include "dev/ccm.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
static uint8_t
ccm_auth_crypt_start(uint8_t encrypt, uint8_t len_len, uint8_t key_area,
                     const void *nonce, const void *adata, uint16_t adata_len,
                     const void *data_in, void *data_out, uint16_t data_len,
                     uint8_t mic_len, struct process *process)
{
  uint32_t ctrl;
  uint32_t iv[AES_IV_LEN / sizeof(uint32_t)];

  /* Program AES-CCM authentication/crypto operation */
  ctrl = AES_AES_CTRL_SAVE_CONTEXT |                         /* Save context */
    (((MAX(mic_len, 2) - 2) >> 1) << AES_AES_CTRL_CCM_M_S) | /* M */
    ((len_len - 1) << AES_AES_CTRL_CCM_L_S) |                /* L */
    AES_AES_CTRL_CCM |                                       /* CCM */
    AES_AES_CTRL_CTR_WIDTH_128 |                             /* CTR width 128 */
    AES_AES_CTRL_CTR |                                       /* CTR */
    (encrypt ? AES_AES_CTRL_DIRECTION_ENCRYPT : 0);          /* En/decryption */

  /* Prepare the crypto initialization vector
   * Flags: L' = L - 1 */
  ((uint8_t *)iv)[0] = len_len - 1;
  /* Nonce */
  rom_util_memcpy(&((uint8_t *)iv)[CCM_FLAGS_LEN], nonce,
                  CCM_NONCE_LEN_LEN - len_len);
  /* Initialize counter to 0 */
  rom_util_memset(&((uint8_t *)iv)[AES_IV_LEN - len_len], 0, len_len);

  return aes_auth_crypt_start(ctrl, key_area, iv, adata, adata_len,
                              data_in, data_out, data_len, process);
}
/*---------------------------------------------------------------------------*/
static uint8_t
ccm_auth_crypt_get_result(const void *cdata, uint16_t cdata_len,
                          void *mic, uint8_t mic_len)
{
  uint32_t tag[AES_TAG_LEN / sizeof(uint32_t)];
  uint16_t data_len;
  uint8_t ret;

  ret = aes_auth_crypt_get_result(NULL, tag);
  if(ret != CRYPTO_SUCCESS) {
    return ret;
  }

  if(cdata != NULL) {
    /* Check MIC */
    data_len = cdata_len - mic_len;
    if(rom_util_memcmp(tag, &((const uint8_t *)cdata)[data_len], mic_len)) {
      ret = AES_AUTHENTICATION_FAILED;
    }
  }

  /* Copy tag to MIC */
  rom_util_memcpy(mic, tag, mic_len);

  return ret;
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_encrypt_start(uint8_t len_len, uint8_t key_area, const void *nonce,
                       const void *adata, uint16_t adata_len, const void *pdata,
                       uint16_t pdata_len, void *cdata, uint8_t mic_len,
                       struct process *process)
{
  return ccm_auth_crypt_start(true, len_len, key_area, nonce, adata, adata_len,
                              pdata, cdata, pdata_len, mic_len, process);
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_encrypt_get_result(void *mic, uint8_t mic_len)
{
  return ccm_auth_crypt_get_result(NULL, 0, mic, mic_len);
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_decrypt_start(uint8_t len_len, uint8_t key_area, const void *nonce,
                       const void *adata, uint16_t adata_len, const void *cdata,
                       uint16_t cdata_len, void *pdata, uint8_t mic_len,
                       struct process *process)
{
  uint16_t data_len = cdata_len - mic_len;

  return ccm_auth_crypt_start(false, len_len, key_area, nonce, adata, adata_len,
                              cdata, pdata, data_len, mic_len, process);
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_decrypt_get_result(const void *cdata, uint16_t cdata_len,
                            void *mic, uint8_t mic_len)
__attribute__ ((alias("ccm_auth_crypt_get_result")));

/** @} */
