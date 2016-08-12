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
 * \addtogroup cc2538-sha256
 * @{
 *
 * \file
 * Implementation of the cc2538 SHA-256 driver
 */
#include "contiki.h"
#include "sys/cc.h"
#include "dev/rom-util.h"
#include "dev/aes.h"
#include "dev/sha256.h"
#include "reg.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define BLOCK_SIZE      64
#define OUTPUT_LEN      32
/*---------------------------------------------------------------------------*/
/** \brief Starts a new hash session in hardware
 * \param state Hash state
 * \param data Pointer to input message
 * \param hash Destination of the hash (32 bytes)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/SHA256 error code
 */
static uint8_t
new_hash(sha256_state_t *state, const void *data, void *hash)
{
  /* Workaround for AES registers not retained after PM2 */
  REG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_DMA_IN_DONE |
                         AES_CTRL_INT_EN_RESULT_AV;

  /* Configure master control module and enable DMA path to the SHA-256 engine
   * + Digest readout */
  REG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_TAG | AES_CTRL_ALG_SEL_HASH;

  /* Clear any outstanding events */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_RESULT_AV;

  /* Configure hash engine
   * Indicate start of a new hash session and SHA-256 */
  REG(AES_HASH_MODE_IN) = AES_HASH_MODE_IN_SHA256_MODE |
                          AES_HASH_MODE_IN_NEW_HASH;

  /* If the final digest is required (pad the input DMA data), write the
   * following register */
  if(state->final_digest) {
    /* Write length of the message (lo) */
    REG(AES_HASH_LENGTH_IN_L) = (uint32_t)state->length;
    /* Write length of the message (hi) */
    REG(AES_HASH_LENGTH_IN_H) = (uint32_t)(state->length >> 32);
    /* Pad the DMA-ed data */
    REG(AES_HASH_IO_BUF_CTRL) = AES_HASH_IO_BUF_CTRL_PAD_DMA_MESSAGE;
  }

  /* Enable DMA channel 0 for message data */
  REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
  /* Base address of the data in ext. memory */
  REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)data;
  if(state->final_digest) {
    /* Input data length in bytes, equal to the message */
    REG(AES_DMAC_CH0_DMALENGTH) = state->curlen;
  } else {
    REG(AES_DMAC_CH0_DMALENGTH) = BLOCK_SIZE;
  }

  /* Enable DMA channel 1 for result digest */
  REG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH_CTRL_EN;
  /* Base address of the digest buffer */
  REG(AES_DMAC_CH1_EXTADDR) = (uint32_t)hash;
  /* Length of the result digest */
  REG(AES_DMAC_CH1_DMALENGTH) = OUTPUT_LEN;

  /* Wait for completion of the operation */
  while(!(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV));

  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    /* Clear the DMA error */
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR;
    /* Disable master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return CRYPTO_DMA_BUS_ERROR;
  }

  /* Clear the interrupt */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;
  /* Disable master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;
  /* Clear mode */
  REG(AES_AES_CTRL) = 0x00000000;

  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/** \brief Resumes an already started hash session in hardware
 * \param state Hash state
 * \param data Pointer to the input message
 * \param hash Pointer to the destination of the hash (32 bytes)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/SHA256 error code
 */
static uint8_t
resume_hash(sha256_state_t *state, const void *data, void *hash)
{
  /* Workaround for AES registers not retained after PM2 */
  REG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_DMA_IN_DONE |
                         AES_CTRL_INT_EN_RESULT_AV;

  /* Configure master control module and enable the DMA path to the SHA-256
   * engine */
  REG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_HASH;

  /* Clear any outstanding events */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_RESULT_AV;

  /* Configure hash engine
   * Indicate the start of a resumed hash session and SHA-256 */
  REG(AES_HASH_MODE_IN) = AES_HASH_MODE_IN_SHA256_MODE;

  /* If the final digest is required (pad the input DMA data) */
  if(state->final_digest) {
    /* Write length of the message (lo) */
    REG(AES_HASH_LENGTH_IN_L) = (uint32_t)state->length;
    /* Write length of the message (hi) */
    REG(AES_HASH_LENGTH_IN_H) = (uint32_t)(state->length >> 32);
  }

  /* Write the initial digest */
  REG(AES_HASH_DIGEST_A) = (uint32_t)state->state[0];
  REG(AES_HASH_DIGEST_B) = (uint32_t)state->state[1];
  REG(AES_HASH_DIGEST_C) = (uint32_t)state->state[2];
  REG(AES_HASH_DIGEST_D) = (uint32_t)state->state[3];
  REG(AES_HASH_DIGEST_E) = (uint32_t)state->state[4];
  REG(AES_HASH_DIGEST_F) = (uint32_t)state->state[5];
  REG(AES_HASH_DIGEST_G) = (uint32_t)state->state[6];
  REG(AES_HASH_DIGEST_H) = (uint32_t)state->state[7];

  /* If final digest, pad the DMA-ed data */
  if(state->final_digest) {
    REG(AES_HASH_IO_BUF_CTRL) = AES_HASH_IO_BUF_CTRL_PAD_DMA_MESSAGE;
  }

  /* Enable DMA channel 0 for message data */
  REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;
  /* Base address of the data in ext. memory */
  REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)data;
  /* Input data length in bytes, equal to the message */
  if(state->final_digest) {
    REG(AES_DMAC_CH0_DMALENGTH) = state->curlen;
  } else {
    REG(AES_DMAC_CH0_DMALENGTH) = BLOCK_SIZE;
  }

  /* Wait for completion of the operation */
  while(!(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV));

  /* Check for any DMA Bus errors */
  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    /* Clear the DMA error */
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR;
    /* Disable master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return CRYPTO_DMA_BUS_ERROR;
  }

  /* Read digest */
  ((uint32_t *)hash)[0] = REG(AES_HASH_DIGEST_A);
  ((uint32_t *)hash)[1] = REG(AES_HASH_DIGEST_B);
  ((uint32_t *)hash)[2] = REG(AES_HASH_DIGEST_C);
  ((uint32_t *)hash)[3] = REG(AES_HASH_DIGEST_D);
  ((uint32_t *)hash)[4] = REG(AES_HASH_DIGEST_E);
  ((uint32_t *)hash)[5] = REG(AES_HASH_DIGEST_F);
  ((uint32_t *)hash)[6] = REG(AES_HASH_DIGEST_G);
  ((uint32_t *)hash)[7] = REG(AES_HASH_DIGEST_H);

  /* Acknowledge reading of the digest */
  REG(AES_HASH_IO_BUF_CTRL) = AES_HASH_IO_BUF_CTRL_OUTPUT_FULL;

  /* Clear the interrupt */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;
  /* Disable master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;
  /* Clear mode */
  REG(AES_AES_CTRL) = 0x00000000;

  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
sha256_init(sha256_state_t *state)
{
  if(state == NULL) {
    return CRYPTO_NULL_ERROR;
  }

  state->curlen = 0;
  state->length = 0;
  state->new_digest = true;
  state->final_digest = false;
  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
sha256_process(sha256_state_t *state, const void *data, uint32_t len)
{
  uint32_t n;
  uint8_t ret;

  if(state == NULL || data == NULL) {
    return CRYPTO_NULL_ERROR;
  }

  if(state->curlen > sizeof(state->buf)) {
    return CRYPTO_INVALID_PARAM;
  }

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return CRYPTO_RESOURCE_IN_USE;
  }

  if(len > 0 && state->new_digest) {
    if(state->curlen == 0 && len > BLOCK_SIZE) {
      rom_util_memcpy(state->buf, data, BLOCK_SIZE);
      ret = new_hash(state, state->buf, state->state);
      if(ret != CRYPTO_SUCCESS) {
        return ret;
      }
      state->new_digest = false;
      state->length += BLOCK_SIZE << 3;
      data += BLOCK_SIZE;
      len -= BLOCK_SIZE;
    } else {
      n = MIN(len, BLOCK_SIZE - state->curlen);
      rom_util_memcpy(&state->buf[state->curlen], data, n);
      state->curlen += n;
      data += n;
      len -= n;
      if(state->curlen == BLOCK_SIZE && len > 0) {
        ret = new_hash(state, state->buf, state->state);
        if(ret != CRYPTO_SUCCESS) {
          return ret;
        }
        state->new_digest = false;
        state->length += BLOCK_SIZE << 3;
        state->curlen = 0;
      }
    }
  }

  while(len > 0 && !state->new_digest) {
    if(state->curlen == 0 && len > BLOCK_SIZE) {
      rom_util_memcpy(state->buf, data, BLOCK_SIZE);
      ret = resume_hash(state, state->buf, state->state);
      if(ret != CRYPTO_SUCCESS) {
        return ret;
      }
      state->length += BLOCK_SIZE << 3;
      data += BLOCK_SIZE;
      len -= BLOCK_SIZE;
    } else {
      n = MIN(len, BLOCK_SIZE - state->curlen);
      rom_util_memcpy(&state->buf[state->curlen], data, n);
      state->curlen += n;
      data += n;
      len -= n;
      if(state->curlen == BLOCK_SIZE && len > 0) {
        ret = resume_hash(state, state->buf, state->state);
        if(ret != CRYPTO_SUCCESS) {
          return ret;
        }
        state->length += BLOCK_SIZE << 3;
        state->curlen = 0;
      }
    }
  }

  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
sha256_done(sha256_state_t *state, void *hash)
{
  uint8_t ret;

  if(state == NULL || hash == NULL) {
    return CRYPTO_NULL_ERROR;
  }

  if(state->curlen > sizeof(state->buf)) {
    return CRYPTO_INVALID_PARAM;
  }

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return CRYPTO_RESOURCE_IN_USE;
  }

  /* Increase the length of the message */
  state->length += state->curlen << 3;
  state->final_digest = true;
  if(state->new_digest) {
    ret = new_hash(state, state->buf, hash);
    if(ret != CRYPTO_SUCCESS) {
      return ret;
    }
  } else {
    ret = resume_hash(state, state->buf, hash);
    if(ret != CRYPTO_SUCCESS) {
      return ret;
    }
  }
  state->new_digest = false;
  state->final_digest = false;

  return CRYPTO_SUCCESS;
}

/** @} */
