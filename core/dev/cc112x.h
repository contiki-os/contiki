/**
 * \addtogroup cc112x
 * @{
 *
 * \file
 * CC112x radio driver header file
 *
 * \author Martin Chaplet <m.chaplet@kerlink.fr>
 */

/*
 * Copyright (c) 2012, Kerlink
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
 * $Id: $
 */

#ifndef __CC112X_H__
#define __CC112X_H__

#include "contiki.h"
#include "dev/spi.h"
#include "dev/radio.h"
#include "dev/cc112x_const.h"


typedef struct
{
  uint16_t  addr;
  uint8_t   data;
}registerSetting_t;


int cc112x_init(void);

/**
 * Maximum length of a radio packet.
 * Variable packet length mode allows a maximum of 255 bytes per packet,
 * from which we must substract the length byte.
 * So, the maximum acceptable value for CC112X_MAX_PACKET_LEN is 254.
 *
 * A value of 127 fits 802.15.4 frames.
 */
#define CC112X_MAX_PACKET_LEN	127

int cc112x_set_channel(int channel);
int cc112x_get_channel(void);

extern int16_t cc112x_last_rssi;
extern uint8_t cc112x_last_lqi;

int cc112x_rssi(void);

extern const struct radio_driver cc112x_driver;

void cc112x_set_txpower(int8_t power);
int cc112x_get_txpower(void);

int cc112x_interrupt(void);

int cc112x_on(void);
int cc112x_off(void);

/************************************************************************/
/* Additional SPI Macros for the CC112X */
/************************************************************************/
/* Send a strobe to the CC112X */
#define CC112X_STROBE(s)                                \
  do {                                                  \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE(CC112X_WRITE | s);                        \
    CC112X_SPI_DISABLE();                               \
  } while (0)

/* Write to a register in the CC112X */
#define CC112X_WRITE_REG(adr,data)                      \
  do {                                                  \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE_FAST(CC112X_WRITE | adr);                 \
    SPI_WRITE_FAST((uint8_t)(data & 0xff));             \
    SPI_WAITFORTx_ENDED();                              \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Write to registers in burst mode */
#define CC112X_WRITE_BURST(adr,buffer,count)            \
  do {                                                  \
    uint8_t i;                                          \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE_FAST(CC112X_WRITE | CC112X_BURST | adr);  \
    for(i = 0; i < (count); i++) {                      \
      SPI_WRITE_FAST(((uint8_t *)(buffer))[i]);         \
    }                                                   \
    SPI_WAITFORTx_ENDED();                              \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Write to an extended register in the CC112X */
#define CC112X_WRITE_EXT_REG(adr,data)                 \
  do {                                                  \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE_FAST(CC112X_WRITE | CC112X_EXTMEM);       \
    SPI_WRITE_FAST((uint8_t)(adr & 0xff));              \
    SPI_WRITE_FAST((uint8_t)(data & 0xff));             \
    SPI_WAITFORTx_ENDED();                              \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Write to extended registers in burst mode */
#define CC112X_WRITE_EXT_BURST(adr,buffer,count)       \
  do {                                                  \
    uint8_t i;                                          \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE_FAST(CC112X_WRITE | CC112X_BURST | CC112X_EXTMEM);  \
    SPI_WRITE_FAST((uint8_t)(adr & 0xff));              \
    for(i = 0; i < (count); i++) {                      \
      SPI_WRITE_FAST(((uint8_t *)(buffer))[i]);         \
    }                                                   \
    SPI_WAITFORTx_ENDED();                              \
    CC112X_SPI_DISABLE();                               \
  } while(0)


/* Read a register in the CC112X */
#define CC112X_READ_REG(adr,data)                       \
  do {                                                  \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE(CC112X_READ | adr);                       \
    (void)SPI_RXBUF;                                    \
    SPI_READ(data);                                     \
    clock_delay(1);                                     \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Read registers in burst mode */
#define CC112X_READ_BURST(adr,buffer,count)             \
  do {                                                  \
    uint8_t i;                                          \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE(CC112X_READ | CC112X_BURST | adr);        \
    (void)SPI_RXBUF;                                    \
    for(i = 0; i < (count); i++) {                      \
      SPI_READ(((uint8_t *)(buffer))[i]);               \
    }                                                   \
    clock_delay(1);                                     \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Read an extended register in the CC112X */
#define CC112X_READ_EXT_REG(adr,data)                  \
  do {                                                  \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE_FAST(CC112X_READ | CC112X_EXTMEM);        \
    SPI_WRITE((uint8_t)(adr & 0xff));                   \
    (void)SPI_RXBUF;                                    \
    SPI_READ(data);                                     \
    clock_delay(1);                                     \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Read extended registers in burst mode */
#define CC112X_READ_EXT_BURST(adr,buffer,count)        \
  do {                                                  \
    uint8_t i;                                          \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE_FAST(CC112X_WRITE | CC112X_BURST | CC112X_EXTMEM);  \
    SPI_WRITE((uint8_t)(adr & 0xff));                   \
    (void)SPI_RXBUF;                                    \
    for(i = 0; i < (count); i++) {                      \
      SPI_READ(((uint8_t *)(buffer))[i]);               \
    }                                                   \
    clock_delay(1);                                     \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Read form the FIFO */
#define CC112X_READ_FIFO_BYTE(data)                     \
  do {                                                  \
    CC112X_SPI_ENABLE();                                \
    SPI_WRITE(CC112X_READ | CC112X_RXFIFO);             \
    (void)SPI_RXBUF;                                    \
    SPI_READ(data);                                     \
    clock_delay(1);                                     \
    CC112X_SPI_DISABLE();                               \
  } while(0)

/* Read form the FIFO in burst mode */
#define CC112X_READ_FIFO_BUF(buffer,count)                      \
  do {                                                          \
    uint8_t i;                                                  \
    CC112X_SPI_ENABLE();                                        \
    SPI_WRITE(CC112X_READ | CC112X_BURST | CC112X_RXFIFO);      \
    (void)SPI_RXBUF;                                            \
    for(i = 0; i < (count); i++) {                              \
      SPI_READ(((uint8_t *)(buffer))[i]);                       \
    }                                                           \
    clock_delay(1);                                             \
    CC112X_SPI_DISABLE();                                       \
  } while(0)

/* Write to the FIFO in burst mode */
#define CC112X_WRITE_FIFO_BUF(buffer,count)                             \
  do {                                                                  \
    uint8_t i;                                                          \
    CC112X_SPI_ENABLE();                                                \
    SPI_WRITE_FAST(CC112X_WRITE | CC112X_BURST | CC112X_TXFIFO);        \
    for(i = 0; i < (count); i++) {                                      \
      SPI_WRITE_FAST(((uint8_t *)(buffer))[i]);                         \
    }                                                                   \
    SPI_WAITFORTx_ENDED();                                              \
    CC112X_SPI_DISABLE();                                               \
  } while(0)

/* Read form the FIFO in Direct mode */
#define CC112X_READ_TXFIFO_DIRECT(adr,buffer,count)            \
  do {                                                          \
    uint8_t i;                                                  \
    CC112X_SPI_ENABLE();                                        \
    SPI_WRITE_FAST(CC112X_READ | CC112X_BURST | CC112X_DIRECT_FIFO); \
    SPI_WRITE((uint8_t)(adr & 0xff));                           \
    (void)SPI_RXBUF;                                            \
    for(i = 0; i < (count); i++) {                              \
      SPI_READ(((uint8_t *)(buffer))[i]);                       \
    }                                                           \
    clock_delay(1);                                             \
    CC112X_SPI_DISABLE();                                       \
  } while(0)

/* Read form the FIFO in Direct mode */
#define CC112X_READ_RXFIFO_DIRECT(adr,buffer,count)            \
  do {                                                          \
    uint8_t i;                                                  \
    CC112X_SPI_ENABLE();                                        \
    SPI_WRITE_FAST(CC112X_READ | CC112X_BURST | CC112X_DIRECT_FIFO); \
    SPI_WRITE((uint8_t)(adr & 0xff | 0x80));                     \
    (void)SPI_RXBUF;                                            \
    for(i = 0; i < (count); i++) {                              \
      SPI_READ(((uint8_t *)(buffer))[i]);                       \
    }                                                           \
    clock_delay(1);                                             \
    CC112X_SPI_DISABLE();                                       \
  } while(0)

/* Read status of the CC112X, with RX FIFO info */
#define CC112X_GET_STATUS(s)                    \
  do {                                          \
    CC112X_SPI_ENABLE();                        \
    SPI_WRITE(CC112X_READ | CC112X_SNOP);       \
    s = SPI_RXBUF;                              \
    CC112X_SPI_DISABLE();                       \
  } while (0)

/* Read status of the CC112X, with TX FIFO info */
#define CC112X_GET_STATUS_TX(s)                 \
  do {                                          \
    CC112X_SPI_ENABLE();                        \
    SPI_WRITE(CC112X_WRITE | CC112X_SNOP);      \
    s = SPI_RXBUF;                              \
    CC112X_SPI_DISABLE();                       \
  } while (0)

#endif /* __CC112X_H__ */
/** @} */
