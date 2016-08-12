/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 *         CCM* header implementation exploiting the JN516x
 *         cryptographic co-processor
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "ccm-star.h"
#include <AHI_AES.h>
#include <string.h>

static tsReg128 current_key;
static int current_key_is_new = 1;

/*---------------------------------------------------------------------------*/
static void
aead(const uint8_t *nonce,
     uint8_t *m, uint8_t m_len,
     const uint8_t *a, uint8_t a_len,
     uint8_t *result, uint8_t mic_len,
     int forward)
{
  tsReg128 nonce_aligned;
  memcpy(&nonce_aligned, nonce, sizeof(nonce_aligned));
  if(forward) {
    bACI_CCMstar(
      &current_key,
      current_key_is_new,
      XCV_REG_AES_SET_MODE_CCM,
      mic_len,
      a_len,
      m_len,
      &nonce_aligned,
      (uint8_t *)a,
      (uint8_t *)m,
      (uint8_t *)m,
      result,
      NULL
      );
  } else {
    bool_t auth;
    bACI_CCMstar(
      &current_key,
      current_key_is_new,
      XCV_REG_AES_SET_MODE_CCM_D,
      mic_len,
      a_len,
      m_len,
      &nonce_aligned,
      (uint8_t *)a,
      (uint8_t *)m,
      (uint8_t *)m,
      (uint8_t *)a + a_len + m_len,
      &auth
      );
    /* To comply with the CCM_STAR interface, copy MIC to result in case of success */
    if(result != NULL) {
      if(auth) {
        memcpy(result, a + a_len + m_len, mic_len);
      } else {
        /* Otherwise, corrupt the result */
        memcpy(result, a + a_len + m_len, mic_len);
        result[0]++;
      }
    }
  }

  current_key_is_new = 0;
}
/*---------------------------------------------------------------------------*/
static void
set_key(const uint8_t *key)
{
  if(memcmp(&current_key, key, sizeof(current_key)) == 0) {
    current_key_is_new = 0;
  } else {
    memcpy(&current_key, key, sizeof(current_key));
    current_key_is_new = 1;
  }
}
/*---------------------------------------------------------------------------*/
const struct ccm_star_driver ccm_star_driver_jn516x = {
  set_key,
  aead
};
/*---------------------------------------------------------------------------*/
