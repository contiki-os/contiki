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
 * \addtogroup cc26xx-aes
 * @{
 *
 * \defgroup cc26xx-ccm cc26xx AES-CCM
 *
 * Driver for the cc26xx AES-CCM mode of the security core
 * @{
 *
 * \file
 * Header file for the cc26xx AES-CCM driver
 */
#ifndef CCM_H_
#define CCM_H_

#include <stdbool.h>
#include <stdint.h>

#include "contiki.h"
#include "driverlib/crypto.h"

#include "dev/aes.h"
/*---------------------------------------------------------------------------*/
/** \name AES-CCM constants
 * @{
 */
#define CCM_FLAGS_LEN           1
#define CCM_NONCE_LEN_LEN       (AES_IV_LEN - CCM_FLAGS_LEN)
#define CCM_MIC_MAX_LEN         AES_TAG_LEN
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES-CCM functions
 * @{
 */
typedef enum {
    ccm2BYTE    = 2, ccmSHORT = ccm2BYTE
  , ccm4BYTE    = 4, ccmLONG  = ccm4BYTE
  , ccm8BYTE    = 8
} CCMLenSize;
typedef unsigned        AES_CCMSize;
typedef uint_fast8_t    AES_MICSize;

typedef AESResult CCMResult;

/** \brief Starts a CCM authentication and encryption operation
 * \param len_len Number of octets in length field (2, 4, or 8)
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param nonce Pointer to nonce (\c CCM_NONCE_LEN_LEN - \p len_len octets)
 * \param adata Pointer to additional authenticated data in SRAM, or \c NULL
 * \param adata_len Length of additional authenticated data in octets, or \c 0
 * \param pdata Pointer to message to authenticate and encrypt in SRAM, or
 * \c NULL
 * \param pdata_len Length of message to authenticate and encrypt in octets, or
 * \c 0
 * \param cdata Pointer to encrypted message in SRAM (may be \p pdata), or
 * \c NULL
 * \param mic_len Number of octets in authentication field (even value between 0
 * and \c CCM_MIC_MAX_LEN)
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CCM error code
 */
CCMResult ccm_auth_encrypt_start(CCMLenSize len_len, AESKeyArea key_area
                               , const void *nonce
                               , const void *adata, AES_CCMSize adata_len
                               , const void *pdata, AES_CCMSize pdata_len
                               , void *cdata, AES_MICSize mic_len
                               , struct process *notify_proc);

/** \brief Checks the status of the CCM authentication and encryption operation
 * \retval false Result not yet available, and no error occurred
 * \retval true Result available, or error occurred
 */
#define ccm_auth_encrypt_check_status aes_auth_crypt_check_status

/** \brief Gets the result of the CCM authentication and encryption operation
 * \param mic Pointer to authentication field, or \c NULL
 * \param mic_len Number of octets in authentication field (even value between 0
 * and \c CCM_MIC_MAX_LEN)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CCM error code
 * \note This function must be called only after \c ccm_auth_encrypt_start().
 */
CCMResult ccm_auth_encrypt_get_result(void *mic, AES_MICSize mic_len);

/** \brief Starts a CCM authentication checking and decryption operation
 * \param len_len Number of octets in length field (2, 4, or 8)
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param nonce Pointer to nonce (\c CCM_NONCE_LEN_LEN - \p len_len octets)
 * \param adata Pointer to additional authenticated data in SRAM, or \c NULL
 * \param adata_len Length of additional authenticated data in octets, or \c 0
 * \param cdata Pointer to encrypted and authenticated message in SRAM
 * \param cdata_len Length of encrypted and authenticated message in octets
 * \param pdata Pointer to decrypted message in SRAM (may be \p cdata), or
 * \c NULL
 * \param mic_len Number of octets in authentication field (even value between 0
 * and \c CCM_MIC_MAX_LEN)
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CCM error code
 */
CCMResult ccm_auth_decrypt_start(CCMLenSize len_len, AESKeyArea key_area
                       , const void *nonce
                       , const void *adata, AES_CCMSize adata_len
                       , const void *cdata, AES_CCMSize cdata_len
                       , void *pdata, AES_MICSize mic_len,
                       struct process *process);

/** \brief Checks the status of the CCM authentication checking and decryption
 * operation
 * \retval \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CCM error code
 */
#define ccm_auth_decrypt_check_status aes_auth_crypt_check_status

/** \brief Gets the result of the CCM authentication checking and decryption
 * operation
 * \param cdata Pointer to encrypted and authenticated message
 * \param cdata_len Length of encrypted and authenticated message in octets
 * \param mic Pointer to authentication field, or \c NULL
 * \param mic_len Number of octets in authentication field (even value between 0
 * and \c CCM_MIC_MAX_LEN)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CCM error code
 * \note This function must be called only after \c ccm_auth_decrypt_start().
 */
CCMResult ccm_auth_decrypt_get_result(const void *cdata, AES_CCMSize cdata_len,
                                    void *mic, AES_MICSize mic_len);

/** @} */

#endif /* CCM_H_ */

/**
 * @}
 * @}
 */
