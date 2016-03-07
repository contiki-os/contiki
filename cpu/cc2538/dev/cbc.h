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
 * \defgroup cc2538-cbc cc2538 AES-CBC
 *
 * Driver for the cc2538 AES-CBC mode of the security core
 * @{
 *
 * \file
 * Header file for the cc2538 AES-CBC driver
 */
#ifndef CBC_H_
#define CBC_H_

#include "contiki.h"
#include "dev/aes.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name AES-CBC constants
 * @{
 */
#define CBC_IV_LEN      AES_IV_LEN
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES-CBC functions
 * @{
 */

/** \brief Starts a CBC crypto operation
 * \param encrypt \c true to encrypt, or \c false to decrypt
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param iv Pointer to 128-bit initialization vector
 * \param mdata_in Pointer to input message in SRAM
 * \param mdata_out Pointer to output message in SRAM (may be \p mdata_in)
 * \param mdata_len Length of message in octets
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CBC error code
 */
uint8_t cbc_crypt_start(uint8_t encrypt, uint8_t key_area, const void *iv,
                        const void *mdata_in, void *mdata_out,
                        uint16_t mdata_len, struct process *process);

/** \brief Checks the status of the CBC crypto operation
 * \return \c CRYPTO_PENDING if operation still pending, \c CRYPTO_SUCCESS if
 * successful, or CRYPTO/AES/CBC error code
 * \note This function must be called only after \c cbc_crypt_start().
 */
int8_t cbc_crypt_check_status(void);

/** @} */

#endif /* CBC_H_ */

/**
 * @}
 * @}
 */
