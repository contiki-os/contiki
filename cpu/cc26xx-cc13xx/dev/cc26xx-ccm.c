/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
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
 * \addtogroup cc26xx-ccm
 * @{
 *
 * \file
 * Implementation of the cc26xx AES-CCM driver
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "contiki.h"
#include "sys/cc.h"
#include "dev/ccm.h"

//*****************************************************************************
// Current AES operation initialized to None
//*****************************************************************************
extern
volatile uint32_t g_ui32CurrentAesOp;

/*---------------------------------------------------------------------------*/
typedef enum {
      ccmENCRIPT = (1 << CRYPTO_AESCTL_DIR_S)
    , ccmDECRIPT = 0
} CCMDir;

static
CCMResult ccm_auth_crypt_start(  CCMDir dir
                               , CCMLenSize len_len, AESKeyArea key_area
                               , const void *nonce
                               , const void *adata, AES_CCMSize adata_len
                               , const void *pdata, AES_CCMSize pdata_len
                               , void *cdata, AES_MICSize mic_len
                               , struct process *notify_proc)
{
    uint32_t ctrl;
    uint32_t i;
    CCMResult res;
    AESNonce  iv;

    // Set current operating state of the Crypto module.
    g_ui32CurrentAesOp = CRYPTO_AES_CCM;

    // Configure AES engine.
    ctrl = ((len_len - 1) << CRYPTO_AESCTL_CCM_L_S);
    if ( mic_len >= 2 ) {
        ctrl |= ((( mic_len - 2 ) >> 1 ) << CRYPTO_AESCTL_CCM_M_S );
    }
    ctrl |= dir
         |  CRYPTO_AESCTL_CCM |  CRYPTO_AESCTL_CTR
         |  CRYPTO_AESCTL_SAVE_CONTEXT
         // force reset to new context
         |  CRYPTO_AESCTL_SAVED_CONTEXT_RDY
         | (KEY_STORE_SIZE_128 << CRYPTO_AESCTL_KEY_SIZE_S)
         | (CRYPTO_AES_CTR_128 << CRYPTO_AESCTL_CTR_WIDTH_S)
         ;

    // Prepare the initialization vector (IV),
    // Length of Nonce l(n) = 15 - ui32FieldLength.
    iv.b[0] = len_len - 1;
    for(i = 0; i < 12; i++)
    {
        iv.b[i + 1] = ((uint8_t*)nonce)[i];
    }
    if(len_len == 2)
    {
        iv.b[13] = ((uint8_t*)nonce)[12];
    }
    else
    {
        iv.b[13] = 0;
    }
    iv.b[14] = 0;
    iv.b[15] = 0;

    res = aes_auth_crypt_start(ctrl, key_area, &iv
             , adata, adata_len
             , pdata, cdata, pdata_len
             , notify_proc);
    return res;
}
/*---------------------------------------------------------------------------*/
CCMResult ccm_auth_decrypt_get_result(const void *cdata, AES_CCMSize cdata_len,
                          void *mic, AES_MICSize mic_len)
{
  AESBlock  tag;
  AES_CCMSize   data_len;
  CCMResult ret;

  ret = aes_auth_crypt_get_result(NULL, &tag);
  if(ret != CRYPTO_SUCCESS) {
    return ret;
  }

  if ((cdata != NULL) && (mic == NULL)) {
    //* Check MIC, if it is not stores aside
    data_len = cdata_len - mic_len;
    if(memcmp(tag.b, &((const uint8_t *)cdata)[data_len], mic_len)) {
      ret = AES_AUTHENTICATION_FAILED;
    }
  }

  if (mic != NULL)
      //  Copy tag to MIC
  memcpy(mic, tag.b, mic_len);

  return ret;
}
/*---------------------------------------------------------------------------*/
CCMResult ccm_auth_encrypt_start(CCMLenSize len_len, AESKeyArea key_area
                       , const void *nonce
                       , const void *adata, AES_CCMSize adata_len
                       , const void *pdata, AES_CCMSize pdata_len
                       , void *cdata, AES_MICSize mic_len
                       , struct process *notify_proc)
{
  return ccm_auth_crypt_start(ccmENCRIPT, len_len, key_area, nonce
                              , adata, adata_len
                              , pdata, pdata_len
                              , cdata, mic_len
                              , notify_proc);
}
/*---------------------------------------------------------------------------*/
CCMResult ccm_auth_encrypt_get_result(void *mic, AES_MICSize mic_len)
{
  return ccm_auth_decrypt_get_result(NULL, 0, mic, mic_len);
}
/*---------------------------------------------------------------------------*/
CCMResult ccm_auth_decrypt_start(CCMLenSize len_len, AESKeyArea key_area
                       , const void *nonce
                       , const void *adata, AES_CCMSize adata_len
                       , const void *cdata, AES_CCMSize cdata_len
                       , void *pdata, AES_MICSize mic_len,
                       struct process *process)
{
  uint16_t data_len = cdata_len - mic_len;

  return ccm_auth_crypt_start(ccmDECRIPT, len_len, key_area, nonce
                              , adata, adata_len
                              , cdata, data_len
                              , pdata, mic_len
                              , process);
}

/** @} */
