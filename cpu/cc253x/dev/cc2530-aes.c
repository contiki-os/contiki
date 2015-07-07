/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 */

/**
 * \file
 *     Implementation of the cc2530 AES driver
 *
 * \author
 *     Justin King-Lacroix - <justin.king-lacroix@cs.ox.ac.uk>
 */
#include "cc2530-aes.h"

#include "cc253x.h"

#include <assert.h>

/* AES coprocessor registers:
 *
 * ENCCS: control
 * ENCDI: data in
 * ENCDO: data out
 *
 * this implementation doesn't yet support
 */

/* see RFC 3610 */
#define CCM_STAR_AUTH_FLAGS(Adata, M) (uint8_t)((Adata ? (uint8_t)(1u << 6) : (uint8_t)0) | (((uint8_t)(M - 2u) >> 1) << 3) | (uint8_t)1u)
#define CCM_STAR_ENCRYPTION_FLAGS     1

#define CC2530_AES_SUBWORD_SIZE 4

/*---------------------------------------------------------------------------*/
static void
wait_for_ready(void) {
    while(!CC2530_AES_ENCCS_RDY(ENCCS))
        ;
}
/*---------------------------------------------------------------------------*/
static void
send_block(const uint8_t *data) {
    uint8_t i;

    for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
        ENCDI = data[i];
    }

}
/*---------------------------------------------------------------------------*/
static void
receive_block(uint8_t *data) {
    uint8_t i;

    for(i = 0; i < AES_128_BLOCK_SIZE; i++) {
        data[i] = ENCDO;
    }
}
/*---------------------------------------------------------------------------*/
static void
send_word(const uint8_t *data) {
    uint8_t i;

    for(i = 0; i < CC2530_AES_SUBWORD_SIZE; i++) {
        ENCDI = data[i];
    }

}
/*---------------------------------------------------------------------------*/
static void
receive_word(uint8_t *data) {
    uint8_t i;

    for(i = 0; i < CC2530_AES_SUBWORD_SIZE; i++) {
        data[i] = ENCDO;
    }
}
/*---------------------------------------------------------------------------*/
static void
set_key(const uint8_t *key)
{
    //tell the AES coprocessor we're giving it a key
    ENCCS = CC2530_AES_ENCCS(0, CC2530_AES_CMD_LOADKEY, 1);

    send_block(key);

    //wait for it to finish receiving it
    wait_for_ready();
}
/*---------------------------------------------------------------------------*/
static void
aes_encrypt(uint8_t *plaintext_and_result) {
    //tell it we're doing an ECB encryption (equivalent to raw AES encryption)
    ENCCS = CC2530_AES_ENCCS(CC2530_AES_MODE_ECB, CC2530_AES_CMD_ENCRYPT, 1);

    //send it the plaintext
    send_block(plaintext_and_result);

    //read out the ciphertext
    receive_block(plaintext_and_result);

    wait_for_ready();
}
/*---------------------------------------------------------------------------*/
static void
set_nonce(uint8_t *iv,
      uint8_t flags,
      const uint8_t *nonce,
      uint8_t counter)
{
    /* 1 byte||      8 bytes    ||    4 bytes    || 1 byte  || 2 bytes */
    /* flags || extended_source_address || frame_counter || sec_lvl || counter */

    iv[0] = flags;
    memcpy(iv + 1, nonce, CCM_STAR_NONCE_LENGTH);
    iv[14] = 0;
    iv[15] = counter;
}
/*---------------------------------------------------------------------------*/
static void
encrypt_block_chunked(uint8_t* block, uint8_t mode) {
    uint8_t j;

    ENCCS = CC2530_AES_ENCCS(mode, CC2530_AES_CMD_LOADIV, 1);
    for(j = 0; j < (AES_128_BLOCK_SIZE / CC2530_AES_SUBWORD_SIZE); j++) {
        send_word(block);
        receive_word(block);
        block += CC2530_AES_SUBWORD_SIZE;
    }
    wait_for_ready();
}
/*---------------------------------------------------------------------------*/
static void
mic(const uint8_t *m,  uint8_t m_len,
    const uint8_t *nonce,
    const uint8_t *a,  uint8_t a_len,
    uint8_t *result,
    uint8_t mic_len) {
    uint8_t tmp[AES_128_BLOCK_SIZE];
    uint8_t i;
    uint16_t bytes_sent = 0;
    uint16_t t_len = AES_128_BLOCK_SIZE + ((m_len + AES_128_BLOCK_SIZE - 1) & ~(AES_128_BLOCK_SIZE - 1)) + (a_len ? ((a_len + 2 + AES_128_BLOCK_SIZE - 1) & ~(AES_128_BLOCK_SIZE - 1)) : 0);

    //the IV for the authentication phase is all-zeros
    memset(tmp, 0, AES_128_BLOCK_SIZE);

    //send the IV to the chip, and set its mode to CBC-MAC
    ENCCS = CC2530_AES_ENCCS(CC2530_AES_MODE_CBC_MAC, CC2530_AES_CMD_LOADIV, 1);
    send_block(tmp);

    /* GOTCHA:
     * The AES coprocessor requires the message to be CBC-MACed to be input in CBC_MAC mode...
     * *except* the last block, which must be input in CBC mode.
     *
     * We will have to do this last block detection continuously throughout this function, and it starts right here.
     */
#define BLOCK_PREAMBLE wait_for_ready(); if(bytes_sent + AES_128_BLOCK_SIZE == t_len) ENCCS = CC2530_AES_ENCCS(CC2530_AES_MODE_CBC, CC2530_AES_CMD_ENCRYPT, 1); else ENCCS = CC2530_AES_ENCCS(CC2530_AES_MODE_CBC_MAC, CC2530_AES_CMD_ENCRYPT, 1)

    BLOCK_PREAMBLE;

    //generate the first authenticated block (aka B0), and send it to the chip
    set_nonce(tmp, CCM_STAR_AUTH_FLAGS(a_len, mic_len), nonce, m_len);
    send_block(tmp);
    bytes_sent += AES_128_BLOCK_SIZE;

    if (a_len > 0) {
        //if there's associated data, we prepend its (2-byte) length to the stream
        BLOCK_PREAMBLE;

        ENCDI = 0;
        ENCDI = a_len;
        bytes_sent += 2;

        //... then send over the rest of the first block of associated data...
        for (i = 0; i < a_len && i < AES_128_BLOCK_SIZE - 2; i++) {
            ENCDI = a[i];
            bytes_sent++;
        }

        //... then send over any intermediate blocks...
        for (; a_len - i > AES_128_BLOCK_SIZE; i += AES_128_BLOCK_SIZE) {
            BLOCK_PREAMBLE;
            send_block(a + i);
            bytes_sent += AES_128_BLOCK_SIZE;
        }

        //... then send over the final block...
        if (i < a_len) {
            BLOCK_PREAMBLE;
            for (; i < a_len; i++) {
                ENCDI = a[i];
                bytes_sent++;
            }
        }

        if (bytes_sent & (AES_128_BLOCK_SIZE - 1)) {
            //... and zero-pad to an integer multiple of the block size
            for (i = bytes_sent & (AES_128_BLOCK_SIZE - 1); i < AES_128_BLOCK_SIZE; i++) {
                ENCDI = 0;
                bytes_sent++;
            }
        }
    }

    assert(bytes_sent + ((m_len + AES_128_BLOCK_SIZE - 1) & ~(AES_128_BLOCK_SIZE - 1)) == t_len);

    //now, we send over the data blocks
    for (i = 0; (m_len - i) >= AES_128_BLOCK_SIZE; i += AES_128_BLOCK_SIZE) {
        BLOCK_PREAMBLE;
        send_block(m + i);
        bytes_sent += AES_128_BLOCK_SIZE;
    }

    if (i < m_len) {
        //if there are any data bytes that aren't block-aligned, send them...
        BLOCK_PREAMBLE;
        for (; i < m_len; i++) {
            ENCDI = m[i];
            bytes_sent++;
        }

        //... and zero-pad to an integer multiple of the block size
        for (i &= AES_128_BLOCK_SIZE - 1; i < AES_128_BLOCK_SIZE; i++) {
            ENCDI = 0;
            bytes_sent++;
        }
    }

    assert(bytes_sent == t_len);

    //phew. done. now we read out the CBC-MAC result
    for (i = 0; i < mic_len; i++)
        result[i] = ENCDO;
    //zero pad
    for (; i < AES_128_BLOCK_SIZE; i++)
        ENCDO;

    wait_for_ready();

    //finally, we CTR-encrypt the CBC-MAC result, with counter 0:

    //generate the CTR IV
    set_nonce(tmp, CCM_STAR_ENCRYPTION_FLAGS, nonce, 0);

    //load it into the chip
    ENCCS = CC2530_AES_ENCCS(CC2530_AES_MODE_CTR, CC2530_AES_CMD_LOADIV, 1);
    send_block(tmp);
    wait_for_ready();

    //send the CBC-MAC result, padded with zeros
    memcpy(tmp, result, mic_len);
    memset(tmp, 0, AES_128_BLOCK_SIZE - mic_len);

    //CTR-encrypt
    encrypt_block_chunked(tmp, CC2530_AES_MODE_CTR);

    //output the final MIC
    memcpy(result, tmp, mic_len);

#undef BLOCK_PREAMBLE
}
/*---------------------------------------------------------------------------*/
static void
ctr(uint8_t *m, uint8_t m_len, const uint8_t* nonce)
{
    uint8_t tmp[AES_128_BLOCK_SIZE];
    uint8_t i;

    //generate the encryption-phase IV (aka A0)
    set_nonce(tmp, CCM_STAR_ENCRYPTION_FLAGS, nonce, 1);

    //send the IV to the chip, and set its mode to CTR
    ENCCS = CC2530_AES_ENCCS(CC2530_AES_MODE_CTR, CC2530_AES_CMD_LOADIV, 1);
    send_block(tmp);
    wait_for_ready();

    //encrypt blockwise in CTR mode
    for(i = 0; i < (m_len & ~(uint8_t)(AES_128_BLOCK_SIZE - 1)); i += AES_128_BLOCK_SIZE)
    //for(i = 0; i < (m_len - (m_len % (AES_128_BLOCK_SIZE - 1))); i += AES_128_BLOCK_SIZE)
        encrypt_block_chunked(m + i, CC2530_AES_MODE_CTR);

    //if the message isn't block-aligned, we can safely pad with zeros and discard the encrypted padding
    if(i < m_len) {
        memcpy(tmp, m + i, m_len - i);
        memset(tmp + m_len - i, 0, AES_128_BLOCK_SIZE - (m_len - i));

        encrypt_block_chunked(tmp, CC2530_AES_MODE_CTR);

        memcpy(m + i, tmp, m_len - i);
    }
}
/*---------------------------------------------------------------------------*/
const struct aes_128_driver cc2530_aes_128_driver = {
    set_key,
    aes_encrypt,
};
/*---------------------------------------------------------------------------*/
const struct ccm_star_driver cc2530_ccm_star_driver = {
    .set_key = set_key,
    .ctr = ctr,
    .mic = mic,
};
