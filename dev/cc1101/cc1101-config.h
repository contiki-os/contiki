/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef CC1101_CONFIG_H
#define CC1101_CONFIG_H

#include "contiki-conf.h"

#ifdef CC1101_CONF_ETSI
#error CC1101_CONF_ETSI not supported, see cc1101-config.h
#endif /* CC1101_CONF_ETSI */
#ifdef CC1101_CONF_FCC
#error CC1101_CONF_FCC not supported, see cc1101-config.h
#endif /* CC1101_CONF_FCC */

/* 802.15.4g compliant settings */

/* PA ramping = false */
/* Data format = Normal mode */
/* Channel spacing = 199.951172 */
/* Manchester enable = false */
/* Packet length = 255 */
/* Packet length mode = Variable packet length mode. Packet length configured by the first byte after sync word */
/* Preamble count = 4 */
/* Whitening = false */
/* Modulation format = GFSK */
/* Data rate = 49.9878 */
/* CRC enable = true */
/* Channel number = 0 */
/* CRC autoflush = false */
/* Address config = No address check */
/* Carrier frequency = 863.125 */
/* Base frequency = 863.125 */
/* Deviation = 25.390625 */
/* Sync word qualifier mode = 30/32 sync word bits detected */
/* Modulated = true */
/* Device address = 0 */
/* TX power = 0 */

#define RX_FILTER_BW					135
#define CURRENT_OPTIMIZED				0

#define CC1101_SETTING_IOCFG2           0x29
#define CC1101_SETTING_IOCFG1           0x2E
#define CC1101_SETTING_IOCFG0           0x06
#define CC1101_SETTING_FIFOTHR          0x47
#define CC1101_SETTING_SYNC1            0xD3
#define CC1101_SETTING_SYNC0            0x91
#define CC1101_SETTING_PKTLEN           128
#define CC1101_SETTING_PKTCTRL1         0x04
#define CC1101_SETTING_PKTCTRL0         0x05
#define CC1101_SETTING_ADDR             0x00
#define CC1101_SETTING_CHANNR           0x00

#if CURRENT_OPTIMIZED
#define CC1101_SETTING_FSCTRL1          0x08
#else
#define CC1101_SETTING_FSCTRL1          0x06
#endif

#define CC1101_SETTING_FSCTRL0          0x00
#define CC1101_SETTING_FREQ2            0x21
#define CC1101_SETTING_FREQ1            0x32
#define CC1101_SETTING_FREQ0            0x76

#if RX_FILTER_BW == 100
#define CC1101_SETTING_MDMCFG4          0xCA
#elif RX_FILTER_BW == 135
#define CC1101_SETTING_MDMCFG4          0xAA
#elif RX_FILTER_BW == 160
#define CC1101_SETTING_MDMCFG4          0x9A
#elif RX_FILTER_BW == 200
#define CC1101_SETTING_MDMCFG4          0x8A
#else
#error Unsupported RX filter BW
#endif

#define CC1101_SETTING_MDMCFG3          0xF8

#if CURRENT_OPTIMIZED
#define CC1101_SETTING_MDMCFG2          0x93
#else
#define CC1101_SETTING_MDMCFG2          0x13
#endif

#define CC1101_SETTING_MDMCFG1          0x22
#define CC1101_SETTING_MDMCFG0          0xF8
#define CC1101_SETTING_DEVIATN          0x40
#define CC1101_SETTING_MCSM2            0x07
#define CC1101_SETTING_MCSM1            0x30
#define CC1101_SETTING_MCSM0            0x18
#define CC1101_SETTING_FOCCFG           0x16
#define CC1101_SETTING_BSCFG            0x6C
#define CC1101_SETTING_AGCCTRL2         0x43
#define CC1101_SETTING_AGCCTRL1         0x40
#define CC1101_SETTING_AGCCTRL0         0x91
#define CC1101_SETTING_WOREVT1          0x87
#define CC1101_SETTING_WOREVT0          0x6B
#define CC1101_SETTING_WORCTRL          0xFB
#define CC1101_SETTING_FREND1           0x56
#define CC1101_SETTING_FREND0           0x10
#define CC1101_SETTING_FSCAL3           0xE9
#define CC1101_SETTING_FSCAL2           0x2A
#define CC1101_SETTING_FSCAL1           0x00
#define CC1101_SETTING_FSCAL0           0x1F
#define CC1101_SETTING_RCCTRL1          0x41
#define CC1101_SETTING_RCCTRL0          0x00
#define CC1101_SETTING_FSTEST           0x59
#define CC1101_SETTING_PTEST            0x7F
#define CC1101_SETTING_AGCTEST          0x3F
#define CC1101_SETTING_TEST2            0x81
#define CC1101_SETTING_TEST1            0x35
#define CC1101_SETTING_TEST0            0x09
#define CC1101_SETTING_PARTNUM          0x00
#define CC1101_SETTING_VERSION          0x04
#define CC1101_SETTING_FREQEST          0x00
#define CC1101_SETTING_LQI              0x00
#define CC1101_SETTING_RSSI             0x00
#define CC1101_SETTING_MARCSTATE        0x00
#define CC1101_SETTING_WORTIME1         0x00
#define CC1101_SETTING_WORTIME0         0x00
#define CC1101_SETTING_PKTSTATUS        0x00
#define CC1101_SETTING_VCO_VC_DAC       0x00
#define CC1101_SETTING_TXBYTES          0x00
#define CC1101_SETTING_RXBYTES          0x00
#define CC1101_SETTING_RCCTRL1_STATUS   0x00
#define CC1101_SETTING_RCCTRL0_STATUS   0x00




/* bit-field and other definitions */
#define PKTCTRL1_CRC_AUTOFLUSH BV(3)
#define PKTCTRL1_APPEND_STATUS BV(2)
#define PKTCTRL0_CRC_EN BV(2)
#define PKTCTRL0_PKTLEN_VARIABLE BV(0)

#define FIFOTHR_RXFIFO33_TXFIFO32 7
#define FIFOTHR_RXFIFO12_TXFIFO53 2

#define CCA_MODE 3 /* Below threshold or receiving packet */
#define RXOFF_MODE 3 /* Return to RX after packet received */
#define TXOFF_MODE 3 /* Return to RX after packet transmitted */

/* GDO configuration options */
#define IOCFG0_RXFIFO_EOF_PACKET  0x01
#define IOCFG0_RX_OVERFLOW        0x04
#define IOCFG0_TX_UNDERFLOW       0x05
#define IOCFG0_SYNC_WORD          0x06
#define IOCFG0_RXFIFO_CRC_OK      0x07
#define IOCFG0_CCA                0x09

/* re-define some settings */
#undef CC1101_SETTING_FIFOTHR
#define CC1101_SETTING_FIFOTHR    3

#undef CC1101_SETTING_PKTLEN
#define CC1101_SETTING_PKTLEN     128

#undef CC1101_SETTING_PKTCTRL1
#define CC1101_SETTING_PKTCTRL1   PKTCTRL1_APPEND_STATUS

#undef CC1101_SETTING_PKTCTRL0
#define CC1101_SETTING_PKTCTRL0   (PKTCTRL0_CRC_EN | PKTCTRL0_PKTLEN_VARIABLE)

#undef CC1101_SETTING_IOCFG2
#define CC1101_SETTING_IOCFG2     IOCFG0_RX_OVERFLOW

#undef CC1101_SETTING_IOCFG0
#define CC1101_SETTING_IOCFG0     IOCFG0_RXFIFO_EOF_PACKET

#undef CC1101_SETTING_MCSM1
#define CC1101_SETTING_MCSM1 (TXOFF_MODE) | (RXOFF_MODE << 2) | (CCA_MODE << 4)

#define FS_AUTOCAL_NONE   0x00
#define FS_AUTOCAL_ALWAYS 0x10
#define FS_AUTOCAL_4TH    0x30

#define PO_TIMEOUT_DEFAULT 0x08

#undef CC1101_SETTING_MCSM0
#define CC1101_SETTING_MCSM0 (FS_AUTOCAL_NONE | PO_TIMEOUT_DEFAULT)
#endif /* CC1101_CONFIG_H */
