/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 * \addtogroup cc2538-ccm
 * @{
 *
 * \file
 * Implementation of the cc2538 AES-CCM driver
 */
#include "contiki.h"
#include "dev/crypto.h"
#include "dev/ccm.h"
#include "dev/nvic.h"
#include "reg.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_encrypt_start(uint8_t len_len, uint8_t key_area, const void *nonce,
                       const void *adata, uint16_t adata_len, void *pdata,
                       uint16_t pdata_len, uint8_t mic_len,
                       struct process *process)
{
  uint32_t iv[4];

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return AES_RESOURCE_IN_USE;
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

  /* Prepare the encryption initialization vector */
  ((uint8_t *)iv)[0] = len_len - 1;                   /* Flags: L' = L - 1 */
  memcpy(&((uint8_t *)iv)[1], nonce, 15 - len_len);   /* Nonce */
  memset(&((uint8_t *)iv)[16 - len_len], 0, len_len); /* Initialize counter to 0 */

  /* Write initialization vector */
  REG(AES_AES_IV_0) = iv[0];
  REG(AES_AES_IV_1) = iv[1];
  REG(AES_AES_IV_2) = iv[2];
  REG(AES_AES_IV_3) = iv[3];

  /* Program AES-CCM encryption */
  REG(AES_AES_CTRL) = AES_AES_CTRL_SAVE_CONTEXT |            /* Save context */
    (((MAX(mic_len, 2) - 2) >> 1) << AES_AES_CTRL_CCM_M_S) | /* M */
    ((len_len - 1) << AES_AES_CTRL_CCM_L_S) |                /* L */
    AES_AES_CTRL_CCM |                                       /* CCM */
    AES_AES_CTRL_CTR_WIDTH_128 |                             /* CTR width 128 */
    AES_AES_CTRL_CTR |                                       /* CTR */
    AES_AES_CTRL_DIRECTION_ENCRYPT;                          /* Encryption */

  /* Write the length of the crypto block (lo) */
  REG(AES_AES_C_LENGTH_0) = pdata_len;
  /* Write the length of the crypto block (hi) */
  REG(AES_AES_C_LENGTH_1) = 0;

  /* Write the length of the AAD data block (may be non-block size-aligned) */
  REG(AES_AES_AUTH_LENGTH) = adata_len;

  if(adata_len != 0) {
    /* Configure DMAC to fetch the AAD data
     * Enable DMA channel 0 */
    REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the AAD input data in ext. memory */
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
      return AES_DMA_BUS_ERROR;
    }
  }

  /* Clear interrupt status */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  if(process != NULL) {
    crypto_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_AES);
    nvic_interrupt_enable(NVIC_INT_AES);
  }

  /* Enable result available bit in interrupt enable */
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_RESULT_AV;

  if(pdata_len != 0) {
    /* Configure DMAC
     * Enable DMA channel 0 */
    REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the payload data in ext. memory */
    REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)pdata;
    /* Payload data length in bytes */
    REG(AES_DMAC_CH0_DMALENGTH) = pdata_len;

    /* Enable DMA channel 1 */
    REG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the output data buffer */
    REG(AES_DMAC_CH1_EXTADDR) = (uint32_t)pdata;
    /* Output data length in bytes */
    REG(AES_DMAC_CH1_DMALENGTH) = pdata_len;
  }

  return AES_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_encrypt_check_status(void)
{
  return !!(REG(AES_CTRL_INT_STAT) &
            (AES_CTRL_INT_STAT_DMA_BUS_ERR | AES_CTRL_INT_STAT_KEY_ST_WR_ERR |
             AES_CTRL_INT_STAT_KEY_ST_RD_ERR | AES_CTRL_INT_STAT_RESULT_AV));
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_encrypt_get_result(void *mic, uint8_t mic_len)
{
  uint32_t aes_ctrl_int_stat;
  uint32_t tag[4];

  aes_ctrl_int_stat = REG(AES_CTRL_INT_STAT);
  /* Clear the error bits */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR |
                          AES_CTRL_INT_CLR_KEY_ST_WR_ERR |
                          AES_CTRL_INT_CLR_KEY_ST_RD_ERR;

  nvic_interrupt_disable(NVIC_INT_AES);
  crypto_register_process_notification(NULL);

  /* Disable the master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;

  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    return AES_DMA_BUS_ERROR;
  }
  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_KEY_ST_WR_ERR) {
    return AES_KEYSTORE_WRITE_ERROR;
  }
  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_KEY_ST_RD_ERR) {
    return AES_KEYSTORE_READ_ERROR;
  }

  /* Read tag
   * Wait for the context ready bit */
  while(!(REG(AES_AES_CTRL) & AES_AES_CTRL_SAVED_CONTEXT_READY));

  /* Read the tag registers */
  tag[0] = REG(AES_AES_TAG_OUT_0);
  tag[1] = REG(AES_AES_TAG_OUT_1);
  tag[2] = REG(AES_AES_TAG_OUT_2);
  tag[3] = REG(AES_AES_TAG_OUT_3);

  /* Clear the interrupt status */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  /* Copy tag to MIC */
  memcpy(mic, tag, mic_len);

  return AES_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_decrypt_start(uint8_t len_len, uint8_t key_area, const void *nonce,
                       const void *adata, uint16_t adata_len, void *cdata,
                       uint16_t cdata_len, uint8_t mic_len,
                       struct process *process)
{
  uint16_t pdata_len = cdata_len - mic_len;
  uint32_t iv[4];

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return AES_RESOURCE_IN_USE;
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

  /* Prepare the decryption initialization vector */
  ((uint8_t *)iv)[0] = len_len - 1;                   /* Flags: L' = L - 1 */
  memcpy(&((uint8_t *)iv)[1], nonce, 15 - len_len);   /* Nonce */
  memset(&((uint8_t *)iv)[16 - len_len], 0, len_len); /* Initialize counter to 0 */

  /* Write initialization vector */
  REG(AES_AES_IV_0) = iv[0];
  REG(AES_AES_IV_1) = iv[1];
  REG(AES_AES_IV_2) = iv[2];
  REG(AES_AES_IV_3) = iv[3];

  /* Program AES-CCM decryption */
  REG(AES_AES_CTRL) = AES_AES_CTRL_SAVE_CONTEXT |            /* Save context */
    (((MAX(mic_len, 2) - 2) >> 1) << AES_AES_CTRL_CCM_M_S) | /* M */
    ((len_len - 1) << AES_AES_CTRL_CCM_L_S) |                /* L */
    AES_AES_CTRL_CCM |                                       /* CCM */
    AES_AES_CTRL_CTR_WIDTH_128 |                             /* CTR width 128 */
    AES_AES_CTRL_CTR;                                        /* CTR */

  /* Write the length of the crypto block (lo) */
  REG(AES_AES_C_LENGTH_0) = pdata_len;
  /* Write the length of the crypto block (hi) */
  REG(AES_AES_C_LENGTH_1) = 0;

  /* Write the length of the AAD data block (may be non-block size-aligned) */
  REG(AES_AES_AUTH_LENGTH) = adata_len;

  if(adata_len != 0) {
    /* Configure DMAC to fetch the AAD data
     * Enable DMA channel 0 */
    REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the AAD input data in ext. memory */
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
      return AES_DMA_BUS_ERROR;
    }
  }

  /* Clear interrupt status */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  if(process != NULL) {
    crypto_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_AES);
    nvic_interrupt_enable(NVIC_INT_AES);
  }

  /* Enable result available bit in interrupt enable */
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_RESULT_AV;

  if(pdata_len != 0) {
    /* Configure DMAC
     * Enable DMA channel 0 */
    REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the payload data in ext. memory */
    REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)cdata;
    /* Payload data length in bytes */
    REG(AES_DMAC_CH0_DMALENGTH) = pdata_len;

    /* Enable DMA channel 1 */
    REG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH_CTRL_EN;
    /* Base address of the output data buffer */
    REG(AES_DMAC_CH1_EXTADDR) = (uint32_t)cdata;
    /* Output data length in bytes */
    REG(AES_DMAC_CH1_DMALENGTH) = pdata_len;
  }

  return AES_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_decrypt_check_status(void)
{
  /* Check if result is available or some error has occured */
  return ccm_auth_encrypt_check_status();
}
/*---------------------------------------------------------------------------*/
uint8_t
ccm_auth_decrypt_get_result(const void *cdata, uint16_t cdata_len,
                            void *mic, uint8_t mic_len)
{
  uint32_t aes_ctrl_int_stat;
  uint16_t pdata_len = cdata_len - mic_len;
  uint32_t tag[4];

  aes_ctrl_int_stat = REG(AES_CTRL_INT_STAT);
  /* Clear the error bits */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR |
                          AES_CTRL_INT_CLR_KEY_ST_WR_ERR |
                          AES_CTRL_INT_CLR_KEY_ST_RD_ERR;

  nvic_interrupt_disable(NVIC_INT_AES);
  crypto_register_process_notification(NULL);

  /* Disable the master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;

  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    return AES_DMA_BUS_ERROR;
  }
  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_KEY_ST_WR_ERR) {
    return AES_KEYSTORE_WRITE_ERROR;
  }
  if(aes_ctrl_int_stat & AES_CTRL_INT_STAT_KEY_ST_RD_ERR) {
    return AES_KEYSTORE_READ_ERROR;
  }

  /* Read tag
   * Wait for the context ready bit */
  while(!(REG(AES_AES_CTRL) & AES_AES_CTRL_SAVED_CONTEXT_READY));

  /* Read the tag registers */
  tag[0] = REG(AES_AES_TAG_OUT_0);
  tag[1] = REG(AES_AES_TAG_OUT_1);
  tag[2] = REG(AES_AES_TAG_OUT_2);
  tag[3] = REG(AES_AES_TAG_OUT_3);

  /* Clear the interrupt status */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  /* Check MIC */
  if(memcmp(tag, &((const uint8_t *)cdata)[pdata_len], mic_len)) {
    return CCM_AUTHENTICATION_FAILED;
  }

  /* Copy tag to MIC */
  memcpy(mic, tag, mic_len);

  return AES_SUCCESS;
}

/** @} */
