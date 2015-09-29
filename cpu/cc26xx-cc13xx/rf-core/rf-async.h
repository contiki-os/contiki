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
 * \addtogroup rf-core
 * @{
 *
 * \defgroup rf-async CC13xx/CC26xx core functions for asynchronous driver
 *
 * @{
 *
 * \file
 * Header file for the CC13xx/CC26xx RF asynchronous driver
 */
#ifndef RF_ASYNC_H_
#define RF_ASYNC_H_
/*---------------------------------------------------------------------------*/
#include "rf-core/rf-core.h"

#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief return true iff the cmdsta register value isn't an error value
 */
static inline bool
rf_async_cmdsta_ok(uint32_t cmdsta)
{
  return (cmdsta & RF_CORE_CMDSTA_RESULT_MASK) == RF_CORE_CMDSTA_DONE;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief return true iff the command has DONE_OK status
 */
static inline bool
rf_async_cmd_done_ok(uint32_t cmd)
{
  rfc_radioOp_t *op = (rfc_radioOp_t *)cmd;
  uint32_t masked_status = (op->status & RF_CORE_RADIO_OP_MASKED_STATUS);
  return masked_status == RF_CORE_RADIO_OP_STATUS_DONE_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief The prototype of callbacks given to rf_async_send_cmd
 *
 * Called on completion of rf_async_send_cmd
 *
 * \param cmdsta the value the command left in RFC_DBELL_O_CMDSTA
 * \param cmd the original command that was run
 * \param ptr an arbitrary void* passed into rf_async_send_cmd
 */
typedef void (*rf_async_cb_func)(uint32_t cmdsta, uint32_t cmd, void *ptr);
/*---------------------------------------------------------------------------*/
/**
 * \brief Send a command to the rf core asynchronously
 * \param cmd the command structure to send
 * \param cb callback to run on command completion (can't be NULL)
 * \param ptr arbitrary void* to give the cb function on completion
 * \param isr the ISR mask of the ISR which will indicate that this command
 *        has completed (or zero if we're just to wait for CMD_ACK)
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR.
 *
 * This function starts an RF command which will run asyncronously and complete
 * later in an interrupt. If this function returns RF_CORE_CMD_OK then the
 * given callback function will run later from interrupt context. It is ok
 * to call rf_async_send_cmd() again from within the callback function.
 *
 */
uint_fast8_t rf_async_send_cmd(uint32_t cmd, rf_async_cb_func cb, void *ptr,
                               uint32_t isr);

/**
 * \brief Return True if there is an asyncronous command executing
 * \return True ifff a rf-core command is executing asyncronously
 */
bool rf_async_cmd_running(void);

/**
 * \brief Turn on the power domain and clocks, then start the RAT
 *
 * When the callback runs, the cmd will be a START_RAT command
 *
 * \param cb callback to run on completion of the START_RAT
 * \param ptr arbitrary pointer that will be given back to the callback
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR.
 */
uint_fast8_t rf_async_boot(rf_async_cb_func cb, void *ptr);

/**
 * \brief Initilization function for rf_async interface
 *
 * Must be called before using any other function in this module
 *
 * \param ack_isr_cb non-null callback function which will run every time an
 * rx isr occurs
 */
void rf_async_init(void (*ack_isr_cb)(void));

/**
 * \brief Stops a running background command
 * \warning If no background operation is actually ongoing, this can hang!
 *
 * Stops the currently running background command. If no background command
 * is actually running, this function will hang (the callback will never run)
 * because we are waiting for a IRQ_LAST_COMMAND_DONE interrupt that will
 * never run!
 *
 * Stop is typically more graceful than abort.
 *
 * \param cb callback to run on command completion
 * \param ptr arbitrary void* to give the cb function on completion
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR.
 */
uint_fast8_t rf_async_stop_bg(rf_async_cb_func cb, void *ptr);

/**
 * \brief Aborts a running background command
 * \warning If no background operation is actually ongoing, this can hang!
 *
 * Aborts the currently running background command. If no background command
 * is actually running, this function will hang (the callback will never run)
 * because we are waiting for a IRQ_LAST_COMMAND_DONE interrupt that will
 * never run!
 *
 * \param cb callback to run on command completion
 * \param ptr arbitrary void* to give the cb function on completion
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR.
 */
uint_fast8_t rf_async_abort_bg(rf_async_cb_func cb, void *ptr);

/**
 * \brief powers down the frequency synthesizer (and analog components)
 *
 * If this command is not run before powering down the radio (assuming that
 * the synthesizer is running)the radio will consume a bunch of extra power.
 *
 * This function handles initilization of the given command structure.
 *
 * \param cmd space for the command to run
 * \param cb callback to run on command completion
 * \param ptr arbitrary void* to give the cb function on completion
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR.
 */
uint_fast8_t rf_async_fs_powerdown(rfc_CMD_FS_POWERDOWN_t *cmd,
                                   rf_async_cb_func cb, void *ptr);

/**
 * \brief powers down power domains and clocks, resets interrupts
 */
void rf_async_power_down(void);
/*---------------------------------------------------------------------------*/
/* rf-core functions we want to re-use */
#define rf_async_is_accessible rf_core_is_accessible
#define rf_async_set_modesel   rf_core_set_modesel
#define rf_async_init_radio_op rf_core_init_radio_op
/*---------------------------------------------------------------------------*/
#endif /* RF_ASYNC_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
