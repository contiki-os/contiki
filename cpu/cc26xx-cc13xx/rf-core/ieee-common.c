/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup rf-core-ieee
 * @{
 *
 *
 * \file
 * Implementation of the CC13xx/CC26xx common ieee functions
 *
 * This file completley handles management of radio rx buffers (including
 * memory allocation), and provides some functionality related to rf-core
 * overrides, initilization of RX background commands and transmitter
 * power.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "ieee-common.h"
#include "dev/radio.h"
#include "dev/cc26xx-uart.h"
#include "dev/oscillators.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "sys/cc.h"
#include "lpm.h"
#include "ti-lib.h"
#include "rf-core/rf-core.h"
#include "rf-core/rf-ble.h"
/*---------------------------------------------------------------------------*/
/* RF core and RF HAL API */
#include "hw_rfc_dbell.h"
#include "hw_rfc_pwr.h"
/*---------------------------------------------------------------------------*/
/* RF Core Mailbox API */
#include "rf-core/api/mailbox.h"
#include "rf-core/api/common_cmd.h"
#include "rf-core/api/ieee_cmd.h"
#include "rf-core/api/data_entry.h"
#include "rf-core/api/ieee_mailbox.h"
/*---------------------------------------------------------------------------*/
#include "smartrf-settings.h"
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define STATUS_CRC_OK      0x80
#define STATUS_CORRELATION 0x7f
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current status of a running Radio Op command
 * \param a A pointer with the buffer used to initiate the command
 * \return The value of the Radio Op buffer's status field
 *
 * TODO This really belongs in some header file (maybe rf-core.h?)
 *
 * This macro can be used to e.g. return the status of a previously
 * initiated background operation, or of an immediate command
 */
#define RF_RADIO_OP_GET_STATUS(a) (((rfc_radioOp_t *)a)->status)
/*---------------------------------------------------------------------------*/
static const output_config_t output_power[] = {
  { 5, 0x30, 0x00, 0x93 },
  { 4, 0x24, 0x00, 0x93 },
  { 3, 0x1c, 0x00, 0x5a },
  { 2, 0x18, 0x00, 0x4e },
  { 1, 0x14, 0x00, 0x42 },
  { 0, 0x21, 0x01, 0x31 },
  { -3, 0x18, 0x01, 0x25 },
  { -6, 0x11, 0x01, 0x1d },
  { -9, 0x0e, 0x01, 0x19 },
  { -12, 0x0b, 0x01, 0x14 },
  { -15, 0x0b, 0x03, 0x0c },
  { -18, 0x09, 0x03, 0x0c },
  { -21, 0x07, 0x03, 0x0c },
};

#define OUTPUT_CONFIG_COUNT (sizeof(output_power) / sizeof(output_config_t))

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MIN     (output_power[OUTPUT_CONFIG_COUNT - 1].dbm)
#define OUTPUT_POWER_MAX     (output_power[0].dbm)
#define OUTPUT_POWER_UNKNOWN 0xFFFF
/*---------------------------------------------------------------------------*/
#ifdef IEEE_COMMON_CONF_APPEND_TIMESTAMP
#define IEEE_COMMON_APPEND_TIMESTAMP IEEE_COMMON_CONF_APPEND_TIMESTAMP
#else
#define IEEE_COMMON_APPEND_TIMESTAMP 1
#endif
/*---------------------------------------------------------------------------*/
/* Overrides for IEEE 802.15.4, differential mode */
static uint32_t ieee_overrides[] = {
  0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */
  0x4001402D, /* Synth: Correct CKVD latency setting (address) */
  0x00608402, /* Synth: Correct CKVD latency setting (value) */
  /* 0x4001405D, *//* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */
  /* 0x1801F800, *//* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */
  0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */
  0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */
  0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */
  0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */
  0x002B50DC, /* Adjust AGC DC filter */
  0x05000243, /* Increase synth programming timeout */
  0x002082C3, /* Increase synth programming timeout */
  0xFFFFFFFF, /* End of override list */
};
/*---------------------------------------------------------------------------*/
#define DATA_ENTRY_LENSZ_NONE 0
#define DATA_ENTRY_LENSZ_BYTE 1
#define DATA_ENTRY_LENSZ_WORD 2 /* 2 bytes */

/* Four receive buffers entries with room for 1 IEEE802.15.4 frame in each */
static uint8_t rx_buf_0[IEEE_RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_1[IEEE_RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_2[IEEE_RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_3[IEEE_RX_BUF_SIZE] CC_ALIGN(4);

/* The RX Data Queue */
static dataQueue_t rx_data_queue = { 0 };

/* Receive entry pointer to keep track of read items */
volatile static uint8_t *rx_read_entry;
/*---------------------------------------------------------------------------*/
/* RF stats data structure */
static uint8_t rf_stats[16] = { 0 };
/*---------------------------------------------------------------------------*/
const output_config_t *
ieee_common_get_power_config(radio_value_t power)
{
  int i;

  for(i = OUTPUT_CONFIG_COUNT - 1; i >= 0; --i) {
    if(power <= output_power[i].dbm) {
      return &(output_power[i]);
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
const output_config_t *
ieee_common_get_power_config_max(void)
{
  return &(output_power[0]);
}
/*---------------------------------------------------------------------------*/
const output_config_t *
ieee_common_get_power_config_min(void)
{
  return &(output_power[OUTPUT_CONFIG_COUNT - 1]);
}
/*---------------------------------------------------------------------------*/
const uint32_t *
ieee_common_get_overrides(void)
{
  return ieee_overrides;
}
/*---------------------------------------------------------------------------*/
void
ieee_common_init_rf_params(rfc_CMD_IEEE_RX_t *cmd)
{
  memset(cmd, 0x00, RF_CMD_BUFFER_SIZE);

  cmd->commandNo = CMD_IEEE_RX;
  cmd->status = RF_CORE_RADIO_OP_STATUS_IDLE;
  cmd->pNextOp = NULL;
  cmd->startTime = 0x00000000;
  cmd->startTrigger.triggerType = TRIG_NOW;
  cmd->condition.rule = COND_NEVER;
  cmd->channel = RF_CORE_CHANNEL;

  cmd->rxConfig.bAutoFlushCrc = 1;
  cmd->rxConfig.bAutoFlushIgn = 0;
  cmd->rxConfig.bIncludePhyHdr = 0;
  cmd->rxConfig.bIncludeCrc = 1;
  cmd->rxConfig.bAppendRssi = 1;
  cmd->rxConfig.bAppendCorrCrc = 1;
  cmd->rxConfig.bAppendSrcInd = 0;
  cmd->rxConfig.bAppendTimestamp = IEEE_COMMON_APPEND_TIMESTAMP;

  cmd->pRxQ = &rx_data_queue;
  cmd->pOutput = (rfc_ieeeRxOutput_t *)rf_stats;

#if IEEE_MODE_PROMISCOUS
  cmd->frameFiltOpt.frameFiltEn = 0;
#else
  cmd->frameFiltOpt.frameFiltEn = 1;
#endif

  cmd->frameFiltOpt.frameFiltStop = 1;

#if IEEE_MODE_AUTOACK
  cmd->frameFiltOpt.autoAckEn = 1;
#else
  cmd->frameFiltOpt.autoAckEn = 0;
#endif

  cmd->frameFiltOpt.slottedAckEn = 0;
  cmd->frameFiltOpt.autoPendEn = 0;
  cmd->frameFiltOpt.defaultPend = 0;
  cmd->frameFiltOpt.bPendDataReqOnly = 0;
  cmd->frameFiltOpt.bPanCoord = 0;
  cmd->frameFiltOpt.maxFrameVersion = 2;
  cmd->frameFiltOpt.bStrictLenFilter = 0;

  /* Receive all frame types */
  cmd->frameTypes.bAcceptFt0Beacon = 1;
  cmd->frameTypes.bAcceptFt1Data = 1;
  cmd->frameTypes.bAcceptFt2Ack = 1;
  cmd->frameTypes.bAcceptFt3MacCmd = 1;
  cmd->frameTypes.bAcceptFt4Reserved = 1;
  cmd->frameTypes.bAcceptFt5Reserved = 1;
  cmd->frameTypes.bAcceptFt6Reserved = 1;
  cmd->frameTypes.bAcceptFt7Reserved = 1;

  /* Configure CCA settings */
  cmd->ccaOpt.ccaEnEnergy = 1;
  cmd->ccaOpt.ccaEnCorr = 1;
  cmd->ccaOpt.ccaEnSync = 1;
  cmd->ccaOpt.ccaCorrOp = 1;
  cmd->ccaOpt.ccaSyncOp = 0;
  cmd->ccaOpt.ccaCorrThr = 3;

  cmd->ccaRssiThr = IEEE_MODE_RSSI_THRESHOLD;

  cmd->numExtEntries = 0x00;
  cmd->numShortEntries = 0x00;
  cmd->pExtEntryList = 0;
  cmd->pShortEntryList = 0;

  cmd->endTrigger.triggerType = TRIG_NEVER;
  cmd->endTime = 0x00000000;
}
/*---------------------------------------------------------------------------*/
uint8_t
ieee_common_rf_is_on(rfc_CMD_IEEE_RX_t *cmd)
{
  if(!rf_core_is_accessible()) {
    return 0;
  }

  return RF_RADIO_OP_GET_STATUS(cmd) == RF_CORE_RADIO_OP_STATUS_ACTIVE;
}
/*---------------------------------------------------------------------------*/
void
ieee_common_init_rx_buffers(void)
{
  rfc_dataEntry_t *entry;

  entry = (rfc_dataEntry_t *)rx_buf_0;
  entry->pNextEntry = rx_buf_1;
  entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
  entry->length = sizeof(rx_buf_0) - 8;

  entry = (rfc_dataEntry_t *)rx_buf_1;
  entry->pNextEntry = rx_buf_2;
  entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
  entry->length = sizeof(rx_buf_0) - 8;

  entry = (rfc_dataEntry_t *)rx_buf_2;
  entry->pNextEntry = rx_buf_3;
  entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
  entry->length = sizeof(rx_buf_0) - 8;

  entry = (rfc_dataEntry_t *)rx_buf_3;
  entry->pNextEntry = rx_buf_0;
  entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
  entry->length = sizeof(rx_buf_0) - 8;
}
/*---------------------------------------------------------------------------*/
void
ieee_common_set_rx_buffers_pending(void)
{
  ((rfc_dataEntry_t *)rx_buf_0)->status = DATA_ENTRY_STATUS_PENDING;
  ((rfc_dataEntry_t *)rx_buf_1)->status = DATA_ENTRY_STATUS_PENDING;
  ((rfc_dataEntry_t *)rx_buf_2)->status = DATA_ENTRY_STATUS_PENDING;
  ((rfc_dataEntry_t *)rx_buf_3)->status = DATA_ENTRY_STATUS_PENDING;
}
/*---------------------------------------------------------------------------*/
void
ieee_common_set_rx_pending_if_busy(void)
{
  if(((rfc_dataEntry_t *)rx_buf_0)->status == DATA_ENTRY_STATUS_BUSY) {
    ((rfc_dataEntry_t *)rx_buf_0)->status = DATA_ENTRY_STATUS_PENDING;
  }
  if(((rfc_dataEntry_t *)rx_buf_1)->status == DATA_ENTRY_STATUS_BUSY) {
    ((rfc_dataEntry_t *)rx_buf_1)->status = DATA_ENTRY_STATUS_PENDING;
  }
  if(((rfc_dataEntry_t *)rx_buf_2)->status == DATA_ENTRY_STATUS_BUSY) {
    ((rfc_dataEntry_t *)rx_buf_2)->status = DATA_ENTRY_STATUS_PENDING;
  }
  if(((rfc_dataEntry_t *)rx_buf_3)->status == DATA_ENTRY_STATUS_BUSY) {
    ((rfc_dataEntry_t *)rx_buf_3)->status = DATA_ENTRY_STATUS_PENDING;
  }
}
/*---------------------------------------------------------------------------*/
static void
release_data_entry(void)
{
  rfc_dataEntryGeneral_t *entry = (rfc_dataEntryGeneral_t *)rx_read_entry;

  /* Clear the length byte */
  rx_read_entry[8] = 0;

  /* Set status to 0 "Pending" in element */
  entry->status = DATA_ENTRY_STATUS_PENDING;
  rx_read_entry = entry->pNextEntry;
}
/*---------------------------------------------------------------------------*/
int
ieee_common_read_frame(void *buf, uint16_t buf_len, int8_t *rssi,
                       uint8_t *corr_lqi, uint32_t *timestamp)
{
  int len = 0;
  rfc_dataEntryGeneral_t *entry = (rfc_dataEntryGeneral_t *)rx_read_entry;

  if(entry->status != DATA_ENTRY_STATUS_FINISHED) {
    /* No available data */
    return 0;
  }

  if(rx_read_entry[8] < 4) {
    PRINTF("RF: too short\n");
    RIMESTATS_ADD(tooshort);

    release_data_entry();
    return 0;
  }

  if(IEEE_COMMON_APPEND_TIMESTAMP) {
  	len = rx_read_entry[8] - 8;
  } else {
  	len = rx_read_entry[8] - 4;
  }

  if(len > buf_len) {
    PRINTF("RF: too long\n");
    RIMESTATS_ADD(toolong);

    release_data_entry();
    return 0;
  }

  memcpy(buf, (char *)&rx_read_entry[9], len);

  *rssi = (int8_t)rx_read_entry[9 + len + 2];
  *corr_lqi = (uint8_t)rx_read_entry[9 + len + 2] & STATUS_CORRELATION;

  if(IEEE_COMMON_APPEND_TIMESTAMP) {
  	memcpy(timestamp, (char *)rx_read_entry + 9 + len + 4, 4);
  }

  release_data_entry();

  return len;
}
/*---------------------------------------------------------------------------*/
int ieee_common_frame_wait(rtimer_clock_t max_wait)
{
  rfc_dataEntryGeneral_t *entry = (rfc_dataEntryGeneral_t *)rx_read_entry;
  rtimer_clock_t t0 = RTIMER_NOW();

  while(entry->status == DATA_ENTRY_STATUS_BUSY
        && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + max_wait));

  return entry->status == DATA_ENTRY_STATUS_FINISHED;
}
/*---------------------------------------------------------------------------*/
int
ieee_common_pending_packet(void)
{
  volatile rfc_dataEntry_t *entry = (rfc_dataEntry_t *)rx_data_queue.pCurrEntry;
  int rv = 0;

  /* Go through all RX buffers and check their status */
  do {
    if(entry->status == DATA_ENTRY_STATUS_FINISHED) {
      rv = 1;
      process_poll(&rf_core_process);
    }

    entry = (rfc_dataEntry_t *)entry->pNextEntry;
  } while(entry != (rfc_dataEntry_t *)rx_data_queue.pCurrEntry);

  /* If we didn't find an entry at status finished, no frames are pending */
  return rv;
}
/*---------------------------------------------------------------------------*/
int
ieee_common_incoming_packet(void)
{
  volatile rfc_dataEntry_t *entry = (rfc_dataEntry_t *)rx_data_queue.pCurrEntry;
  uint8_t status;

  /* Go through all RX buffers and check their status */
  do {
  	status = entry->status;
  	if(status == DATA_ENTRY_STATUS_BUSY) {
  		return 1;
  	}
    if(status == DATA_ENTRY_STATUS_FINISHED) {
    	return 1;
    }

    entry = (rfc_dataEntry_t *)entry->pNextEntry;
  } while(entry != (rfc_dataEntry_t *)rx_data_queue.pCurrEntry);

  return 0;
}
/*---------------------------------------------------------------------------*/
void
ieee_common_init_data_queue(void)
{
  memset(rx_buf_0, 0, IEEE_RX_BUF_SIZE);
  memset(rx_buf_1, 0, IEEE_RX_BUF_SIZE);
  memset(rx_buf_2, 0, IEEE_RX_BUF_SIZE);
  memset(rx_buf_3, 0, IEEE_RX_BUF_SIZE);

  /* Set of RF Core data queue. Circular buffer, no last entry */
  rx_data_queue.pCurrEntry = rx_buf_0;

  rx_data_queue.pLastEntry = NULL;

  /* Initialize current read pointer to first element (used in ISR) */
  rx_read_entry = rx_buf_0;

  /* Populate the RF parameters data structure with default values */
  ieee_common_init_rx_buffers();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */