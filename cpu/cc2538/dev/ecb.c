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
 * \addtogroup cc2538-ecb
 * @{
 *
 * \file
 * Implementation of the cc2538 AES-ECB driver
 */
#include "contiki.h"
#include "dev/ecb.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
uint8_t
ecb_crypt_start(uint8_t encrypt, uint8_t key_area, const void *mdata_in,
                void *mdata_out, uint16_t mdata_len, struct process *process)
{
  uint32_t ctrl;

  /* Program AES-ECB crypto operation */
  ctrl = encrypt ? AES_AES_CTRL_DIRECTION_ENCRYPT : 0; /* En/decryption */

  return aes_auth_crypt_start(ctrl, key_area, NULL, NULL, 0,
                              mdata_in, mdata_out, mdata_len, process);
}
/*---------------------------------------------------------------------------*/
int8_t
ecb_crypt_check_status(void)
{
  return aes_auth_crypt_check_status() ? aes_auth_crypt_get_result(NULL, NULL) :
                                         CRYPTO_PENDING;
}

/** @} */
