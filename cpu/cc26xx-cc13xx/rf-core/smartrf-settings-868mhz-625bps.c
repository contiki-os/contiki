/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"

#include "rf-core/dot-15-4g.h"
#include "driverlib/rf_mailbox.h"
#include "driverlib/rf_common_cmd.h"
#include "driverlib/rf_prop_cmd.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#ifdef SMARTRF_SETTINGS_CONF_BOARD_OVERRIDES
#define SMARTRF_SETTINGS_BOARD_OVERRIDES SMARTRF_SETTINGS_CONF_BOARD_OVERRIDES
#else
#define SMARTRF_SETTINGS_BOARD_OVERRIDES
#endif
/*---------------------------------------------------------------------------*/
#ifdef SMARTRF_SETTINGS_CONF_BAND_OVERRIDES
#define SMARTRF_SETTINGS_BAND_OVERRIDES SMARTRF_SETTINGS_CONF_BAND_OVERRIDES
#else
#define SMARTRF_SETTINGS_BAND_OVERRIDES
#endif
/*---------------------------------------------------------------------------*/
/* RSSI offset configuration for the 431-527MHz band */
#ifdef SMARTRF_SETTINGS_CONF_RSSI_OFFSET_431_527
#define SMARTRF_SETTINGS_RSSI_OFFSET_431_527 SMARTRF_SETTINGS_CONF_RSSI_OFFSET_431_527
#else
#define SMARTRF_SETTINGS_RSSI_OFFSET_431_527 0x000288A3
#endif
/*---------------------------------------------------------------------------*/
/* RSSI offset configuration for the 779-930MHz band */
#ifdef SMARTRF_SETTINGS_CONF_RSSI_OFFSET_779_930
#define SMARTRF_SETTINGS_RSSI_OFFSET_779_930 SMARTRF_SETTINGS_CONF_RSSI_OFFSET_779_930
#else
#define SMARTRF_SETTINGS_RSSI_OFFSET_779_930 0x00FB88A3
#endif
/*---------------------------------------------------------------------------*/
#ifdef SMARTRF_SETTINGS_CONF_OVERRIDE_TRIM_OFFSET
#define SMARTRF_SETTINGS_OVERRIDE_TRIM_OFFSET SMARTRF_SETTINGS_CONF_OVERRIDE_TRIM_OFFSET
#else
#define SMARTRF_SETTINGS_OVERRIDE_TRIM_OFFSET 0x00038883
#endif
/*---------------------------------------------------------------------------*/
/* Select RSSI offset value based on the frequency band */
#if DOT_15_4G_FREQUENCY_BAND_ID == DOT_15_4G_FREQUENCY_BAND_470
#define RSSI_OFFSET SMARTRF_SETTINGS_RSSI_OFFSET_431_527
#else
#define RSSI_OFFSET SMARTRF_SETTINGS_RSSI_OFFSET_779_930
#endif
/*---------------------------------------------------------------------------*/
/* Overrides for CMD_PROP_RADIO_DIV_SETUP */
static uint32_t overrides[] =
{
  /* override_use_patch_prop_lrm.xml */
  /* PHY: Use MCE ROM bank 3, RFE RAM patch */
  MCE_RFE_OVERRIDE(0, 3, 0, 1, 0, 0),
  /* override_synth_prop_863_930_div5.xml */
  /* Synth: Set recommended RTRIM to 7 */
  HW_REG_OVERRIDE(0x4038, 0x0037),
  /* Synth: Set Fref to 4 MHz */
  (uint32_t)0x000684A3,
  /* Synth: Configure fine calibration setting */
  HW_REG_OVERRIDE(0x4020, 0x7F00),
  /* Synth: Configure fine calibration setting */
  HW_REG_OVERRIDE(0x4064, 0x0040),
  /* Synth: Configure fine calibration setting */
  (uint32_t)0xB1070503,
  /* Synth: Configure fine calibration setting */
  (uint32_t)0x05330523,
  /* Synth: Set loop bandwidth after lock to 20 kHz */
  (uint32_t)0x0A480583,
  /* Synth: Set loop bandwidth after lock to 20 kHz */
  (uint32_t)0x7AB80603,
  /* Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference) */
  ADI_REG_OVERRIDE(1, 4, 0x9F),
  /* Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1) */
  ADI_HALFREG_OVERRIDE(1, 7, 0x4, 0x4),
  /* Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering */
  (uint32_t)0x02010403,
  /* Synth: Configure extra PLL filtering */
  (uint32_t)0x00108463,
  /* Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us) */
  (uint32_t)0x04B00243,
  /* override_phy_rx_aaf_bw_0xd.xml */
  /* Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD) */
  ADI_HALFREG_OVERRIDE(0, 61, 0xF, 0xD),
  /* override_phy_gfsk_rx.xml */
  /* Rx: Set LNA bias current trim offset to 3 */
  (uint32_t)0x00038883,
  /* Rx: Freeze RSSI on sync found event */
  HW_REG_OVERRIDE(0x6084, 0x35F1),
  /* override_phy_gfsk_pa_ramp_agc_reflevel_0x1a.xml */
  /* Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x1A. */
  HW_REG_OVERRIDE(0x6088, 0x411A),
  /* Tx: Configure PA ramping setting */
  HW_REG_OVERRIDE(0x608C, 0x8213),
  /* override_phy_lrm_rom_dsss8.xml */
  /* PHY: Configure DSSS=8 */
  HW_REG_OVERRIDE(0x505C, 0x073C),
  /* override_phy_rx_rssi_offset_5db.xml */
  /* Rx: Set RSSI offset to adjust reported RSSI by +5 dB */
  (uint32_t)0x00FB88A3,
  /* TX power override */
  /* Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8) */
  ADI_REG_OVERRIDE(0, 12, 0xF8),
  (uint32_t)0xFFFFFFFF,
};
/*---------------------------------------------------------------------------*/
/* CMD_PROP_RADIO_DIV_SETUP */
rfc_CMD_PROP_RADIO_DIV_SETUP_t smartrf_settings_cmd_prop_radio_div_setup =
{
  .commandNo = 0x3807,
  .status = 0x0000,
  .pNextOp = 0,   /* INSERT APPLICABLE POINTER: (uint8_t*)&xxx */
  .startTime = 0x00000000,
  .startTrigger.triggerType = 0x0,
  .startTrigger.bEnaCmd = 0x0,
  .startTrigger.triggerNo = 0x0,
  .startTrigger.pastTrig = 0x0,
  .condition.rule = 0x1,
  .condition.nSkip = 0x0,
  .modulation.modType = 0x0,
  .modulation.deviation = 0x14,
  .symbolRate.preScale = 0xF,
  .symbolRate.rateWord = 0x199A,
  .rxBw = 0x20,
  .preamConf.nPreamBytes = 0x3,
  .preamConf.preamMode = 0x0,
  .formatConf.nSwBits = 0x18,
  .formatConf.bBitReversal = 0x0,
  .formatConf.bMsbFirst = 0x0,
  .formatConf.fecMode = 0x8,
  .formatConf.whitenMode = 0x7,
  .config.frontEndMode = 0x0,
  .config.biasMode = 0x1,
  .config.analogCfgMode = 0x0,
  .config.bNoFsPowerUp = 0x0,
  .txPower = 0xA73F,
  .pRegOverride = overrides,
  .centerFreq = 0x0364,
  .intFreq = 0x8000,
  .loDivider = 0x05,
};
/*---------------------------------------------------------------------------*/
/* CMD_FS */
rfc_CMD_FS_t smartrf_settings_cmd_fs =
{
  .commandNo = 0x0803,
  .status = 0x0000,
  .pNextOp = 0,   /* INSERT APPLICABLE POINTER: (uint8_t*)&xxx */
  .startTime = 0x00000000,
  .startTrigger.triggerType = 0x0,
  .startTrigger.bEnaCmd = 0x0,
  .startTrigger.triggerNo = 0x0,
  .startTrigger.pastTrig = 0x0,
  .condition.rule = 0x1,
  .condition.nSkip = 0x0,
  .frequency = 0x0364,
  .fractFreq = 0x0000,
  .synthConf.bTxMode = 0x0,
  .synthConf.refFreq = 0x0,
  .__dummy0 = 0x00,
  .__dummy1 = 0x00,
  .__dummy2 = 0x00,
  .__dummy3 = 0x0000,
};
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* CMD_PROP_TX_ADV */
rfc_CMD_PROP_TX_ADV_t smartrf_settings_cmd_prop_tx_adv =
{
  .commandNo = 0x3803,
  .status = 0x0000,
  .pNextOp = 0,
  .startTime = 0x00000000,
  .startTrigger.triggerType = 0x0,
  .startTrigger.bEnaCmd = 0x0,
  .startTrigger.triggerNo = 0x0,
  .startTrigger.pastTrig = 0x0,
  .condition.rule = 0x1,
  .condition.nSkip = 0x0,
  .pktConf.bFsOff = 0x0,
  .pktConf.bUseCrc = 0x1,
  .pktConf.bCrcIncSw = 0x0, /* .4g mode */
  .pktConf.bCrcIncHdr = 0x0, /* .4g mode */
  .numHdrBits = 0x10 /* 16: .4g mode */,
  .pktLen = 0x0000,
  .startConf.bExtTxTrig = 0x0,
  .startConf.inputMode = 0x0,
  .startConf.source = 0x0,
  .preTrigger.triggerType = TRIG_REL_START,
  .preTrigger.bEnaCmd = 0x0,
  .preTrigger.triggerNo = 0x0,
  .preTrigger.pastTrig = 0x1,
  .preTime = 0x00000000,
  .syncWord = 0x0055904e,
  .pPkt = 0,
};
/*---------------------------------------------------------------------------*/
/* CMD_PROP_RX_ADV */
rfc_CMD_PROP_RX_ADV_t smartrf_settings_cmd_prop_rx_adv =
{
  .commandNo = 0x3804,
  .status = 0x0000,
  .pNextOp = 0,
  .startTime = 0x00000000,
  .startTrigger.triggerType = 0x0,
  .startTrigger.bEnaCmd = 0x0,
  .startTrigger.triggerNo = 0x0,
  .startTrigger.pastTrig = 0x0,
  .condition.rule = 0x1,
  .condition.nSkip = 0x0,
  .pktConf.bFsOff = 0x0,
  .pktConf.bRepeatOk = 0x1,
  .pktConf.bRepeatNok = 0x1,
  .pktConf.bUseCrc = 0x1,
  .pktConf.bCrcIncSw = 0x0, /* .4g mode */
  .pktConf.bCrcIncHdr = 0x0, /* .4g mode */
  .pktConf.endType = 0x0,
  .pktConf.filterOp = 0x1,
  .rxConf.bAutoFlushIgnored = 0x1,
  .rxConf.bAutoFlushCrcErr = 0x1,
  .rxConf.bIncludeHdr = 0x0,
  .rxConf.bIncludeCrc = 0x0,
  .rxConf.bAppendRssi = 0x1,
  .rxConf.bAppendTimestamp = 0x0,
  .rxConf.bAppendStatus = 0x1,
  .syncWord0 = 0x0055904e,
  .syncWord1 = 0x00000000,
  .maxPktLen = 0x0000, /* To be populated by the driver. */
  .hdrConf.numHdrBits = 0x10, /* 16: .4g mode */
  .hdrConf.lenPos = 0x0, /* .4g mode */
  .hdrConf.numLenBits = 0x0B, /* 11 = 0x0B .4g mode */
  .addrConf.addrType = 0x0,
  .addrConf.addrSize = 0x0,
  .addrConf.addrPos = 0x0,
  .addrConf.numAddr = 0x0,
  .lenOffset = -4, /* .4g mode */
  .endTrigger.triggerType = TRIG_NEVER,
  .endTrigger.bEnaCmd = 0x0,
  .endTrigger.triggerNo = 0x0,
  .endTrigger.pastTrig = 0x0,
  .endTime = 0x00000000,
  .pAddr = 0,
  .pQueue = 0,
  .pOutput = 0,
};