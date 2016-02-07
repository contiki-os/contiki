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
/**
 * \addtogroup rf-core
 * @{
 *
 * \defgroup rf-core-prop CC13xx Prop mode driver
 *
 * @{
 *
 * \file
 * Implementation of the CC13xx prop mode NETSTACK_RADIO driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc26xx-uart.h"
#include "dev/oscillators.h"
#include "dev/watchdog.h"
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
#include "rf-core/dot-15-4g.h"
/*---------------------------------------------------------------------------*/
/* RF core and RF HAL API */
#include "hw_rfc_dbell.h"
#include "hw_rfc_pwr.h"
/*---------------------------------------------------------------------------*/
/* RF Core Mailbox API */
#include "rf-core/api/mailbox.h"
#include "rf-core/api/common_cmd.h"
#include "rf-core/api/data_entry.h"
#include "rf-core/api/prop_mailbox.h"
#include "rf-core/api/prop_cmd.h"
/*---------------------------------------------------------------------------*/
/* CC13xxware patches */
#include "rf_patches/rf_patch_cpe_genfsk.h"
/*---------------------------------------------------------------------------*/
#include "rf-core/smartrf-settings.h"
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
/* Data entry status field constants */
#define DATA_ENTRY_STATUS_PENDING    0x00 /* Not in use by the Radio CPU */
#define DATA_ENTRY_STATUS_ACTIVE     0x01 /* Open for r/w by the radio CPU */
#define DATA_ENTRY_STATUS_BUSY       0x02 /* Ongoing r/w */
#define DATA_ENTRY_STATUS_FINISHED   0x03 /* Free to use and to free */
#define DATA_ENTRY_STATUS_UNFINISHED 0x04 /* Partial RX entry */
/*---------------------------------------------------------------------------*/
/* Data whitener. 1: Whitener, 0: No whitener */
#ifdef PROP_MODE_CONF_DW
#define PROP_MODE_DW PROP_MODE_CONF_DW
#else
#define PROP_MODE_DW 0
#endif

#ifdef PROP_MODE_CONF_USE_CRC16
#define PROP_MODE_USE_CRC16 PROP_MODE_CONF_USE_CRC16
#else
#define PROP_MODE_USE_CRC16 0
#endif
/*---------------------------------------------------------------------------*/
#ifdef PROP_MODE_CONF_SNIFFER
#define PROP_MODE_SNIFFER PROP_MODE_CONF_SNIFFER
#else
#define PROP_MODE_SNIFFER 0
#endif

#if PROP_MODE_SNIFFER
static const uint8_t magic[] = { 0x53, 0x6E, 0x69, 0x66 };
#endif
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current status of a running Radio Op command
 * \param a A pointer with the buffer used to initiate the command
 * \return The value of the Radio Op buffer's status field
 *
 * This macro can be used to e.g. return the status of a previously
 * initiated background operation, or of an immediate command
 */
#define RF_RADIO_OP_GET_STATUS(a) GET_FIELD_V(a, radioOp, status)
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

#ifdef PROP_MODE_CONF_RSSI_THRESHOLD
#define PROP_MODE_RSSI_THRESHOLD PROP_MODE_CONF_RSSI_THRESHOLD
#else
#define PROP_MODE_RSSI_THRESHOLD 0xA6
#endif

static int8_t rssi_threshold = PROP_MODE_RSSI_THRESHOLD;
/*---------------------------------------------------------------------------*/
static int on(void);
static int off(void);

static rfc_propRxOutput_t rx_stats;
/*---------------------------------------------------------------------------*/
/* Defines and variables related to the .15.4g PHY HDR */
#define DOT_4G_MAX_FRAME_LEN    2047
#define DOT_4G_PHR_LEN             2

/* PHY HDR bits */
#define DOT_4G_PHR_CRC16  0x10
#define DOT_4G_PHR_DW     0x08

#if PROP_MODE_USE_CRC16
/* CRC16 */
#define DOT_4G_PHR_CRC_BIT DOT_4G_PHR_CRC16
#define CRC_LEN            2
#else
/* CRC32 */
#define DOT_4G_PHR_CRC_BIT 0
#define CRC_LEN            4
#endif

#if PROP_MODE_DW
#define DOT_4G_PHR_DW_BIT DOT_4G_PHR_DW
#else
#define DOT_4G_PHR_DW_BIT 0
#endif
/*---------------------------------------------------------------------------*/
/* How long to wait for an ongoing ACK TX to finish before starting frame TX */
#define TX_WAIT_TIMEOUT       (RTIMER_SECOND >> 11)

/* How long to wait for the RF to enter RX in rf_cmd_ieee_rx */
#define ENTER_RX_WAIT_TIMEOUT (RTIMER_SECOND >> 10)
/*---------------------------------------------------------------------------*/
/* TX Power dBm lookup table - values from SmartRF Studio */
typedef struct output_config {
  radio_value_t dbm;
  uint16_t tx_power; /* Value for the PROP_DIV_RADIO_SETUP.txPower field */
} output_config_t;

static const output_config_t output_power[] = {
  { 14, 0xa73f },
  { 13, 0xa73f }, /* 12.5 */
  { 12, 0xb818 },
  { 11, 0x50da },
  { 10, 0x38d3 },
  {  9, 0x2ccd },
  {  8, 0x24cb },
  {  7, 0x20c9 },
  {  6, 0x1cc7 },
  {  5, 0x18c6 },
  {  4, 0x18c5 },
  {  3, 0x14c4 },
  {  2, 0x1042 },
  {  1, 0x10c3 },
  {  0, 0x0041 },
  {-10, 0x08c0 },
};

#define OUTPUT_CONFIG_COUNT (sizeof(output_power) / sizeof(output_config_t))

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MIN     (output_power[OUTPUT_CONFIG_COUNT - 1].dbm)
#define OUTPUT_POWER_MAX     (output_power[0].dbm)
#define OUTPUT_POWER_UNKNOWN 0xFFFF

/* Default TX Power - position in output_power[] */
const output_config_t *tx_power_current = &output_power[1];
/*---------------------------------------------------------------------------*/
#ifdef PROP_MODE_CONF_LO_DIVIDER
#define PROP_MODE_LO_DIVIDER   PROP_MODE_CONF_LO_DIVIDER
#else
#define PROP_MODE_LO_DIVIDER   0x05
#endif
/*---------------------------------------------------------------------------*/
#define DATA_ENTRY_LENSZ_NONE 0
#define DATA_ENTRY_LENSZ_BYTE 1
#define DATA_ENTRY_LENSZ_WORD 2 /* 2 bytes */

#define RX_BUF_SIZE 140
/* Receive buffers: 1 frame in each */
static uint8_t rx_buf_0[RX_BUF_SIZE] CC_ALIGN(4);
static uint8_t rx_buf_1[RX_BUF_SIZE] CC_ALIGN(4);

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
static uint8_t
rf_is_on(void)
{
  if(!rf_core_is_accessible()) {
    return 0;
  }

  return smartrf_settings_cmd_prop_rx_adv.status == RF_CORE_RADIO_OP_STATUS_ACTIVE;
}
/*---------------------------------------------------------------------------*/
static uint8_t
transmitting(void)
{
  return smartrf_settings_cmd_prop_tx_adv.status == RF_CORE_RADIO_OP_STATUS_ACTIVE;
}
/*---------------------------------------------------------------------------*/
static radio_value_t
get_rssi(void)
{
  uint32_t cmd_status;
  int8_t rssi;
  uint8_t was_off = 0;
  rfc_CMD_GET_RSSI_t cmd;

  /* If we are off, turn on first */
  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("get_rssi: on() failed\n");
      return RF_CMD_CCA_REQ_RSSI_UNKNOWN;
    }
  }

  memset(&cmd, 0x00, sizeof(cmd));
  cmd.commandNo = CMD_GET_RSSI;

  rssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;

  if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) == RF_CORE_CMD_OK) {
    /* Current RSSI in bits 23:16 of cmd_status */
    rssi = (cmd_status >> 16) & 0xFF;
  }

  /* If we were off, turn back off */
  if(was_off) {
    off();
  }

  return rssi;
}
/*---------------------------------------------------------------------------*/
static uint8_t
get_channel(void)
{
  uint32_t freq_khz;

  freq_khz = smartrf_settings_cmd_fs.frequency * 1000;

  /*
   * For some channels, fractFreq * 1000 / 65536 will return 324.99xx.
   * Casting the result to uint32_t will truncate decimals resulting in the
   * function returning channel - 1 instead of channel. Thus, we do a quick
   * positive integer round up.
   */
  freq_khz += (((smartrf_settings_cmd_fs.fractFreq * 1000) + 65535) / 65536);

  return (freq_khz - DOT_15_4G_CHAN0_FREQUENCY) / DOT_15_4G_CHANNEL_SPACING;
}
/*---------------------------------------------------------------------------*/
static void
set_channel(uint8_t channel)
{
  uint32_t new_freq;
  uint16_t freq, frac;

  new_freq = DOT_15_4G_CHAN0_FREQUENCY + (channel * DOT_15_4G_CHANNEL_SPACING);

  freq = (uint16_t)(new_freq / 1000);
  frac = (new_freq - (freq * 1000)) * 65536 / 1000;

  PRINTF("set_channel: %u = 0x%04x.0x%04x (%lu)\n", channel, freq, frac,
         new_freq);

  smartrf_settings_cmd_prop_radio_div_setup.centerFreq = freq;
  smartrf_settings_cmd_fs.frequency = freq;
  smartrf_settings_cmd_fs.fractFreq = frac;
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
 * The caller must make sure to send a new CMD_PROP_RADIO_DIV_SETP to the
 * radio after calling this function.
 */
static void
set_tx_power(radio_value_t power)
{
  int i;

  for(i = OUTPUT_CONFIG_COUNT - 1; i >= 0; --i) {
    if(power <= output_power[i].dbm) {
      /*
       * Merely save the value. It will be used in all subsequent usages of
       * CMD_PROP_RADIO_DIV_SETP, including one immediately after this function
       * has returned
       */
      tx_power_current = &output_power[i];

      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
prop_div_radio_setup(void)
{
  uint32_t cmd_status;
  rfc_radioOp_t *cmd = (rfc_radioOp_t *)&smartrf_settings_cmd_prop_radio_div_setup;

  /* Adjust loDivider depending on the selected band */
  smartrf_settings_cmd_prop_radio_div_setup.loDivider = PROP_MODE_LO_DIVIDER;

  /* Update to the correct TX power setting */
  smartrf_settings_cmd_prop_radio_div_setup.txPower = tx_power_current->tx_power;

  /* Send Radio setup to RF Core */
  if(rf_core_send_cmd((uint32_t)cmd, &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("prop_div_radio_setup: DIV_SETUP, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd->status);
    return RF_CORE_CMD_ERROR;
  }

  /* Wait until radio setup is done */
  if(rf_core_wait_cmd_done(cmd) != RF_CORE_CMD_OK) {
    PRINTF("prop_div_radio_setup: DIV_SETUP wait, CMDSTA=0x%08lx,"
           "status=0x%04x\n", cmd_status, cmd->status);
    return RF_CORE_CMD_ERROR;
  }

  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
static uint8_t
rf_cmd_prop_rx()
{
  uint32_t cmd_status;
  rtimer_clock_t t0;
  volatile rfc_CMD_PROP_RX_ADV_t *cmd_rx_adv;
  int ret;

  cmd_rx_adv = (rfc_CMD_PROP_RX_ADV_t *)&smartrf_settings_cmd_prop_rx_adv;
  cmd_rx_adv->status = RF_CORE_RADIO_OP_STATUS_IDLE;

  /*
   * Set the max Packet length. This is for the payload only, therefore
   * 2047 - length offset
   */
  cmd_rx_adv->maxPktLen = DOT_4G_MAX_FRAME_LEN - cmd_rx_adv->lenOffset;

  ret = rf_core_send_cmd((uint32_t)cmd_rx_adv, &cmd_status);

  if(ret != RF_CORE_CMD_OK) {
    PRINTF("rf_cmd_prop_rx: send_cmd ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, cmd_rx_adv->status);
    return RF_CORE_CMD_ERROR;
  }

  t0 = RTIMER_NOW();

  while(cmd_rx_adv->status != RF_CORE_RADIO_OP_STATUS_ACTIVE &&
        (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ENTER_RX_WAIT_TIMEOUT)));

  /* Wait to enter RX */
  if(cmd_rx_adv->status != RF_CORE_RADIO_OP_STATUS_ACTIVE) {
    PRINTF("rf_cmd_prop_rx: CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd_rx_adv->status);
    return RF_CORE_CMD_ERROR;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
rx_on_prop(void)
{
  int ret;

  if(rf_is_on()) {
    PRINTF("rx_on_prop: We were on. PD=%u, RX=0x%04x\n",
           rf_core_is_accessible(), smartrf_settings_cmd_prop_rx_adv.status);
    return RF_CORE_CMD_OK;
  }

  /* Put CPE in RX using the currently configured parameters */
  ret = rf_cmd_prop_rx();

  if(ret) {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
rx_off_prop(void)
{
  uint32_t cmd_status;
  int ret;

  /* If we are off, do nothing */
  if(!rf_is_on()) {
    return RF_CORE_CMD_OK;
  }

  /* Send a CMD_ABORT command to RF Core */
  if(rf_core_send_cmd(CMDR_DIR_CMD(CMD_ABORT), &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("rx_off_prop: CMD_ABORT status=0x%08lx\n", cmd_status);
    /* Continue nonetheless */
  }

  while(rf_is_on());

  if(smartrf_settings_cmd_prop_rx_adv.status == PROP_DONE_STOPPED ||
     smartrf_settings_cmd_prop_rx_adv.status == PROP_DONE_ABORT) {
    /* Stopped gracefully */
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ret = RF_CORE_CMD_OK;
  } else {
    PRINTF("rx_off_prop: status=0x%04x\n",
           smartrf_settings_cmd_prop_rx_adv.status);
    ret = RF_CORE_CMD_ERROR;
  }

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
LPM_MODULE(prop_lpm_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
static int
prop_fs(void)
{
  uint32_t cmd_status;
  rfc_radioOp_t *cmd = (rfc_radioOp_t *)&smartrf_settings_cmd_fs;

  /* Send the command to the RF Core */
  if(rf_core_send_cmd((uint32_t)cmd, &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("prop_fs: CMD_FS, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd->status);
    return RF_CORE_CMD_ERROR;
  }

  /* Wait until the command is done */
  if(rf_core_wait_cmd_done(cmd) != RF_CORE_CMD_OK) {
    PRINTF("prop_fs: CMD_FS wait, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd->status);
    return RF_CORE_CMD_ERROR;
  }

  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
static void
soft_off_prop(void)
{
  uint32_t cmd_status;
  volatile rfc_radioOp_t *cmd = rf_core_get_last_radio_op();

  if(!rf_core_is_accessible()) {
    return;
  }

  /* Send a CMD_ABORT command to RF Core */
  if(rf_core_send_cmd(CMDR_DIR_CMD(CMD_ABORT), &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("soft_off_prop: CMD_ABORT status=0x%08lx\n", cmd_status);
    return;
  }

  while((cmd->status & RF_CORE_RADIO_OP_MASKED_STATUS) ==
        RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING);
}
/*---------------------------------------------------------------------------*/
static uint8_t
soft_on_prop(void)
{
  if(prop_div_radio_setup() != RF_CORE_CMD_OK) {
    PRINTF("soft_on_prop: prop_div_radio_setup() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  if(prop_fs() != RF_CORE_CMD_OK) {
    PRINTF("soft_on_prop: prop_fs() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  return rx_on_prop();
}
/*---------------------------------------------------------------------------*/
static const rf_core_primary_mode_t mode_prop = {
  soft_off_prop,
  soft_on_prop,
};
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  rfc_dataEntry_t *entry;

  lpm_register_module(&prop_lpm_module);

  if(ti_lib_chipinfo_chip_family_is_cc13xx() == false) {
    return RF_CORE_CMD_ERROR;
  }

  rf_core_set_modesel();

  /* Initialise RX buffers */
  memset(rx_buf_0, 0, RX_BUF_SIZE);
  memset(rx_buf_1, 0, RX_BUF_SIZE);

  entry = (rfc_dataEntry_t *)rx_buf_0;
  entry->status = DATA_ENTRY_STATUS_PENDING;
  entry->config.type = DATA_ENTRY_TYPE_GEN;
  entry->config.lenSz = DATA_ENTRY_LENSZ_WORD;
  entry->length = RX_BUF_SIZE - 8;
  entry->pNextEntry = rx_buf_1;

  entry = (rfc_dataEntry_t *)rx_buf_1;
  entry->status = DATA_ENTRY_STATUS_PENDING;
  entry->config.type = DATA_ENTRY_TYPE_GEN;
  entry->config.lenSz = DATA_ENTRY_LENSZ_WORD;
  entry->length = RX_BUF_SIZE - 8;
  entry->pNextEntry = rx_buf_0;

  /* Set of RF Core data queue. Circular buffer, no last entry */
  rx_data_queue.pCurrEntry = rx_buf_0;
  rx_data_queue.pLastEntry = NULL;

  /* Initialize current read pointer to first element (used in ISR) */
  rx_read_entry = rx_buf_0;

  smartrf_settings_cmd_prop_rx_adv.pQueue = &rx_data_queue;
  smartrf_settings_cmd_prop_rx_adv.pOutput = (uint8_t *)&rx_stats;

  set_channel(RF_CORE_CHANNEL);

  if(on() != RF_CORE_CMD_OK) {
    PRINTF("init: on() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  rf_core_primary_mode_register(&mode_prop);

  process_start(&rf_core_process, NULL);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  int len = MIN(payload_len, TX_BUF_PAYLOAD_LEN);

  memcpy(&tx_buf[TX_BUF_HDR_LEN], payload, len);
  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  int ret;
  uint8_t was_off = 0;
  uint32_t cmd_status;
  volatile rfc_CMD_PROP_TX_ADV_t *cmd_tx_adv;

  /* Length in .15.4g PHY HDR. Includes the CRC but not the HDR itself */
  uint16_t total_length;

  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("transmit: on() failed\n");
      return RADIO_TX_ERR;
    }
  }

  /*
   * Prepare the .15.4g PHY header
   * MS=0, Length MSBits=0, DW and CRC configurable
   * Total length = transmit_len (payload) + CRC length
   *
   * The Radio will flip the bits around, so tx_buf[0] must have the length
   * LSBs (PHR[15:8] and tx_buf[1] will have PHR[7:0]
   */
  total_length = transmit_len + CRC_LEN;

  tx_buf[0] = total_length & 0xFF;
  tx_buf[1] = (total_length >> 8) + DOT_4G_PHR_DW_BIT + DOT_4G_PHR_CRC_BIT;

  /* Prepare the CMD_PROP_TX_ADV command */
  cmd_tx_adv = (rfc_CMD_PROP_TX_ADV_t *)&smartrf_settings_cmd_prop_tx_adv;

  /*
   * pktLen: Total number of bytes in the TX buffer, including the header if
   * one exists, but not including the CRC (which is not present in the buffer)
   */
  cmd_tx_adv->pktLen = transmit_len + DOT_4G_PHR_LEN;
  cmd_tx_adv->pPkt = tx_buf;

  /* Abort RX */
  rx_off_prop();

  /* Enable the LAST_COMMAND_DONE interrupt to wake us up */
  rf_core_cmd_done_en(false);

  ret = rf_core_send_cmd((uint32_t)cmd_tx_adv, &cmd_status);

  if(ret) {
    /* If we enter here, TX actually started */
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

    watchdog_periodic();

    /* Idle away while the command is running */
    while((cmd_tx_adv->status & RF_CORE_RADIO_OP_MASKED_STATUS)
          == RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING) {
      lpm_sleep();
    }

    if(cmd_tx_adv->status == RF_CORE_RADIO_OP_STATUS_PROP_DONE_OK) {
      /* Sent OK */
      RIMESTATS_ADD(lltx);
      ret = RADIO_TX_OK;
    } else {
      /* Operation completed, but frame was not sent */
      PRINTF("transmit: Not Sent OK status=0x%04x\n",
             cmd_tx_adv->status);
      ret = RADIO_TX_ERR;
    }
  } else {
    /* Failure sending the CMD_PROP_TX command */
    PRINTF("transmit: PROP_TX_ERR ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, cmd_tx_adv->status);
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
  rf_core_cmd_done_dis();

  /* Workaround. Set status to IDLE */
  cmd_tx_adv->status = RF_CORE_RADIO_OP_STATUS_IDLE;

  rx_on_prop();

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
static int
read_frame(void *buf, unsigned short buf_len)
{
  rfc_dataEntryGeneral_t *entry = (rfc_dataEntryGeneral_t *)rx_read_entry;
  uint8_t *data_ptr = &entry->data;
  int len = 0;

  if(entry->status == DATA_ENTRY_STATUS_FINISHED) {

    /*
     * First 2 bytes in the data entry are the length.
     * Our data entry consists of: Payload + RSSI (1 byte) + Status (1 byte)
     * This length includes all of those.
     */
    len = (*(uint16_t *)data_ptr);
    data_ptr += 2;
    len -= 2;

    if(len > 0) {
      if(len <= buf_len) {
        memcpy(buf, data_ptr, len);
      }

      packetbuf_set_attr(PACKETBUF_ATTR_RSSI, (int8_t)data_ptr[len]);

#if PROP_MODE_SNIFFER
      {
        int i;

        cc26xx_uart_write_byte(magic[0]);
        cc26xx_uart_write_byte(magic[1]);
        cc26xx_uart_write_byte(magic[2]);
        cc26xx_uart_write_byte(magic[3]);

        cc26xx_uart_write_byte(len + 2);

        for(i = 0; i < len; ++i) {
          cc26xx_uart_write_byte(((uint8_t *)(buf))[i]);
        }

        cc26xx_uart_write_byte((uint8_t)rx_stats.lastRssi);
        cc26xx_uart_write_byte(0x80);

        while(cc26xx_uart_busy() == UART_BUSY);
      }
#endif
    }

    /* Move read entry pointer to next entry */
    rx_read_entry = entry->pNextEntry;
    entry->status = DATA_ENTRY_STATUS_PENDING;
  }

  return len;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  uint8_t was_off = 0;
  uint32_t cmd_status;
  int8_t rssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Indicate a clear channel
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    return RF_CCA_CLEAR;
  }

  if(!rf_core_is_accessible()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("channel_clear: on() failed\n");
      if(was_off) {
        off();
      }
      return RF_CCA_CLEAR;
    }
  } else {
    if(transmitting()) {
      PRINTF("channel_clear: called while in TX\n");
      return RF_CCA_CLEAR;
    }
  }

  while(rssi == RF_CMD_CCA_REQ_RSSI_UNKNOWN || rssi == 0) {
    if(rf_core_send_cmd(CMDR_DIR_CMD(CMD_GET_RSSI), &cmd_status)
       != RF_CORE_CMD_OK) {
      break;
    }
    /* Current RSSI in bits 23:16 of cmd_status */
    rssi = (cmd_status >> 16) & 0xFF;
  }

  if(was_off) {
    off();
  }

  if(rssi >= rssi_threshold) {
    return RF_CCA_BUSY;
  }

  return RF_CCA_CLEAR;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  if(!rf_is_on()) {
    return 0;
  }

  if(channel_clear() == RF_CCA_CLEAR) {
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  int rv = 0;
  volatile rfc_dataEntry_t *entry = (rfc_dataEntry_t *)rx_data_queue.pCurrEntry;

  /* Go through all RX buffers and check their status */
  do {
    if(entry->status == DATA_ENTRY_STATUS_FINISHED) {
      rv += 1;
      process_poll(&rf_core_process);
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
   * Request the HF XOSC as the source for the HF clock. Needed before we can
   * use the FS. This will only request, it will _not_ perform the switch.
   */
  oscillators_request_hf_xosc();

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Abort, but pretend everything is OK.
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    return RF_CORE_CMD_OK;
  }

  if(rf_is_on()) {
    PRINTF("on: We were on. PD=%u, RX=0x%04x \n", rf_core_is_accessible(),
           smartrf_settings_cmd_prop_rx_adv.status);
    return RF_CORE_CMD_OK;
  }

  if(!rf_core_is_accessible()) {
    if(rf_core_power_up() != RF_CORE_CMD_OK) {
      PRINTF("on: rf_core_power_up() failed\n");

      rf_core_power_down();

      return RF_CORE_CMD_ERROR;
    }

    rf_patch_cpe_genfsk();

    if(rf_core_start_rat() != RF_CORE_CMD_OK) {
      PRINTF("on: rf_core_start_rat() failed\n");

      rf_core_power_down();

      return RF_CORE_CMD_ERROR;
    }
  }

  rf_core_setup_interrupts();

  /*
   * Trigger a switch to the XOSC, so that we can subsequently use the RF FS
   * This will block until the XOSC is actually ready, but give how we
   * requested it early on, this won't be too long a wait/
   */
  oscillators_switch_to_hf_xosc();

  if(prop_div_radio_setup() != RF_CORE_CMD_OK) {
    PRINTF("on: prop_div_radio_setup() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  if(prop_fs() != RF_CORE_CMD_OK) {
    PRINTF("on: prop_fs() failed\n");
    return RF_CORE_CMD_ERROR;
  }

  return rx_on_prop();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  rfc_dataEntry_t *entry;

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Abort, but pretend everything is OK.
   */
  if(rf_ble_is_active() == RF_BLE_ACTIVE) {
    return RF_CORE_CMD_OK;
  }

  rx_off_prop();
  rf_core_power_down();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  /* Switch HF clock source to the RCOSC to preserve power */
  oscillators_switch_to_hf_rc();

  /* We pulled the plug, so we need to restore the status manually */
  smartrf_settings_cmd_prop_rx_adv.status = RF_CORE_RADIO_OP_STATUS_IDLE;

  entry = (rfc_dataEntry_t *)rx_buf_0;
  entry->status = DATA_ENTRY_STATUS_PENDING;

  entry = (rfc_dataEntry_t *)rx_buf_1;
  entry->status = DATA_ENTRY_STATUS_PENDING;

  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    /* On / off */
    *value = rf_is_on() ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = (radio_value_t)get_channel();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    *value = get_tx_power();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    *value = rssi_threshold;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = get_rssi();

    if(*value == RF_CMD_CCA_REQ_RSSI_UNKNOWN) {
      return RADIO_RESULT_ERROR;
    } else {
      return RADIO_RESULT_OK;
    }
  case RADIO_CONST_CHANNEL_MIN:
    *value = 0;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = DOT_15_4G_CHANNEL_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = OUTPUT_POWER_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = OUTPUT_POWER_MAX;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  uint8_t was_off = 0;
  radio_result_t rv = RADIO_RESULT_OK;

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
    if(value < 0 ||
       value > DOT_15_4G_CHANNEL_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    if(get_channel() == (uint8_t)value) {
      /* We already have that very same channel configured.
       * Nothing to do here. */
      return RADIO_RESULT_OK;
    }

    set_channel((uint8_t)value);
    break;
  case RADIO_PARAM_TXPOWER:
    if(value < OUTPUT_POWER_MIN || value > OUTPUT_POWER_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    soft_off_prop();

    set_tx_power(value);

    if(soft_on_prop() != RF_CORE_CMD_OK) {
      PRINTF("set_value: soft_on_prop() failed\n");
      rv = RADIO_RESULT_ERROR;
    }

    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    rssi_threshold = (int8_t)value;
    break;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }

  /* If we reach here we had no errors. Apply new settings */
  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("set_value: on() failed (2)\n");
      return RADIO_RESULT_ERROR;
    }
  }

  if(rx_off_prop() != RF_CORE_CMD_OK) {
    PRINTF("set_value: rx_off_prop() failed\n");
    rv = RADIO_RESULT_ERROR;
  }

  if(rx_on_prop() != RF_CORE_CMD_OK) {
    PRINTF("set_value: rx_on_prop() failed\n");
    rv = RADIO_RESULT_ERROR;
  }

  /* If we were off, turn back off */
  if(was_off) {
    off();
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver prop_mode_driver = {
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
