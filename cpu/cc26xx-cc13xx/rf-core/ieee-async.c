/*
 * Copyright (c) 2015, Scanimetrics - http://www.scanimetrics.com
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
/**
 * \addtogroup rf-async
 * @{
 *
 * \defgroup rf-async-ieee CC26xx asynchronous IEEE driver
 *
 * @{
 *
 * \file
 * Implementation of CC26xx asynchronous IEEE driver
 *
 */
/*---------------------------------------------------------------------------*/
#include "net/netstack.h"
#if NETSTACK_USE_RFASYNC
/*---------------------------------------------------------------------------*/
/* Contiki Core / System Interfaces */
#include "contiki-conf.h"
#include "ieee-common.h"
#include "sys/process.h"
#include "sys/energest.h"
#include "dev/radio-async.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
/*---------------------------------------------------------------------------*/
/* CC26xx specific interfaces */
#include "ti-lib.h"
#include "lpm.h"
#include "dev/oscillators.h"
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
#include "rf-async.h"
#include "rf-core.h"
/*---------------------------------------------------------------------------*/
/* Builtin Libraries */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#ifdef __GNUC__
#define CC_ALIGN_ATTR(n) __attribute__ ((aligned(n)))
#else
#define CC_ALIGN_ATTR(n)
#endif
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef RF_IEEE_CONF_TX_POWER_DBM
#define RF_IEEE_TX_POWER_DBM 5
#else
#define RF_IEEE_TX_POWER_DBM RF_IEEE_CONF_TX_POWER_DBM
#endif

#define ACQUIRE_SUCCESS 0
#define ACQUIRE_FAILURE 1

#define UNLOCKED 0
#define LOCKED   1

#define READ_ENTRY_NEXT(e) ((volatile rfc_dataEntry_t *)((e)->pNextEntry))
/*---------------------------------------------------------------------------*/
/* Static Function Prototypes */
static int lock_acquire(volatile uint32_t *lock);
static void lock_release(volatile uint32_t *lock);
static int cmd_acquire(void);
static void cmd_release(void);
static int tx_packet_acquire(void);
static void tx_packet_release(void);
static uint_fast8_t send_cca_req(rf_async_cb_func cb, void *ptr);
static uint_fast8_t send_ieee_tx(rf_async_cb_func cb, void *ptr);
static void channel_check_cb(uint32_t status, uint32_t cmd_addr, void *ptr);
static enum radio_async_result load_tx_buf(const void *payload, uint16_t len);
static enum radio_async_result tx_packet(radio_cb_func cb, void *ptr);
static void tx_packet_cb(uint32_t status, uint32_t cmd_addr, void *ptr);
static void user_cb_exit(rtimer_clock_t now, enum radio_async_done status);
static bool cca_req_channel_is_clear(rfc_CMD_IEEE_CCA_REQ_t *cmd);
static bool cca_req_is_recieving(rfc_CMD_IEEE_CCA_REQ_t *cmd);
static void rf_rx_done_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr);
static uint_fast8_t rf_cmd_ieee_rx(void);
static void rf_radio_setup_done_cb(uint32_t cmdsta, uint32_t cmd_addr,
                                   void *ptr);
static uint_fast8_t rf_radio_setup(void);
static void boot_done_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr);
static void fs_powerdown_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr);
static void rx_stop_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr);
static uint8_t request(void);
static void rx_isr_cb(void);

static enum radio_async_result send(const void *payload, uint16_t len,
                                    radio_cb_func cb, void *ptr);
static enum radio_async_result transmit(radio_cb_func cb, void *ptr);
static enum radio_async_result prepare(const void *payload, uint16_t len);
static enum radio_async_result channel_clear(radio_cb_func cb, void *ptr);
static enum radio_async_result receiving_packet(radio_cb_func cb, void *ptr);
static enum radio_async_result on(radio_cb_func cb, void *ptr);
static enum radio_async_result off(radio_cb_func cb, void *ptr);
static enum radio_async_result init(void);
static void set_receiver_callbacks(void (*ack_cb)(uint8_t),
                                   void (*rx_cb)(bool));
static enum radio_async_result start_ack_detect(uint8_t seqno);
static enum radio_async_result stop_ack_detect(void);
/*---------------------------------------------------------------------------*/
/* Static Data */

/* Memory for the foreground commands we are interested in running */
static union {
  rfc_radioOp_t common;
  rfc_CMD_GET_RSSI_t get_rssi;
  rfc_CMD_IEEE_CCA_REQ_t iee_cca_req;
  rfc_CMD_IEEE_TX_t ieee_tx;
  rfc_CMD_RADIO_SETUP_t radio_setup;
  rfc_CMD_SET_TX_POWER_t set_tx_power;
  rfc_CMD_FS_POWERDOWN_t fs_powerdown;
} fg_cmd_block;

static volatile struct {
  uint32_t lock;
  radio_cb_func cb;
  void *ptr;
} active_cmd;

static struct {
  uint8_t tx_buf[IEEE_TX_BUF_MAX_LEN] CC_ALIGN_ATTR(4);
  uint16_t tx_len;
  volatile uint32_t lock;
} active_tx;

static volatile struct {
  void (*cb)(uint8_t seqno);
  uint8_t seqno;
  bool enabled;
} ack_detect;

static void (*rx_user_cb)(bool pending);
/*
 * A buffer to send a CMD_IEEE_RX and to subsequently monitor its status
 * Do not use this buffer for any commands other than CMD_IEEE_RX
 */
static uint8_t cmd_ieee_rx_buf[RF_CMD_BUFFER_SIZE] CC_ALIGN_ATTR(4);

const output_config_t *tx_power_current;

LPM_MODULE(cc26xx_rf_lpm_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
/* TODO - Should replace with a generic locking interface, we're going to see
   this sort of lock / unlock code duplicated alot otherwise */
static int
lock_acquire(volatile uint32_t *lock)
{
  int ret = ACQUIRE_FAILURE;
  bool interrupts_disabled = ti_lib_int_master_disable();

  if(*lock == UNLOCKED) {
    *lock = LOCKED;
    ret = ACQUIRE_SUCCESS;
  }

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
lock_release(volatile uint32_t *lock)
{
  *lock = UNLOCKED;
}
/*---------------------------------------------------------------------------*/
static int
cmd_acquire(void)
{
  return lock_acquire(&(active_cmd.lock));
}
/*---------------------------------------------------------------------------*/
static void
cmd_release(void)
{
  lock_release(&(active_cmd.lock));
}
/*---------------------------------------------------------------------------*/
static int
tx_packet_acquire(void)
{
  return lock_acquire(&(active_tx.lock));
}
/*---------------------------------------------------------------------------*/
static void
tx_packet_release(void)
{
  lock_release(&(active_tx.lock));
}
/*---------------------------------------------------------------------------*/
static void
user_cb_exit(rtimer_clock_t now, enum radio_async_done status)
{
  struct radio_async_cb_info info;
  radio_cb_func user_cb;
  void *user_ptr;

  info.time = now;
  info.status = status;

  user_cb = active_cmd.cb;
  user_ptr = active_cmd.ptr;

  cmd_release();
  user_cb(info, user_ptr);
}
/*---------------------------------------------------------------------------*/
static bool
cca_req_channel_is_clear(rfc_CMD_IEEE_CCA_REQ_t *cmd)
{
  return cmd->ccaInfo.ccaState != RF_CMD_CCA_REQ_CCA_STATE_BUSY;
}
/*---------------------------------------------------------------------------*/
static bool
cca_req_is_recieving(rfc_CMD_IEEE_CCA_REQ_t *cmd)
{
  return cmd->ccaInfo.ccaState == RF_CMD_CCA_REQ_CCA_STATE_BUSY;
}
/*---------------------------------------------------------------------------*/
static uint_fast8_t
send_cca_req(rf_async_cb_func cb, void *ptr)
{
  rfc_CMD_IEEE_CCA_REQ_t *cmd = &(fg_cmd_block.iee_cca_req);

  /* TODO - see about initilizing this structure more efficiently */
  rf_async_init_radio_op((rfc_radioOp_t *)cmd, sizeof(*cmd), CMD_IEEE_CCA_REQ);

  return rf_async_send_cmd((uint32_t)cmd, cb, ptr, 0);
}
/*---------------------------------------------------------------------------*/
static void
channel_check_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{
  rfc_CMD_IEEE_CCA_REQ_t *cmd = (rfc_CMD_IEEE_CCA_REQ_t *)cmd_addr;
  bool (*check_cca)(rfc_CMD_IEEE_CCA_REQ_t *) = ptr;

  rtimer_clock_t now;
  enum radio_async_done done_status;

  now = RTIMER_NOW();
  done_status = RADIO_ASYNC_DONE_ERR;

  if(!rf_async_cmdsta_ok(cmdsta)) {
    PRINTF("channel_check err: 0x%lx\n", cmdsta);
    done_status = RADIO_ASYNC_DONE_ERR;
  } else if(cmd->currentRssi == RF_CMD_CCA_REQ_RSSI_UNKNOWN) {

    /* Try Again... */
    if(send_cca_req(channel_check_cb, ptr) == RF_CORE_CMD_OK) {
      return;
    }
    PRINTF("cca retry fail\n");
    done_status = RADIO_ASYNC_DONE_ERR;
  } else if(check_cca(cmd)) {
    done_status = RADIO_ASYNC_DONE_POSITIVE;
  } else {
    done_status = RADIO_ASYNC_DONE_NEGATIVE;
  }

  user_cb_exit(now, done_status);
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
channel_clear(radio_cb_func cb, void *ptr)
{
  uint_fast8_t result;

  if(cmd_acquire() == ACQUIRE_FAILURE) {
    return RADIO_ASYNC_RESULT_IN_USE;
  }

  active_cmd.cb = cb;
  active_cmd.ptr = ptr;
  result = send_cca_req(channel_check_cb, cca_req_channel_is_clear);

  if(result != RF_CORE_CMD_OK) {
    cmd_release();
    return RADIO_ASYNC_RESULT_ERROR;
  }

  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
receiving_packet(radio_cb_func cb, void *ptr)
{
  uint_fast8_t result;

  if(cmd_acquire() == ACQUIRE_FAILURE) {
    return RADIO_ASYNC_RESULT_IN_USE;
  }

  active_cmd.cb = cb;
  active_cmd.ptr = ptr;
  result = send_cca_req(channel_check_cb, cca_req_is_recieving);

  if(result != RF_CORE_CMD_OK) {
    cmd_release();
    return RADIO_ASYNC_RESULT_ERROR;
  }

  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static uint_fast8_t
send_ieee_tx(rf_async_cb_func cb, void *ptr)
{
  rfc_CMD_IEEE_TX_t *cmd = &(fg_cmd_block.ieee_tx);

  rf_async_init_radio_op((rfc_radioOp_t *)cmd, sizeof(*cmd), CMD_IEEE_TX);
  cmd->payloadLen = active_tx.tx_len;
  cmd->pPayload = active_tx.tx_buf + IEEE_TX_BUF_HDR_LEN;

  return rf_async_send_cmd((uint32_t)cmd, cb, ptr, IRQ_LAST_FG_COMMAND_DONE);
}
/*---------------------------------------------------------------------------*/
static void
tx_packet_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{
  rfc_CMD_IEEE_TX_t *cmd = (rfc_CMD_IEEE_TX_t *)cmd_addr;

  rtimer_clock_t now;
  enum radio_async_done done_status;
  bool cmd_status_ok;

  now = RTIMER_NOW();
  done_status = RADIO_ASYNC_DONE_ERR;

  cmd_status_ok = (cmd->status == RF_CORE_RADIO_OP_STATUS_IEEE_DONE_OK);

  if(rf_async_cmdsta_ok(cmdsta) && cmd_status_ok) {
    done_status = RADIO_ASYNC_DONE_OK;
  } else {
    PRINTF("txerr: 0x%lx 0x%x\n", cmdsta, cmd->status);
    done_status = RADIO_ASYNC_DONE_ERR;
  }

  tx_packet_release();
  user_cb_exit(now, done_status);
}
/*---------------------------------------------------------------------------*/
/**
 * \note it is assumed we have the appropriate locks
 */
static enum radio_async_result
tx_packet(radio_cb_func cb, void *ptr)
{
  uint_fast8_t result;

  active_cmd.cb = cb;
  active_cmd.ptr = ptr;

  result = send_ieee_tx(tx_packet_cb, NULL);

  if(result != RF_CORE_CMD_OK) {
    return RADIO_ASYNC_RESULT_ERROR;
  }

  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \note it is assumed we have the appropriate locks
 */
static enum radio_async_result
load_tx_buf(const void *payload, uint16_t len)
{
  if(len > IEEE_TX_BUF_PAYLOAD_LEN) {
    return RADIO_ASYNC_RESULT_ERROR;
  }

  active_tx.tx_len = len;
  memcpy(active_tx.tx_buf + IEEE_TX_BUF_HDR_LEN, payload, len);

  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
prepare(const void *payload, uint16_t len)
{
  enum radio_async_result load_ret;

  if(tx_packet_acquire() == ACQUIRE_FAILURE) {
    return RADIO_ASYNC_RESULT_IN_USE;
  }

  load_ret = load_tx_buf(payload, len);

  tx_packet_release();

  return load_ret;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
transmit(radio_cb_func cb, void *ptr)
{
  enum radio_async_result err = RADIO_ASYNC_RESULT_ERROR;
  enum radio_async_result ret_tx;

  if(tx_packet_acquire() == ACQUIRE_FAILURE) {
    err = RADIO_ASYNC_RESULT_IN_USE;
    goto fail2;
  }
  if(cmd_acquire() == ACQUIRE_FAILURE) {
    err = RADIO_ASYNC_RESULT_IN_USE;
    goto fail1;
  }

  ret_tx = tx_packet(cb, ptr);

  if(ret_tx != RADIO_ASYNC_RESULT_OK) {
    err = ret_tx;
    goto fail0;
  }

  return ret_tx;

fail0:
  cmd_release();
fail1:
  tx_packet_release();
fail2:
  return err;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
send(const void *payload, uint16_t len, radio_cb_func cb, void *ptr)
{
  enum radio_async_result err = RADIO_ASYNC_RESULT_ERROR;
  enum radio_async_result ret_load;
  enum radio_async_result ret_tx;

  if(tx_packet_acquire() == ACQUIRE_FAILURE) {
    err = RADIO_ASYNC_RESULT_IN_USE;
    goto fail2;
  }
  if(cmd_acquire() == ACQUIRE_FAILURE) {
    err = RADIO_ASYNC_RESULT_IN_USE;
    goto fail1;
  }

  ret_load = load_tx_buf(payload, len);

  if(ret_load != RADIO_ASYNC_RESULT_OK) {
    err = ret_load;
    goto fail0;
  }

  ret_tx = tx_packet(cb, ptr);

  if(ret_tx != RADIO_ASYNC_RESULT_OK) {
    err = ret_tx;
    goto fail0;
  }

  return ret_tx;

fail0:
  cmd_release();
fail1:
  tx_packet_release();
fail2:
  return err;
}
/*---------------------------------------------------------------------------*/
static int
read_frame(void *buf, unsigned short buf_len)
{
	int8_t rssi;
	uint8_t lqi;
	uint32_t rat_timestamp;
	int len;

  len = ieee_common_read_frame(buf, buf_len, &rssi, &lqi, &rat_timestamp);

  if(len == 0) {
  	return 0;
  }

  packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
  packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, lqi);
  RIMESTATS_ADD(llrx);

  return len;
}
/*---------------------------------------------------------------------------*/
static void
rf_rx_done_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{
  rtimer_clock_t now;
  enum radio_async_done done_status;

  now = RTIMER_NOW();
  done_status = RADIO_ASYNC_DONE_ERR;

  if(rf_async_cmdsta_ok(cmdsta)) {
    done_status = RADIO_ASYNC_DONE_OK;
  } else {
    PRINTF("rf setup err: 0x%lx\n", cmdsta);
    done_status = RADIO_ASYNC_DONE_ERR;
  }

  /* The regular driver waits for the status of the command to turn
     to active before giving control to the user. There is no interrupt we
     can wait for to indicate this so instead we just set a reasonable
     TIME_UNTIL_RX_STABLE. */

  user_cb_exit(now, done_status);
}
/*---------------------------------------------------------------------------*/
static uint_fast8_t
rf_cmd_ieee_rx(void)
{
  return rf_async_send_cmd((uint32_t)cmd_ieee_rx_buf, rf_rx_done_cb, NULL, 0);
}
/*---------------------------------------------------------------------------*/
static void
rf_radio_setup_done_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{

  if(!rf_async_cmdsta_ok(cmdsta)) {
    PRINTF("setup err: 0x%lx\n", cmdsta);
    user_cb_exit(RTIMER_NOW(), RADIO_ASYNC_DONE_ERR);
    return;
  }

  if(rf_cmd_ieee_rx() != RF_CORE_CMD_OK) {
    PRINTF("rf_cmd_ieee_rx failed\n");
    user_cb_exit(RTIMER_NOW(), RADIO_ASYNC_DONE_ERR);
    return;
  }
}
/*---------------------------------------------------------------------------*/
static uint_fast8_t
rf_radio_setup()
{
  rfc_CMD_RADIO_SETUP_t *cmd = (&fg_cmd_block.radio_setup);

  rf_async_init_radio_op((rfc_radioOp_t *)cmd, sizeof(*cmd), CMD_RADIO_SETUP);

  cmd->txPower.IB = tx_power_current->register_ib;
  cmd->txPower.GC = tx_power_current->register_gc;
  cmd->txPower.tempCoeff = tx_power_current->temp_coeff;
  cmd->pRegOverride = (uint32_t *)ieee_common_get_overrides();
  cmd->mode = 1;

  return rf_async_send_cmd((uint32_t)cmd, rf_radio_setup_done_cb, NULL,
                           IRQ_LAST_COMMAND_DONE);
}
/*---------------------------------------------------------------------------*/
static void
boot_done_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{
  if(!rf_async_cmdsta_ok(cmdsta)) {
    PRINTF("boot err %lu\n", cmdsta);
    user_cb_exit(RTIMER_NOW(), RADIO_ASYNC_DONE_ERR);
    return;
  }

  /* is there any issue with calling this from an ISR? */
  oscillators_switch_to_hf_xosc();

  if(rf_radio_setup() != RF_CORE_CMD_OK) {
    PRINTF("rf_radio_setup failed\n");
    user_cb_exit(RTIMER_NOW(), RADIO_ASYNC_DONE_ERR);
    return;
  }
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
on(radio_cb_func cb, void *ptr)
{
  if(cmd_acquire() == ACQUIRE_FAILURE) {
    return RADIO_ASYNC_RESULT_IN_USE;
  }

  /*
   * Request the HF XOSC as the source for the HF clock. Needed before we can
   * use the FS. This will only request, it will _not_ perform the switch.
   */
  oscillators_request_hf_xosc();

  if(ieee_common_rf_is_on((rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf)) {
    /* We would have problems if the RF was already on! */
    goto fail;
  }

  ieee_common_init_rx_buffers();

  /* the original driver disables interrupts on power off and re-enables
     on powering on. It doesn't seem to be necessarry however. */

  active_cmd.cb = cb;
  active_cmd.ptr = ptr;

  if(rf_async_boot(boot_done_cb, NULL) != RF_CORE_CMD_OK) {
    PRINTF("boot failure\n");
    goto fail;
  }

  return RADIO_ASYNC_RESULT_OK;
fail:
  cmd_release();
  return RADIO_ASYNC_RESULT_ERROR;
}
/*---------------------------------------------------------------------------*/
static void
fs_powerdown_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{
  rtimer_clock_t now;
  enum radio_async_done done_status;

  now = RTIMER_NOW();
  done_status = RADIO_ASYNC_DONE_ERR;

  if(!rf_async_cmdsta_ok(cmdsta) || !rf_async_cmd_done_ok(cmd_addr)) {
    done_status = RADIO_ASYNC_DONE_ERR;
  } else {
    /* TODO - check to see if it is really reasonable to do this from an ISR.
       this function involves polling on the PRCM registers while domains and
       clocks power down. */
    rf_async_power_down();

    /* TODO - this may also take too long to belong in an interrupt... */
    oscillators_switch_to_hf_rc();

    done_status = RADIO_ASYNC_DONE_OK;
  }

  user_cb_exit(now, done_status);
}
/*---------------------------------------------------------------------------*/
static void
rx_stop_cb(uint32_t cmdsta, uint32_t cmd_addr, void *ptr)
{
  rfc_CMD_FS_POWERDOWN_t *cmd = &(fg_cmd_block.fs_powerdown);
  if(!rf_async_cmdsta_ok(cmdsta)) {
    user_cb_exit(RTIMER_NOW(), RADIO_ASYNC_DONE_ERR);
    return;
  }

  if(rf_async_fs_powerdown(cmd, fs_powerdown_cb, ptr) != RF_CORE_CMD_OK) {
    PRINTF("fspd failed\n");
    user_cb_exit(RTIMER_NOW(), RADIO_ASYNC_DONE_ERR);
    return;
  }
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
off(radio_cb_func cb, void *ptr)
{
  if(cmd_acquire() == ACQUIRE_FAILURE) {
    return RADIO_ASYNC_RESULT_IN_USE;
  }

  active_cmd.cb = cb;
  active_cmd.ptr = ptr;

  if(!ieee_common_rf_is_on((rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf)) {
    /* we will have major problems if the rf is not on! */
    goto fail;
  }

  if(rf_async_stop_bg(rx_stop_cb, NULL) != RF_CORE_CMD_OK) {
    goto fail;
  }

  return RADIO_ASYNC_RESULT_OK;
fail:
  cmd_release();
  return RADIO_ASYNC_RESULT_ERROR;
}
/*---------------------------------------------------------------------------*/
static void
rx_isr_cb(void)
{
  volatile rfc_dataEntry_t *entry = ieee_common_last_data_entry();

  /* Note that, if the system is extremely busy, we might end up skipping over
     some packets and perhaps missing important things such as ACKs. I doubt
     this if very likley so for now we don't try to account for this. */

  if(!ieee_common_packet_finished(entry)) {
    PRINTF("rx_isr: no pkt\n");
    return;
  }

  if(ack_detect.enabled) {
    if(ieee_common_is_ack(entry, ack_detect.seqno)) {
      stop_ack_detect();
      ack_detect.cb(ack_detect.seqno);
    }
  } else {
    rx_user_cb(ieee_common_pending_is_set(entry));
  }
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
start_ack_detect(uint8_t seqno)
{
  ack_detect.enabled = true;
  ack_detect.seqno = seqno;
  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
stop_ack_detect(void)
{
  ack_detect.enabled = false;
  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
get_value(radio_param_t par, radio_value_t *val)
{
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;

  switch(par) {
  case RADIO_PARAM_PAN_ID:
    *val = (radio_value_t)cmd->localPanID;
    break;
  case RADIO_PARAM_16BIT_ADDR:
    *val = (radio_value_t)cmd->localShortAddr;
    break;
  case RADIO_PARAM_CHANNEL:
    *val = (radio_value_t)cmd->channel;
    break;
  default:
    return RADIO_ASYNC_RESTULT_NOT_SUPPORTED;
  }

  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
set_value(radio_param_t par, radio_value_t val)
{
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;
  const output_config_t *pow_config;

  switch(par) {
  case RADIO_PARAM_CHANNEL:
    if(!ieee_mode_channel_valid(val)) {
      return RADIO_ASYNC_RESULT_INVALID_VALUE;
    }
    cmd->channel = val;
    return RADIO_ASYNC_RESULT_REQUIRE_CYCLE;
  case RADIO_PARAM_PAN_ID:
    cmd->localPanID = (uint16_t)val;
    return RADIO_ASYNC_RESULT_REQUIRE_CYCLE;
  case RADIO_PARAM_16BIT_ADDR:
    cmd->localShortAddr = (uint16_t)val;
    return RADIO_ASYNC_RESULT_REQUIRE_CYCLE;
  case RADIO_PARAM_TXPOWER:
    pow_config = ieee_common_get_power_config(val);
    if(pow_config == NULL) {
      return RADIO_ASYNC_RESULT_INVALID_VALUE;
    }
    tx_power_current = pow_config;
    return RADIO_ASYNC_RESULT_REQUIRE_CYCLE;
  case RADIO_PARAM_CCA_THRESHOLD:
    cmd->ccaRssiThr = (int8_t)val;
    return RADIO_ASYNC_RESULT_REQUIRE_CYCLE;
  default:
    return RADIO_ASYNC_RESTULT_NOT_SUPPORTED;
  }

  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
get_object(radio_param_t param, void *dest, size_t size)
{
  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != sizeof(cmd->localExtAddr) || !dest) {
      return RADIO_ASYNC_RESULT_INVALID_VALUE;
    }

    memcpy(dest, &cmd->localExtAddr, sizeof(cmd->localExtAddr));
    return RADIO_ASYNC_RESULT_OK;
  }
  return RADIO_ASYNC_RESTULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
set_object(radio_param_t param, const void *src, size_t size)
{
  int i;
  uint8_t *dst;

  rfc_CMD_IEEE_RX_t *cmd = (rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf;
  const int ext_addr_len = sizeof(cmd->localExtAddr);

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != ext_addr_len || !src) {
      return RADIO_ASYNC_RESULT_INVALID_VALUE;
    }
    dst = (uint8_t *)(&cmd->localExtAddr);
    for(i = 0; i < ext_addr_len; i++) {
      dst[i] = ((uint8_t *)src)[ext_addr_len - 1 - i];
    }

    return RADIO_ASYNC_RESULT_REQUIRE_CYCLE;
  }
  return RADIO_ASYNC_RESTULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static enum radio_async_result
init(void)
{
  rf_async_init(rx_isr_cb);
  lpm_register_module(&cc26xx_rf_lpm_module);

  tx_power_current = ieee_common_get_power_config(RF_IEEE_TX_POWER_DBM);

  rf_async_set_modesel();
  ieee_common_init_data_queue();
  ieee_common_init_rf_params((rfc_CMD_IEEE_RX_t *)cmd_ieee_rx_buf);

  process_start(&rf_core_process, NULL);
  return RADIO_ASYNC_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static void
set_receiver_callbacks(void (*ack_cb)(uint8_t), void (*rx_cb)(bool))
{
  rx_user_cb = rx_cb;
  ack_detect.cb = ack_cb;
}
/*---------------------------------------------------------------------------*/
static uint8_t
request(void)
{
  /* Note: we assume that interrupts are disabled here (if they weren't we
     might turn the radio right back on after this function anyways) */
  if(rf_async_is_accessible() || (active_cmd.lock == LOCKED)) {
    return LPM_MODE_SLEEP;
  }

  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_async_driver ieee_async_driver = {
  init,
  set_receiver_callbacks,
  prepare,
  transmit,
  send,
  read_frame,
  channel_clear,
  receiving_packet,
  ieee_common_pending_packet,
  on,
  off,
  start_ack_detect,
  stop_ack_detect,
  get_value,
  set_value,
  get_object,
  set_object,
};
/*---------------------------------------------------------------------------*/
#endif /* NETSTACK_USE_RFASYNC */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
