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

#include "cmac-128.h"
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

/*---------------------------------------------------------------------------*/

void update(CMAC_128_CTX *ctx, uint8_t *data, size_t data_len);
void finish(CMAC_128_CTX *ctx, uint8_t *mac, size_t mac_len);

/*---------------------------------------------------------------------------*/

void
init(CMAC_128_CTX *ctx, uint8_t *key, size_t key_length)
{
  ctx->buf_pos = 0;
  memset(ctx->mac, 0, 16);

  if(key_length == 16) {
    memcpy(ctx->key, key, 16);
#if DEBUG
    printf("Key16    ");
    print_hex(ctx->key, 16);
    printf("\n");
#endif
    return;
  }

  memset(ctx->key, 0, 16);
  update(ctx, key, key_length);
  finish(ctx, ctx->key, 16);

  ctx->buf_pos = 0;
  memset(ctx->mac, 0, 16);

#if DEBUG
  printf("KeyXX    ");
  print_hex(ctx->key, 16);
  printf("\n");
#endif
}
void
update(CMAC_128_CTX *ctx, uint8_t *data, size_t data_len)
{
  uint32_t i = 0;

  AES_ADV.clear();
  AES_ADV.set_data(AES_ADV.KEY, ctx->key, 16);
  AES_ADV.set_data(AES_ADV.IV, ctx->mac, 16);

  while(data_len > 0 && ctx->buf_pos < 16) {
    ctx->buf[ctx->buf_pos++] = data[i++];
    data_len -= 1;
  }
  if(data_len == 0) {
    return;
  }

  if(data_len > 0) {
    AES_ADV.set_data(AES_ADV.DATA, ctx->buf, 16);
    AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);
  }

  for(; data_len > 16; i += 16) {
    AES_ADV.set_data(AES_ADV.DATA, data + i, 16);
    AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);
    data_len -= 16;
  }
  memcpy(ctx->buf, data + i, data_len);
  ctx->buf_pos = data_len;

  AES_ADV.get_data(ctx->mac, AES_ADV.CBC_RESULT, 16);
}
void
finish(CMAC_128_CTX *ctx, uint8_t *mac, size_t mac_len)
{
  uint32_t i;
  uint8_t subkey[16];

  AES_ADV.clear();
  AES_ADV.set_data(AES_ADV.KEY, ctx->key, 16);

  /* Calculate Subkey - BEGIN */
  AES_ADV.set_data(AES_ADV.DATA, NULL, 0);
  AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);
  AES_ADV.get_data(subkey, AES_ADV.CBC_RESULT, 16);
#if DEBUG
  printf("K0       ");
  print_hex(subkey, 16);
  printf("\n");
#endif
  i = (ctx->buf_pos == 16 ? 1 : 2);
  while(i > 0) {
    uint8_t j, msb = subkey[0] & 0x80;
    for(j = 0; j < 15; j++) {
      subkey[j] <<= 1;
      subkey[j] |= (subkey[j + 1] >> 7);
    }
    subkey[15] <<= 1;
    if(msb) {
      subkey[15] ^= 0x87;
    }
    i--;
  }
#if DEBUG
  printf("KX       ");
  print_hex(subkey, 16);
  printf("\n");
#endif
  /* Calculate Subkey - END */

  for(i = 0; i < ctx->buf_pos; i++) {
    subkey[i] ^= ctx->buf[i];
  }
  if(i < 16) {
    subkey[i] ^= 128;
  }

  AES_ADV.clear();
  AES_ADV.set_data(AES_ADV.KEY, ctx->key, 16);
  AES_ADV.set_data(AES_ADV.IV, ctx->mac, 16);

  AES_ADV.set_data(AES_ADV.DATA, subkey, 16);
  AES_ADV.exec(AES_ADV_ENCRYPT, AES_ADV_CBC);
  AES_ADV.get_data(mac, AES_ADV.CBC_RESULT, mac_len);

  ctx->buf_pos = 0;
  memset(ctx->mac, 0, 16);

#if DEBUG
  printf("AES_CMAC ");
  print_hex(mac, mac_len);
  printf("\n");
#endif
}
/*---------------------------------------------------------------------------*/

const struct cmac_128_driver cmac_128_driver = {
  init,
  update,
  finish
};
/*---------------------------------------------------------------------------*/
