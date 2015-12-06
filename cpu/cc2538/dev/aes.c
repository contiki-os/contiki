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
 * \addtogroup cc2538-aes
 * @{
 *
 * \file
 * Implementation of the cc2538 AES driver
 */
#include "contiki.h"
#include "dev/rom-util.h"
#include "dev/aes.h"
#include "reg.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
uint8_t
aes_load_keys(const void *keys, uint8_t key_size, uint8_t count,
              uint8_t start_area)
{
  uint32_t aes_key_store_size;
  uint32_t areas;
  uint64_t aligned_keys[16];
  int i;

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return CRYPTO_RESOURCE_IN_USE;
  }

  /* 192-bit keys must be padded to 256 bits */
  if(key_size == AES_KEY_STORE_SIZE_KEY_SIZE_192) {
    for(i = 0; i < count; i++) {
      rom_util_memcpy(&aligned_keys[i << 2], &((uint64_t *)keys)[i * 3], 24);
      aligned_keys[(i << 2) + 3] = 0;
    }
  }

  /* Change count to the number of 128-bit key areas */
  if(key_size != AES_KEY_STORE_SIZE_KEY_SIZE_128) {
    count <<= 1;
  }

  /* The keys base address needs to be 4-byte aligned */
  if(key_size != AES_KEY_STORE_SIZE_KEY_SIZE_192) {
    rom_util_memcpy(aligned_keys, keys, count << 4);
  }

  /* Workaround for AES registers not retained after PM2 */
  REG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_DMA_IN_DONE |
                         AES_CTRL_INT_EN_RESULT_AV;

  /* Configure master control module */
  REG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_KEYSTORE;

  /* Clear any outstanding events */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  /* Configure key store module (areas, size)
   * Note that writing AES_KEY_STORE_SIZE deletes all stored keys */
  aes_key_store_size = REG(AES_KEY_STORE_SIZE);
  if((aes_key_store_size & AES_KEY_STORE_SIZE_KEY_SIZE_M) != key_size) {
    REG(AES_KEY_STORE_SIZE) = (aes_key_store_size &
                               ~AES_KEY_STORE_SIZE_KEY_SIZE_M) | key_size;
  }

  /* Free possibly already occupied key areas */
  areas = ((0x00000001 << count) - 1) << start_area;
  REG(AES_KEY_STORE_WRITTEN_AREA) = areas;

  /* Enable key areas to write */
  REG(AES_KEY_STORE_WRITE_AREA) = areas;

  /* Configure DMAC
   * Enable DMA channel 0 */
  REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;

  /* Base address of the keys in ext. memory */
  REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)aligned_keys;

  /* Total keys length in bytes (e.g. 16 for 1 x 128-bit key) */
  REG(AES_DMAC_CH0_DMALENGTH) = (REG(AES_DMAC_CH0_DMALENGTH) &
                                 ~AES_DMAC_CH_DMALENGTH_DMALEN_M) |
                                (count << (4 + AES_DMAC_CH_DMALENGTH_DMALEN_S));

  /* Wait for operation to complete */
  while(!(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV));

  /* Check for absence of errors in DMA and key store */
  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR;
    /* Disable master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return CRYPTO_DMA_BUS_ERROR;
  }
  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_WR_ERR) {
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_KEY_ST_WR_ERR;
    /* Disable master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return AES_KEYSTORE_WRITE_ERROR;
  }

  /* Acknowledge the interrupt */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  /* Disable master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;

  /* Check status, if error return error code */
  if((REG(AES_KEY_STORE_WRITTEN_AREA) & areas) != areas) {
    return AES_KEYSTORE_WRITE_ERROR;
  }

  return CRYPTO_SUCCESS;
}

/** @} */
