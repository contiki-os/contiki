/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 */
/**
 * \author Dragos Bogdan <Dragos.Bogdan@Analog.com>
 * Contributors: Ian Martin <martini@redwirellc.com>
 */

#ifndef __ADF7023_CONFIG_H__
#define __ADF7023_CONFIG_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "ADF7023.h"

#define LENGTH_OFFSET          4
#define PACKET_LENGTH_MAX    240
#define ADDRESS_MATCH_OFFSET   0
#define ADDRESS_LENGTH         0

#define F_PFD 26 /* MHz */

#ifndef CHANNEL_FREQ_MHZ
/* #define CHANNEL_FREQ_MHZ 433 // Wrong antenna (432993072 Hz) */
/* #define CHANNEL_FREQ_MHZ 868 // Europe */
#define CHANNEL_FREQ_MHZ 915   /* ISM band center frequency for the Americas, Greenland and some of the eastern Pacific Islands. */
#endif

#define CHANNEL_FREQ (((uint32_t)CHANNEL_FREQ_MHZ << 16) / F_PFD)

/******************************************************************************/
/************************* Variables Declarations *****************************/
/******************************************************************************/
struct ADF7023_BBRAM ADF7023_BBRAMDefault =
{
  /* interruptMask0 - 0x100 */
  BBRAM_INTERRUPT_MASK_0_INTERRUPT_TX_EOF |
  BBRAM_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT,
  /* interruptMask1 - 0x101 */
  0x00,
  /* numberOfWakeups0 - 0x102 */
  0x00,
  /* numberOfWakeups1 - 0x103 */
  0x00,
  /* numberOfWakeupsIrqThreshold0 - 0x104 */
  0xFF,
  /* numberOfWakeupsIrqThreshold1 - 0x105 */
  0xFF,
  /* rxDwellTime - 0x106 */
  0x00,
  /* parmtimeDivider - 0x107 */
  0x33,
  /* swmRssiThresh - 0x108 */
  0x31,
  /* channelFreq0 - 0x109 */
  (CHANNEL_FREQ >> 0) & 0xff,
  /* channelFreq1 - 0x10A */
  (CHANNEL_FREQ >> 8) & 0xff,
  /* channelFreq2 - 0x10B */
  (CHANNEL_FREQ >> 16) & 0xff,
  /* radioCfg0 - 0x10C */
  BBRAM_RADIO_CFG_0_DATA_RATE_7_0(0xE8),          /* Data rate: 100 kbps */
  /* radioCfg1 - 0x10D */
  BBRAM_RADIO_CFG_1_FREQ_DEVIATION_11_8(0x00) |   /* Frequency deviation: 25 Hz */
  BBRAM_RADIO_CFG_1_DATA_RATE_11_8(0x03),         /* Data rate: 100 kbps */
  /* radioCfg2 - 0x10E */
  BBRAM_RADIO_CFG_2_FREQ_DEVIATION_7_0(0xFA),    /* Frequency deviation: 25 Hz */
  /* radioCfg3 - 0x10F */
  0x31,
  /* radioCfg4 - 0x110 */
  0x16,
  /* radioCfg5 - 0x111 */
  0x00,
  /* radioCfg6 - 0x112 */
  BBRAM_RADIO_CFG_6_DISCRIM_PHASE(0x2),
  /* radioCfg7 - 0x113 */
  BBRAM_RADIO_CFG_7_AGC_LOCK_MODE(3),
  /* radioCfg8 - 0x114 */
  BBRAM_RADIO_CFG_8_PA_SINGLE_DIFF_SEL |
  BBRAM_RADIO_CFG_8_PA_LEVEL(0xF) |
  BBRAM_RADIO_CFG_8_PA_RAMP(1),
  /* radioCfg9 - 0x115 */
  BBRAM_RADIO_CFG_9_IFBW(2),
  /* radioCfg10 - 0x116 */
  BBRAM_RADIO_CFG_10_AFC_SCHEME(2) |
  BBRAM_RADIO_CFG_10_AFC_LOCK_MODE(3),
  /* radioCfg11 - 0x117 */
  BBRAM_RADIO_CFG_11_AFC_KP(3) |
  BBRAM_RADIO_CFG_11_AFC_KI(7),
  /* imageRejectCalPhase - 0x118 */
  0x00,
  /* imageRejectCalAmplitude - 0x119 */
  0x00,
  /* modeControl - 0x11A */
  BBRAM_MODE_CONTROL_BB_CAL,
  /* preambleMatch - 0x11B */
  0x0C,
  /* symbolMode - 0x11C */
  BBRAM_SYMBOL_MODE_MANCHESTER_ENC,
  /* preambleLen - 0x11D */
  0x20,
  /* crcPoly0 - 0x11E */
  0x00,
  /* crcPoly1 - 0x11F */
  0x00,
  /* syncControl - 0x120 */
  BBRAM_SYNC_CONTROL_SYNC_WORD_LENGTH(8),
  /* syncByte0 - 0x121 */
  0x00,
  /* syncByte1 - 0x122 */
  0x00,
  /* syncByte2 - 0x123 */
  0x12,
  /* txBaseAdr - 0x124 */
  ADF7023_TX_BASE_ADR,
  /* rxBaseAdr - 0x125 */
  ADF7023_RX_BASE_ADR,
  /* 0x126 (PACKET_LENGTH_CONTROL) = */ 0x20 | LENGTH_OFFSET,
  /* 0x127 (PACKET_LENGTH_MAX)     = */ PACKET_LENGTH_MAX,
  /* staticRegFix - 0x128 */
  0x00,
  /* 0x129 (ADDRESS_MATCH_OFFSET)  = */ ADDRESS_MATCH_OFFSET,
  /* 0x12a (ADDRESS_LENGTH)        = */ ADDRESS_LENGTH,
  /* addressFiltering0 - 0x12B */
  0x01,
  /* addressFiltering1 - 0x12C */
  0xFF,
  /* addressFiltering2 - 0x12D */
  0xFF,
  /* addressFiltering3 - 0x12E */
  0xFF,
  /* addressFiltering4 - 0x12F */
  0x02,
  /* addressFiltering5 - 0x130 */
  0x0F,
  /* addressFiltering6 - 0x131 */
  0xFF,
  /* addressFiltering7 - 0x132 */
  0x0F,
  /* addressFiltering8 - 0x133 */
  0xFF,
  /* addressFiltering9 - 0x134 */
  0x00,
  /* addressFiltering10 - 0x135 */
  0x00,
  /* addressFiltering11 - 0x136 */
  0x00,
  /* addressFiltering12 - 0x137 */
  0x00,
  /* rssiWaitTime - 0x138 */
  0x00,
  /* testmodes - 0x139 */
  0x00,
  /* transitionClockDiv - 0x13A */
  0x00,
  /* reserved0 - 0x13B */
  0x00,
  /* reserved1 - 0x13C */
  0x00,
  /* reserved2 - 0x13D */
  0x00,
  /* rxSynthLockTime - 0x13E */
  0x00,
  /* txSynthLockTime - 0x13F */
  0x00,
};

#endif /* __ADF7023_CONFIG_H__ */
