/*
 * Copyright (c) 2015, Lars Schmertmann <SmallLars@t-online.de>.
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
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "lib/aes-128.h"
#include "lib/aes-adv.h"
#include <string.h>

/*---------------------------------------------------------------------------*/

static aes_128_state_t
init(void)
{
  if(AES_ADV.init() == AES_ADV_OK) {
    return AES_128_OK;
  }
  return AES_128_ERROR;
}
/*---------------------------------------------------------------------------*/
static void
set_key(const uint8_t *key, size_t key_len)
{
  AES_ADV.set_data(AES_ADV.KEY, key, key_len);
}
/*---------------------------------------------------------------------------*/
static void
encrypt(uint8_t *result, const uint8_t *plaintext)
{
  AES_ADV.set_data(AES_ADV.IV, NULL, 0);
  AES_ADV.set_data(AES_ADV.DATA, plaintext, 16);
  AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);
  AES_ADV.get_data(result, AES_ADV.CBC_RESULT, 16);
}
/*---------------------------------------------------------------------------*/
static void
clear(void)
{
  AES_ADV.clear();
}
/*---------------------------------------------------------------------------*/
const struct aes_128_driver aes_128_driver = {
  init,
  set_key,
  encrypt,
  NULL,     /* not suported */
  clear
};
/*---------------------------------------------------------------------------*/
