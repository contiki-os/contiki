/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         CC2420 driver header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __CC2420_H__
#define __CC2420_H__

#include "contiki.h"
#include "dev/spi.h"
#include "dev/radio.h"
#include "dev/cc2420_const.h"

int cc2420_init(void);

#define CC2420_MAX_PACKET_LEN      127

int cc2420_set_channel(int channel);
int cc2420_get_channel(void);

void cc2420_set_pan_addr(unsigned pan,
                                unsigned addr,
                                const uint8_t *ieee_addr);

extern signed char cc2420_last_rssi;
extern uint8_t cc2420_last_correlation;

int cc2420_rssi(void);

extern const struct radio_driver cc2420_driver;

/**
 * \param power Between 1 and 31.
 */
void cc2420_set_txpower(uint8_t power);
int cc2420_get_txpower(void);
#define CC2420_TXPOWER_MAX  31
#define CC2420_TXPOWER_MIN   0

/**
 * Interrupt function, called from the simple-cc2420-arch driver.
 *
 */
int cc2420_interrupt(void);

/* XXX hack: these will be made as Chameleon packet attributes */
extern rtimer_clock_t cc2420_time_of_arrival,
  cc2420_time_of_departure;
extern int cc2420_authority_level_of_sender;

int cc2420_on(void);
int cc2420_off(void);

void cc2420_set_cca_threshold(int value);

/************************************************************************/
/* Additional SPI Macros for the CC2420 */
/************************************************************************/
/* Send a strobe to the CC2420 */
#define CC2420_STROBE(s)                                   \
  do {                                                  \
    CC2420_SPI_ENABLE();                                \
    SPI_WRITE(s);                                       \
    CC2420_SPI_DISABLE();                               \
  } while (0)

/* Write to a register in the CC2420                         */
/* Note: the SPI_WRITE(0) seems to be needed for getting the */
/* write reg working on the Z1 / MSP430X platform            */
#define CC2420_WRITE_REG(adr,data)                              \
  do {                                                       \
    CC2420_SPI_ENABLE();                                     \
    SPI_WRITE_FAST(adr);                                     \
    SPI_WRITE_FAST((uint8_t)((data) >> 8));                  \
    SPI_WRITE_FAST((uint8_t)(data & 0xff));                  \
    SPI_WAITFORTx_ENDED();                                   \
    SPI_WRITE(0);                                            \
    CC2420_SPI_DISABLE();                                    \
  } while(0)

/* Read a register in the CC2420 */
#define CC2420_READ_REG(adr,data)                          \
  do {                                                  \
    CC2420_SPI_ENABLE();                                \
    SPI_WRITE(adr | 0x40);                              \
    data = (uint8_t)SPI_RXBUF;                          \
    SPI_TXBUF = 0;                                      \
    SPI_WAITFOREORx();                                  \
    data = SPI_RXBUF << 8;                              \
    SPI_TXBUF = 0;                                      \
    SPI_WAITFOREORx();                                  \
    data |= SPI_RXBUF;                                  \
    CC2420_SPI_DISABLE();                               \
  } while(0)

#define CC2420_READ_FIFO_BYTE(data)                        \
  do {                                                  \
    CC2420_SPI_ENABLE();                                \
    SPI_WRITE(CC2420_RXFIFO | 0x40);                    \
    (void)SPI_RXBUF;                                    \
    SPI_READ(data);                                     \
    clock_delay(1);                                     \
    CC2420_SPI_DISABLE();                               \
  } while(0)

#define CC2420_READ_FIFO_BUF(buffer,count)                                 \
  do {                                                                  \
    uint8_t i;                                                          \
    CC2420_SPI_ENABLE();                                                \
    SPI_WRITE(CC2420_RXFIFO | 0x40);                                    \
    (void)SPI_RXBUF;                                                    \
    for(i = 0; i < (count); i++) {                                      \
      SPI_READ(((uint8_t *)(buffer))[i]);                               \
    }                                                                   \
    clock_delay(1);                                                     \
    CC2420_SPI_DISABLE();                                               \
  } while(0)

#define CC2420_WRITE_FIFO_BUF(buffer,count)                                \
  do {                                                                  \
    uint8_t i;                                                          \
    CC2420_SPI_ENABLE();                                                \
    SPI_WRITE_FAST(CC2420_TXFIFO);                                           \
    for(i = 0; i < (count); i++) {                                      \
      SPI_WRITE_FAST(((uint8_t *)(buffer))[i]);                              \
    }                                                                   \
    SPI_WAITFORTx_ENDED();                                              \
    CC2420_SPI_DISABLE();                                               \
  } while(0)

/* Write to RAM in the CC2420 */
#define CC2420_WRITE_RAM(buffer,adr,count)                 \
  do {                                                       \
    uint8_t i;                                               \
    CC2420_SPI_ENABLE();                                     \
    SPI_WRITE_FAST(0x80 | ((adr) & 0x7f));                   \
    SPI_WRITE_FAST(((adr) >> 1) & 0xc0);                     \
    for(i = 0; i < (count); i++) {                           \
      SPI_WRITE_FAST(((uint8_t*)(buffer))[i]);               \
    }                                                        \
    SPI_WAITFORTx_ENDED();                                   \
    CC2420_SPI_DISABLE();                                    \
  } while(0)

/* Write reverse to RAM in the CC2420 */
#define CC2420_WRITE_RAM_REV(buffer,adr,count)               \
  do {                                                       \
    uint8_t i;                                               \
    CC2420_SPI_ENABLE();                                     \
    SPI_WRITE_FAST(0x80 | (adr & 0x7f));                     \
    SPI_WRITE_FAST((adr >> 1) & 0xc0);                       \
    for(i = (count); i > 0; i--) {                           \
      SPI_WRITE_FAST(((uint8_t*)(buffer))[i - 1]);           \
    }                                                        \
    SPI_WAITFORTx_ENDED();                                   \
    CC2420_SPI_DISABLE();                                    \
  } while(0)

/* Read from RAM in the CC2420 */
#define CC2420_READ_RAM(buffer,adr,count)                    \
  do {                                                       \
    uint8_t i;                                               \
    CC2420_SPI_ENABLE();                                     \
    SPI_WRITE(0x80 | ((adr) & 0x7f));                        \
    SPI_WRITE((((adr) >> 1) & 0xc0) | 0x20);                 \
    SPI_RXBUF;                                               \
    for(i = 0; i < (count); i++) {                           \
      SPI_READ(((uint8_t*)(buffer))[i]);                     \
    }                                                        \
    CC2420_SPI_DISABLE();                                    \
  } while(0)

/* Read status of the CC2420 */
#define CC2420_GET_STATUS(s)                       \
  do {                                          \
    CC2420_SPI_ENABLE();                        \
    SPI_WRITE(CC2420_SNOP);                     \
    s = SPI_RXBUF;                              \
    CC2420_SPI_DISABLE();                       \
  } while (0)

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
int cc2420_aes_set_key(const uint8_t *key, int index);

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
int cc2420_aes_cipher(uint8_t *data, int len, int key_index);

#endif /* __CC2420_H__ */
