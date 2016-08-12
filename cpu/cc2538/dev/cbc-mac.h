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
 * \addtogroup cc2538-aes
 * @{
 *
 * \defgroup cc2538-cbc-mac cc2538 AES-CBC-MAC
 *
 * Driver for the cc2538 AES-CBC-MAC mode of the security core
 * @{
 *
 * \file
 * Header file for the cc2538 AES-CBC-MAC driver
 */
#ifndef CBC_MAC_H_
#define CBC_MAC_H_

#include "contiki.h"
#include "dev/aes.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name AES-CBC-MAC constants
 * @{
 */
#define CBC_MAC_MAC_LEN         AES_TAG_LEN
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES-CBC-MAC functions
 * @{
 */

/** \brief Starts a CBC-MAC authentication operation
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param mdata Pointer to message to authenticate in SRAM
 * \param mdata_len Length of message to authenticate in octets
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CBC-MAC error code
 * \warning CBC-MAC is not secure for variable-length messages. There are a few
 * workarounds that can be implemented by the caller, like prepending the
 * message length to the first block of the message before passing it.
 */
uint8_t cbc_mac_auth_start(uint8_t key_area, const void *mdata,
                           uint16_t mdata_len, struct process *process);

/** \brief Checks the status of the CBC-MAC authentication operation
 * \retval false Result not yet available, and no error occurred
 * \retval true Result available, or error occurred
 */
#define cbc_mac_auth_check_status aes_auth_crypt_check_status

/** \brief Gets the result of the CBC-MAC authentication operation
 * \param mac_in Pointer to 128-bit input MAC, or \c NULL
 * \param mac_out Pointer to 128-bit output MAC, or \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES/CBC-MAC error code
 * \note This function must be called only after \c cbc_mac_auth_start().
 */
uint8_t cbc_mac_auth_get_result(const void *mac_in, void *mac_out);

/** @} */

#endif /* CBC_MAC_H_ */

/**
 * @}
 * @}
 */
