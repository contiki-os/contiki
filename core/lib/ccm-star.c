/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
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

/**
 * \file
 *         AES_128-based CCM* implementation.
 * \author
 *         Original: Konrad Krentz <konrad.krentz@gmail.com>
 *         Generified version: Justin King-Lacroix <justin.kinglacroix@gmail.com>
 */

#include "ccm-star.h"
#include "lib/aes-128.h"
#include <string.h>

/* see RFC 3610 */
#define CCM_STAR_AUTH_FLAGS(Adata, M) ((Adata ? (1u << 6) : 0) | (((M - 2u) >> 1) << 3) | 1u)
#define CCM_STAR_ENCRYPTION_FLAGS     1

/*---------------------------------------------------------------------------*/
static void
set_nonce(uint8_t *iv,
    uint8_t flags,
    const uint8_t *nonce,
    uint8_t counter)
{
  /* 1 byte||          8 bytes        ||    4 bytes    || 1 byte  || 2 bytes */
  /* flags || extended_source_address || frame_counter || sec_lvl || counter */

  iv[0] = flags;
  memcpy(iv + 1, nonce, CCM_STAR_NONCE_LENGTH);
  iv[14] = 0;
  iv[15] = counter;
}
/*---------------------------------------------------------------------------*/
/* XORs the block m[pos] ... m[pos + 15] with K_{counter} */
static void
ctr_step(const uint8_t *nonce,
    uint8_t pos,
    uint8_t *m_and_result,
    uint8_t m_len,
    uint8_t counter)
{
  uint8_t a[AES_128_BLOCK_SIZE];
  uint8_t i;
  
  set_nonce(a, CCM_STAR_ENCRYPTION_FLAGS, nonce, counter);
  AES_128.encrypt(a);
  
  for(i = 0; (pos + i < m_len) && (i < AES_128_BLOCK_SIZE); i++) {
    m_and_result[pos + i] ^= a[i];
  }
}
/*---------------------------------------------------------------------------*/
static void
mic(const uint8_t *m,  uint8_t m_len,
    const uint8_t *nonce,
    const uint8_t *a,  uint8_t a_len,
    uint8_t *result,
    uint8_t mic_len)
{
  uint8_t x[AES_128_BLOCK_SIZE];
  uint8_t pos;
  uint8_t i;
  
  set_nonce(x, CCM_STAR_AUTH_FLAGS(a_len, mic_len), nonce, m_len);
  AES_128.encrypt(x);
  
  if(a_len > 0) {
    x[1] = x[1] ^ a_len;
    for(i = 2; (i - 2 < a_len) && (i < AES_128_BLOCK_SIZE); i++) {
      x[i] ^= a[i - 2];
    }
    
    AES_128.encrypt(x);
    
    pos = 14;
    while(pos < a_len) {
      for(i = 0; (pos + i < a_len) && (i < AES_128_BLOCK_SIZE); i++) {
        x[i] ^= a[pos + i];
      }
      pos += AES_128_BLOCK_SIZE;
      AES_128.encrypt(x);
    }
  }
  
  if(m_len > 0) {
    m = a + a_len;
    pos = 0;
    while(pos < m_len) {
      for(i = 0; (pos + i < m_len) && (i < AES_128_BLOCK_SIZE); i++) {
        x[i] ^= m[pos + i];
      }
      pos += AES_128_BLOCK_SIZE;
      AES_128.encrypt(x);
    }
  }
  
  ctr_step(nonce, 0, x, AES_128_BLOCK_SIZE, 0);
  
  memcpy(result, x, mic_len);
}
/*---------------------------------------------------------------------------*/
static void
ctr(uint8_t *m, uint8_t m_len, const uint8_t* nonce)
{
  uint8_t pos;
  uint8_t counter;
  
  pos = 0;
  counter = 1;
  while(pos < m_len) {
    ctr_step(nonce, pos, m, m_len, counter++);
    pos += AES_128_BLOCK_SIZE;
  }
}
/*---------------------------------------------------------------------------*/
static void set_key(const uint8_t *key) {
    AES_128.set_key((uint8_t*)key);
}
/*---------------------------------------------------------------------------*/
const struct ccm_star_driver ccm_star_driver = {
  mic,
  ctr,
  set_key
};
/*---------------------------------------------------------------------------*/
