/******************************************************************************
*  Filename:       mailbox.h
*  Revised:        $ $
*  Revision:       $ $
*
*  Description:    Definitions for interface between system and radio CPU
*
*  Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef _MAILBOX_H
#define _MAILBOX_H

#include <stdint.h>
#include <string.h>

/// Type definition for RAT
typedef uint32_t ratmr_t;



/// Type definition for a data queue
typedef struct {
   uint8_t *pCurrEntry;   ///< Pointer to the data queue entry to be used, NULL for an empty queue
   uint8_t *pLastEntry;   ///< Pointer to the last entry in the queue, NULL for a circular queue
} dataQueue_t;



/// \name CPE interrupt definitions
/// Interrupt masks for the CPE interrupt in RDBELL. 
///@{
#define IRQN_COMMAND_DONE           0           ///< Radio operation command finished
#define IRQN_LAST_COMMAND_DONE      1           ///< Last radio operation command in a chain finished
#define IRQN_FG_COMMAND_DONE        2           ///< FG level Radio operation command finished
#define IRQN_LAST_FG_COMMAND_DONE   3           ///< Last FG level radio operation command in a chain finished
#define IRQN_TX_DONE                4           ///< Packet transmitted
#define IRQN_TX_ACK                 5           ///< ACK packet transmitted
#define IRQN_TX_CTRL                6           ///< Control packet transmitted
#define IRQN_TX_CTRL_ACK            7           ///< Acknowledgement received on a transmitted control packet
#define IRQN_TX_CTRL_ACK_ACK        8           ///< Acknowledgement received on a transmitted control packet, and acknowledgement transmitted for that packet
#define IRQN_TX_RETRANS             9           ///< Packet retransmitted
#define IRQN_TX_ENTRY_DONE          10          ///< Tx queue data entry state changed to Finished
#define IRQN_TX_BUFFER_CHANGED      11          ///< A buffer change is complete
#define IRQN_BG_COMMAND_SUSPENDED   12          ///< A background level radio operation command has been suspended
#define IRQN_RX_OK                  16          ///< Packet received with CRC OK, payload, and not to be ignored
#define IRQN_RX_NOK                 17          ///< Packet received with CRC error
#define IRQN_RX_IGNORED             18          ///< Packet received with CRC OK, but to be ignored
#define IRQN_RX_EMPTY               19          ///< Packet received with CRC OK, not to be ignored, no payload
#define IRQN_RX_CTRL                20          ///< Control packet received with CRC OK, not to be ignored
#define IRQN_RX_CTRL_ACK            21          ///< Control packet received with CRC OK, not to be ignored, then ACK sent
#define IRQN_RX_BUF_FULL            22          ///< Packet received that did not fit in the Rx queue
#define IRQN_RX_ENTRY_DONE          23          ///< Rx queue data entry changing state to Finished
#define IRQN_RX_DATA_WRITTEN        24          ///< Data written to partial read Rx buffer
#define IRQN_RX_N_DATA_WRITTEN      25          ///< Specified number of bytes written to partial read Rx buffer
#define IRQN_RX_ABORTED             26          ///< Packet reception stopped before packet was done
#define IRQN_SYNTH_NO_LOCK          28          ///< The synth has gone out of lock after calibration
#define IRQN_MODULES_UNLOCKED       29          ///< As part of the boot process, the CM0 has opened access to RF core modules and memories
#define IRQN_BOOT_DONE              30          ///< The RF core CPU boot is finished

#define IRQN_INTERNAL_ERROR         31          ///< Internal error observed

#define IRQ_COMMAND_DONE            (1U << IRQN_COMMAND_DONE)
#define IRQ_LAST_COMMAND_DONE       (1U << IRQN_LAST_COMMAND_DONE)
#define IRQ_FG_COMMAND_DONE         (1U << IRQN_FG_COMMAND_DONE)
#define IRQ_LAST_FG_COMMAND_DONE    (1U << IRQN_LAST_FG_COMMAND_DONE)

#define IRQ_TX_DONE                 (1U << IRQN_TX_DONE)
#define IRQ_TX_ACK                  (1U << IRQN_TX_ACK)
#define IRQ_TX_CTRL                 (1U << IRQN_TX_CTRL)
#define IRQ_TX_CTRL_ACK             (1U << IRQN_TX_CTRL_ACK)
#define IRQ_TX_CTRL_ACK_ACK         (1U << IRQN_TX_CTRL_ACK_ACK)
#define IRQ_TX_RETRANS              (1U << IRQN_TX_RETRANS)

#define IRQ_TX_ENTRY_DONE           (1U << IRQN_TX_ENTRY_DONE)
#define IRQ_TX_BUFFER_CHANGED       (1U << IRQN_TX_BUFFER_CHANGED)

#define IRQ_BG_COMMAND_SUSPENDED    (1U << IRQN_BG_COMMAND_SUSPENDED)

#define IRQ_RX_OK                   (1U << IRQN_RX_OK)
#define IRQ_RX_NOK                  (1U << IRQN_RX_NOK)
#define IRQ_RX_IGNORED              (1U << IRQN_RX_IGNORED)
#define IRQ_RX_EMPTY                (1U << IRQN_RX_EMPTY)
#define IRQ_RX_CTRL                 (1U << IRQN_RX_CTRL)
#define IRQ_RX_CTRL_ACK             (1U << IRQN_RX_CTRL_ACK)
#define IRQ_RX_BUF_FULL             (1U << IRQN_RX_BUF_FULL)
#define IRQ_RX_ENTRY_DONE           (1U << IRQN_RX_ENTRY_DONE)
#define IRQ_RX_DATA_WRITTEN         (1U << IRQN_RX_DATA_WRITTEN)
#define IRQ_RX_N_DATA_WRITTEN       (1U << IRQN_RX_N_DATA_WRITTEN)
#define IRQ_RX_ABORTED              (1U << IRQN_RX_ABORTED)

#define IRQ_SYNTH_NO_LOCK           (1U << IRQN_SYNTH_NO_LOCK)
#define IRQ_MODULES_UNLOCKED        (1U << IRQN_MODULES_UNLOCKED)
#define IRQ_BOOT_DONE               (1U << IRQN_BOOT_DONE)
#define IRQ_INTERNAL_ERROR          (1U << IRQN_INTERNAL_ERROR)
///@}



/// \name CMDSTA values
/// Values returned in result byte of CMDSTA
///@{
#define CMDSTA_Pending 0x00              ///< The command has not yet been parsed
#define CMDSTA_Done 0x01                 ///< Command successfully parsed

#define CMDSTA_IllegalPointer 0x81       ///< The pointer signaled in CMDR is not valid
#define CMDSTA_UnknownCommand 0x82       ///< The command number in the command structure is unknown
#define CMDSTA_UnknownDirCommand 0x83    ///< The command number for a direct command is unknown, or the
                                         ///< command is not a direct command
#define CMDSTA_ContextError 0x85         ///< An immediate or direct command was issued in a context
                                         ///< where it is not supported
#define CMDSTA_SchedulingError 0x86      ///< A radio operation command was attempted to be scheduled
                                         ///< while another operation was already running in the RF core
#define CMDSTA_ParError 0x87             ///< There were errors in the command parameters that are parsed
                                         ///< on submission.
#define CMDSTA_QueueError 0x88           ///< An operation on a data entry queue was attempted that was
                                         ///< not supported by the queue in its current state
#define CMDSTA_QueueBusy 0x89            ///< An operation on a data entry was attempted while that entry
                                         ///< was busy
///@}

/// \name Macros for use with command definition files
/// The script create_command.pl generates header files from command and structure definitions in the
/// *_def.txt files. These are the macros to access the definitions
///@{

/// Get a field from a structure
//
/// Gets a field from a structure defined in a _def.txt file. This may be used both in assignments and
/// references (e.g. GET_FIELD(pCmd1, CMD_TEST, testParam) = GET_FIELD(pCmd2, CMD_DUMMY, dummyParam);)
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the accessed field as defined in the _def.txt file
///
#define GET_FIELD(ptr, cmd, field) \
(*((_TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field))))

/// Get a field from a structure, reading as volatile
//
/// Gets a field from a structure defined in a _def.txt file, reading it as a volatile parameter, which
/// takes into account that it may be changed by the other side. This may be used both in assignments and
/// references (e.g. GET_FIELD(pCmd1, CMD_TEST, testParam) = GET_FIELD(pCmd2, CMD_DUMMY, dummyParam);)
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the accessed field as defined in the _def.txt file
///
#define GET_FIELD_V(ptr, cmd, field) \
(*((volatile _TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field))))

/// Get the pointer to a field from a structure
//
/// Gets the pointer to a field from a structure defined in a _def.txt file.
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the accessed field as defined in the _def.txt file
///
#define GET_FIELD_PTR(ptr, cmd, field) \
((_TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))

/// Get the volatile pointer to a field from a structure
//
/// Gets the volatile pointer to a field from a structure defined in a _def.txt file.
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the accessed field as defined in the _def.txt file
///
#define GET_FIELD_VPTR(ptr, cmd, field) \
((volatile _TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))

/// Get bits from a bit field
//
/// Returns bits from a bit field defined in a _def.txt file.
/// \param[in]  value
///     The value of the entire field that contains the bit field
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
///
#define GET_BITS(value, cmd, field, bitfield) \
(((value) >> (_BITPOS_##cmd##_##field##_##bitfield)) & \
            ((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1))

/// Get bits from a bit field as a signed value
//
/// Returns sign extended bits from a bit field defined in a _def.txt file.
/// \param[in]  value
///     The value of the entire field that contains the bit field
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
///
#define GET_BITS_S(value, cmd, field, bitfield) \
(((int)(value) << (32 - ((_BITPOS_##cmd##_##field##_##bitfield) + (_NBITS_##cmd##_##field##_##bitfield)))) >> \
                    (32 - (_NBITS_##cmd##_##field##_##bitfield)))

/// Set bits in a bit field
//
/// Modifies a bit field defined in a _def.txt file.
/// \param[in,out]  value
///     The value of the entire field that contains the bit field
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
/// \param[in]  bvalue
///     The value to set in the bitfield
///
#define SET_BITS(value, cmd, field, bitfield, bvalue) \
(((value) = ((value) & \
   (~(((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1) << (_BITPOS_##cmd##_##field##_##bitfield))) | \
   ((bvalue) << (_BITPOS_##cmd##_##field##_##bitfield)))))


/// Get bits from a bit field in a structure
//
/// Returns bits from a bit field in a structure defined in a _def.txt file.
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
///
#define GET_BITFIELD(ptr, cmd, field, bitfield) \
((*((_TYPE_##cmd##_##field *) ((((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))) >> \
   ((_BITPOS_##cmd##_##field##_##bitfield)))  & \
   ((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1))

/// Get bits from a bit field in a structure, reading as volatile
//
/// Returns bits from a bit field in a structure defined in a _def.txt file, reading it as a
/// volatile parameter.
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
///
#define GET_BITFIELD_V(ptr, cmd, field, bitfield) \
((*((volatile _TYPE_##cmd##_##field *) ((((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))) >> \
   ((_BITPOS_##cmd##_##field##_##bitfield)))  & \
   ((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1))


/// Set bits in a bit field in a structure
//
/// Modifies a bit field in a field in a structure defined in a _def.txt file.
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
/// \param[in]  value
///     The value to set in the bitfield
///
#define SET_BITFIELD(ptr, cmd, field, bitfield, value) \
((*((_TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))) = \
   ((*((_TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))) & \
   (~(((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1) << (_BITPOS_##cmd##_##field##_##bitfield))) | \
   (((uint32_t)(value)) << (_BITPOS_##cmd##_##field##_##bitfield))))

/// Set bits in a bit field in a structure, reading and writing as volatile
//
/// Modifies a bit field in a field in a structure defined in a _def.txt file, accessing it as a volatile
/// parameter.
/// \param[in]  ptr
///     Pointer to the structure
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
/// \param[in]  value
///     The value to set in the bitfield
///
#define SET_BITFIELD_V(ptr, cmd, field, bitfield, value) \
((*((volatile _TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))) = \
   ((*((volatile _TYPE_##cmd##_##field *) (((uint8_t *)(ptr)) + (_POSITION_##cmd##_##field)))) & \
   (~(((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1) << (_BITPOS_##cmd##_##field##_##bitfield))) | \
   (((uint32_t)(value)) << (_BITPOS_##cmd##_##field##_##bitfield))))

/// Get the value of specific bifield in a field with the remaining bits set to 0
//
/// Returns a bitfield so that the value of the full field can be obtained by bitwise
/// OR between these
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  field
///     Name of the field that contains the bit field definition as defined in the _def.txt file
/// \param[in]  bitfield
///     Name of the bitfield as defined in the _def.txt file
/// \param[in]  value
///     The value to use in the bitfield
///
#define BITVALUE(cmd, field, bitfield, value) \
((((uint32_t)(value)) & ((1U << (_NBITS_##cmd##_##field##_##bitfield)) - 1)) << \
 (_BITPOS_##cmd##_##field##_##bitfield))

/// Get the size of a structure
//
/// Gets the size of a structure defined in a _def.txt file.
///
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
///
#define SIZEOF_STRUCT(cmd) \
(_SIZEOF_##cmd)

/// Get the size of a radio operation command structure
//
/// Gets the size of a radio operation command structure defined in a _def.txt file. The difference from
/// SIZEOF_STRUCT is for legacy reasons only.
///
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
///
#define SIZEOF_RADIO_OP(cmd) \
(_SIZEOF_##cmd)

/// Initializes a structure to an initialization set
//
/// Sets the value of a structure to its given initialization values
/// \param[in]  ptr
///     Pointer to the structure, must be word aligned
/// \param[in]  cmd
///     Name of the command or structure as defined in the _def.txt file
/// \param[in]  set
///     Index of the set of initializations to use
///
#define INIT_STRUCT(ptr, cmd, set) \
(memcpy(((uint32_t *)(ptr)) + (_START_INIT_WIDX_##cmd), (__init_##cmd[(set)]), \
        (_N_INIT_WORDS_##cmd) * sizeof(uint32_t)))

///@}


/// \name Macros for sending direct commands
///@{
/// Direct command with no parameter
#define CMDR_DIR_CMD(cmdId) (((cmdId) << 16) | 1)

/// Direct command with 1-byte parameter
#define CMDR_DIR_CMD_1BYTE(cmdId, par) (((cmdId) << 16) | ((par) << 8) | 1)

/// Direct command with 2-byte parameter
#define CMDR_DIR_CMD_2BYTE(cmdId, par) (((cmdId) << 16) | ((par) & 0xFFFC) | 1)

///@}



/// \name Definitions for trigger types
///@{
#define TRIG_NOW 0            ///< Triggers immediately
#define TRIG_NEVER 1          ///< Never trigs
#define TRIG_ABSTIME 2        ///< Trigs at an absolute time
#define TRIG_REL_SUBMIT 3     ///< Trigs at a time relative to the command was submitted
#define TRIG_REL_START 4      ///< Trigs at a time relative to the command started
#define TRIG_REL_PREVSTART 5  ///< Trigs at a time relative to the previous command in the chain started
#define TRIG_REL_FIRSTSTART 6 ///< Trigs at a time relative to the first command in the chain started
#define TRIG_REL_PREVEND 7    ///< Trigs at a time relative to the previous command in the chain ended
#define TRIG_REL_EVT1 8       ///< Trigs at a time relative to the context defined "Event 1"
#define TRIG_REL_EVT2 9       ///< Trigs at a time relative to the context defined "Event 2"
#define TRIG_EXTERNAL 10      ///< Trigs at an external event to the radio timer
#define TRIG_PAST_BM 0x80     ///< Bitmask for setting pastTrig bit in order to trig immediately if
                              ///< trigger happened in the past
///@}


/// \name Definitions for conditional execution
///@{
#define COND_ALWAYS 0         ///< Always run next command (except in case of Abort)
#define COND_NEVER 1          ///< Never run next command
#define COND_STOP_ON_FALSE 2  ///< Run next command if this command returned True, stop if it returned
                              ///< False
#define COND_STOP_ON_TRUE 3   ///< Stop if this command returned True, run next command if it returned
                              ///< False
#define COND_SKIP_ON_FALSE 4  ///< Run next command if this command returned True, skip a number of
                              ///< commands if it returned False
#define COND_SKIP_ON_TRUE 5   ///< Skip a number of commands if this command returned True, run next
                              ///< command if it returned False
///@}



/// \name Radio operation status
/// Radio operation status format:
///@{
/// \name Operation not finished
///@{
#define IDLE             0x0000   ///< Operation not started
#define PENDING          0x0001   ///< Start of command is pending
#define ACTIVE           0x0002   ///< Running
#define SKIPPED          0x0003   ///< Operation skipped due to condition in another command
///@}
/// \name Operation finished normally
///@{
#define DONE_OK          0x0400   ///< Operation ended normally
#define DONE_COUNTDOWN   0x0401   ///< Counter reached zero
#define DONE_RXERR       0x0402   ///< Operation ended with CRC error
#define DONE_TIMEOUT     0x0403   ///< Operation ended with timeout
#define DONE_STOPPED     0x0404   ///< Operation stopped after CMD_STOP command
#define DONE_ABORT       0x0405   ///< Operation aborted by CMD_ABORT command
#define DONE_FAILED      0x0406   ///< Scheduled immediate command failed
///@}
/// \name Operation finished with error
///@{
#define ERROR_PAST_START 0x0800   ///< The start trigger occurred in the past
#define ERROR_START_TRIG 0x0801   ///< Illegal start trigger parameter
#define ERROR_CONDITION  0x0802   ///< Illegal condition for next operation
#define ERROR_PAR        0x0803   ///< Error in a command specific parameter
#define ERROR_POINTER    0x0804   ///< Invalid pointer to next operation
#define ERROR_CMDID      0x0805   ///< Next operation has a command ID that is undefined or not a radio
                                  ///< operation command
#define ERROR_WRONG_BG   0x0806   ///< FG level command not compatible with running BG level command
#define ERROR_NO_SETUP   0x0807   ///< Operation using Rx or Tx attemted without CMD_RADIO_SETUP
#define ERROR_NO_FS      0x0808   ///< Operation using Rx or Tx attemted without frequency synth configured
#define ERROR_SYNTH_PROG 0x0809   ///< Synthesizer calibration failed
#define ERROR_TXUNF      0x080A   ///< Tx underflow observed
#define ERROR_RXOVF      0x080B   ///< Rx overflow observed
#define ERROR_NO_RX      0x080C   ///< Attempted to access data from Rx when no such data was yet received
#define ERROR_PENDING    0x080D   ///< Command submitted in the future with another command at different level pending
///@}
///@}


/// \name Data entry types
///@{
#define DATA_ENTRY_TYPE_GEN 0     ///< General type: Tx entry or single element Rx entry
#define DATA_ENTRY_TYPE_MULTI 1   ///< Multi-element Rx entry type
#define DATA_ENTRY_TYPE_PTR 2     ///< Pointer entry type
#define DATA_ENTRY_TYPE_PARTIAL 3 ///< Partial read entry type
///@


/// \name Data entry statuses
///@{
#define DATA_ENTRY_PENDING 0      ///< Entry not yet used
#define DATA_ENTRY_ACTIVE 1       ///< Entry in use by radio CPU
#define DATA_ENTRY_BUSY 2         ///< Entry being updated
#define DATA_ENTRY_FINISHED 3     ///< Radio CPU is finished accessing the entry
#define DATA_ENTRY_UNFINISHED 4   ///< Radio CPU is finished accessing the entry, but packet could not be finished
///@}

/// Difference between length and size of rxData field in multi-element Rx entry
#define DATA_ENTRY_MULTI_LEN_OFFSET (_POSITION_dataEntry_rxData - _POSITION_dataEntry_data)


/// \name Macros for RF register override
///@{
/// Macro for ADI half-size value-mask combination
#define ADI_VAL_MASK(addr, mask, value) \
(((addr) & 1) ? (((mask) & 0x0F) | (((value) & 0x0F) << 4)) : \
 ((((mask) & 0x0F) << 4) | ((value) & 0x0F)))
/// 32-bit write of 16-bit value
#define HW_REG_OVERRIDE(addr, val) ((((uintptr_t) (addr)) & 0xFFFC) | ((uint32_t)(val) << 16))
/// ADI register, full-size write
#define ADI_REG_OVERRIDE(adiNo, addr, val) (2 | ((uint32_t)(val) << 16) | \
(((addr) & 0x3F) << 24) | (((adiNo) ? 1U : 0) << 31))
/// 2 ADI registers, full-size write
#define ADI_2REG_OVERRIDE(adiNo, addr, val, addr2, val2) \
(2 | ((uint32_t)(val2) << 2) | (((addr2) & 0x3F) << 10) | ((uint32_t)(val) << 16) | \
(((addr) & 0x3F) << 24) | (((adiNo) ? 1U : 0) << 31))
/// ADI register, half-size read-modify-write
#define ADI_HALFREG_OVERRIDE(adiNo, addr, mask, val) (2 | (ADI_VAL_MASK(addr, mask, val) << 16) | \
(((addr) & 0x3F) << 24) | (1U << 30) | (((adiNo) ? 1U : 0) << 31))
/// 2 ADI registers, half-size read-modify-write
#define ADI_2HALFREG_OVERRIDE(adiNo, addr, mask, val, addr2, mask2, val2) \
(2 | (ADI_VAL_MASK(addr2, mask2, val2) << 2) | (((addr2) & 0x3F) << 10) | \
(ADI_VAL_MASK(addr, mask, val) << 16) | (((addr) & 0x3F) << 24) | (1U << 30) | (((adiNo) ? 1U : 0) << 31))

/// 16-bit SW register as defined in radio_par_def.txt
#define SW_REG_OVERRIDE(cmd, field, val) (3 | ((_POSITION_##cmd##_##field) << 4) | ((uint32_t)(val) << 16))
/// SW register as defined in radio_par_def.txt with added index (for use with registers > 16 bits).
#define SW_REG_IND_OVERRIDE(cmd, field, offset, val) (3 | \
(((_POSITION_##cmd##_##field) + ((offset) << 1)) << 4) | ((uint32_t)(val) << 16))
/// 8-bit SW register as defined in radio_par_def.txt
#define SW_REG_BYTE_OVERRIDE(cmd, field, val) (0x8003 | ((_POSITION_##cmd##_##field) << 4) | \
((uint32_t)(val) << 16))
/// Two 8-bit SW registers as defined in radio_par_def.txt; the one given by field and the next byte.
#define SW_REG_2BYTE_OVERRIDE(cmd, field, val0, val1) (3 | (((_POSITION_##cmd##_##field) & 0xFFFE) << 4) | \
                                                       (((uint32_t)(val0) << 16) & 0x00FF0000) | ((uint32_t)(val1) << 24))
#define HW16_ARRAY_OVERRIDE(addr, length) (1 | (((uintptr_t) (addr)) & 0xFFFC) | ((uint32_t)(length) << 16))
#define HW32_ARRAY_OVERRIDE(addr, length) (1 | (((uintptr_t) (addr)) & 0xFFFC) | \
((uint32_t)(length) << 16) | (1U << 30))
#define ADI_ARRAY_OVERRIDE(adiNo, addr, bHalfSize, length) (1 | ((((addr) & 0x3F) << 2)) | \
((!!(bHalfSize)) << 8) | ((!!(adiNo)) << 9) | ((uint32_t)(length) << 16) | (2U << 30))
#define SW_ARRAY_OVERRIDE(cmd, firstfield, length) (1 | (((_POSITION_##cmd##_##firstfield)) << 2) | \
((uint32_t)(length) << 16) | (3U << 30))
#define MCE_RFE_OVERRIDE(bMceRam, mceRomBank, mceMode, bRfeRam, rfeRomBank, rfeMode) \
   (7 | ((!!(bMceRam)) << 8) | (((mceRomBank) & 0x07) << 9) | ((!!(bRfeRam)) << 12) | (((rfeRomBank) & 0x07) << 13) | \
    (((mceMode) & 0x00FF) << 16) | (((rfeMode) & 0x00FF) << 24))
#define BAW_OVERRIDE(freqOffset) (0x000B | ((freqOffset) << 16))
#define NEW_OVERRIDE_SEGMENT(address) (((((uintptr_t)(address)) & 0x03FFFFFC) << 6) | 0x000F | \
   (((((uintptr_t)(address) >> 24) == 0x20) ? 0x01 : \
     (((uintptr_t)(address) >> 24) == 0x21) ? 0x02 : \
     (((uintptr_t)(address) >> 24) == 0xA0) ? 0x03 : \
     (((uintptr_t)(address) >> 24) == 0x00) ? 0x04 : \
     (((uintptr_t)(address) >> 24) == 0x10) ? 0x05 : \
     (((uintptr_t)(address) >> 24) == 0x11) ? 0x06 : \
     (((uintptr_t)(address) >> 24) == 0x40) ? 0x07 : \
     (((uintptr_t)(address) >> 24) == 0x50) ? 0x08 : \
     0x09) << 4)) // Use illegal value for illegal address range
/// End of string for override register
#define END_OVERRIDE 0xFFFFFFFF


#define FWPAR_8BIT_ADDR(cmd, field) (0x1800 | (_POSITION_##cmd##_##field))
#define FWPAR_16BIT_ADDR(cmd, field) (0x1000 | (_POSITION_##cmd##_##field))
#define FWPAR_32BIT_ADDR(cmd, field) (0x0000 | (_POSITION_##cmd##_##field))

/// ADI address-value pair
#define ADI_ADDR_VAL(addr, value) ((((addr) & 0x7F) << 8) | ((value) & 0xFF))
#define ADI_ADDR_VAL_MASK(addr, mask, value) ((((addr) & 0x7F) << 8) | ADI_VAL_MASK(addr, mask, value))

/// Low half-word
#define LOWORD(value) ((value) & 0xFFFF)
/// High half-word
#define HIWORD(value) ((value) >> 16)
///@}


#endif
