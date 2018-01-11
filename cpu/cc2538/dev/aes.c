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
#include "dev/nvic.h"
#include "dev/aes.h"
#include "reg.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
uint8_t
aes_load_keys(const void *keys, uint8_t key_size, uint8_t count,
              uint8_t start_area)
{
  uint32_t aes_key_store_size;
  uint32_t areas;
  uint64_t aligned_keys[AES_KEY_AREAS * 128 / 8 / sizeof(uint64_t)];
  int i;

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return CRYPTO_RESOURCE_IN_USE;
  }

  /* 192-bit keys must be padded to 256 bits */
  if(key_size == AES_KEY_STORE_SIZE_KEY_SIZE_192) {
    for(i = 0; i < count; i++) {
      rom_util_memcpy(&aligned_keys[i << 2], &((const uint64_t *)keys)[i * 3],
                      192 / 8);
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
/*---------------------------------------------------------------------------*/
uint8_t
aes_auth_crypt_start(uint32_t ctrl, uint8_t key_area, const void *iv,
                     const void *adata, uint16_t adata_len,
                     const void *data_in, void *data_out, uint16_t data_len,
                     struct process *process)
{
  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return CRYPTO_RESOURCE_IN_USE;
  }

  /* Workaround for AES registers not retained after PM2 */
  REG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_DMA_IN_DONE |
                         AES_CTRL_INT_EN_RESULT_AV;

  REG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_AES;
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  REG(AES_KEY_STORE_READ_AREA) = key_area;

  /* Wait until key is loaded to the AES module */
  while(REG(AES_KEY_STORE_READ_AREA) & AES_KEY_STORE_READ_AREA_BUSY);

  /* Check for Key Store read error */
  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_RD_ERR) {
    /* Clear the Keystore Read error bit */
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_KEY_ST_RD_ERR;
    /* Disable the master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return AES_KEYSTORE_READ_ERROR;
  }

  if(iv != NULL) {
    /* Write initialization vector */
    REG(AES_AES_IV_0) = ((const uint32_t *)iv)[0];
    REG(AES_AES_IV_1) = ((const uint32_t *)iv)[1];
    REG(AES_AES_IV_2) = ((const uint32_t *)iv)[2];
    REG(AES_AES_IV_3) = ((const uint32_t *)iv)[3];
  }

  /* Program AES authentication/crypto operation */
  REG(AES_AES_CTRL) = ctrl;

  /* Write the length of the payload block (lo) */
  REG(AES_AES_C_LENGTH_0) = data_len;
  /* Write the length of the payload block (hi) */
  REG(AES_AES_C_LENGTH_1) = 0;

  /* For combined modes only (CCM or GCM) */
  if(ctrl & (AES_AES_CTRL_CCM | AES_AES_CTRL_GCM)) {
    /* Write the length of the AAD data block (may be non-block size-aligned) */
    REG(AES_AES_AUTH_LENGTH) = adata_len;

    if(adata_len != 0) {
      /* Configure DMAC to fetch the AAD data
       * Enable DMA channel 0 */
      REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
      /* Base address of the AAD data buffer */
      REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)adata;
      /* AAD data length in bytes */
      REG(AES_DMAC_CH0_DMALENGTH) = adata_len;

      /* Wait for completion of the AAD data transfer, DMA_IN_DONE */
      while(!(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_IN_DONE));

      /* Check for the absence of error */
      if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
        /* Clear the DMA error */
        REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR;
        /* Disable the master control / DMA clock */
        REG(AES_CTRL_ALG_SEL) = 0x00000000;
        return CRYPTO_DMA_BUS_ERROR;
      }

      /* Clear interrupt status */
      REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE;
    }
  }

  /* Enable result available bit in interrupt enable */
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_RESULT_AV;

  if(process != NULL) {
    crypto_register_process_notification(process);
    NVIC_ClearPendingIRQ(AES_IRQn);
    NVIC_EnableIRQ(AES_IRQn);
  }

  if(data_len != 0) {
    /* Configure DMAC
     * Enable DMA channel 0 */
    REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the input payload data buffer */
    REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)data_in;
    /* Input payload data length in bytes */
    REG(AES_DMAC_CH0_DMALENGTH) = data_len;

    if(data_out != NULL) {
      /* Enable DMA channel 1 */
      REG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH_CTRL_EN;
      /* Base address of the output payload data buffer */
      REG(AES_DMAC_CH1_EXTADDR) = (uint32_t)data_out;
      /* Output payload data length in bytes */
      REG(AES_DMAC_CH1_DMALENGTH) = data_len;
    }
  }

  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
aes_auth_crypt_check_status(void)
{
  return !!(REG(AES_CTRL_INT_STAT) &
            (AES_CTRL_INT_STAT_DMA_BUS_ERR | AES_CTRL_INT_STAT_KEY_ST_WR_ERR |
             AES_CTRL_INT_STAT_KEY_ST_RD_ERR | AES_CTRL_INT_STAT_RESULT_AV));
}
/*---------------------------------------------------------------------------*/
uint8_t
aes_auth_crypt_get_result(void *iv, void *tag)
{
  uint32_t aes_ctrl_int_stat;

  aes_ctrl_int_stat = REG(AES_CTRL_INT_STAT);
  /* Clear the error bits */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR |
                          AES_CTRL_INT_CLR_KEY_ST_WR_ERR |
                          AES_CTRL_INT_CLR_KEY_ST_RD_ERR;

  NVIC_DisableIRQ(AES_IRQn);
  crypto_register_process_notification(NULL);

  /* Disable the master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;

  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    return CRYPTO_DMA_BUS_ERROR;
  }
  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_KEY_ST_WR_ERR) {
    return AES_KEYSTORE_WRITE_ERROR;
  }
  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_KEY_ST_RD_ERR) {
    return AES_KEYSTORE_READ_ERROR;
  }

  if(iv != NULL || tag != NULL) {
    /* Read result
     * Wait for the context ready bit */
    while(!(REG(AES_AES_CTRL) & AES_AES_CTRL_SAVED_CONTEXT_READY));

    if(iv != NULL) {
      /* Read the initialization vector registers */
      ((uint32_t *)iv)[0] = REG(AES_AES_IV_0);
      ((uint32_t *)iv)[1] = REG(AES_AES_IV_1);
      ((uint32_t *)iv)[2] = REG(AES_AES_IV_2);
      ((uint32_t *)iv)[3] = REG(AES_AES_IV_3);
    }

    if(tag != NULL) {
      /* Read the tag registers */
      ((uint32_t *)tag)[0] = REG(AES_AES_TAG_OUT_0);
      ((uint32_t *)tag)[1] = REG(AES_AES_TAG_OUT_1);
      ((uint32_t *)tag)[2] = REG(AES_AES_TAG_OUT_2);
      ((uint32_t *)tag)[3] = REG(AES_AES_TAG_OUT_3);
    }
  }

  /* Clear the interrupt status */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  return CRYPTO_SUCCESS;
}

/** @} */
