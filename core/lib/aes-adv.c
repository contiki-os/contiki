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
 *         AES_ADV implementation.
 * \author
 *         Lars Schmertmann <SmallLars@t-online.de>
 */

#include "lib/aes-adv.h"
#include "lib/aes-128.h"
#include <string.h>

/*---------------------------------------------------------------------------*/

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTHEX(...) print_hex(__VA_ARGS__)
static void
print_hex(uint8_t *d, uint8_t l)
{
  int i;
  for(i = 0; i < l; i++) {
    printf("%02X", d[i]);
  }
  printf("\n");
}
#else
#define PRINTHEX(...)
#endif

/*---------------------------------------------------------------------------*/
static uint8_t data[AES_ADV_BLOCK_SIZE];
static uint8_t ctr[AES_ADV_BLOCK_SIZE];
static uint8_t ctr_result[AES_ADV_BLOCK_SIZE];
static uint8_t cbc_result[AES_ADV_BLOCK_SIZE];
/*---------------------------------------------------------------------------*/
static aes_adv_result_t
init(void)
{
  if(AES_128.init() == AES_128_OK) {
    return AES_ADV_OK;
  }
  return AES_ADV_ERROR;
}
/*---------------------------------------------------------------------------*/
static void
get_data(uint8_t *dst, const void *src, size_t len)
{
  if(len > 16) {
    len = 16;
  }

  memcpy(dst, src, len);
}
/*---------------------------------------------------------------------------*/
static void
set_data(void *dst, const uint8_t *src, size_t len)
{
  if(len > 16) {
    len = 16;
  }

  if(dst == AES_ADV.KEY) {
    AES_128.set_key(src, len);
    return;
  }

  memset(dst, 0, AES_ADV_BLOCK_SIZE);
  memcpy(dst, src, len);
}
/*---------------------------------------------------------------------------*/
static aes_adv_result_t
aes_exec(aes_adv_direction_t dir, aes_adv_mode_t mode)
{
  uint8_t i;

  if(dir == AES_ADV_DECRYPT && mode == AES_ADV_CBC) {
    return AES_ADV_NOT_SUPPORTED;
  }

  if(mode == AES_ADV_CCM || mode == AES_ADV_CTR) {
    AES_128.encrypt(ctr_result, ctr);
    for(i = 0; i < AES_ADV_BLOCK_SIZE; i++) {
      ctr_result[i] ^= data[i];
    }
    ctr[15]++;
    for(i = AES_ADV_BLOCK_SIZE - 1; i > 0; --i) {
      if(ctr[i] != 0) {
        break;
      }
      ctr[i - 1]++;
    }
  }
  if(mode == AES_ADV_CCM || mode == AES_ADV_CBC) {
    PRINTHEX(data, 16);
    for(i = 0; i < AES_ADV_BLOCK_SIZE; i++) {
      cbc_result[i] ^= data[i];
    }
    PRINTHEX(cbc_result, 16);
    AES_128.encrypt(cbc_result, cbc_result);
    PRINTHEX(cbc_result, 16);
  }

  return AES_ADV_OK;
}
/*---------------------------------------------------------------------------*/
static void
clear(void)
{
  AES_128.clear();
  memset(data, 0, AES_ADV_BLOCK_SIZE);
  memset(ctr, 0, AES_ADV_BLOCK_SIZE);
  memset(ctr_result, 0, AES_ADV_BLOCK_SIZE);
  memset(cbc_result, 0, AES_ADV_BLOCK_SIZE);
}
/*---------------------------------------------------------------------------*/
const struct aes_adv_driver aes_adv_driver = {
  (void *)1,
  cbc_result,
  data,
  ctr,
  ctr_result,
  cbc_result,
  init,
  get_data,
  set_data,
  aes_exec,
  clear
};
/*---------------------------------------------------------------------------*/
