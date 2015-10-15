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
 *         CCM-128 header file.
 *
 *         Example usage encryption:
 *             uint8_t data[20];
 *             memcpy(data, "Hello World!", 12);
               CCM_128.set_key("MY16BYTELONGKEY.");
               CCM_128.configure(10, 8);
 *             CCM_128.crypt(0, "NONCE_ABCD", data, 12, NULL, 0);
 *             CCM_128.get_mac(data + 12);
 *             // result is cryptoptext with MAC
 *
 *         Example usage decryption:
 *             uint8_t data[20];
 *             memcpy(data, "Cryptoptext with MAC", 20);
               CCM_128.set_key("MY16BYTELONGKEY.");
               CCM_128.configure(10, 8);
 *             // decrypt with useless mac calculation
 *             CCM_128.crypt(0, "NONCE_ABCD", data, 12, NULL, 0);
 *             // calculate mac only on plaintext
 *             CCM_128.crypt(1, "NONCE_ABCD", data, 12, NULL, 0);
 *             // get new MAC
 *             uint8_t mac[8];
 *             CCM_128.get_mac(mac);
 *             // compare old and new MAC
 *             memcmp(mac, data + 12, 8);
 *
 * \author
 *         Lars Schmertmann <SmallLars@t-online.de>
 */
#ifndef CCM_128_H_
#define CCM_128_H_

#include <stddef.h>
#include "contiki.h"

#ifdef CCM_128_CONF
#define CCM_128 CCM_128_CONF
#else /* CCM_128_CONF */
#define CCM_128 ccm_128_driver
#endif /* CCM_128_CONF */

/* Return values for configure. */
typedef enum {
  CCM_128_OK,
  CCM_128_NOT_SUPPORTED,
  CCM_128_ERROR
} ccm_128_return_t;

/**
 * Structure of CCM-128 drivers.
 */
struct ccm_128_driver {
  /**
   * \brief  Sets the key in use.
   *
   *         Default implementation calls AES_128.set_key()
   *
   * \param  key      The key to use.
   * \param  key_len  The length of the key.
   */
  void (*set_key)(const uint8_t *key, size_t key_len);

  /**
   * \brief  CCM-Configuration
   *
   *         RFC 3610 allows you to choose a length for nonce and MAC.
   *         The size of the field for message length is 15 - nonce_length byte.
   *
   * \param  nonce_len  Standard conform values are 7, 8, 9, 10, 11, 12 and 13
   * \param  mac_len    Standard conform values are 4, 6, 8, 10, 12, 14, and 16
   *
   * \return  CCM_128_OK             if initialisation was successful
   *          CCM_128_NOT_SUPPORTED  if parameter are not valid
   *          CCM_128_ERROR          if initialisation failed
   */
  ccm_128_return_t (*configure)(size_t nonce_length, size_t mac_length);

  /**
   * \brief  CCM encryption and decryption
   *
   *         Inplace encryption and decryption including MAC calculation.
   *         Its possible to calculate MAC only to check the MAC of received
   *         encrypted data.
   *
   * \param  mac_only   1 if mac calculation only, else 0
   * \param  nonce      Pointer to the nonce
   * \param  data       Pointer to the plain or ciphertext
   * \param  data_len   Length of the plain or ciphertext
   * \param  adata      Pointer to optional addidtional data used for mac calculation
   * \param  adata_len  Length of the additional data
   */
  void (*aead)(uint32_t mac_only, const uint8_t *nonce, uint8_t *data, size_t data_len, const uint8_t *adata, size_t adata_len);

  /**
   * \brief  Copy MAC from CCM-Module
   *
   *         Copies the MAC from CCM-Module into memory.
   *         The length given in configuration is used.
   *
   * \param  dst  Destination in memory for MAC
   */
  void (*get_mac)(uint8_t *dst);
};

extern const struct ccm_128_driver CCM_128;

#endif /* CCM_128_H_ */
