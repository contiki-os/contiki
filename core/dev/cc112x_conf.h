/**
 * \addtogroup cc112x
 * @{
 *
 * \file
 * Configuration register value sets for the CC112X
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

#ifndef __CC112X_CONF_H__
#define __CC112X_CONF_H__

#include "dev/cc112x.h"
#include "dev/cc112x_const.h"

/*
 * The values in CC112X_CONFIG_REGISTERS come from SmartRF Studio 7 v1.7.2
 */

// RX filter BW = 25.000000
// Address config = No address check
// Packet length = 19
// Symbol rate = 1.2
// PA ramping = true
// Carrier frequency = 169.000000
// Bit rate = 1.2
// Whitening = false
// Manchester enable = false
// Modulation format = 2-FSK
// Packet length mode = Variable
// Device address = 0
// TX power = -11
// Deviation = 3.997803

static const registerSetting_t preferredSettings[]=
{
  {CC112X_IOCFG3,            0xB0},
  {CC112X_IOCFG2,            0xB0},
  {CC112X_IOCFG1,            0xB0},
  {CC112X_IOCFG0,            0x06},
  {CC112X_SYNC_CFG1,         0x0B},
  {CC112X_SYNC_CFG0,         0x17},
  {CC112X_DCFILT_CFG,        0x1C},
  {CC112X_PREAMBLE_CFG1,     0x18},
  {CC112X_IQIC,              0xC6},
  {CC112X_CHAN_BW,           0x08},
  {CC112X_MDMCFG0,           0x05},
  {CC112X_AGC_REF,           0x20},
  {CC112X_AGC_CS_THR,        0x19},
  {CC112X_AGC_CFG1,          0xA9},
  {CC112X_AGC_CFG0,          0xCF},
  {CC112X_FIFO_CFG,          0x00},
  {CC112X_SETTLING_CFG,      0x03},
  {CC112X_FS_CFG,            0x1A},
  {CC112X_PKT_CFG2,          0x00},
  {CC112X_PKT_CFG1,          0x05},
  {CC112X_PKT_CFG0,          0x20},
  {CC112X_PA_CFG2,           0x03},
  {CC112X_PA_CFG1,           0x56},
  {CC112X_PA_CFG0,           0x4C},
  {CC112X_PKT_LEN,           0xFF},
  {CC112X_IF_MIX_CFG,        0x00},
  {CC112X_FREQOFF_CFG,       0x22},
  {CC112X_FREQ2,             0x69},
  {CC112X_FREQ1,             0xA0},
  {CC112X_FS_DIG1,           0x00},
  {CC112X_FS_DIG0,           0x5F},
  {CC112X_FS_CAL0,           0x0E},
  {CC112X_FS_DIVTWO,         0x03},
  {CC112X_FS_DSM0,           0x33},
  {CC112X_FS_DVC0,           0x17},
  {CC112X_FS_PFD,            0x50},
  {CC112X_FS_PRE,            0x6E},
  {CC112X_FS_REG_DIV_CML,    0x14},
  {CC112X_FS_SPARE,          0xAC},
  {CC112X_FS_VCO0,           0xB4},
  {CC112X_XOSC1,             0x03},
};

static const registerSetting_t sniffSettings[] =
{
  {CC112X_IOCFG0            ,0x06}, // Route sync signal to GPIO0
  {CC112X_FS_DIG1           ,0x00},
  {CC112X_FS_DIG0           ,0x5F},
  {CC112X_FS_CAL1           ,0X40},
  {CC112X_FS_CAL0           ,0x0E},
  {CC112X_FS_DIVTWO         ,0x03},
  {CC112X_FS_DSM0           ,0x33},
  {CC112X_FS_DVC0           ,0x17},
  {CC112X_FS_PFD            ,0x50},
  {CC112X_FS_PRE            ,0x6E},
  {CC112X_FS_REG_DIV_CML    ,0x14},
  {CC112X_FS_SPARE          ,0xAC},
  {CC112X_FS_VCO0           ,0xB4},
  {CC112X_XOSC5             ,0x0E},
  {CC112X_XOSC4             ,0xA0},
  {CC112X_XOSC3             ,0x03},
  {CC112X_XOSC1             ,0x03},
  {CC112X_ANALOG_SPARE      ,0x00},
  {CC112X_FIFO_CFG          ,0x00},
  {CC112X_DEV_ADDR          ,0x00},
  {CC112X_SETTLING_CFG      ,0x03},
  {CC112X_FS_CFG            ,0x12}, //////////////////////////////////////////////////////////////////
  {CC112X_PKT_CFG2          ,0x00},
  {CC112X_PKT_CFG1          ,0x05}, // Address check off and CRC check on
  {CC112X_PKT_CFG0          ,0x20},
  {CC112X_RFEND_CFG1        ,0x0F}, // Stay in RX after RX, No timeout for sync word search
  {CC112X_RFEND_CFG0        ,0x00}, // IDle after TX, no interferring from MARC
  {CC112X_PKT_LEN           ,0xFF},
  {CC112X_SYNC3             ,0x93},/////////////////////////////////////////////////////////////////////
  {CC112X_SYNC2             ,0x0B},
  {CC112X_SYNC1             ,0x51},
  {CC112X_SYNC0             ,0xDE},
  {CC112X_SYNC_CFG1         ,0x0B},
  {CC112X_SYNC_CFG0         ,0x17},
  {CC112X_DEVIATION_M       ,0x06}, // (4000 Hz)
  {CC112X_MODCFG_DEV_E      ,0x03}, // (4000 Hz)
  {CC112X_DCFILT_CFG        ,0x1C}, //
  {CC112X_PREAMBLE_CFG1     ,0x18}, // 4" byte preamble
  {CC112X_PREAMBLE_CFG0     ,0x2A}, //
  {CC112X_FREQ_IF_CFG       ,0x40}, // (62500 Hz)
  {CC112X_IQIC              ,0xC6}, //
  {CC112X_CHAN_BW           ,0x08}, // (25000" Hz)
  {CC112X_MDMCFG1           ,0x46}, //
  {CC112X_MDMCFG0           ,0x05}, //
  {CC112X_DRATE2            ,0x43}, // (1200 bps)
  {CC112X_DRATE1            ,0xA9}, // (1200 bps)
  {CC112X_DRATE0            ,0x2A}, // (1200 bps)
  {CC112X_AGC_REF           ,0x20},
  {CC112X_AGC_CS_THR        ,0x19},
  {CC112X_AGC_GAIN_ADJUST   ,0x00},
  {CC112X_AGC_CFG3          ,0x91},
  {CC112X_AGC_CFG2          ,0x20},
  {CC112X_AGC_CFG1          ,0xA9},
  {CC112X_AGC_CFG0          ,0xCF},
  {CC112X_PA_CFG2           ,0x7F},
  {CC112X_PA_CFG1           ,0x56},
  {CC112X_PA_CFG0           ,0x7C},
  {CC112X_IF_MIX_CFG        ,0x00},
  {CC112X_FREQOFF_CFG       ,0x22},
  {CC112X_TOC_CFG           ,0x0B},
  {CC112X_SOFT_TX_DATA_CFG  ,0x00}
};

#endif /* __CC112X_CONF_H__ */
/** @} */
