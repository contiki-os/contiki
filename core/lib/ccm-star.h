/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
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
 *         CCM* header file.
 * \author
 *         Original: Konrad Krentz <konrad.krentz@gmail.com>
 *         Generified version: Justin King-Lacroix <justin.kinglacroix@gmail.com>
 */
#ifndef CCM_STAR_H_
#define CCM_STAR_H_

#include "contiki.h"

#ifdef CCM_STAR_CONF
#define CCM_STAR CCM_STAR_CONF
#else /* CCM_STAR_CONF */
#define CCM_STAR ccm_star_driver
#endif /* CCM_STAR_CONF */

#define CCM_STAR_NONCE_LENGTH 13

/**
 * Structure of CCM* drivers.
 */
struct ccm_star_driver {
  
  /**
   * \brief         Sets the key in use. Default implementation calls AES_128.set_key().
   * \param key     The key to use.
   */
  void (* set_key)(const uint8_t* key);
  
  /**
   * \brief         Combines authentication and encryption.
   * \param nonce   The nonce to use. CCM_STAR_NONCE_LENGTH bytes long.
   * \param m       message to encrypt or decrypt
   * \param a       Additional authenticated data
   * \param result  The generated MIC will be put here
   * \param mic_len The size of the MIC to be generated. <= 16.
   * \param forward != 0 if used in forward direction.
   */
  void (* aead)(const uint8_t* nonce,
      uint8_t* m, uint8_t m_len,
      const uint8_t* a, uint8_t a_len,
      uint8_t *result, uint8_t mic_len,
      int forward);
};

extern const struct ccm_star_driver CCM_STAR;

#endif /* CCM_STAR_H_ */
