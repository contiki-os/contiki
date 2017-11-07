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
 * \addtogroup cc26xx-crypto
 * @{
 *
 * \defgroup cc26xx-aes cc26xx AES
 *
 * Driver for the cc26xx AES modes of the security core
 * @{
 *
 * \file
 * Header file for the cc2538 AES driver
 */
#ifndef AES_H_
#define AES_H_

#include <stdbool.h>
#include <stdint.h>

#include "contiki.h"
#include "crypto.h"
#include "driverlib/crypto.h"
/*---------------------------------------------------------------------------*/
// ports for cc2538
enum AESKeySizeID{
    AES_KEY_STORE_SIZE_KEY_SIZE_128     = KEY_STORE_SIZE_128
  , AES_KEY_STORE_SIZE_KEY_SIZE_192     = KEY_STORE_SIZE_192
  , AES_KEY_STORE_SIZE_KEY_SIZE_256     = KEY_STORE_SIZE_256
};
// some AES constants like cc2538 ported from cc26xx
enum {
    AES_AUTHENTICATION_FAILED           = CCM_AUTHENTICATION_FAILED
};
/*---------------------------------------------------------------------------*/
/** \name AES constants
 * @{
 */
#define AES_KEY_AREAS   8
#define AES_BLOCK_LEN   (128 / 8)
#define AES_IV_LEN      AES_BLOCK_LEN
#define AES_TAG_LEN     AES_BLOCK_LEN
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES functions
 * @{
 */

typedef uint_fast8_t    AESResult;
typedef uint_fast8_t    AESKeyArea;

/** \brief Writes keys into the Key RAM
 * \param keys Pointer to AES Keys
 *        !!! assume that it is aligned to 4 byte.
 * \param key_size Key size: \c AES_KEY_STORE_SIZE_KEY_SIZE_x
 * \param count Number of keys (1 to \c AES_KEY_AREAS - \p start_area for
 * 128-bit keys, 1 to (\c AES_KEY_AREAS - \p start_area) / 2 for 192- and
 * 256-bit keys)
 * \param start_area Start area in Key RAM where to store the keys (0 to
 * \c AES_KEY_AREAS - 1, must be even for 192- and 256-bit keys)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES error code
 * \note Calling this function with a value of \p key_size different from the
 * one passed for the previous calls causes the deletion of all previously
 * stored keys.
 */
AESResult aes_load_keys(const void *keys, uint8_t key_size, uint8_t count,
                        AESKeyArea start_area);


typedef unsigned        AESSize;
typedef uint_fast8_t    AES_MICSize;
typedef union {
        uint32_t w[4];
        uint8_t  b[16];
} AESBlock;
typedef AESBlock AESNonce;

/** \brief Starts an AES authentication/crypto operation
 * \param ctrl Contents of the \c AES_AES_CTRL register
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param iv Pointer to 128-bit initialization vector, or \c NULL
 * \param adata Pointer to additional authenticated data in SRAM, or \c NULL
 * \param adata_len Length of additional authenticated data in octets, or \c 0
 * \param data_in Pointer to input payload data in SRAM, or \c NULL
 * \param data_out Pointer to output payload data in SRAM (may be \p data_in),
 * or \c NULL
 * \param data_len Length of payload data in octets, or \c 0
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES error code
 * \note This function is only supposed to be called by the AES drivers.
 */
AESResult aes_auth_crypt_start(uint32_t ctrl
         , AESKeyArea key_area, const AESNonce* iv
         , const void *adata, AESSize adata_len
         , const void *data_in, void *data_out, AESSize data_len
         , struct process *notify_proc);

/** \brief Checks the status of the AES authentication/crypto operation
 * \retval CRYPTO_PENDING Result not yet available
 * , and no error occurred
 * \retval CRYPTO_OK Result available
 * \note This function is only supposed to be called by the AES drivers.
 */
AESResult aes_auth_crypt_check_status(void);

/** \brief Gets the result of the AES authentication/crypto operation
 * \param iv Pointer to 128-bit result initialization vector, or \c NULL
 * \param tag Pointer to 128-bit result tag, or \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES error code
 * \note This function must be called only after \c aes_auth_crypt_start().
 * \note This function is only supposed to be called by the AES drivers.
 */
AESResult aes_auth_crypt_get_result(AESNonce *iv, AESBlock* tag);

/** @} */

#endif /* AES_H_ */

/**
 * @}
 * @}
 */
