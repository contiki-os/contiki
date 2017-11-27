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
 * \addtogroup rf-core-prop CC13xx Prop mode driver
 * @{
 *
 * \file
 * Implementation of the CC26xx/CC13xx prop mode NETSTACK_RADIO driver
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
#include "rf-core/rf-switch.h"
#include "rf-core/rf-ble.h"
#include "rf-core/rf-rat.h"
#include "rf-core/prop-mode.h"
#include "rf-core/dot-15-4g.h"
/*---------------------------------------------------------------------------*/
/* RF core and RF HAL API */
#include "hw_rfc_dbell.h"
#include "hw_rfc_pwr.h"
/*---------------------------------------------------------------------------*/
/* RF Core Mailbox API */
#include "driverlib/rf_mailbox.h"
#include "driverlib/rf_common_cmd.h"
#include "driverlib/rf_data_entry.h"
#include "driverlib/rf_prop_mailbox.h"
#include "driverlib/rf_prop_cmd.h"
/*---------------------------------------------------------------------------*/
/* CC13xxware patches */
#include "rf_patches/rf_patch_cpe_genfsk.h"
#include "rf_patches/rf_patch_rfe_genfsk.h"
/*---------------------------------------------------------------------------*/
#include "rf-core/smartrf-settings.h"
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if 0
#define PRINTF_FAIL(...)  printf(__VA_ARGS__)
#else
#define PRINTF_FAIL(...)  PRINTF(__VA_ARGS__)
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
static int8_t rssi_last      = RF_CMD_CCA_REQ_RSSI_UNKNOWN;
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
/* TX power table for the 431-527MHz band */
#ifdef PROP_MODE_CONF_TX_POWER_431_527
#define PROP_MODE_TX_POWER_431_527 PROP_MODE_CONF_TX_POWER_431_527
#else
#define PROP_MODE_TX_POWER_431_527 prop_mode_tx_power_431_527
#endif
/*---------------------------------------------------------------------------*/
/* TX power table for the 779-930MHz band */
#ifdef PROP_MODE_CONF_TX_POWER_779_930
#define PROP_MODE_TX_POWER_779_930 PROP_MODE_CONF_TX_POWER_779_930
#else
#define PROP_MODE_TX_POWER_779_930 prop_mode_tx_power_779_930
#endif
/*---------------------------------------------------------------------------*/
/* Select power table based on the frequency band */
#if DOT_15_4G_FREQUENCY_BAND_ID==DOT_15_4G_FREQUENCY_BAND_470
#define TX_POWER_DRIVER PROP_MODE_TX_POWER_431_527
#else
#define TX_POWER_DRIVER PROP_MODE_TX_POWER_779_930
#endif
/*---------------------------------------------------------------------------*/
extern const prop_mode_tx_power_config_t TX_POWER_DRIVER[];

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MAX     (TX_POWER_DRIVER[0].dbm)
#define OUTPUT_POWER_UNKNOWN 0xFFFF

/* Default TX Power - position in output_power[] */
const prop_mode_tx_power_config_t *tx_power_current = &TX_POWER_DRIVER[1];
/*---------------------------------------------------------------------------*/
#ifdef PROP_MODE_CONF_LO_DIVIDER
#define PROP_MODE_LO_DIVIDER   PROP_MODE_CONF_LO_DIVIDER
#else
#define PROP_MODE_LO_DIVIDER   0x05
#endif
/*---------------------------------------------------------------------------*/
#ifdef PROP_MODE_CONF_RX_BUF_CNT
#define PROP_MODE_RX_BUF_CNT PROP_MODE_CONF_RX_BUF_CNT
#else
#define PROP_MODE_RX_BUF_CNT 4
#endif
/*---------------------------------------------------------------------------*/
#define DATA_ENTRY_LENSZ_NONE 0
#define DATA_ENTRY_LENSZ_BYTE 1
#define DATA_ENTRY_LENSZ_WORD 2 /* 2 bytes */

/*
 * RX buffers.
 * PROP_MODE_RX_BUF_CNT buffers of RX_BUF_SIZE bytes each. The start of each
 * buffer must be 4-byte aligned, therefore RX_BUF_SIZE must divide by 4
 */
#define RX_BUF_SIZE 140
static uint8_t rx_buf[PROP_MODE_RX_BUF_CNT][RX_BUF_SIZE] CC_ALIGN(4);

/* The RX Data Queue */
static dataQueue_t rx_data_queue = { 0 };

/* Receive entry pointer to keep track of read items */
volatile static uint8_t *rx_read_entry;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SFD timestamp in RTIMER ticks */
//static volatile uint32_t last_packet_timestamp = 0;
/* XXX: don't know what exactly is this, looks like the time to Tx 3 octets */
#define TIMESTAMP_OFFSET  -(USEC_TO_RADIO(32 * 3) - 1) /* -95.75 usec */

extern int32_t rat_offset;


#if PROP_MODE_RAT_SYNC_STYLE >= PROP_MODE_RAT_SYNC_AGRESSIVE
void   rat_sync_op_start(void);
void   rat_sync_op_end(void);
rtimer_clock_t   rat_sync_check(rtimer_clock_t stamp);
#else
#define   rat_sync_op_start(...)
#define   rat_sync_op_end(...)
#define   rat_sync_check(x) x
#endif
/*---------------------------------------------------------------------------*/
/* Are we currently in poll mode? */
#ifndef RF_CORE_POLL_MODE
static uint8_t poll_mode = 0;
#else
static const uint8_t poll_mode = RF_CORE_POLL_MODE;
#endif
/*---------------------------------------------------------------------------*/
/* The outgoing frame buffer */
#define TX_BUF_PAYLOAD_LEN 180
#define TX_BUF_HDR_LEN       2

static uint8_t tx_buf[TX_BUF_HDR_LEN + TX_BUF_PAYLOAD_LEN] CC_ALIGN(4);
/*---------------------------------------------------------------------------*/
static
bool rx_is_on(void)
{
  if(!rf_core_is_accessible()) {
    return 0;
  }

  return smartrf_settings_cmd_prop_rx_adv.status == RF_CORE_RADIO_OP_STATUS_ACTIVE;
}

static inline
uint8_t rf_is_on(void){
    return rx_is_on();
}
/*---------------------------------------------------------------------------*/
static
bool transmitting(void)
{
  return smartrf_settings_cmd_prop_tx_adv.status == RF_CORE_RADIO_OP_STATUS_ACTIVE;
}
/*---------------------------------------------------------------------------*/
static
int_fast8_t read_rssi(void){
    uint32_t cmd_status;
    unsigned char attempts = 0;
    rfc_CMD_GET_RSSI_t cmd;
    int_fast8_t rssi;
    rssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;

    while((rssi == RF_CMD_CCA_REQ_RSSI_UNKNOWN || rssi == 0) && ++attempts < 10) {
      cmd.commandNo = CMD_GET_RSSI;

      if(rf_core_send_cmd((uint32_t)&cmd, &cmd_status) == RF_CORE_CMD_ERROR) {
        PRINTF("get_rssi: CMDSTA=0x%08lx\n", cmd_status);
        break;
      } else {
        /* Current RSSI in bits 23:16 of cmd_status */
        rssi = (int8_t)((cmd_status >> 16) & 0xFF);
      }
    }
    //if (poll_mode)
    // TODO is LAST_RSSI should takes this value, even when read_frame assigs it?
        rssi_last = rssi;
    return rssi;
}


static radio_value_t
get_rssi(void)
{
  int_fast8_t rssi;
  int_fast8_t was_off = 0;

  /* If we are off, turn on first */
  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CORE_CMD_OK) {
      PRINTF("get_rssi: on() failed\n");
      return RF_CMD_CCA_REQ_RSSI_UNKNOWN;
    }
  }

  rssi = read_rssi();

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
static uint8_t
get_tx_power_array_last_element(void)
{
  const prop_mode_tx_power_config_t *array = TX_POWER_DRIVER;
  uint8_t count = 0;

  while(array->tx_power != OUTPUT_POWER_UNKNOWN) {
    count++;
    array++;
  }
  return count - 1;
}
/*---------------------------------------------------------------------------*/
/* Returns the current TX power in dBm */
static radio_value_t
get_tx_power(void)
{
  return tx_power_current->dbm;
}
/*---------------------------------------------------------------------------*/
enum {
    RADIO_RESULT_NOCHANGE = RADIO_RESULT_OK-1
};
/*
 * The caller must make sure to send a new CMD_PROP_RADIO_DIV_SETUP to the
 * radio after calling this function.
 */
static int
set_tx_power(radio_value_t power)
{
  int i;

  for(i = get_tx_power_array_last_element(); i >= 0; --i) {
    if(power <= TX_POWER_DRIVER[i].dbm) {
      /*
       * Merely save the value. It will be used in all subsequent usages of
       * CMD_PROP_RADIO_DIV_SETP, including one immediately after this function
       * has returned
       */
      if (tx_power_current == &TX_POWER_DRIVER[i])
          return RADIO_RESULT_NOCHANGE;
      tx_power_current = &TX_POWER_DRIVER[i];
      return RADIO_RESULT_OK;
    }
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static
bool rf_cmd_status_is_running(volatile rfc_radioOp_t *cmd){
    uint32_t now_status = (cmd->status & RF_CORE_RADIO_OP_MASKED_STATUS);
    return (now_status == RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING);
}

#if DEBUG == 0
static
int rf_cmd_exec(rfc_radioOp_t *cmd)
{
    uint32_t cmd_status;
    /* Send Radio setup to RF Core */
    if(rf_core_send_cmd((uint32_t)cmd, &cmd_status) != RF_CORE_CMD_OK) {
      return RF_CORE_CMD_ERROR;
    }

    /* Wait until radio setup is done */
    if(rf_core_wait_cmd_done(cmd) != RF_CORE_CMD_OK) {
      return RF_CORE_CMD_ERROR;
    }
    return RF_CORE_CMD_OK;
}
#define rf_cmd_execute(cmd, name) rf_cmd_exec(cmd)
#else
static
int rf_cmd_execute(rfc_radioOp_t *cmd, const char* name )
{
    uint32_t cmd_status;
    /* Send Radio setup to RF Core */
    if(rf_core_send_cmd((uint32_t)cmd, &cmd_status) != RF_CORE_CMD_OK) {
      PRINTF("%s: CMDSTA=0x%08lx, status=0x%04x\n"
              ,name, cmd_status, cmd->status);
      return RF_CORE_CMD_ERROR;
    }

    /* Wait until radio setup is done */
    if(rf_core_wait_cmd_done(cmd) != RF_CORE_CMD_OK) {
      PRINTF("%s wait, CMDSTA=0x%08lx, status=0x%04x\n"
              ,name, cmd_status, cmd->status);
      return RF_CORE_CMD_ERROR;
    }
    return RF_CORE_CMD_OK;
}
#endif
/*---------------------------------------------------------------------------*/
static int
prop_div_radio_setup(void)
{
  rfc_radioOp_t *cmd = (rfc_radioOp_t *)&smartrf_settings_cmd_prop_radio_div_setup;

  rf_switch_select_path(RF_SWITCH_PATH_SUBGHZ);

  /* Adjust loDivider depending on the selected band */
  smartrf_settings_cmd_prop_radio_div_setup.loDivider = PROP_MODE_LO_DIVIDER;

  /* Update to the correct TX power setting */
  smartrf_settings_cmd_prop_radio_div_setup.txPower = tx_power_current->tx_power;

  /* Adjust RF Front End and Bias based on the board */
  smartrf_settings_cmd_prop_radio_div_setup.config.frontEndMode =
    RF_CORE_PROP_FRONT_END_MODE;
  smartrf_settings_cmd_prop_radio_div_setup.config.biasMode =
    RF_CORE_PROP_BIAS_MODE;

  return rf_cmd_execute(cmd, "prop_div_radio_setup: DIV_SETUP");
}
/*---------------------------------------------------------------------------*/
static uint8_t
rf_cmd_prop_rx()
{
  uint32_t cmd_status;
  volatile rfc_CMD_PROP_RX_ADV_t *cmd_rx_adv;
  int ret;

  cmd_rx_adv = (rfc_CMD_PROP_RX_ADV_t *)&smartrf_settings_cmd_prop_rx_adv;
  cmd_rx_adv->status = RF_CORE_RADIO_OP_STATUS_IDLE;

  /*
   * Set the max Packet length. This is for the payload only, therefore
   * 2047 - length offset
   */
  cmd_rx_adv->maxPktLen = DOT_4G_MAX_FRAME_LEN - cmd_rx_adv->lenOffset;

  rat_sync_op_start();
  ret = rf_core_send_cmd((uint32_t)cmd_rx_adv, &cmd_status);

  if(ret != RF_CORE_CMD_OK) {
    PRINTF("rf_cmd_prop_rx: send_cmd ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, cmd_rx_adv->status);
    return ret;
  }

  LIMITED_BUSYWAIT((cmd_rx_adv->status != RF_CORE_RADIO_OP_STATUS_ACTIVE)
                   , ENTER_RX_WAIT_TIMEOUT);

  /* Wait to enter RX */
  if(cmd_rx_adv->status != RF_CORE_RADIO_OP_STATUS_ACTIVE) {
    PRINTF("rf_cmd_prop_rx: CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, cmd_rx_adv->status);
    return RF_CORE_CMD_ERROR;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
init_rx_buffers(void)
{
  rfc_dataEntry_t *entry;
  int i;

  for(i = 0; i < PROP_MODE_RX_BUF_CNT; i++) {
    entry = (rfc_dataEntry_t *)rx_buf[i];
    entry->status = DATA_ENTRY_STATUS_PENDING;
    entry->config.type = DATA_ENTRY_TYPE_GEN;
    entry->config.lenSz = DATA_ENTRY_LENSZ_WORD;
    entry->length = RX_BUF_SIZE - 8;
    entry->pNextEntry = rx_buf[i + 1];
  }

  ((rfc_dataEntry_t *)rx_buf[PROP_MODE_RX_BUF_CNT - 1])->pNextEntry = rx_buf[0];
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
    rat_sync_op_end();
    return RF_CORE_CMD_OK;
  }

  /* Send a CMD_ABORT command to RF Core */
  if(rf_core_send_cmd(CMDR_DIR_CMD(CMD_ABORT), &cmd_status) != RF_CORE_CMD_OK) {
    PRINTF("rx_off_prop: CMD_ABORT status=0x%08lx\n", cmd_status);
    /* Continue nonetheless */
  }

  LIMITED_BUSYWAIT(rf_is_on(), RF_TURN_OFF_WAIT_TIMEOUT);

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

  rat_sync_op_end();

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
  rfc_radioOp_t *cmd = (rfc_radioOp_t *)&smartrf_settings_cmd_fs;
  return rf_cmd_execute(cmd, "prop_fs: CMD_FS");
}

static
int prop_txpower(void){
    rfc_CMD_SET_TX_POWER_t cmd_power =
    {
      .commandNo    = CMD_SET_TX_POWER,
      .txPower      = tx_power_current->tx_power,
    };

    rfc_radioOp_t *cmd = (rfc_radioOp_t *)&cmd_power;
    return rf_cmd_execute(cmd, "prop_txpower: CMD_SET_TX_POWER");
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

  LIMITED_BUSYWAIT(rf_cmd_status_is_running(cmd), RF_TURN_OFF_WAIT_TIMEOUT);
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
const struct rf_rat_controler rfrat_prop_mode = {
    &(rf_is_on), &(on), &(off)
};
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  lpm_register_module(&prop_lpm_module);

  if(ti_lib_chipinfo_chip_family_is_cc13xx() == false) {
    return RF_CORE_CMD_ERROR;
  }

  /* Initialise RX buffers */
  memset(rx_buf, 0, sizeof(rx_buf));

  /* Set of RF Core data queue. Circular buffer, no last entry */
  rx_data_queue.pCurrEntry = rx_buf[0];
  rx_data_queue.pLastEntry = NULL;

  /* Initialize current read pointer to first element (used in ISR) */
  rx_read_entry = rx_buf[0];

  smartrf_settings_cmd_prop_rx_adv.pQueue = &rx_data_queue;
  smartrf_settings_cmd_prop_rx_adv.pOutput = (uint8_t *)&rx_stats;

  set_channel(RF_CORE_CHANNEL);

  if(on() != RF_CORE_CMD_OK) {
    PRINTF("init: on() failed\n");
    return RF_CORE_CMD_ERROR;
  }
  rf_rat_monitor_init(&rfrat_prop_mode);
  //* have probed that RFcore ok, turn it off for power-save
  off();

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
  return 0;
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
  rf_core_cmd_done_en(false, poll_mode);

  ret = rf_core_send_cmd((uint32_t)cmd_tx_adv, &cmd_status);

  if(ret) {
    /* If we enter here, TX actually started */
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

    watchdog_periodic();

    /* Idle away while the command is running */
    while((cmd_tx_adv->status & RF_CORE_RADIO_OP_MASKED_STATUS)
          == RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING) {
        /* Note: import from ieee-mode:
         * for now sleeping while Tx'ing in polling mode is disabled.
         * To enable it:
         *  1) make the `lpm_sleep()` call here unconditional;
         *  2) change the radio ISR priority to allow radio ISR to interrupt rtimer ISR.
         */
        if(!poll_mode) {
      lpm_sleep();
    }
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
  rf_core_cmd_done_dis(poll_mode);

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

      int8_t rssi = data_ptr[len];
      rssi_last = rssi;
      if(!poll_mode) {
        /* Not in poll mode: packetbuf should not be accessed in interrupt context.
         * In poll mode, the last packet RSSI and link quality can be obtained through
         * RADIO_PARAM_LAST_RSSI and RADIO_PARAM_LAST_LINK_QUALITY */
      packetbuf_set_attr(PACKETBUF_ATTR_RSSI, (int)rssi ) ;
      packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, 0x7F);
      }//if(!poll_mode)
    }

    //! TODO in PollMode rx_stats may inconsistent, when ISR looks over multiple IRQs
    /* correct timestamp so that it refers to the end of the SFD */
    rf_rat_last_timestamp(rx_stats.timeStamp + TIMESTAMP_OFFSET);

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

  rssi = read_rssi();

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
    int_fast8_t rssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;

  if(!rx_is_on()) {
    return 0;
  }

  rssi = read_rssi();

  return (rssi >= rssi_threshold);
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  int rv = 0;
#if RF_CORE_PENDING == RF_CORE_PENDING_READS
  volatile rfc_dataEntry_t *entry = (rfc_dataEntry_t *)rx_read_entry;
#else
  volatile rfc_dataEntry_t *entry = (rfc_dataEntry_t *)rx_data_queue.pCurrEntry;
#endif

  /* Go through all RX buffers and check their status */
  do {
    if(entry->status == DATA_ENTRY_STATUS_FINISHED) {
      rv += 1;
      if(!poll_mode){
      process_poll(&rf_core_process);
    }
    }
#if RF_CORE_PENDING == RF_CORE_PENDING_READS
    else
        break;
#endif

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
    return RF_CORE_CMD_OK;
  }

  /*
   * Request the HF XOSC as the source for the HF clock. Needed before we can
   * use the FS. This will only request, it will _not_ perform the switch.
   */
  oscillators_request_hf_xosc();

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

    /* Keep track of RF Core mode */
    rf_core_set_modesel();

    /* Apply patches to radio core */
    rf_patch_cpe_genfsk();
    while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
    rf_patch_rfe_genfsk();

    /* Initialize bus request */
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) =
      CMDR_DIR_CMD_1BYTE(CMD_BUS_REQUEST, 1);

    /* set VCOLDO reference */
    ti_lib_rfc_adi3vco_ldo_voltage_mode(true);

    /* Let CC13xxware automatically set a correct value for RTRIM for us */
    ti_lib_rfc_rtrim((rfc_radioOp_t *)&smartrf_settings_cmd_prop_radio_div_setup);

    /* Make sure BUS_REQUEST is done */
    while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

    if(rf_core_start_rat() != RF_CORE_CMD_OK) {
      PRINTF("on: rf_core_start_rat() failed\n");

      rf_core_power_down();

      return RF_CORE_CMD_ERROR;
    }
    rf_rat_check_overflow(true);
  }

  rf_core_setup_interrupts(poll_mode);

  init_rx_buffers();

  /*
   * Trigger a switch to the XOSC, so that we can subsequently use the RF FS
   * This will block until the XOSC is actually ready, but give how we
   * requested it early on, this won't be too long a wait/
   */
  oscillators_switch_to_hf_xosc();

  //* apply setup radio chanel settings
  int ret = soft_on_prop();
  return ret;
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
    return RF_CORE_CMD_OK;
  }

  rx_off_prop();
  rf_core_power_down();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  /* Switch HF clock source to the RCOSC to preserve power */
  oscillators_switch_to_hf_rc();

  /* We pulled the plug, so we need to restore the status manually */
  smartrf_settings_cmd_prop_rx_adv.status = RF_CORE_RADIO_OP_STATUS_IDLE;

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
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    //*  filtering need to implements
    //*  *value |= RADIO_RX_MODE_ADDRESS_FILTER;
    //*  AUTOACK not supports in pop-mode
    //*  *value |= RADIO_RX_MODE_AUTOACK;
    if(poll_mode) {
      *value |= RADIO_RX_MODE_POLL_MODE;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    *value = 0;
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
  case RADIO_PARAM_LAST_RSSI:
      *value = rssi_last;
      if(rssi_last == RF_CMD_CCA_REQ_RSSI_UNKNOWN) {
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
    *value = TX_POWER_DRIVER[get_tx_power_array_last_element()].dbm;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = OUTPUT_POWER_MAX;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
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

typedef int (*radio_prop_func)(void);
static
radio_result_t update_prop(radio_prop_func f){
    bool is_rx = rx_is_on();
    if (is_rx){
        /* If we reach here we had no errors. Apply new settings */
        if(rx_off_prop() != RF_CORE_CMD_OK) {
        PRINTF("set_value: stop rf failed\n");
        //* fails to stop currrent op, changes take effect on next on()
        return RADIO_RESULT_ERROR;
    }
    }
    else if (transmitting()){
        while (transmitting());
    }
    else {
        /* If off, the new configuration will be applied the next time radio is started */
        return RADIO_RESULT_OK;
    }

    if(f() != RF_CORE_CMD_OK) {
      PRINTF("update_prop: prop failed\n");
      return RADIO_RESULT_ERROR;
    }

    /* Restart the radio timer (RAT).
       This causes resynchronization between RAT and RTC: useful for TSCH. */
    if(rf_core_restart_rat() == RF_CORE_CMD_OK) {
        rf_rat_check_overflow(false);
    }

    if (is_rx){
        if (rx_on_prop() == RF_CORE_CMD_OK)
            return RADIO_RESULT_OK;
        else {
            PRINTF("set_value:rx restart failed\n");
            return RADIO_RESULT_ERROR;
        }
    }
    else
        return RADIO_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  //uint8_t was_off = 0;
  int rv = RADIO_RESULT_OK;

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
    return update_prop(&(prop_fs));

  case RADIO_PARAM_TXPOWER:
    if(value < TX_POWER_DRIVER[get_tx_power_array_last_element()].dbm ||
       value > OUTPUT_POWER_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    rv = set_tx_power(value);
    if (rv < (int)RADIO_RESULT_OK)
        return RADIO_RESULT_OK;
    if (rv == RADIO_RESULT_OK)
      return update_prop(&(prop_txpower));
    return rv;

  case RADIO_PARAM_RX_MODE:
      if(value & ~(RADIO_RX_MODE_POLL_MODE)) {
        return RADIO_RESULT_INVALID_VALUE;
      }
#ifndef RF_CORE_POLL_MODE
      poll_mode = (value & RADIO_RX_MODE_POLL_MODE) != 0;
    return RADIO_RESULT_OK;
#else
      if ( (poll_mode != 0) == ((value & RADIO_RX_MODE_POLL_MODE) != 0) )
          return RADIO_RESULT_OK;
      else
          return RADIO_RESULT_INVALID_VALUE;
#endif

  case RADIO_PARAM_TX_MODE:
    if(value & ~(RADIO_TX_MODE_SEND_ON_CCA)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    return set_send_on_cca((value & RADIO_TX_MODE_SEND_ON_CCA) != 0);

  case RADIO_PARAM_CCA_THRESHOLD:
    rssi_threshold = (int8_t)value;
    break;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }

  PRINTF("set_value: strange\n");
  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
    if(param == RADIO_PARAM_LAST_PACKET_TIMESTAMP) {
      if(size != sizeof(rtimer_clock_t) || !dest) {
        return RADIO_RESULT_INVALID_VALUE;
      }
      rtimer_clock_t stamp = rf_rat_calc_last_rttime();
      *(rtimer_clock_t *)dest = stamp;

      return RADIO_RESULT_OK;
    }

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

#if PROP_MODE_RAT_SYNC_STYLE >= PROP_MODE_RAT_SYNC_AGRESSIVE
//static
struct {
    rtimer_clock_t     op_start;
    rtimer_clock_t     op_end;
}   rat_sync = {0, ~0};

void   rat_sync_op_start(void){
    if (rat_sync.op_end != rat_sync.op_start){
        //* prev operation have finished
    rat_sync.op_start = RTIMER_NOW();
    rat_sync.op_end = rat_sync.op_start;
}
}

void   rat_sync_op_end(void){
    if (rat_sync.op_end == rat_sync.op_start){
    rat_sync.op_end = RTIMER_NOW();
}
}

bool rat_sync_validate(rtimer_clock_t stamp){
    if ( RTIMER_CLOCK_LT(stamp, rat_sync.op_start) ){
        //* looks RAT time points before rx start
        PRINTF("rat_sync_check: stamp %lu violates rx start %lu\n"
                , stamp, rat_sync.op_start);
        return false;
    }
    else {
        if (rat_sync.op_start == rat_sync.op_end){
            rtimer_clock_t now = RTIMER_NOW();
            if (RTIMER_CLOCK_LT(now, stamp)){
                //* looks RAT time points after now
                PRINTF("rat_sync_check: stamp %lu violates now %lu\n"
                        , stamp, now);
                return false;
            }
        }
        else if (RTIMER_CLOCK_LT(rat_sync.op_end, stamp)){
                //* looks RAT time points after rx end
                PRINTF("rat_sync_check: stamp %lu violates rx end %lu\n"
                    , stamp, rat_sync.op_end);
                return false;
        }
    }
    return true;
}

int32_t rat_sync_miss(void){
    rf_rat_time_t rat = rf_rat_now();
    rtimer_clock_t now = RTIMER_NOW();
    rf_rat_last_timestamp(rat);
    rtimer_clock_t rat_now = rf_rat_calc_last_rttime();
    if (rat_now != now){
        PRINTF("rat_sync_validate: RAT[%lu] unsync RT [%lu]\n"
                , rat_now, now);
    }
    return rat_now - now;
}

rtimer_clock_t rat_sync_check(rtimer_clock_t stamp){
    if (rat_sync_validate(stamp))
        return stamp;
    //* here cause RAT stamp have strange value.
    //* resync RAT, and try correct stamp
    rf_rat_debug_dump();
    if(!rf_core_is_accessible()) {
      return stamp;
    }
    int32_t rat_miss = rat_sync_miss();
    if (rat_miss == 0){
        PRINTF("rat_sync_check: sync ok, unckown stamp fail\n");
        rf_rat_debug_dump();
        return stamp;
    }
    bool was_on = rx_is_on();
    if (was_on)
    rx_off_prop();
    //int32_t last_offset = rat_offset;
    if(rf_core_restart_rat() == RF_CORE_CMD_OK) {
        rf_rat_check_overflow(true);
    }
    if (was_on)
        rx_on_prop();
    int32_t rat_remiss = rat_sync_miss();
    if (rat_remiss != 0)
        PRINTF_FAIL("rat_sync_check: failed resync from %ld -> %ld\n", rat_miss , rat_remiss);
    return stamp - rat_miss;
}

#endif

/**
 * @}
 */
