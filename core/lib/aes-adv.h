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
 *         AES-ADV header file.
 * \author
 *         Lars Schmertmann <SmallLars@t-online.de>
 */

#ifndef AES_ADV_H_
#define AES_ADV_H_

#include <stddef.h>
#include "contiki.h"

#ifdef AES_ADV_CONF
#define AES_ADV AES_ADV_CONF
#else /* AES_ADV_CONF */
#define AES_ADV aes_adv_driver
#endif /* AES_ADV_CONF */

#define AES_ADV_BLOCK_SIZE 16
#define AES_ADV_KEY_LENGTH 16

/* Return values for init and exec. */
typedef enum {
  AES_ADV_OK,
  AES_ADV_NOT_SUPPORTED,
  AES_ADV_ERROR
} aes_adv_result_t;

/* Possible directions for AES execution. */
typedef enum {
  AES_ADV_ENCRYPT,
  AES_ADV_DECRYPT
} aes_adv_direction_t;

/* Possible AES modes. */
typedef enum {
  AES_ADV_CTR,
  AES_ADV_CBC,
  AES_ADV_CCM,
} aes_adv_mode_t;

/**
 * Structure of AES drivers.
 */
struct aes_adv_driver {
  void *KEY;
  void *IV;
  void *DATA;
  void *CTR;
  void *CTR_RESULT;
  void *CBC_RESULT;

  /**
   * \brief  Initialisation of AES-Module
   *
   *         Initialisation of the AES-Module.
   *         Must be called once before the module is used.
   *
   * \return  AES_ADV_OK     if initialisation was successful
   *          AES_ADV_ERROR  if initialisation failed
   */
  aes_adv_result_t (*init)(void);

  /**
   * \brief  Copy data from AES-Module
   *
   *         Copies up to 16 byte from AES-Module into memory.
   *         Possible values in AES-Module are:
   *         AES_ADV.KEY, AES_ADV.IV, AES_ADV.DATA, AES_ADV.CTR,
   *         AES_ADV.CTR_RESULT and AES_ADV.CBC_RESULT.
   *
   * \param  dst  Destination in memory for AES data
   * \param  src  AES value to read from
   * \param  len  Bytes to copy (max 16)
   */
  void (*get_data)(uint8_t *dst, const void *src, size_t len);

  /**
   * \brief  Copy data to AES-Module
   *
   *         Copies up to 16 byte from memory into AES-Module.
   *         Possible values in AES-Module are:
   *         AES_ADV.KEY, AES_ADV.IV, AES_ADV.DATA, AES_ADV.CTR,
   *         AES_ADV.CTR_RESULT and AES_ADV.CBC_RESULT.
   *         Unsing len == 0 will set 16 zero bytes.
   *
   * \param  dst  AES value to write
   * \param  src  Source in memory for AES data
   * \param  len  Bytes to copy (max 16) - If len < 16 dst is zero padded
   */
  void (*set_data)(void *dst, const uint8_t *src, size_t len);

  /**
   * \brief  Execute AES calculation
   *
   *         After data copy into AES registers this function will start
   *         AES calculation and waiting for finish so the result is
   *         available in the result registers after function call.
   *         Depending on the mode CTR, CBC or both modes (CCM) are used.
   *         On CTR mode counter is increased by one at the end.
   *
   * \param  dir   Possible directions are ENCRYPT or DECRYPT.
   * \param  mode  Possible modes are CTR, CBC or CCM.
   *
   * \return  AES_ADV_OK             if execution was successful
              AES_ADV_NOT_SUPPORTED  if combination of dir and mode isn't supported
   *          AES_ADV_ERROR          if execution failed
   */
  aes_adv_result_t (*exec)(aes_adv_direction_t dir, aes_adv_mode_t mode);

  /**
   * \brief  Clear memory of AES-Module
   *
   *         Fills the values of AES-Module with zeros.
   */
  void (*clear)(void);
};

extern const struct aes_adv_driver AES_ADV;

#endif /* AES_ADV_H_ */
