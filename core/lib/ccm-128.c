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
 *         AES_ADV-based CCM implementation (RFC 3610).
 * \author
 *         Lars Schmertmann <SmallLars@t-online.de>
 */

#include "ccm-128.h"
#include "aes-adv.h"
#include <string.h>

/*---------------------------------------------------------------------------*/

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
void
print_hex(uint8_t *d, uint8_t l)
{
  int i;
  for(i = 0; i < l; i++) {
    printf("%02X", d[i]);
  }
}
#else
#define PRINTF(...)
#endif

#define min(x, y) ((x) < (y) ? (x) : (y))

static size_t nonce_len;
static size_t mac_len;

/*---------------------------------------------------------------------------*/

static void
set_key(const uint8_t *key, size_t key_len)
{
  AES_ADV.set_data(AES_ADV.KEY, key, key_len);
}
/*---------------------------------------------------------------------------*/

static ccm_128_return_t
configure(size_t nonce_length, size_t mac_length)
{
  nonce_len = nonce_length;
  mac_len = mac_length;
  return CCM_128_OK;
}
/*---------------------------------------------------------------------------*/

static void
aead(uint32_t mac_only, const uint8_t *nonce, uint8_t *data, size_t data_len, const uint8_t *adata, size_t adata_len)
{

  uint8_t abs_0[16];      /* memory for a_0, b_0 and s_0 */
  uint32_t i;

  /* initialize iv */
  AES_ADV.set_data(AES_ADV.IV, NULL, 0);

  /* generate b_0 */
  abs_0[0] = (64 * (adata_len > 0 ? 1 : 0)) + (8 * ((mac_len - 2) / 2)) + (14 - nonce_len);
  memcpy(abs_0 + 1, nonce, nonce_len);
  for(i = 15; i > nonce_len; i--) {
    abs_0[i] = (data_len >> ((15 - i) * 8)) & 0xFF;
  }
#if DEBUG
  printf("b_0: ");
  print_hex(abs_0, 16);
  printf("\n");
#endif
  AES_ADV.set_data(AES_ADV.DATA, abs_0, 16);
  AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);

  /* use additional data for mac calculation */
  if(adata != NULL && adata_len > 0) {
    uint8_t lenblock[16];
    if(adata_len < 65280) {  /* < (2^16 - 2^8) */
      lenblock[0] = (adata_len >> 8) & 0xFF;
      lenblock[1] = (adata_len >> 0) & 0xFF;
      i = 14;
    } else { /* >= (2^16 - 2^8) */
      lenblock[0] = 0xFF;
      lenblock[1] = 0xFE;
      lenblock[2] = (adata_len >> 24) & 0xFF;
      lenblock[3] = (adata_len >> 16) & 0xFF;
      lenblock[4] = (adata_len >> 8) & 0xFF;
      lenblock[5] = (adata_len >> 0) & 0xFF;
      i = 10;
    }
    memcpy(lenblock + 16 - i, adata, i);
#if DEBUG
    printf("b_1: ");
    print_hex(lenblock, 16 - i + min(i, adata_len));
    printf("\n");
#endif
    AES_ADV.set_data(AES_ADV.DATA, lenblock, 16 - i + min(i, adata_len));
    AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);

    for(; i < adata_len; i += 16) {
#if DEBUG
      printf("b_%u: ", (i / 16) + 2);
      print_hex(adata + i, min(16, adata_len - i));
      printf("\n");
#endif
      AES_ADV.set_data(AES_ADV.DATA, adata + i, min(16, adata_len - i));
      AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);
    }
  }

  /* initalize counter. nonce is already included */
  abs_0[0] = 14 - nonce_len;
  for(i = 14; i > nonce_len; i--) {
    abs_0[i] = 0;
  }
  abs_0[15] = 1;
  AES_ADV.set_data(AES_ADV.CTR, abs_0, 16);

  /* crypto loop */
  for(i = 0; i < data_len; i += 16) {
#if DEBUG
    uint8_t temp[16];
    printf("a_%u :", (i / 16) + 1);
    print_hex(temp, 16);
    printf("\n");
#endif
    AES_ADV.set_data(AES_ADV.DATA, data + i, min(16, data_len - i));
    AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CCM);
    /* replace the input with ctr result if needed */
    if(!mac_only) {
      AES_ADV.get_data(data + i, AES_ADV.CTR_RESULT, min(16, data_len - i));
    }
  }

  /* generate a_0, encrypt to s_0 and x-or with cbc result */
  abs_0[15] = 0;
#if DEBUG
  printf("a_0 :");
  print_hex(abs_0, 16);
  printf("\n");
#endif
  AES_ADV.set_data(AES_ADV.CTR, abs_0, 16);

  /* read/write cbc result */
  AES_ADV.get_data(abs_0, AES_ADV.CBC_RESULT, 16);
  AES_ADV.set_data(AES_ADV.DATA, abs_0, 16);
  AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CTR);
}
/*---------------------------------------------------------------------------*/

static void
get_mac(uint8_t *dst)
{
  AES_ADV.get_data(dst, AES_ADV.CTR_RESULT, mac_len);
}
/*---------------------------------------------------------------------------*/

const struct ccm_128_driver ccm_128_driver = {
  set_key,
  configure,
  aead,
  get_mac
};
/*---------------------------------------------------------------------------*/
