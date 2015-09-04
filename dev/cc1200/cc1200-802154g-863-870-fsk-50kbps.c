/*
 * Copyright (c) 2015, Weptech elektronik GmbH Germany
 * http://www.weptech.de
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
 * This file is part of the Contiki operating system.
 */

#include "cc1200-rf-cfg.h"
#include "cc1200-const.h"

/*
 * This is a setup for the following configuration:
 *
 * 802.15.4g
 * =========
 * Table 68f: Frequency band identifier 4 (863-870 MHz)
 * Table 68g: Modulation scheme identifier 0 (Filtered FSK)
 * Table 68h: Mode #1 (50kbps)
 */

/* Base frequency in kHz */
#define RF_CFG_CHAN_CENTER_F0           863125
/* Channel spacing in kHz */
#define RF_CFG_CHAN_SPACING             200
/* The minimum channel */
#define RF_CFG_MIN_CHANNEL              0
/* The maximum channel */
#define RF_CFG_MAX_CHANNEL              33
/* The maximum output power in dBm */
#define RF_CFG_MAX_TXPOWER              CC1200_CONST_TX_POWER_MAX
/* The carrier sense level used for CCA in dBm */
#define RF_CFG_CCA_THRESHOLD            (-91)
/*---------------------------------------------------------------------------*/
static const char rf_cfg_descriptor[] = "802.15.4g 863-870MHz MR-FSK mode #1";
/*---------------------------------------------------------------------------*/
/* 
 * Register settings exported from SmartRF Studio using the standard template
 * "trxEB RF Settings Performance Line".
 */

// Modulation format = 2-GFSK
// Whitening = false
// Packet length = 255
// Packet length mode = Variable
// Packet bit length = 0
// Symbol rate = 50
// Deviation = 24.948120
// Carrier frequency = 867.999878
// Device address = 0
// Manchester enable = false
// Address config = No address check
// Bit rate = 50
// RX filter BW = 104.166667

static const registerSetting_t preferredSettings[]=
{
  {CC1200_IOCFG2,            0x06},
  {CC1200_SYNC3,             0x6E},
  {CC1200_SYNC2,             0x4E},
  {CC1200_SYNC1,             0x90},
  {CC1200_SYNC0,             0x4E},
  {CC1200_SYNC_CFG1,         0xE5},
  {CC1200_SYNC_CFG0,         0x23},
  {CC1200_DEVIATION_M,       0x47},
  {CC1200_MODCFG_DEV_E,      0x0B},
  {CC1200_DCFILT_CFG,        0x56},

  /*
   * 18.1.1.1 Preamble field
   *  The Preamble field shall contain phyFSKPreambleLength (as defined in 9.3)
   *  multiples of the 8-bit sequence “01010101” for filtered 2FSK.
   *  The Preamble field shall contain phyFSKPreambleLength multiples of the
   *  16-bit sequence “0111 0111 0111 0111” for filtered 4FSK.
   *
   * We need to define this in order to be able to compute e.g. timeouts for the
   * MAC layer. According to 9.3, phyFSKPreambleLength can be configured between
   * 4 and 1000. We set it to 4. Attention: Once we use a long wake-up preamble,
   * the timing parameters have to change accordingly. Will we use a shorter
   * preamble for an ACK in this case???
   */
  {CC1200_PREAMBLE_CFG1,     0x19},

  {CC1200_PREAMBLE_CFG0,     0xBA},
  {CC1200_IQIC,              0xC8},
  {CC1200_CHAN_BW,           0x84},
  {CC1200_MDMCFG1,           0x42},
  {CC1200_MDMCFG0,           0x05},
  {CC1200_SYMBOL_RATE2,      0x94},
  {CC1200_SYMBOL_RATE1,      0x7A},
  {CC1200_SYMBOL_RATE0,      0xE1},
  {CC1200_AGC_REF,           0x27},
  {CC1200_AGC_CS_THR,        0xF1},
  {CC1200_AGC_CFG1,          0x11},
  {CC1200_AGC_CFG0,          0x90},
  {CC1200_FIFO_CFG,          0x00},
  {CC1200_FS_CFG,            0x12},
  {CC1200_PKT_CFG2,          0x24},
  {CC1200_PKT_CFG0,          0x20},
  {CC1200_PKT_LEN,           0xFF},
  {CC1200_IF_MIX_CFG,        0x18},
  {CC1200_TOC_CFG,           0x03},
  {CC1200_MDMCFG2,           0x02},
  {CC1200_FREQ2,             0x56},
  {CC1200_FREQ1,             0xCC},
  {CC1200_FREQ0,             0xCC},
  {CC1200_IF_ADC1,           0xEE},
  {CC1200_IF_ADC0,           0x10},
  {CC1200_FS_DIG1,           0x04},
  {CC1200_FS_DIG0,           0x50},
  {CC1200_FS_CAL1,           0x40},
  {CC1200_FS_CAL0,           0x0E},
  {CC1200_FS_DIVTWO,         0x03},
  {CC1200_FS_DSM0,           0x33},
  {CC1200_FS_DVC1,           0xF7},
  {CC1200_FS_DVC0,           0x0F},
  {CC1200_FS_PFD,            0x00},
  {CC1200_FS_PRE,            0x6E},
  {CC1200_FS_REG_DIV_CML,    0x1C},
  {CC1200_FS_SPARE,          0xAC},
  {CC1200_FS_VCO0,           0xB5},
  {CC1200_IFAMP,             0x05},
  {CC1200_XOSC5,             0x0E},
  {CC1200_XOSC1,             0x03},
};
/*---------------------------------------------------------------------------*/
/* Global linkage: symbol name must be different in each exported file! */
const cc1200_rf_cfg_t cc1200_802154g_863_870_fsk_50kbps = {
  .cfg_descriptor = rf_cfg_descriptor,
  .register_settings = preferredSettings,
  .size_of_register_settings = sizeof(preferredSettings),
  .tx_pkt_lifetime = (RTIMER_SECOND / 20),
  .chan_center_freq0 = RF_CFG_CHAN_CENTER_F0,
  .chan_spacing = RF_CFG_CHAN_SPACING,
  .min_channel = RF_CFG_MIN_CHANNEL,
  .max_channel = RF_CFG_MAX_CHANNEL,
  .max_txpower = RF_CFG_MAX_TXPOWER,
  .cca_threshold = RF_CFG_CCA_THRESHOLD,
};
/*---------------------------------------------------------------------------*/
