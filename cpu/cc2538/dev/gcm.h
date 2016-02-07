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
 * \addtogroup cc2538-aes
 * @{
 *
 * \defgroup cc2538-gcm cc2538 AES-GCM
 *
 * Driver for the cc2538 AES-GCM mode of the security core
 * @{
 *
 * \file
 * Header file for the cc2538 AES-GCM driver
 */
#ifndef GCM_H_
#define GCM_H_

#include "contiki.h"
#include "dev/aes.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name AES-GCM constants
 * @{
 */
#define GCM_IV_LEN      (96 / 8)
#define GCM_TAG_LEN     AES_TAG_LEN
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES-GCM functions
 * @{
 */

/** \brief Starts a GCM authentication and encryption operation
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param iv Pointer to 96-bit initialization vector
 * \param adata Pointer to additional authenticated data in SRAM, or \c NULL
 * \param adata_len Length of additional authenticated data in octets, or \c 0
 * \param pdata Pointer to message to authenticate and encrypt in SRAM, or
 * \c NULL
 * \param pdata_len Length of message to authenticate and encrypt in octets, or
 * \c 0
 * \param cdata Pointer to encrypted message in SRAM (may be \p pdata), or
 * \c NULL
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/GCM error code
 */
uint8_t gcm_auth_encrypt_start(uint8_t key_area, const void *iv,
                               const void *adata, uint16_t adata_len,
                               const void *pdata, uint16_t pdata_len,
                               void *cdata, struct process *process);

/** \brief Checks the status of the GCM authentication and encryption operation
 * \retval false Result not yet available, and no error occurred
 * \retval true Result available, or error occurred
 */
#define gcm_auth_encrypt_check_status aes_auth_crypt_check_status

/** \brief Gets the result of the GCM authentication and encryption operation
 * \param tag Pointer to 128-bit authentication tag, or \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/GCM error code
 * \note This function must be called only after \c gcm_auth_encrypt_start().
 */
uint8_t gcm_auth_encrypt_get_result(void *tag);

/** \brief Starts a GCM authentication checking and decryption operation
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param iv Pointer to 96-bit initialization vector
 * \param adata Pointer to additional authenticated data in SRAM, or \c NULL
 * \param adata_len Length of additional authenticated data in octets, or \c 0
 * \param cdata Pointer to encrypted message in SRAM, or \c NULL
 * \param cdata_len Length of encrypted message in octets, or \c 0
 * \param pdata Pointer to decrypted message in SRAM (may be \p cdata), or
 * \c NULL
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/GCM error code
 */
uint8_t gcm_auth_decrypt_start(uint8_t key_area, const void *iv,
                               const void *adata, uint16_t adata_len,
                               const void *cdata, uint16_t cdata_len,
                               void *pdata, struct process *process);

/** \brief Checks the status of the GCM authentication checking and decryption
 * operation
 * \retval false Result not yet available, and no error occurred
 * \retval true Result available, or error occurred
 */
#define gcm_auth_decrypt_check_status aes_auth_crypt_check_status

/** \brief Gets the result of the GCM authentication checking and decryption
 * operation
 * \param tag_in Pointer to 128-bit input authentication tag, or \c NULL
 * \param tag_out Pointer to 128-bit output authentication tag, or \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/GCM error code
 * \note This function must be called only after \c gcm_auth_decrypt_start().
 */
uint8_t gcm_auth_decrypt_get_result(const void *tag_in, void *tag_out);

/** @} */

#endif /* GCM_H_ */

/**
 * @}
 * @}
 */
