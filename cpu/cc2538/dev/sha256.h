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
 * \addtogroup cc2538-crypto
 * @{
 *
 * \defgroup cc2538-sha256 cc2538 SHA-256
 *
 * Driver for the cc2538 SHA-256 mode of the security core
 * @{
 *
 * \file
 * Header file for the cc2538 SHA-256 driver
 */
#ifndef SHA256_H_
#define SHA256_H_

#include "contiki.h"
#include "dev/crypto.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name SHA-256 structures
 * @{
 */
typedef struct {
  uint64_t length;
  uint32_t state[8];
  uint32_t curlen;
  uint8_t  buf[64];
  uint8_t  new_digest;
  uint8_t  final_digest;
} sha256_state_t;
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SHA-256 functions
 * @{
 */

/** \brief Initializes the hash state
 * \param state Pointer to hash state to initialize
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/SHA256 error code
 */
uint8_t sha256_init(sha256_state_t *state);

/** \brief Processes a block of memory through the hash
 * \param state Pointer to hash state
 * \param data Pointer to the data to hash
 * \param len Length of the data to hash in bytes (octets)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/SHA256 error code
 * \note This function must be called only after \c sha256_init().
 */
uint8_t sha256_process(sha256_state_t *state, const void *data, uint32_t len);

/** \brief Terminates hash session to get the digest
 * \param state Pointer to hash state
 * \param hash Pointer to hash
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/SHA256 error code
 * \note This function must be called only after \c sha256_process().
 */
uint8_t sha256_done(sha256_state_t *state, void *hash);

/** @} */

#endif /* SHA256_H_ */

/**
 * @}
 * @}
 */
