/*
 * Copyright (c) 2014, Robert Quattlebaum
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
#ifndef CRYPT_AES_H_
#define CRYPT_AES_H_

#include <stdint.h>
#include "aes-arch.h"

typedef uint8_t[16] aes128_key_t;
typedef uint8_t[16] aes_block_t;
typedef uint8_t     aes_context_t;

#ifndef AES_DEFAULT_CONTEXT_ID
/* The default context id is zero. */
#define AES_DEFAULT_CONTEXT_ID    0
#endif

#ifndef AES_CONTEXT_ID_COUNT
/* The default context id count is 1. */
#define AES_CONTEXT_ID_COUNT      1
#endif

/** @brief Sets the key to be used for subsequent `aes128_*` calls. */
void aes128_set_key(aes_context_t contextid, const aes128_key_t key);

/** @brief Single-block In-place AES128 ECB encryption */
void aes128_ecb_encrypt(aes_context_t contextid, aes_block_t block);

/** @brief Single-block In-place AES128 ECB decryption.
 *
 *  @note May not be available on some platforms. */
void aes128_ecb_decrypt(aes_context_t contextid, aes_block_t block);

typedef uint8_t aes_ccm_mode_t;

void aes128_ccm_encrypt(
  aes_context_t contextid,
  aes_ccm_mode_t mode,
  uint8_t *msg, uint16_t msglen,
  uint8_t *tag, uint16_t taglen,
  const uint8_t *nonce, uint16_t nonceLen,
  const uint8_t *header, uint16_t headerLen
);

bool aes128_ccm_decrypt(
  aes_context_t contextid,
  aes_ccm_mode_t mode,
  uint8_t *msg, uint16_t msglen,
  const uint8_t *tag,	uint16_t taglen,
  const uint8_t *nonce, uint16_t nonceLen,
  const uint8_t *header, uint16_t headerLen
);

#endif /* CRYPT_AES_H_ */
