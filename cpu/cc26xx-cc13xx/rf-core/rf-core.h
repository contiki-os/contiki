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
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup rf-core CC13xx/CC26xx RF core
 *
 * Different flavours of chips of the CC13xx/CC26xx family have different
 * radio capability. For example, the CC2650 can operate in IEEE 802.15.4 mode
 * at 2.4GHz, but it can also operate in BLE mode. The CC1310 only supports
 * sub-ghz mode.
 *
 * However, there are many radio functionalities that are identical across
 * all chips. The rf-core driver provides support for this common functionality
 *
 * @{
 *
 * \file
 * Header file for the CC13xx/CC26xx RF core driver
 */
/*---------------------------------------------------------------------------*/
#ifndef RF_CORE_H_
#define RF_CORE_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "rf-core/api/common_cmd.h"

#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/* The channel to use in IEEE or prop mode. */
#ifdef RF_CORE_CONF_CHANNEL
#define RF_CORE_CHANNEL RF_CORE_CONF_CHANNEL
#else
#define RF_CORE_CHANNEL 25
#endif /* RF_CORE_CONF_IEEE_MODE_CHANNEL */
/*---------------------------------------------------------------------------*/
#define RF_CORE_CMD_ERROR                     0
#define RF_CORE_CMD_OK                        1
/*---------------------------------------------------------------------------*/
/**
 * \brief A data strcuture representing the radio's primary mode of operation
 *
 * The CC13xx / CC26xx radio supports up to potentially 3 modes: IEEE, Prop and
 * BLE. Within Contiki, we assume that the radio is by default in one of IEEE
 * or Prop in order to support standard 6LoWPAN / .15.4 operation. The BLE
 * mode interrupts this so called "primary" mode in order to send BLE adv
 * messages. Once BLE is done advertising, we need to be able to restore the
 * previous .15.4 mode. Unfortunately, the only way this can be done with
 * NETSTACK_RADIO API is by fully power-cycling the radio, which is something
 * we do not want to do.
 *
 * Thus, we declare a secondary data structure for primary mode drivers (IEEE
 * or Prop). We use this data structure to issue "soft off" and "back on"
 * commands. Soft off in this context means stopping RX (e.g. the respective
 * IEEE RX operation), but without shutting down the RF core (which is what
 * NETSTACK_RADIO.off() would have done). We then remember what mode we were
 * using in order to be able to re-enter RX mode for this mode.
 *
 * A NETSTACK_RADIO driver will declare those two functions somewhere within
 * its module of implementation. During its init() routine, it will notify
 * the RF core module so that the latter can abort and restore operations.
 */
typedef struct rf_core_primary_mode_s {
  /**
   * \brief A pointer to a function used to abort the current radio op
   */
  void (*abort)(void);

  /**
   * \brief A pointer to a function that will restore the previous radio op
   * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR
   */
  uint8_t (*restore)(void);
} rf_core_primary_mode_t;
/*---------------------------------------------------------------------------*/
/* RF Command status constants - Correspond to values in the CMDSTA register */
#define RF_CORE_CMDSTA_PENDING         0x00
#define RF_CORE_CMDSTA_DONE            0x01
#define RF_CORE_CMDSTA_ILLEGAL_PTR     0x81
#define RF_CORE_CMDSTA_UNKNOWN_CMD     0x82
#define RF_CORE_CMDSTA_UNKNOWN_DIR_CMD 0x83
#define RF_CORE_CMDSTA_CONTEXT_ERR     0x85
#define RF_CORE_CMDSTA_SCHEDULING_ERR  0x86
#define RF_CORE_CMDSTA_PAR_ERR         0x87
#define RF_CORE_CMDSTA_QUEUE_ERR       0x88
#define RF_CORE_CMDSTA_QUEUE_BUSY      0x89

/* Status values starting with 0x8 correspond to errors */
#define RF_CORE_CMDSTA_ERR_MASK        0x80

/* CMDSTA is 32-bits. Return value in bits 7:0 */
#define RF_CORE_CMDSTA_RESULT_MASK     0xFF

#define RF_CORE_RADIO_OP_STATUS_IDLE   0x0000
/*---------------------------------------------------------------------------*/
#define RF_CORE_NOT_ACCESSIBLE 0x00
#define RF_CORE_ACCESSIBLE     0x01
/*---------------------------------------------------------------------------*/
/* RF Radio Op status constants. Field 'status' in Radio Op command struct */
#define RF_CORE_RADIO_OP_STATUS_IDLE                     0x0000
#define RF_CORE_RADIO_OP_STATUS_PENDING                  0x0001
#define RF_CORE_RADIO_OP_STATUS_ACTIVE                   0x0002
#define RF_CORE_RADIO_OP_STATUS_SKIPPED                  0x0003
#define RF_CORE_RADIO_OP_STATUS_DONE_OK                  0x0400
#define RF_CORE_RADIO_OP_STATUS_DONE_COUNTDOWN           0x0401
#define RF_CORE_RADIO_OP_STATUS_DONE_RXERR               0x0402
#define RF_CORE_RADIO_OP_STATUS_DONE_TIMEOUT             0x0403
#define RF_CORE_RADIO_OP_STATUS_DONE_STOPPED             0x0404
#define RF_CORE_RADIO_OP_STATUS_DONE_ABORT               0x0405
#define RF_CORE_RADIO_OP_STATUS_ERROR_PAST_START         0x0800
#define RF_CORE_RADIO_OP_STATUS_ERROR_START_TRIG         0x0801
#define RF_CORE_RADIO_OP_STATUS_ERROR_CONDITION          0x0802
#define RF_CORE_RADIO_OP_STATUS_ERROR_PAR                0x0803
#define RF_CORE_RADIO_OP_STATUS_ERROR_POINTER            0x0804
#define RF_CORE_RADIO_OP_STATUS_ERROR_CMDID              0x0805
#define RF_CORE_RADIO_OP_STATUS_ERROR_NO_SETUP           0x0807
#define RF_CORE_RADIO_OP_STATUS_ERROR_NO_FS              0x0808
#define RF_CORE_RADIO_OP_STATUS_ERROR_SYNTH_PROG         0x0809

/* Additional Op status values for IEEE mode */
#define RF_CORE_RADIO_OP_STATUS_IEEE_SUSPENDED           0x2001
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_OK             0x2400
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_BUSY           0x2401
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_STOPPED        0x2402
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_ACK            0x2403
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_ACKPEND        0x2404
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_TIMEOUT        0x2405
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_BGEND          0x2406
#define RF_CORE_RADIO_OP_STATUS_IEEE_DONE_ABORT          0x2407
#define RF_CORE_RADIO_OP_STATUS_ERROR_WRONG_BG           0x0806
#define RF_CORE_RADIO_OP_STATUS_IEEE_ERROR_PAR           0x2800
#define RF_CORE_RADIO_OP_STATUS_IEEE_ERROR_NO_SETUP      0x2801
#define RF_CORE_RADIO_OP_STATUS_IEEE_ERROR_NO_FS         0x2802
#define RF_CORE_RADIO_OP_STATUS_IEEE_ERROR_SYNTH_PROG    0x2803
#define RF_CORE_RADIO_OP_STATUS_IEEE_ERROR_RXOVF         0x2804
#define RF_CORE_RADIO_OP_STATUS_IEEE_ERROR_TXUNF         0x2805

/* Op status values for BLE mode */
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_OK              0x1400
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_RXTIMEOUT       0x1401
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_NOSYNC          0x1402
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_RXERR           0x1403
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_CONNECT         0x1404
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_MAXNACK         0x1405
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_ENDED           0x1406
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_ABORT           0x1407
#define RF_CORE_RADIO_OP_STATUS_BLE_DONE_STOPPED         0x1408
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_PAR            0x1800
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_RXBUF          0x1801
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_NO_SETUP       0x1802
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_NO_FS          0x1803
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_SYNTH_PROG     0x1804
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_RXOVF          0x1805
#define RF_CORE_RADIO_OP_STATUS_BLE_ERROR_TXUNF          0x1806

/* Op status values for proprietary mode */
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_OK             0x3400
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_RXTIMEOUT      0x3401
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_BREAK          0x3402
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_ENDED          0x3403
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_STOPPED        0x3404
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_ABORT          0x3405
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_RXERR          0x3406
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_IDLE           0x3407
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_BUSY           0x3408
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_IDLETIMEOUT    0x3409
#define RF_CORE_RADIO_OP_STATUS_PROP_DONE_BUSYTIMEOUT    0x340A
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_PAR           0x3800
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_RXBUF         0x3801
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_RXFULL        0x3802
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_NO_SETUP      0x3803
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_NO_FS         0x3804
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_RXOVF         0x3805
#define RF_CORE_RADIO_OP_STATUS_PROP_ERROR_TXUNF         0x3806

/* Bits 15:12 signify the protocol */
#define RF_CORE_RADIO_OP_STATUS_PROTO_MASK               0xF000
#define RF_CORE_RADIO_OP_STATUS_PROTO_GENERIC            0x0000
#define RF_CORE_RADIO_OP_STATUS_PROTO_BLE                0x1000
#define RF_CORE_RADIO_OP_STATUS_PROTO_IEEE               0x2000
#define RF_CORE_RADIO_OP_STATUS_PROTO_PROP               0x3000

/* Bits 11:10 signify Running / Done OK / Done with error */
#define RF_CORE_RADIO_OP_MASKED_STATUS                   0x0C00
#define RF_CORE_RADIO_OP_MASKED_STATUS_RUNNING           0x0000
#define RF_CORE_RADIO_OP_MASKED_STATUS_DONE              0x0400
#define RF_CORE_RADIO_OP_MASKED_STATUS_ERROR             0x0800
/*---------------------------------------------------------------------------*/
/* Command Types */
#define RF_CORE_COMMAND_TYPE_MASK                        0x0C00
#define RF_CORE_COMMAND_TYPE_IMMEDIATE                   0x0000
#define RF_CORE_COMMAND_TYPE_RADIO_OP                    0x0800
#define RF_CORE_COMMAND_TYPE_IEEE_BG_RADIO_OP            0x0800
#define RF_CORE_COMMAND_TYPE_IEEE_FG_RADIO_OP            0x0C00

#define RF_CORE_COMMAND_PROTOCOL_MASK                    0x3000
#define RF_CORE_COMMAND_PROTOCOL_COMMON                  0x0000
#define RF_CORE_COMMAND_PROTOCOL_BLE                     0x1000
#define RF_CORE_COMMAND_PROTOCOL_IEEE                    0x2000
#define RF_CORE_COMMAND_PROTOCOL_PROP                    0x3000
/*---------------------------------------------------------------------------*/
/* Make the main driver process visible to mode drivers */
PROCESS_NAME(rf_core_process);
/*---------------------------------------------------------------------------*/
/**
 * \brief Check whether the RF core is accessible
 * \retval RF_CORE_ACCESSIBLE The core is powered and ready for access
 * \retval RF_CORE_NOT_ACCESSIBLE The core is not ready
 *
 * If this function returns RF_CORE_NOT_ACCESSIBLE, rf_core_power_up() must be
 * called before any attempt to access the core.
 */
uint8_t rf_core_is_accessible(void);

/**
 * \brief Sends a command to the RF core.
 *
 * \param cmd The command value or a pointer to a command buffer
 * \param status A pointer to a variable which will hold the status
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR
 *
 * This function supports all three types of command (Radio OP, immediate and
 * direct)
 *
 * For immediate and Radio OPs, cmd is a pointer to the data structure
 * containing the command and its parameters. This data structure must be
 * 4-byte aligned.
 *
 * For direct commands, cmd contains the value of the command alongside its
 * parameters. This value will be written to CMDSTA verbatim, so the command
 * ID must be in the 16 high bits, and the 2 LS bits must be set to 01 by the
 * caller.
 *
 * The caller is responsible of allocating and populating cmd for Radio OP and
 * immediate commands
 *
 * The caller is responsible for allocating status
 *
 * For immediate commands and radio Ops, this function will set the command's
 * status field to RF_CORE_RADIO_OP_STATUS_IDLE before sending it to the RF
 */
uint_fast8_t rf_core_send_cmd(uint32_t cmd, uint32_t *status);

/**
 * \brief Block and wait for a Radio op to complete
 * \param cmd A pointer to any command's structure
 * \retval RF_CORE_CMD_OK the command completed with status _DONE_OK
 * \retval RF_CORE_CMD_ERROR Timeout exceeded or the command completed with
 *         status _DONE_xxx (e.g. RF_CORE_RADIO_OP_STATUS_DONE_TIMEOUT)
 */
uint_fast8_t rf_core_wait_cmd_done(void *cmd);

/**
 * \brief Turn on power to the RFC and boot it.
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR
 */
int rf_core_power_up(void);

/**
 * \brief Disable RFCORE clock domain in the MCU VD and turn off the RFCORE PD
 */
void rf_core_power_down(void);

/**
 * \brief Initialise RF APIs in the RF core
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR
 *
 * Depending on chip family and capability, this function will set the correct
 * value to PRCM.RFCMODESEL
 */
uint8_t rf_core_set_modesel(void);

/**
 * \brief Start the CM0 RAT
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR
 *
 * This function must be called each time the CM0 boots. The boot sequence
 * can be performed automatically by calling rf_core_boot() if patches are not
 * required. If patches are required then the patches must be applied after
 * power up and before calling this function.
 */
uint8_t rf_core_start_rat(void);

/**
 * \brief Boot the RF Core
 * \return RF_CORE_CMD_OK or RF_CORE_CMD_ERROR
 *
 * This function will perform the CM0 boot sequence. It will first power it up
 * and then start the RAT. If a patch is required, then the mode driver must
 * not call this function and perform the sequence manually, applying patches
 * after boot and before calling rf_core_start_rat().
 *
 * The function will return RF_CORE_CMD_ERROR if any of those steps fails. If
 * the boot sequence fails to complete, the RF Core will be powered down.
 */
uint8_t rf_core_boot(void);

/**
 * \brief Setup RF core interrupts
 */
void rf_core_setup_interrupts(void);

/**
 * \brief Enable interrupt on command done.
 * \param fg set true to enable irq on foreground command done and false for
 * background commands or if not in ieee mode.
 *
 * This is used within TX routines in order to be able to sleep the CM3 and
 * wake up after TX has finished
 *
 * \sa rf_core_cmd_done_dis()
 */
void rf_core_cmd_done_en(bool fg);

/**
 * \brief Disable the LAST_CMD_DONE and LAST_FG_CMD_DONE interrupts.
 *
 * This is used within TX routines after TX has completed
 *
 * \sa rf_core_cmd_done_en()
 */
void rf_core_cmd_done_dis(void);

/**
 * \brief Returns a pointer to the most recent proto-dependent Radio Op
 * \return The pointer
 *
 * The RF Core driver will remember the most recent proto-dependent Radio OP
 * issued, so that other modules can inspect its type and state at a subsequent
 * stage. The assumption is that those commands will be issued by a function
 * that will then return. The following commands will be "remembered"
 *
 * - All BLE Radio Ops (0x18nn)
 * - All Prop Radio Ops (0x38nn)
 * - IEEE BG Radio Ops (0x28nn)
 *
 * The following commands are assumed to be executed synchronously and will
 * thus not be remembered by the core and not returned by this function:
 *
 * - Direct commands
 * - Proto-independent commands (including Radio Ops and Immediate ones)
 * - IEEE FG Radio Ops (0x2Cxx)
 *
 * This assumes that all commands will be sent to the radio using
 * rf_core_send_cmd()
 */
rfc_radioOp_t *rf_core_get_last_radio_op(void);

/**
 * \brief Prepare a buffer to host a Radio Op
 * \param buf A pointer to the buffer that will host the Radio Op
 * \param len The buffer's length
 * \param command The command ID
 *
 * The caller is responsible to allocate the buffer
 *
 * This function will not check whether the buffer is large enough to hold the
 * command. This is the caller's responsibility
 *
 * This function will wipe out the buffer's contents.
 */
void rf_core_init_radio_op(rfc_radioOp_t *buf, uint16_t len, uint16_t command);

/**
 * \brief Register a primary mode for radio operation
 * \param mode A pointer to the struct representing the mode
 *
 * A normal NESTACK_RADIO driver will normally register itself by calling
 * this function during its own init().
 *
 * \sa rf_core_primary_mode_t
 */
void rf_core_primary_mode_register(const rf_core_primary_mode_t *mode);

/**
 * \brief Abort the currently running primary radio op
 */
void rf_core_primary_mode_abort(void);

/**
 * \brief Abort the currently running primary radio op
 */
uint8_t rf_core_primary_mode_restore(void);
/*---------------------------------------------------------------------------*/
#endif /* RF_CORE_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
