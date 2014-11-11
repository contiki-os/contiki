/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup SeedEye 
 * @{ */

/**
 * \defgroup mrf24j40 MRF24J40 Driver
 *
 * @{
 */

/**
 * \file   mrf24j40.h
 * \brief  MRF24J40 Driver
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#ifndef MRF24J40_H_
#define MRF24J40_H_

#include <pic32_irq.h>
#include <pic32_spi.h>

extern const struct radio_driver mrf24j40_driver;

#define MRF24J40_DEFAULT_CHANNEL        11

#define MRF24J40_TX_ERR_NONE            0
#define MRF24J40_TX_ERR_NOTSPECIFIED    1
#define MRF24J40_TX_ERR_COLLISION       2
#define MRF24J40_TX_ERR_MAXRETRY        3
#define MRF24J40_TX_WAIT                4

/* Functions prototypes */
void mrf24j40_set_channel(uint16_t ch);
void mrf24j40_set_panid(uint16_t id);
void mrf24j40_set_short_mac_addr(uint16_t addr);
void mrf24j40_set_extended_mac_addr(uint64_t addr);
void mrf24j40_get_short_mac_addr(uint16_t * addr);
void mrf24j40_get_extended_mac_addr(uint64_t * addr);
void mrf24j40_set_tx_power(uint8_t pwr);
void mrf24j40_set_csma_par(uint8_t be, uint8_t nb);
uint8_t mrf24j40_get_status(void);
uint8_t mrf24j40_get_rssi(void);
uint8_t mrf24j40_get_last_rssi(void);
uint8_t mrf24j40_get_last_lqi(void);
int32_t mrf24j40_set_txfifo(const uint8_t * buf, uint8_t buf_len);
int32_t mrf24j40_get_rxfifo(uint8_t * buf, uint8_t buf_len);

/* Long address registers */
#define MRF24J40_RFCON0         (0x200)
#define MRF24J40_RFCON1         (0x201)
#define MRF24J40_RFCON2         (0x202)
#define MRF24J40_RFCON3         (0x203)
#define MRF24J40_RFCON5         (0x205)
#define MRF24J40_RFCON6         (0x206)
#define MRF24J40_RFCON7         (0x207)
#define MRF24J40_RFCON8         (0x208)
#define MRF24J40_SPCAL0         (0x209)
#define MRF24J40_SPCAL1         (0x20A)
#define MRF24J40_SPCAL2         (0x20B)
#define MRF24J40_RFSTATE        (0x20F)
#define MRF24J40_RSSI           (0x210)
#define MRF24J40_SLPCON0        (0x211)
#define MRF24J40_SLPCON1        (0x220)
#define MRF24J40_WAKETIMEL      (0x222)
#define MRF24J40_WAKETIMEH      (0x223)
#define MRF24J40_REMCNTL        (0x224)
#define MRF24J40_REMCNTH        (0x225)
#define MRF24J40_MAINCNT0       (0x226)
#define MRF24J40_MAINCNT1       (0x227)
#define MRF24J40_MAINCNT2       (0x228)
#define MRF24J40_MAINCNT3       (0x229)
#define MRF24J40_TESTMODE       (0x22F)
#define MRF24J40_NORMAL_TX_FIFO (0x000)
#define MRF24J40_BEACON_TX_FIFO (0x080)
#define MRF24J40_GTS1_TX_FIFO   (0x100)
#define MRF24J40_GTS2_TX_FIFO   (0x180)
#define MRF24J40_RX_FIFO        (0x300)
#define MRF24J40_SECURITY_FIFO  (0x280)
#define MRF24J40_UPNONCE0       (0x240)

/* Short address registers */
#define MRF24J40_RXMCR          (0x00)
#define MRF24J40_PANIDL         (0x01)
#define MRF24J40_PANIDH         (0x02)
#define MRF24J40_SADRL          (0x03)
#define MRF24J40_SADRH          (0x04)
#define MRF24J40_EADR0          (0x05)
#define MRF24J40_EADR1          (0x06)
#define MRF24J40_EADR2          (0x07)
#define MRF24J40_EADR3          (0x08)
#define MRF24J40_EADR4          (0x09)
#define MRF24J40_EADR5          (0x0A)
#define MRF24J40_EADR6          (0x0B)
#define MRF24J40_EADR7          (0x0C)
#define MRF24J40_RXFLUSH        (0x0D)
#define MRF24J40_ORDER          (0x10)
#define MRF24J40_TXMCR          (0x11)
#define MRF24J40_ACKTMOUT       (0x12)
#define MRF24J40_ESLOTG1        (0x13)
#define MRF24J40_SYMTICKL       (0x14)
#define MRF24J40_SYMTICKH       (0x15)
#define MRF24J40_PACON0         (0x16)
#define MRF24J40_PACON1         (0x17)
#define MRF24J40_PACON2         (0x18)
#define MRF24J40_TXBCON0        (0x1A)
#define MRF24J40_TXNCON         (0x1B)
#define MRF24J40_TXG1CON        (0x1C)
#define MRF24J40_TXG2CON        (0x1D)
#define MRF24J40_ESLOTG23       (0x1E)
#define MRF24J40_ESLOTG45       (0x1F)
#define MRF24J40_ESLOTG87       (0x20)
#define MRF24J40_TXPEND         (0x21)
#define MRF24J40_WAKECON        (0x22)
#define MRF24J40_FRMOFFSET      (0x23)
#define MRF24J40_TXSTAT         (0x24)
#define MRF24J40_TXBCON1        (0x25)
#define MRF24J40_GATECLK        (0x26)
#define MRF24J40_TXTIME         (0x27)
#define MRF24J40_HSYMTMRL       (0x28)
#define MRF24J40_HSYMTMRH       (0x29)
#define MRF24J40_SOFTRST        (0x2A)
#define MRF24J40_SECCON0        (0x2C)
#define MRF24J40_SECCON1        (0x2D)
#define MRF24J40_TXSTBL         (0x2E)
#define MRF24J40_RXSR           (0x30)
#define MRF24J40_INTSTAT        (0x31)
#define MRF24J40_INTCON         (0x32)
#define MRF24J40_GPIO           (0x33)
#define MRF24J40_TRISGPIO       (0x34)
#define MRF24J40_SLPACK         (0x35)
#define MRF24J40_RFCTL          (0x36)
#define MRF24J40_SECCR2         (0x37)
#define MRF24J40_BBREG0         (0x38)
#define MRF24J40_BBREG1         (0x39)
#define MRF24J40_BBREG2         (0x3A)
#define MRF24J40_BBREG3         (0x3B)
#define MRF24J40_BBREG4         (0x3C)
#define MRF24J40_BBREG6         (0x3E)
#define MRF24J40_CCAEDTH        (0x3F)

/* 
 * TX power setting generation:
 * tx_pwr_set(large_val, small_val) computes the value for the RFCON3 register.
 * Examples: 
 * - if we want to set tx power to -11,2 dB then: 
 *   mrf24j40_tx_pwr_set(#define MRF24J40_PWR_H_MINUS_10dB, #define MRF24J40_PWR_L_MINUS_1.2dB).
 */

#define MRF24J40_PWR_H_MINUS_30dB  (0b11)
#define MRF24J40_PWR_H_MINUS_20dB  (0b10)
#define MRF24J40_PWR_H_MINUS_10dB  (0b01)
#define MRF24J40_PWR_H_0dB         (0b00)

#define MRF24J40_PWR_L_MINUS_6_3dB (0b111)
#define MRF24J40_PWR_L_MINUS_4_9dB (0b110)
#define MRF24J40_PWR_L_MINUS_3_7dB (0b101)
#define MRF24J40_PWR_L_MINUS_2_8dB (0b100)
#define MRF24J40_PWR_L_MINUS_1_9dB (0b011)
#define MRF24J40_PWR_L_MINUS_1_2dB (0b010)
#define MRF24J40_PWR_L_MINUS_0_5dB (0b001)
#define MRF24J40_PWR_L_0dB         (0b000)

#define MRF24J40_TX_PWR_SET(large_val, small_val) ((large_val << 6) | (small_val << 3))

typedef union _TX_status {
  uint8_t val;
  struct TX_bits {
    uint8_t TXNSTAT:1;
    uint8_t TXG1STAT:1;
    uint8_t TXG2STAT:1;
    uint8_t TXG1FNT:1;
    uint8_t TXG2FNT:1;
    uint8_t CCAFAIL:1;
    uint8_t TXNRETRY:2;
  } bits;
} TX_status;

typedef union _INT_status {
  uint8_t val;
  struct INT_bits {
    uint8_t TXNIF:1;
    uint8_t TXG1IF:1;
    uint8_t TXG2IF:1;
    uint8_t RXIF:1;
    uint8_t SECIF:1;
    uint8_t HSYMTMRIF:1;
    uint8_t WAKEIF:1;
    uint8_t SLPIF:1;
  } bits;
} INT_status;

#endif /* MRF24J40_H_ */

/** @} */
/** @} */
