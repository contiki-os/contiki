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
/*---------------------------------------------------------------------------*/
#include "net/netstack.h"
#if NETSTACK_USE_RFASYNC
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup rf-async
 * @{
 *
 * \file
 * Implementation of CC13xx/CC26xx RF common asynchronous functions
 *
 */
/*---------------------------------------------------------------------------*/
/* Contiki Core / System Interfaces */
#include "contiki-conf.h"
#include "sys/process.h"
#include "sys/energest.h"
#include "dev/radio-async.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
/*---------------------------------------------------------------------------*/
/* CC26xx specific interfaces */
#include "ti-lib.h"
/*---------------------------------------------------------------------------*/
/* RF core and RF HAL API */
#include "hw_rfc_dbell.h"
#include "hw_rfc_pwr.h"
/*---------------------------------------------------------------------------*/
#include "rf-async.h"
#include "rf-core.h"
#include "ieee-common.h"
/*---------------------------------------------------------------------------*/
/* Builtin Libraries */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define ACQUIRE_SUCCESS 0
#define ACQUIRE_FAILURE 1

#define CMD_STATE_READY     0
#define CMD_STATE_STARTED   1
#define CMD_STATE_ISR_ONCE  2
#define CMD_STATE_ISR_TWICE 3

/* RF interrupts */
#define RX_FRAME_IRQ IRQ_RX_ENTRY_DONE
#define ERROR_IRQ    IRQ_INTERNAL_ERROR
#define RX_NOK_IRQ   IRQ_RX_NOK

#define cc26xx_rf_cmdack_isr RFCCmdAckIntHandler
/*---------------------------------------------------------------------------*/
static volatile struct {
  uint32_t isr;
  uint32_t cmd;
  void *ptr;
  void (*cb)(uint32_t, uint32_t, void *);
  uint32_t state;
} active_cmd;

static void (*rx_isr_cb)(void);
/*---------------------------------------------------------------------------*/
static int  cmd_lock(void);
static void cmd_release(void);
static void handle_cmd_ack(void);
static void handle_cmd_isr(uint32_t flags);
static void cpe0_isr_func(uint32_t flags);
/*---------------------------------------------------------------------------*/
static int
cmd_lock(void)
{
  int ret = ACQUIRE_FAILURE;
  bool interrupts_disabled = ti_lib_int_master_disable();

  if(active_cmd.state == CMD_STATE_READY) {
    active_cmd.state = CMD_STATE_STARTED;
    ret = ACQUIRE_SUCCESS;
  }

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
cmd_release(void)
{
  active_cmd.state = CMD_STATE_READY;
}
/*---------------------------------------------------------------------------*/
static void
finish_cmd(uint8_t cmd_status)
{
  uint32_t cmd = active_cmd.cmd;
  void *ptr = active_cmd.ptr;
  void (*cb)(uint32_t, uint32_t, void *) = active_cmd.cb;

  active_cmd.isr = 0;

  cmd_release();
  cb(cmd_status, cmd, ptr);
}
/*---------------------------------------------------------------------------*/
static void
handle_cmd_ack(void)
{
  bool command_done = false;
  bool interrupts_disabled;
  uint32_t cmdsta;
  uint32_t cmd_state;

  if(active_cmd.state == CMD_STATE_READY) {
    /* This shouldn't ever happen */
    PRINTF("handle_cmdack: no cmd running\n");
    return;
  }

  if(active_cmd.isr) {

    /* we really only need to mask the cpe0 isr here... */
    interrupts_disabled = ti_lib_int_master_disable();

    cmdsta = HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA);

    cmd_state = active_cmd.state + 1;
    active_cmd.state = cmd_state;

    if(!interrupts_disabled) {
      ti_lib_int_master_enable();
    }

    command_done = (cmd_state == CMD_STATE_ISR_TWICE) ||
      (!rf_async_cmdsta_ok(cmdsta));
  } else {
    command_done = true;

    cmdsta = HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA);
  }

  if(command_done) {
    finish_cmd(cmdsta);
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_cmd_isr(uint32_t isr_flags)
{
  bool command_done = false;
  bool interrupts_disabled;
  uint32_t cmd_state;

  /* we really only need to mask the ACK isr here... */
  interrupts_disabled = ti_lib_int_master_disable();

  cmd_state = active_cmd.state;

  if(cmd_state != CMD_STATE_READY) {

    if(isr_flags & active_cmd.isr) {
      /* disable the ISR Flag that put us here */
      HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) &= ~active_cmd.isr;

      cmd_state = cmd_state + 1;
      active_cmd.state = cmd_state;
    }
  }

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  command_done = (cmd_state == CMD_STATE_ISR_TWICE);

  if(command_done) {
    finish_cmd(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA));
  }
}
/*---------------------------------------------------------------------------*/
bool
rf_async_cmd_running(void)
{
  return active_cmd.state != CMD_STATE_READY;
}
/*---------------------------------------------------------------------------*/
uint_fast8_t
rf_async_send_cmd(uint32_t cmd, rf_async_cb_func cb, void *ptr, uint32_t isr)
{
  uint32_t cpe_enabled;
  uint32_t new_enabled;

  if(cb == NULL) {
    PRINTF("rf_async_send_cmd: NULL cb\n");
    return RF_CORE_CMD_ERROR;
  } else if(cmd_lock() == ACQUIRE_FAILURE) {
    PRINTF("rf_async_send_cmd: acquire fail\n");
    return RF_CORE_CMD_ERROR;
  }

  if(!rf_async_is_accessible()) {
    PRINTF("rf_async_send_cmd: radio is off\n");
    cmd_release();
    return RF_CORE_CMD_ERROR;
  }

  cpe_enabled = HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN);
  new_enabled = cpe_enabled | isr;

  /* clear only those interrupts which were not enabled */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = cpe_enabled;
  /* enable the IRQ that we will finish this command on */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = new_enabled;

  active_cmd.isr = isr;
  active_cmd.cmd = cmd;
  active_cmd.ptr = ptr;
  active_cmd.cb = cb;

  HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = cmd;

  return RF_CORE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
uint_fast8_t
rf_async_start_rat(rf_async_cb_func cb, void *ptr)
{
  return rf_async_send_cmd(CMDR_DIR_CMD(CMD_START_RAT), cb, ptr, 0);
}
/*---------------------------------------------------------------------------*/
uint_fast8_t
rf_async_stop_bg(rf_async_cb_func cb, void *ptr)
{
  return rf_async_send_cmd(CMDR_DIR_CMD(CMD_STOP), cb, ptr,
                           IRQ_LAST_COMMAND_DONE);
}
/*---------------------------------------------------------------------------*/
uint_fast8_t
rf_async_abort_bg(rf_async_cb_func cb, void *ptr)
{
  return rf_async_send_cmd(CMDR_DIR_CMD(CMD_ABORT), cb, ptr,
                           IRQ_LAST_COMMAND_DONE);
}
/*---------------------------------------------------------------------------*/
uint_fast8_t
rf_async_fs_powerdown(rfc_CMD_FS_POWERDOWN_t *cmd, rf_async_cb_func cb,
                      void *ptr)
{
  rf_async_init_radio_op((rfc_radioOp_t *)cmd, sizeof(*cmd), CMD_FS_POWERDOWN);
  return rf_async_send_cmd((uint32_t)cmd, cb, ptr, IRQ_LAST_COMMAND_DONE);
}
/*---------------------------------------------------------------------------*/
void
rf_async_setup_interrupts(void)
{
  rf_core_setup_interrupts(false);

  ti_lib_int_pend_clear(INT_RF_CMD_ACK);
  ti_lib_int_enable(INT_RF_CMD_ACK);
}
/*---------------------------------------------------------------------------*/
uint_fast8_t
rf_async_boot(rf_async_cb_func cb, void *ptr)
{
  rf_core_poweron_pcrm();

  rf_async_setup_interrupts();
  return rf_async_start_rat(cb, ptr);
}
/*---------------------------------------------------------------------------*/
void
rf_async_power_down(void)
{
  /* All of this interrupt management may be unnecesarry but we keep it for now
     because the old driver also did this and because it may help us to recover
     from low level errors. */
  ti_lib_int_disable(INT_RF_CPE0);
  ti_lib_int_disable(INT_RF_CPE1);
  ti_lib_int_disable(INT_RF_CMD_ACK);

  if(rf_async_is_accessible()) {
    HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = 0x0;
    HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = 0x0;
    HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFACKIFG) = 0x0;
    HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = 0x0;
  }

  rf_core_poweroff_prcm();

  ti_lib_int_pend_clear(INT_RF_CPE0);
  ti_lib_int_pend_clear(INT_RF_CPE1);
  ti_lib_int_pend_clear(INT_RF_CMD_ACK);

  ti_lib_int_enable(INT_RF_CPE0);
  ti_lib_int_enable(INT_RF_CPE1);
  ti_lib_int_enable(INT_RF_CMD_ACK);
}
/*---------------------------------------------------------------------------*/
void
rf_async_init(void (*cb)(void))
{
  rx_isr_cb = cb;
  rf_core_set_cep0_isr_func(cpe0_isr_func);
}
/*---------------------------------------------------------------------------*/
static void
cpe0_isr_func(uint32_t flags)
{
  if(flags & RX_FRAME_IRQ) {
    rx_isr_cb();
  }
  handle_cmd_isr(flags);
}
/*---------------------------------------------------------------------------*/
void
cc26xx_rf_cmdack_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFACKIFG) = 0x0;
  handle_cmd_ack();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
#endif /* NETSTACK_USE_RFASYNC */
/*---------------------------------------------------------------------------*/
/** @} */