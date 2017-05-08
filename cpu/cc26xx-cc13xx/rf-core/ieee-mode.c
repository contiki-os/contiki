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
 * \addtogroup rf-core
 * @{
 *
 * \defgroup rf-core-ieee CC13xx/CC26xx IEEE mode driver
 *
 * @{
 *
 * \file
 * Implementation of the CC13xx/CC26xx IEEE mode NETSTACK_RADIO driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
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
#include "sys/ctimer.h"
#include "sys/cc.h"
#include "lpm.h"
#include "ti-lib.h"
#include "rf-core/rf-core.h"
#include "rf-core/rf-switch.h"
#include "rf-core/rf-ble.h"
/*---------------------------------------------------------------------------*/
/* RF core and RF HAL API */
#include "hw_rfc_dbell.h"
#include "hw_rfc_pwr.h"
/*---------------------------------------------------------------------------*/
/* RF Core Mailbox API */
#include "rf-core/api/ieee_cmd.h"
#include "rf-core/api/ieee_mailbox.h"
#include "driverlib/rf_mailbox.h"
#include "driverlib/rf_common_cmd.h"
#include "driverlib/rf_data_entry.h"
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

/* Configuration to enable/disable auto ACKs in IEEE mode */
#ifdef IEEE_MODE_CONF_AUTOACK
#define IEEE_MODE_AUTOACK IEEE_MODE_CONF_AUTOACK
#else
#define IEEE_MODE_AUTOACK 1
#endif /* IEEE_MODE_CONF_AUTOACK */

/* Configuration to enable/disable frame filtering in IEEE mode */
#ifdef IEEE_MODE_CONF_PROMISCOUS
#define IEEE_MODE_PROMISCOUS IEEE_MODE_CONF_PROMISCOUS
#else
#define IEEE_MODE_PROMISCOUS 0
#endif /* IEEE_MODE_CONF_PROMISCOUS */

#ifdef IEEE_MODE_CONF_RSSI_THRESHOLD
#define IEEE_MODE_RSSI_THRESHOLD IEEE_MODE_CONF_RSSI_THRESHOLD
#else
#define IEEE_MODE_RSSI_THRESHOLD 0xA6
#endif /* IEEE_MODE_CONF_RSSI_THRESHOLD */
/*---------------------------------------------------------------------------*/
#define STATUS_CRC_FAIL     0x80 /* bit 7 */
#define STATUS_REJECT_FRAME 0x40 /* bit 6 */
#define STATUS_CORRELATION  0x3f /* bits 0-5 */
/*---------------------------------------------------------------------------*/
/* Data entry status field constants */
#define DATA_ENTRY_STATUS_PENDING    0x00 /* Not in use by the Radio CPU */
#define DATA_ENTRY_STATUS_ACTIVE     0x01 /* Open for r/w by the radio CPU */
#define DATA_ENTRY_STATUS_BUSY       0x02 /* Ongoing r/w */
#define DATA_ENTRY_STATUS_FINISHED   0x03 /* Free to use and to free */
#define DATA_ENTRY_STATUS_UNFINISHED 0x04 /* Partial RX entry */
/*---------------------------------------------------------------------------*/
/* RF stats data structure */
static uint8_t rf_stats[16] = { 0 };
/*---------------------------------------------------------------------------*/
/* The size of the RF commands buffer */
#define RF_CMD_BUFFER_SIZE             128
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current status of a running Radio Op command
 * \param a A pointer with the buffer used to initiate the command
 * \return The value of the Radio Op buffer's status field
 *
 * This macro can be used to e.g. return the status of a previously
 * initiated background operation, or of an immediate command
 */
#define RF_RADIO_OP_GET_STATUS(a) (((rfc_radioOp_t *)a)->status)
/*---------------------------------------------------------------------------*/
/* Special value returned by CMD_IEEE_CCA_REQ when an RSSI is not available */
#define RF_CMD_CCA_REQ_RSSI_UNKNOWN     -128

/* Used for the return value of channel_clear */
#define RF_CCA_CLEAR                       1
#define RF_CCA_BUSY                        0

/* Used as an error return value for get_cca_info */
#define RF_GET_CCA_INFO_ERROR           0xFF

/*
 * Values of the individual bits of the ccaInfo field in CMD_IEEE_CCA_REQ's
 * status struct
 */
#define RF_CMD_CCA_REQ_CCA_STATE_IDLE      0 /* 00 */
#define RF_CMD_CCA_REQ_CCA_STATE_BUSY      1 /* 01 */
#define RF_CMD_CCA_REQ_CCA_STATE_INVALID   2 /* 10 */

#define RF_CMD_CCA_REQ_CCA_CORR_IDLE       (0 << 4)
#define RF_CMD_CCA_REQ_CCA_CORR_BUSY       (1 << 4)
#define RF_CMD_CCA_REQ_CCA_CORR_INVALID    (3 << 4)
#define RF_CMD_CCA_REQ_CCA_CORR_MASK       (3 << 4)

#define RF_CMD_CCA_REQ_CCA_SYNC_BUSY       (1 << 6)
/*---------------------------------------------------------------------------*/
#define IEEE_MODE_CHANNEL_MIN            11
#define IEEE_MODE_CHANNEL_MAX            26
/*---------------------------------------------------------------------------*/
/* How long to wait for an ongoing ACK TX to finish before starting frame TX */
#define TX_WAIT_TIMEOUT       (RTIMER_SECOND >> 11)

/* How long to wait for the RF to enter RX in rf_cmd_ieee_rx */
#define ENTER_RX_WAIT_TIMEOUT (RTIMER_SECOND >> 10)

/* How long to wait for the RF to react on CMD_ABORT: around 1 msec */
#define RF_TURN_OFF_WAIT_TIMEOUT (RTIMER_SECOND >> 10)

#define LIMITED_BUSYWAIT(cond, timeout) do {                         \
    rtimer_clock_t end_time = RTIMER_NOW() + timeout;                \
    while(cond) {                                                    \
      if(!RTIMER_CLOCK_LT(RTIMER_NOW(), end_time)) {                 \
        break;                                                       \
      }                                                              \
    }                                                                \
  } while(0)
/*---------------------------------------------------------------------------*/
/* TX Power dBm lookup table - values from SmartRF Studio */
typedef struct output_config {
  radio_value_t dbm;
  uint16_t tx_power; /* Value for the CMD_RADIO_SETUP.txPower field */
} output_config_t;

static const output_config_t output_power[] = {
  {  5, 0x9330 },
  {  4, 0x9324 },
  {  3, 0x5a1c },
  {  2, 0x4e18 },
  {  1, 0x4214 },
  {  0, 0x3161 },
  { -3, 0x2558 },
  { -6, 0x1d52 },
  { -9, 0x194e },
  {-12, 0x144b },
  {-15, 0x0ccb },
  {-18, 0x0cc9 },
  {-21, 0x0cc7 },
};

#define OUTPUT_CONFIG_COUNT (sizeof(output_power) / sizeof(output_config_t))

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MIN     (output_power[OUTPUT_CONFIG_COUNT - 1].dbm)
#define OUTPUT_POWER_MAX     (output_power[0].dbm)
#define OUTPUT_POWER_UNKNOWN 0xFFFF

/* Default TX Power - position in output_power[] */
const output_config_t *tx_power_current = &output_power[0];
/*---------------------------------------------------------------------------*/
static volatile int8_t last_rssi = 0;
static volatile uint8_t last_corr_lqi = 0;

extern int32_t rat_offset;

/*---------------------------------------------------------------------------*/
/* SFD timestamp in RTIMER ticks */
static volatile uint32_t last_packet_timestamp = 0;
/* SFD timestamp in RAT ticks (but 64 bits) */
static uint64_t last_rat_timestamp64 = 0;

/* For RAT overflow handling */
static struct ctimer rat_overflow_timer;
static volatile uint32_t rat_overflow_counter = 0;
static rtimer_clock_t last_rat_overflow = 0;

/* RAT has 32-bit register, overflows once 18 minutes */
#define RAT_RANGE  4294967296ull
/* approximate value */
#define RAT_OVERFLOW_PERIOD_SECONDS (60 * 18)

/* XXX: don't know what exactly is this, looks like the time to Tx 3 octets */
#define TIMESTAMP_OFFSET  -(USEC_TO_RADIO(32 * 3) - 1) /* -95.75 usec */
/*---------------------------------------------------------------------------*/
/* Are we currently in poll mode? */
static uint8_t poll_mode = 0;

static rfc_CMD_IEEE_MOD_FILT_t filter_cmd;
/*---------------------------------------------------------------------------*/
/*
 * Buffers used to send commands to the RF core (generic and IEEE commands).
 * Some of those buffers are re-usable, some are not.
 *
 * If you are uncertain, declare a new buffer.
 */
/*
 * A buffer to send a CMD_IEEE_RX and to subsequently monitor its status
 * Do not use this buffer for any commands other than CMD_IEEE_RX
 */
static uint8_t cmd_ieee_rx_buf[RF_CMD_BUFFER_SIZE] CC_ALIGN(4);
/*---------------------------------------------------------------------------*/
#define DATA_ENTRY_LENSZ_NONE 0
#define DATA_ENTRY_LENSZ_BYTE 1
#define DATA_ENTRY_LENSZ_WORD 2 /* 2 bytes */

#define RX_BUF_SIZE 144
/* Four receive buffers entries with room for 1 IEEE802.15.4 frame in each */
static uint8_t rx_buf_0[RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_1[RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_2[RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_3[RX_BUF_SIZE] CC_ALIGN(4);

/* The RX Data Queue */
static dataQueue_t rx_data_queue = { 0 };

/* Receive entry pointer to keep track of read items */
volatile static uint8_t *rx_read_entry;
/*---------------------------------------------------------------------------*/
/* The outgoing frame buffer */
#define TX_BUF_PAYLOAD_LEN 180
#define TX_BUF_HDR_LEN       2

static uint8_t tx_buf[TX_BUF_HDR_LEN + TX_BUF_PAYLOAD_LEN] CC_ALIGN(4);
/*---------------------------------------------------------------------------*/
#ifdef IEEE_MODE_CONF_BOARD_OVERRIDES
#define IEEE_MODE_BOARD_OVERRIDES IEEE_MODE_CONF_BOARD_OVERRIDES
#else
#define IEEE_MODE_BOARD_OVERRIDES
#endif
/*---------------------------------------------------------------------------*/
/* Overrides for IEEE 802.15.4, differential mode */
static uint32_t ieee_overrides[] = {
  0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */
  0x4001402D, /* Synth: Correct CKVD latency setting (address) */
  0x00608402, /* Synth: Correct CKVD latency setting (value) */
//  0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */
//  0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */
  0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */
  0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */
  0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */
  0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */
  0x002B50DC, /* Adjust AGC DC filter */
  0x05000243, /* Increase synth programming timeout */
  0x002082C3, /* Increase synth programming timeout */
  IEEE_MODE_BOARD_OVERRIDES
  0xFFFFFFFF, /* End of override list */
};
/*---------------------------------------------------------------------------*/
static int on(void);
static int off(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Checks whether the RFC domain is accessible and the RFC is in IEEE RX
 * \return 1: RFC in RX mode (and therefore accessible too). 0 otherwise
 */
static uint8_t
rf_is_on(void)
{
  if(!rf_core_is_accessible()) {
    return 0;
  }

  return RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) == RF_CORE_RADIO_OP_STATUS_ACTIVE;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Check the RF's TX status
 * \return 1 RF is transmitting
 * \return 0 RF is not transmitting
 *
 * TX mode may be triggered either by a CMD_IEEE_TX or by the automatic
 * transmission of an ACK frame.
 */
static uint8_t
transmitting(void)
{
  uint32_t cmd_status;
  rfc_CMD_IEEE_CCA_REQ_t cmd;

  /* If we are off, we are not in TX */
  if(!rf_core_is_accessible()) {
    return 0;
  }

  memset(&cmd, 0x00, sizeof(cmd));

  cmd.commandNo = CMD_IEEE_CCA_REQ;

  if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) == RF_CORE_CMD_ERROR) {
    PRINTF("transmitting: CMDSTA=0x%08lx\n", cmd_status);
    return 0;
  }

  if((cmd.currentRssi == RF_CMD_CCA_REQ_RSSI_UNKNOWN) &&
     (cmd.ccaInfo.ccaEnergy == RF_CMD_CCA_REQ_CCA_STATE_BUSY)) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns CCA information
 * \return RF_GET_CCA_INFO_ERROR if the RF was not on
 * \return On success, the return value is formatted as per the ccaInfo field
 *         of CMD_IEEE_CCA_REQ
 *
 * It is the caller's responsibility to make sure the RF is on. This function
 * will return RF_GET_CCA_INFO_ERROR if the RF is off
 *
 * This function will in fact wait for a valid CCA state
 */
static uint8_t
get_cca_info(void)
{
  uint32_t cmd_status;
  rfc_CMD_IEEE_CCA_REQ_t cmd;

  if(!rf_is_on()) {
    PRINTF("get_cca_info: Not on\n");
    return RF_GET_CCA_INFO_ERROR;
  }

  memset(&cmd, 0x00, sizeof(cmd));
  cmd.ccaInfo.ccaState = RF_CMD_CCA_REQ_CCA_STATE_INVALID;

  while(cmd.ccaInfo.ccaState == RF_CMD_CCA_REQ_CCA_STATE_INVALID) {
    memset(&cmd, 0x00, sizeof(cmd));
    cmd.commandNo = CMD_IEEE_CCA_REQ;

    if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) == RF_CORE_CMD_ERROR) {
      PRINTF("get_cca_info: CMDSTA=0x%08lx\n", cmd_status);

      return RF_GET_CCA_INFO_ERROR;
    }
  }

  /* We have a valid CCA state. Return the CCA Info */
  return *((uint8_t *)&cmd.ccaInfo);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Reads the current signal strength (RSSI)
 * \return The current RSSI in dBm or CMD_GET_RSSI_UNKNOWN
 *
 * This function reads the current RSSI on the currently configured
 * channel.
 */
static radio_value_t
get_rssi(void)
{
  uint32_t cmd_status;
  uint8_t was_off = 0;
  rfc_CMD_IEEE_CCA_REQ_t cmd;

  /* If we are off, turn on first */
  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("get_rssi: on() failed\n");
      return RF_CMD_CCA_REQ_RSSI_UNKNOWN;
    }
  }

  memset(&cmd, 0x00, sizeof(cmd));
  cmd.ccaInfo.ccaEnergy = RF_CMD_CCA_REQ_CCA_STATE_INVALID;

  while(cmd.ccaInfo.ccaEnergy == RF_CMD_CCA_REQ_CCA_STATE_INVALID) {
    memset(&cmd, 0x00, sizeof(cmd));
    cmd.commandNo = CMD_IEEE_CCA_REQ;

    if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) == RF_CORE_CMD_ERROR) {
      PRINTF("get_rssi: CMDSTA=0x%08lx\n", cmd_status);

      /* Make sure to return RSSI unknown */
      cmd.currentRssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;
      break;
    }
  }

  /* If we were off, turn back off */
  if(was_off) {
    off();
  }

  return cmd.currentRssi;
}
/*---------------------------------------------------------------------------*/
/* Returns the current TX power in dBm */
static radio_value_t
get_tx_power(void)
{
  return tx_power_current->dbm;
}
/*---------------------------------------------------------------------------*/
/*
 * Set TX power to 'at least' power dBm
 * This works with a lookup table. If the value of 'power' does not exist in
 * the lookup table, TXPOWER will be set to the immediately higher available
 * value
 */
static void
set_tx_power(radio_value_t power)
{
  uint32_t cmd_status;
  int i;
  rfc_CMD_SET_TX_POWER_t cmd;

  /* First, find the correct setting and save it */
  for(i = OUTPUT_CONFIG_COUNT - 1; i >= 0; --i) {
    if(power <= output_power[i].dbm) {
      tx_power_current = &output_power[i];
      break;
    }
  }

  /*
   * If the core is not accessible, the new setting will be applied next
   * time we send CMD_RADIO_SETUP, so we don't need to do anything further.
   * If the core is accessible, we can apply the new setting immediately with
   * CMD_SET_TX_POWER
   */
  if(rf_core_is_accessible() == RF_CORE_NOT_ACCESSIBLE) {
    return;
  }

  memset(&cmd, 0x00, sizeof(cmd));
  cmd.commandNo = CMD_SET_TX_POWER;
  cmd.txPower = output_power[i].tx_power;

  if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) == RF_CORE_CMD_ERROR) {
    PRINTF("set_tx_power: CMDSTA=0x%08lx\n", cmd_status);
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
rf_radio_setup()
{
  uint32_t cmd_status;
  rfc_CMD_RADIO_SETUP_t cmd;

  rf_switch_select_path(RF_SWITCH_PATH_2_4GHZ);

  /* Create radio setup command */
  rf_core_init_radio_op((rfc_radioOp_t *)&cmd, sizeof(cmd), CMD_RADIO_SETUP);

  cmd.txPower = tx_power_current->tx_power;
  cmd.pRegOverride = ieee_overrides;
  cmd.config.frontEndMode = RF_CORE_RADIO_SETUP_FRONT_END_MODE;
  cmd.config.biasMode = RF_CORE_RADIO_SETUP_BIAS_MODE;
  cmd.mode = 1;

  /* Send Radio setup to RF Core */
  if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("rf_radio_setup: CMD_RADIO_SETUP, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd.status);
    return RF_CORE_CMD_ERROR;
  }

  /* Wait until radio setup is done */
  if(rf_core_wait_cmd_done(&cmd) != RF_CORE_CMD_OK) {
    PRINTF("rf_radio_setup: CMD_RADIO_SETUP wait, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd.status);
    return RF_CORE_CMD_ERROR;
  }

  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief    Set up radio in IEEE802.15.4 RX mode
 *
 * \return   RF_CORE_CMD_OK   Succeeded
 * \return   RF_CORE_CMD_ERROR   Failed
 *
 * This function assumes that cmd_ieee_rx_buf has been previously populated
 * with correct values. This can be done through init_rf_params (sets defaults)
 * or through Contiki's extended RF API (set_value, set_object)
 */
static uint8_t
rf_cmd_ieee_rx()
{
  uint32_t cmd_status;
  int ret;

  ret = rf_core_send_cmd((uint32_t)cmd_ieee_rx_buf, &cmd_status);

  if(ret != RF_CORE_CMD_OK) {
    PRINTF("rf_cmd_ieee_rx: ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CORE_CMD_ERROR;
  }

  LIMITED_BUSYWAIT(RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) != RF_CORE_RADIO_OP_STATUS_ACTIVE,
                   ENTER_RX_WAIT_TIMEOUT);

  /* Wait to enter RX */
  if(RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) != RF_CORE_RADIO_OP_STATUS_ACTIVE) {
    PRINTF("rf_cmd_ieee_rx: CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CORE_CMD_ERROR;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
init_rx_buffers(void)
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
static void
init_rf_params(void)
{
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;

  memset(cmd_ieee_rx_buf, 0x00, RF_CMD_BUFFER_SIZE);

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
  cmd->rxConfig.bAppendTimestamp = 1;

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

  /* set address filter command */
  filter_cmd.commandNo = CMD_IEEE_MOD_FILT;
  memcpy(&filter_cmd.newFrameFiltOpt, &cmd->frameFiltOpt, sizeof(cmd->frameFiltOpt));
  memcpy(&filter_cmd.newFrameTypes, &cmd->frameTypes, sizeof(cmd->frameTypes));
}
/*---------------------------------------------------------------------------*/
static int
rx_on(void)
{
  int ret;

  /* Get status of running IEEE_RX (if any) */
  if(rf_is_on()) {
    PRINTF("rx_on: We were on. PD=%u, RX=0x%04x \n", rf_core_is_accessible(),
           RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CORE_CMD_OK;
  }

  /* Put CPE in RX using the currently configured parameters */
  ret = rf_cmd_ieee_rx();

  if(ret) {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
rx_off(void)
{
  uint32_t cmd_status;
  int ret;

  /* If we are off, do nothing */
  if(!rf_is_on()) {
    return RF_CORE_CMD_OK;
  }

  /* Wait for ongoing ACK TX to finish */
  while(transmitting());

  /* Send a CMD_ABORT command to RF Core */
  if(rf_core_send_cmd(CMDR_DIR_CMD(CMD_ABORT), &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("RX off: CMD_ABORT status=0x%08lx\n", cmd_status);
    /* Continue nonetheless */
  }

  LIMITED_BUSYWAIT(rf_is_on(), RF_TURN_OFF_WAIT_TIMEOUT);

  if(RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) == IEEE_DONE_STOPPED ||
     RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) == IEEE_DONE_ABORT) {
    /* Stopped gracefully */
    ret = RF_CORE_CMD_OK;
  } else {
    PRINTF("RX off: BG status=0x%04x\n", RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    ret = RF_CORE_CMD_ERROR;
  }

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  return ret;
}
/*---------------------------------------------------------------------------*/
static uint8_t
request(void)
{
  /*
   * We rely on the RDC layer to turn us on and off. Thus, if we are on we
   * will only allow sleep, standby otherwise
   */
  if(rf_is_on()) {
    return LPM_MODE_SLEEP;
  }

  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
LPM_MODULE(cc26xx_rf_lpm_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
static void
soft_off(void)
{
  uint32_t cmd_status;
  volatile rfc_radioOp_t *cmd = rf_core_get_last_radio_op();

  if(!rf_core_is_accessible()) {
    return;
  }

  PRINTF("soft_off: Aborting 0x%04x, Status=0x%04x\n", cmd->commandNo,
         cmd->status);

  /* Send a CMD_ABORT command to RF Core */
  if(rf_core_send_cmd(CMDR_DIR_CMD(CMD_ABORT), &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("soft_off: CMD_ABORT status=0x%08lx\n", cmd_status);
    return;
  }

  LIMITED_BUSYWAIT((cmd->status & RF_CORE_RADIO_OP_MASKED_STATUS) ==
                   RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING, RF_TURN_OFF_WAIT_TIMEOUT);
}
/*---------------------------------------------------------------------------*/
static uint8_t
soft_on(void)
{
  if(rf_radio_setup() != RF_CORE_CMD_OK) {
    PRINTF("on: radio_setup() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  return rx_on();
}
/*---------------------------------------------------------------------------*/
static const rf_core_primary_mode_t mode_ieee = {
  soft_off,
  soft_on,
};
/*---------------------------------------------------------------------------*/
static uint8_t
check_rat_overflow(bool first_time)
{
  static uint32_t last_value;
  uint32_t current_value;
  uint8_t interrupts_disabled;

  /* Bail out if the RF is not on */
  if(!rf_is_on()) {
    return 0;
  }

  interrupts_disabled = ti_lib_int_master_disable();
  if(first_time) {
    last_value = HWREG(RFC_RAT_BASE + RATCNT);
  } else {
    current_value = HWREG(RFC_RAT_BASE + RATCNT);
    if(current_value + RAT_RANGE / 4 < last_value) {
      /* Overflow detected */
      last_rat_overflow = RTIMER_NOW();
      rat_overflow_counter++;
    }
    last_value = current_value;
  }
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
handle_rat_overflow(void *unused)
{
  uint8_t success;
  uint8_t was_off = 0;

  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("overflow: on() failed\n");
      ctimer_set(&rat_overflow_timer, CLOCK_SECOND,
                 handle_rat_overflow, NULL);
      return;
    }
  }

  success = check_rat_overflow(false);

  if(was_off) {
    off();
  }

  if(success) {
    /* Retry after half of the interval */
    ctimer_set(&rat_overflow_timer, RAT_OVERFLOW_PERIOD_SECONDS * CLOCK_SECOND / 2,
               handle_rat_overflow, NULL);
  } else {
    /* Retry sooner */
    ctimer_set(&rat_overflow_timer, CLOCK_SECOND,
               handle_rat_overflow, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  lpm_register_module(&cc26xx_rf_lpm_module);

  rf_core_set_modesel();

  /* Initialise RX buffers */
  memset(rx_buf_0, 0, RX_BUF_SIZE);
  memset(rx_buf_1, 0, RX_BUF_SIZE);
  memset(rx_buf_2, 0, RX_BUF_SIZE);
  memset(rx_buf_3, 0, RX_BUF_SIZE);

  /* Set of RF Core data queue. Circular buffer, no last entry */
  rx_data_queue.pCurrEntry = rx_buf_0;

  rx_data_queue.pLastEntry = NULL;

  /* Initialize current read pointer to first element (used in ISR) */
  rx_read_entry = rx_buf_0;

  /* Populate the RF parameters data structure with default values */
  init_rf_params();

  if(on() != RF_CORE_CMD_OK) {
    PRINTF("init: on() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  rf_core_primary_mode_register(&mode_ieee);

  check_rat_overflow(true);
  ctimer_set(&rat_overflow_timer, RAT_OVERFLOW_PERIOD_SECONDS * CLOCK_SECOND / 2,
             handle_rat_overflow, NULL);

  process_start(&rf_core_process, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  int len = MIN(payload_len, TX_BUF_PAYLOAD_LEN);

  memcpy(&tx_buf[TX_BUF_HDR_LEN], payload, len);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  int ret;
  uint8_t was_off = 0;
  uint32_t cmd_status;
  uint16_t stat;
  uint8_t tx_active = 0;
  rtimer_clock_t t0;
  volatile rfc_CMD_IEEE_TX_t cmd;

  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("transmit: on() failed\n");
      return RADIO_TX_ERR;
    }
  }

  /*
   * We are certainly not TXing a frame as a result of CMD_IEEE_TX, but we may
   * be in the process of TXing an ACK. In that case, wait for the TX to finish
   * or return after approx TX_WAIT_TIMEOUT
   */
  t0 = RTIMER_NOW();

  do {
    tx_active = transmitting();
  } while(tx_active == 1 &&
          (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + TX_WAIT_TIMEOUT)));

  if(tx_active) {
    PRINTF("transmit: Already TXing and wait timed out\n");

    if(was_off) {
      off();
    }

    return RADIO_TX_COLLISION;
  }

  /* Send the CMD_IEEE_TX command */
  rf_core_init_radio_op((rfc_radioOp_t *)&cmd, sizeof(cmd), CMD_IEEE_TX);

  cmd.payloadLen = transmit_len;
  cmd.pPayload = &tx_buf[TX_BUF_HDR_LEN];

  cmd.startTime = 0;
  cmd.startTrigger.triggerType = TRIG_NOW;

  /* Enable the LAST_FG_COMMAND_DONE interrupt, which will wake us up */
  rf_core_cmd_done_en(true, poll_mode);

  ret = rf_core_send_cmd((uint32_t)&cmd, &cmd_status);

  if(ret) {
    /* If we enter here, TX actually started */
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

    /* Idle away while the command is running */
    while((cmd.status & RF_CORE_RADIO_OP_MASKED_STATUS)
          == RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING) {
      /* Note: for now sleeping while Tx'ing in polling mode is disabled.
       * To enable it:
       *  1) make the `lpm_sleep()` call here unconditional;
       *  2) change the radio ISR priority to allow radio ISR to interrupt rtimer ISR.
       */
      if(!poll_mode) {
        lpm_sleep();
      }
    }

    stat = cmd.status;

    if(stat == RF_CORE_RADIO_OP_STATUS_IEEE_DONE_OK) {
      /* Sent OK */
      RIMESTATS_ADD(lltx);
      ret = RADIO_TX_OK;
    } else {
      /* Operation completed, but frame was not sent */
      PRINTF("transmit: ret=%d, CMDSTA=0x%08lx, status=0x%04x\n", ret,
             cmd_status, stat);
      ret = RADIO_TX_ERR;
    }
  } else {
    /* Failure sending the CMD_IEEE_TX command */
    PRINTF("transmit: ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, cmd.status);

    ret = RADIO_TX_ERR;
  }

  /*
   * Update ENERGEST state here, before a potential call to off(), which
   * will correctly update it if required.
   */
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  /*
   * Disable LAST_FG_COMMAND_DONE interrupt. We don't really care about it
   * except when we are transmitting
   */
  rf_core_cmd_done_dis(poll_mode);

  if(was_off) {
    off();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
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
static uint32_t
calc_last_packet_timestamp(uint32_t rat_timestamp)
{
  uint64_t rat_timestamp64;
  uint32_t adjusted_overflow_counter;
  uint8_t was_off = 0;

  if(!rf_is_on()) {
    was_off = 1;
    on();
  }

  if(rf_is_on()) {
    check_rat_overflow(false);
    if(was_off) {
      off();
    }
  }

  adjusted_overflow_counter = rat_overflow_counter;

  /* if the timestamp is large and the last oveflow was recently,
     assume that the timestamp refers to the time before the overflow */
  if(rat_timestamp > (uint32_t)(RAT_RANGE * 3 / 4)) {
    if(RTIMER_CLOCK_LT(RTIMER_NOW(),
                       last_rat_overflow + RAT_OVERFLOW_PERIOD_SECONDS * RTIMER_SECOND / 4)) {
      adjusted_overflow_counter--;
    }
  }

  /* add the overflowed time to the timestamp */
  rat_timestamp64 = rat_timestamp + RAT_RANGE * adjusted_overflow_counter;
  /* correct timestamp so that it refers to the end of the SFD */
  rat_timestamp64 += TIMESTAMP_OFFSET;

  last_rat_timestamp64 = rat_timestamp64 - rat_offset;

  return RADIO_TO_RTIMER(rat_timestamp64 - rat_offset);
}
/*---------------------------------------------------------------------------*/
static int
read_frame(void *buf, unsigned short buf_len)
{
  int len = 0;
  rfc_dataEntryGeneral_t *entry = (rfc_dataEntryGeneral_t *)rx_read_entry;
  uint32_t rat_timestamp;

  /* wait for entry to become finished */
  rtimer_clock_t t0 = RTIMER_NOW();
  while(entry->status == DATA_ENTRY_STATUS_BUSY
      && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (RTIMER_SECOND / 250)));

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

  len = rx_read_entry[8] - 8;

  if(len > buf_len) {
    PRINTF("RF: too long\n");
    RIMESTATS_ADD(toolong);

    release_data_entry();
    return 0;
  }

  memcpy(buf, (char *)&rx_read_entry[9], len);

  last_rssi = (int8_t)rx_read_entry[9 + len + 2];
  last_corr_lqi = (uint8_t)rx_read_entry[9 + len + 3] & STATUS_CORRELATION;

  /* get the timestamp */
  memcpy(&rat_timestamp, (char *)rx_read_entry + 9 + len + 4, 4);

  last_packet_timestamp = calc_last_packet_timestamp(rat_timestamp);

  if(!poll_mode) {
    /* Not in poll mode: packetbuf should not be accessed in interrupt context.
     * In poll mode, the last packet RSSI and link quality can be obtained through
     * RADIO_PARAM_LAST_RSSI and RADIO_PARAM_LAST_LINK_QUALITY */
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, last_corr_lqi);
  }
  RIMESTATS_ADD(llrx);

  release_data_entry();

  return len;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  uint8_t was_off = 0;
  uint8_t cca_info;
  int ret = RF_CCA_CLEAR;

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Indicate a clear channel
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    PRINTF("channel_clear: Interrupt context but BLE in progress\n");
    return RF_CCA_CLEAR;
  }

  if(rf_is_on()) {
    /*
     * Wait for potential leftover ACK still being sent.
     * Strictly speaking, if we are TXing an ACK then the channel is not clear.
     * However, channel_clear is only ever called to determine whether there is
     * someone else's packet in the air, not ours.
     *
     * We could probably even simply return that the channel is clear
     */
    while(transmitting());
  } else {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("channel_clear: on() failed\n");
      if(was_off) {
        off();
      }
      return RF_CCA_CLEAR;
    }
  }

  cca_info = get_cca_info();

  if(cca_info == RF_GET_CCA_INFO_ERROR) {
    PRINTF("channel_clear: CCA error\n");
    ret = RF_CCA_CLEAR;
  } else {
    /*
     * cca_info bits 1:0 - ccaStatus
     * Return 1 (clear) if idle or invalid.
     */
    ret = (cca_info & 0x03) != RF_CMD_CCA_REQ_CCA_STATE_BUSY;
  }

  if(was_off) {
    off();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  uint8_t cca_info;

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. We are not receiving
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    PRINTF("receiving_packet: Interrupt context but BLE in progress\n");
    return 0;
  }

  /* If we are off, we are not receiving */
  if(!rf_is_on()) {
    PRINTF("receiving_packet: We were off\n");
    return 0;
  }

  /* If we are transmitting (can only be an ACK here), we are not receiving */
  if(transmitting()) {
    PRINTF("receiving_packet: We were TXing\n");
    return 0;
  }

  cca_info = get_cca_info();

  /* If we can't read CCA info, return "not receiving" */
  if(cca_info == RF_GET_CCA_INFO_ERROR) {
    return 0;
  }

  /* If sync has been seen, return 1 (receiving) */
  if(cca_info & RF_CMD_CCA_REQ_CCA_SYNC_BUSY) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  volatile rfc_dataEntry_t *entry = (rfc_dataEntry_t *)rx_data_queue.pCurrEntry;
  int rv = 0;

  /* Go through all RX buffers and check their status */
  do {
    if(entry->status == DATA_ENTRY_STATUS_FINISHED
        || entry->status == DATA_ENTRY_STATUS_BUSY) {
      rv = 1;
      if(!poll_mode) {
        process_poll(&rf_core_process);
      }
    }

    entry = (rfc_dataEntry_t *)entry->pNextEntry;
  } while(entry != (rfc_dataEntry_t *)rx_data_queue.pCurrEntry);

  /* If we didn't find an entry at status finished, no frames are pending */
  return rv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Abort, but pretend everything is OK.
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    PRINTF("on: Interrupt context but BLE in progress\n");
    return RF_CORE_CMD_OK;
  }

  /*
   * Request the HF XOSC as the source for the HF clock. Needed before we can
   * use the FS. This will only request, it will _not_ perform the switch.
   */
  oscillators_request_hf_xosc();

  if(rf_is_on()) {
    PRINTF("on: We were on. PD=%u, RX=0x%04x \n", rf_core_is_accessible(),
           RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CORE_CMD_OK;
  }

  init_rx_buffers();

  /*
   * Trigger a switch to the XOSC, so that we can subsequently use the RF FS
   * This will block until the XOSC is actually ready, but give how we
   * requested it early on, this won't be too long a wait.
   * This should be done before starting the RAT.
   */
  oscillators_switch_to_hf_xosc();

  if(rf_core_boot() != RF_CORE_CMD_OK) {
    PRINTF("on: rf_core_boot() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  rf_core_setup_interrupts(poll_mode);

  if(rf_radio_setup() != RF_CORE_CMD_OK) {
    PRINTF("on: radio_setup() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  return rx_on();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Abort, but pretend everything is OK.
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    PRINTF("off: Interrupt context but BLE in progress\n");
    return RF_CORE_CMD_OK;
  }

  while(transmitting());

  /* stopping the rx explicitly results in lower sleep-mode power usage */
  rx_off();
  rf_core_power_down();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

#if !CC2650_FAST_RADIO_STARTUP
  /* Switch HF clock source to the RCOSC to preserve power.
   * This must be done after stopping RAT.
   */
  oscillators_switch_to_hf_rc();
#endif

  /* We pulled the plug, so we need to restore the status manually */
  ((rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf)->status = RF_CORE_RADIO_OP_STATUS_IDLE;

  /*
   * Just in case there was an ongoing RX (which started after we begun the
   * shutdown sequence), we don't want to leave the buffer in state == ongoing
   */
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

  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
/* Enable or disable CCA before sending */
static radio_result_t
set_send_on_cca(uint8_t enable)
{
  if(enable) {
    /* this driver does not have support for CCA on Tx */
    return RADIO_RESULT_NOT_SUPPORTED;
  }
  return RADIO_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;

  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    /* On / off */
    *value = rf_is_on() ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = (radio_value_t)cmd->channel;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_PAN_ID:
    *value = (radio_value_t)cmd->localPanID;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_16BIT_ADDR:
    *value = (radio_value_t)cmd->localShortAddr;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    if(cmd->frameFiltOpt.frameFiltEn) {
      *value |= RADIO_RX_MODE_ADDRESS_FILTER;
    }
    if(cmd->frameFiltOpt.autoAckEn) {
      *value |= RADIO_RX_MODE_AUTOACK;
    }
    if(poll_mode) {
      *value |= RADIO_RX_MODE_POLL_MODE;
    }

    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    *value = 0;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    *value = get_tx_power();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    *value = cmd->ccaRssiThr;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = get_rssi();

    if(*value == RF_CMD_CCA_REQ_RSSI_UNKNOWN) {
      return RADIO_RESULT_ERROR;
    } else {
      return RADIO_RESULT_OK;
    }
  case RADIO_CONST_CHANNEL_MIN:
    *value = IEEE_MODE_CHANNEL_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = IEEE_MODE_CHANNEL_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = OUTPUT_POWER_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = OUTPUT_POWER_MAX;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_LAST_RSSI:
    *value = last_rssi;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_LAST_LINK_QUALITY:
    *value = last_corr_lqi;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  radio_result_t rv = RADIO_RESULT_OK;
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;
  uint8_t old_poll_mode;

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    if(value == RADIO_POWER_MODE_ON) {
      if(on() != RF_CORE_CMD_OK) {
        PRINTF("set_value: on() failed (1)\n");
        return RADIO_RESULT_ERROR;
      }
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_OFF) {
      off();
      return RADIO_RESULT_OK;
    }
    return RADIO_RESULT_INVALID_VALUE;
  case RADIO_PARAM_CHANNEL:
    if(value < IEEE_MODE_CHANNEL_MIN ||
       value > IEEE_MODE_CHANNEL_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    /* Note: this return may lead to long periods when RAT and RTC are not resynchronized */
    if(cmd->channel == (uint8_t)value) {
      /* We already have that very same channel configured.
       * Nothing to do here. */
      return RADIO_RESULT_OK;
    }

    cmd->channel = (uint8_t)value;
    break;
  case RADIO_PARAM_PAN_ID:
    cmd->localPanID = (uint16_t)value;
    break;
  case RADIO_PARAM_16BIT_ADDR:
    cmd->localShortAddr = (uint16_t)value;
    break;
  case RADIO_PARAM_RX_MODE:
  {
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
                 RADIO_RX_MODE_AUTOACK | RADIO_RX_MODE_POLL_MODE)) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    cmd->frameFiltOpt.frameFiltEn = (value & RADIO_RX_MODE_ADDRESS_FILTER) != 0;
    cmd->frameFiltOpt.frameFiltStop = 1;
    cmd->frameFiltOpt.autoAckEn = (value & RADIO_RX_MODE_AUTOACK) != 0;
    cmd->frameFiltOpt.slottedAckEn = 0;
    cmd->frameFiltOpt.autoPendEn = 0;
    cmd->frameFiltOpt.defaultPend = 0;
    cmd->frameFiltOpt.bPendDataReqOnly = 0;
    cmd->frameFiltOpt.bPanCoord = 0;
    cmd->frameFiltOpt.bStrictLenFilter = 0;

    old_poll_mode = poll_mode;
    poll_mode = (value & RADIO_RX_MODE_POLL_MODE) != 0;
    if(poll_mode == old_poll_mode) {
      uint32_t cmd_status;

      /* do not turn the radio on and off, just send an update command */
      memcpy(&filter_cmd.newFrameFiltOpt, &cmd->frameFiltOpt, sizeof(cmd->frameFiltOpt));

      if(rf_core_send_cmd((uint32_t)&filter_cmd, &cmd_status) == RF_CORE_CMD_ERROR) {
        PRINTF("setting address filter failed: CMDSTA=0x%08lx\n", cmd_status);
        return RADIO_RESULT_ERROR;
      }
      return RADIO_RESULT_OK;
    }
    break;
  }

  case RADIO_PARAM_TX_MODE:
    if(value & ~(RADIO_TX_MODE_SEND_ON_CCA)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    return set_send_on_cca((value & RADIO_TX_MODE_SEND_ON_CCA) != 0);

  case RADIO_PARAM_TXPOWER:
    if(value < OUTPUT_POWER_MIN || value > OUTPUT_POWER_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    set_tx_power(value);

    return RADIO_RESULT_OK;

  case RADIO_PARAM_CCA_THRESHOLD:
    cmd->ccaRssiThr = (int8_t)value;
    break;

  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }

  /* If off, the new configuration will be applied the next time radio is started */
  if(!rf_is_on()) {
    return RADIO_RESULT_OK;
  }

  /* If we reach here we had no errors. Apply new settings */
  if(rx_off() != RF_CORE_CMD_OK) {
    PRINTF("set_value: rx_off() failed\n");
    rv = RADIO_RESULT_ERROR;
  }

  /* Restart the radio timer (RAT).
     This causes resynchronization between RAT and RTC: useful for TSCH. */
  if(rf_core_restart_rat() == RF_CORE_CMD_OK) {
    check_rat_overflow(false);
  }

  if(rx_on() != RF_CORE_CMD_OK) {
    PRINTF("set_value: rx_on() failed\n");
    rv = RADIO_RESULT_ERROR;
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  uint8_t *target;
  uint8_t *src;
  int i;
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    target = dest;
    src = (uint8_t *)(&cmd->localExtAddr);

    for(i = 0; i < 8; i++) {
      target[i] = src[7 - i];
    }

    return RADIO_RESULT_OK;
  }

  if(param == RADIO_PARAM_LAST_PACKET_TIMESTAMP) {
    if(size != sizeof(rtimer_clock_t) || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    *(rtimer_clock_t *)dest = last_packet_timestamp;

    return RADIO_RESULT_OK;
  }

  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  radio_result_t rv = RADIO_RESULT_OK;
  int i;
  uint8_t *dst;
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !src) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    dst = (uint8_t *)(&cmd->localExtAddr);

    for(i = 0; i < 8; i++) {
      dst[i] = ((uint8_t *)src)[7 - i];
    }

    /* If off, the new configuration will be applied the next time radio is started */
    if(!rf_is_on()) {
      return RADIO_RESULT_OK;
    }

    if(rx_off() != RF_CORE_CMD_OK) {
      PRINTF("set_object: rx_off() failed\n");
      rv = RADIO_RESULT_ERROR;
    }

    if(rx_on() != RF_CORE_CMD_OK) {
      PRINTF("set_object: rx_on() failed\n");
      rv = RADIO_RESULT_ERROR;
    }

    return rv;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver ieee_mode_driver = {
  init,
  prepare,
  transmit,
  send,
  read_frame,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object,
};
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
