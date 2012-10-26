/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         Interface to the CC2420 AES encryption/decryption functions
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __CC2420_AES_H__
#define __CC2420_AES_H__

/**
 * \brief      Setup an AES key
 * \param key  A pointer to a 16-byte AES key
 * \param index The key index: either 0 or 1.
 *
 *             This function sets up an AES key with the CC2420
 *             chip. The AES key can later be used with the
 *             cc2420_aes_cipher() function to encrypt or decrypt
 *             data.
 *
 *             The CC2420 can store two separate keys in its
 *             memory. The keys are indexed as 0 or 1 and the key
 *             index is given by the 'index' parameter.
 *
 */
void cc2420_aes_set_key(const uint8_t *key, int index);


/**
 * \brief      Encrypt/decrypt data with AES
 * \param data A pointer to the data to be encrypted/decrypted
 * \param len  The length of the data to be encrypted/decrypted
 * \param key_index The key to use. The key must have previously been set up with cc2420_aes_set_key().
 *
 *             This function encrypts/decrypts data with AES. A
 *             pointer to the data is passed as a parameter, and the
 *             function overwrites the data with the encrypted data.
 *
 */
void cc2420_aes_cipher(uint8_t *data, int len, int key_index);


#endif /* __CC2420_AES_H__ */
