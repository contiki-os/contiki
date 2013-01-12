/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-rf cc2538 RF Driver
 *
 * Driver implementation for the cc2538 RF transceiver
 * @{
 *
 * \file
 * Header file for the cc2538 RF driver
 */
#ifndef CC2538_RF_H__
#define CC2538_RF_H__

#include "contiki.h"
#include "dev/radio.h"
#include "dev/rfcore.h"
#include "reg.h"
/*---------------------------------------------------------------------------
 * RF Config
 *---------------------------------------------------------------------------*/
/* Constants */
#define CC2538_RF_CCA_THRES_USER_GUIDE 0xF8
#define CC2538_RF_TX_POWER_RECOMMENDED 0xD5 /* ToDo: Determine value */
#define CC2538_RF_CHANNEL_MIN            11
#define CC2538_RF_CHANNEL_MAX            26
#define CC2538_RF_CHANNEL_SPACING         5
#define CC2538_RF_MAX_PACKET_LEN        127
#define CC2538_RF_MIN_PACKET_LEN          4
#define CC2538_RF_CCA_CLEAR               1
#define CC2538_RF_CCA_BUSY                0
/*---------------------------------------------------------------------------*/
#ifdef CC2538_RF_CONF_TX_POWER
#define CC2538_RF_TX_POWER CC2538_RF_CONF_TX_POWER
#else
#define CC2538_RF_TX_POWER CC2538_RF_TX_POWER_RECOMMENDED
#endif /* CC2538_RF_CONF_TX_POWER */

#ifdef CC2538_RF_CONF_CCA_THRES
#define CC2538_RF_CCA_THRES CC2538_RF_CONF_CCA_THRES
#else
#define CC2538_RF_CCA_THRES CCA_THRES_USER_GUIDE /** User guide recommendation */
#endif /* CC2538_RF_CONF_CCA_THRES */

#ifdef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CHANNEL CC2538_RF_CONF_CHANNEL
#else
#define CC2538_RF_CHANNEL 18
#endif /* CC2538_RF_CONF_CHANNEL */

#ifdef CC2538_RF_CONF_AUTOACK
#define CC2538_RF_AUTOACK CC2538_RF_CONF_AUTOACK
#else
#define CC2538_RF_AUTOACK 1
#endif /* CC2538_RF_CONF_AUTOACK */
/*---------------------------------------------------------------------------
 * Command Strobe Processor
 *---------------------------------------------------------------------------*/
/* OPCODES */
#define CC2538_RF_CSP_OP_ISRXON                0xE3
#define CC2538_RF_CSP_OP_ISTXON                0xE9
#define CC2538_RF_CSP_OP_ISTXONCCA             0xEA
#define CC2538_RF_CSP_OP_ISRFOFF               0xEF
#define CC2538_RF_CSP_OP_ISFLUSHRX             0xED
#define CC2538_RF_CSP_OP_ISFLUSHTX             0xEE

/**
 * \brief Send an RX ON command strobe to the CSP
 */
#define CC2538_RF_CSP_ISRXON()    \
  do { REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISRXON; } while(0)

/**
 * \brief Send a TX ON command strobe to the CSP
 */
#define CC2538_RF_CSP_ISTXON()    \
  do { REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISTXON; } while(0)

/**
 * \brief Send a RF OFF command strobe to the CSP
 */
#define CC2538_RF_CSP_ISRFOFF()   \
  do { REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISRFOFF; } while(0)

/**
 * \brief Flush the RX FIFO
 */
#define CC2538_RF_CSP_ISFLUSHRX()  do { \
  REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHRX; \
  REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHRX; \
} while(0)

/**
 * \brief Flush the TX FIFO
 */
#define CC2538_RF_CSP_ISFLUSHTX()  do { \
  REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHTX; \
  REG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHTX; \
} while(0)
/*---------------------------------------------------------------------------*/
/** The NETSTACK data structure for the cc2538 RF driver */
extern const struct radio_driver cc2538_rf_driver;
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the current operating channel
 * \param channel The desired channel as a value in [11,26]
 * \return Returns a value in [11,26] representing the current channel
 *         or a negative value if \e channel was out of bounds
 */
int8_t cc2538_rf_channel_set(uint8_t channel);

/**
 * \brief Get the current operating channel
 * \return Returns a value in [11,26] representing the current channel
 */
uint8_t cc2538_rf_channel_get();

/**
 * \brief Sets RF TX power
 * \param new_power The desired power level
 * \return The power level in use after the adjustment
 *
 * The value specified in \e new_power will be written directly to the
 * RFCORE_XREG_TXPOWER register. See the datasheet for more details on
 * possible values.
 */
uint8_t cc2538_rf_power_set(uint8_t new_power);

/**
 * \brief Sets addresses and PAN identifier to the relevant RF hardware
 *        registers
 * \param pan The PAN Identifier
 *
 * Values for short and extended addresses are not needed as parameters
 * since they exist in the rimeaddr buffer in the contiki core. They
 * are thus simply copied over from there.
 */
void cc2538_rf_set_addr(uint16_t pan);
/*---------------------------------------------------------------------------*/
#endif /* CC2538_RF_H__ */

/**
 * @}
 * @}
 */
