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
 *         Implementation of the cc2530 RF driver
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#ifndef __CC2530_RF_H__
#define __CC2530_RF_H__

#include "contiki.h"
#include "dev/radio.h"
#include "cc253x.h"
/*---------------------------------------------------------------------------
 * RF Config
 *---------------------------------------------------------------------------*/
#define CC2530_RF_TX_POWER_RECOMMENDED 0xD5
#ifdef CC2530_RF_CONF_TX_POWER
#define CC2530_RF_TX_POWER CC2530_RF_CONF_TX_POWER
#else
#define CC2530_RF_TX_POWER CC2530_RF_TX_POWER_RECOMMENDED
#endif /* CC2530_RF_CONF_TX_POWER */

#ifdef CC2530_RF_CONF_CCA_THRES
#define CC2530_RF_CCA_THRES CC2530_RF_CONF_CCA_THRES
#else
#define CC2530_RF_CCA_THRES CCA_THRES_USER_GUIDE /* User guide recommendation */
#endif /* CC2530_RF_CONF_CCA_THRES */

#ifdef CC2530_RF_CONF_CHANNEL
#define CC2530_RF_CHANNEL CC2530_RF_CONF_CHANNEL
#else
#define CC2530_RF_CHANNEL 18
#endif /* CC2530_RF_CONF_CHANNEL */
#define CC2530_RF_CHANNEL_MIN     11
#define CC2530_RF_CHANNEL_MAX     26
#define CC2530_RF_CHANNEL_SPACING  5

#ifdef CC2530_RF_CONF_AUTOACK
#define CC2530_RF_AUTOACK CC2530_RF_CONF_AUTOACK
#else
#define CC2530_RF_AUTOACK 1
#endif /* CC2530_RF_CONF_AUTOACK */

#ifdef CC2530_RF_CONF_LOW_POWER_RX
#define CC2530_RF_LOW_POWER_RX CC2530_RF_CONF_LOW_POWER_RX
#else
#define CC2530_RF_LOW_POWER_RX 0
#endif /* CC2530_RF_CONF_LOW_POWER_RX */
/*---------------------------------------------------------------------------*/
#define CCA_THRES_USER_GUIDE          0xF8
#define CCA_THRES_ALONE               0xFC   /* -4-76=-80dBm when CC2530 operated alone or with CC2591 in LGM */
#define CCA_THR_HGM                   0x06   /* 6-76=-70dBm when CC2530 operated with CC2591 in HGM */
#define CORR_THR                      0x14
/*---------------------------------------------------------------------------*/
#define CC2530_RF_MAX_PACKET_LEN      127
#define CC2530_RF_MIN_PACKET_LEN        4
/*---------------------------------------------------------------------------*/
#define CC2530_RF_CCA_CLEAR             1
#define CC2530_RF_CCA_BUSY              0

/* Wait for RSSI to be valid. */
#define CC2530_RF_CCA_VALID_WAIT()  while(!(RSSISTAT & RSSIST))
/*---------------------------------------------------------------------------
 * Command Strobe Processor
 *---------------------------------------------------------------------------*/
/* OPCODES */
#define CSP_OP_ISRXON                0xE3
#define CSP_OP_ISTXON                0xE9
#define CSP_OP_ISTXONCCA             0xEA
#define CSP_OP_ISRFOFF               0xEF
#define CSP_OP_ISFLUSHRX             0xED
#define CSP_OP_ISFLUSHTX             0xEE

#define CC2530_CSP_ISRXON()    do { RFST = CSP_OP_ISRXON; } while(0)
#define CC2530_CSP_ISTXON()    do { RFST = CSP_OP_ISTXON; } while(0)
#define CC2530_CSP_ISTXONCCA() do { RFST = CSP_OP_ISTXONCCA; } while(0)
#define CC2530_CSP_ISRFOFF()   do { RFST = CSP_OP_ISRFOFF; } while(0)

/* OP x 2 for flushes */
#define CC2530_CSP_ISFLUSHRX()  do { \
  RFST = CSP_OP_ISFLUSHRX; \
  RFST = CSP_OP_ISFLUSHRX; \
} while(0)
#define CC2530_CSP_ISFLUSHTX()  do { \
  RFST = CSP_OP_ISFLUSHTX; \
  RFST = CSP_OP_ISFLUSHTX; \
} while(0)
/*---------------------------------------------------------------------------*/
extern const struct radio_driver cc2530_rf_driver;
/*---------------------------------------------------------------------------*/
int8_t cc2530_rf_channel_set(uint8_t channel);
#define cc2530_rf_channel_get() ((uint8_t)((FREQCTRL + 44) / 5))
uint8_t cc2530_rf_power_set(uint8_t new_power);
void cc2530_rf_set_addr(uint16_t pan);
/*---------------------------------------------------------------------------*/
#endif /* __CC2530_RF_H__ */
