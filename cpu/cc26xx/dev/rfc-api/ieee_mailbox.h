/******************************************************************************
*  Filename:       ieee_mailbox.h
*  Revised:        $ $
*  Revision:       $ $
*
*  Description:    Definitions for IEEE 802.15.4 interface
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

#ifndef _IEEE_MAILBOX_H
#define _IEEE_MAILBOX_H

#include "mailbox.h"

/// \name CPE interrupt definitions for IEEE 802.15.4
/// Interrupt masks for the CPE interrupt in RDBELL. These are new names for interrupts in mailbox.h,
/// used for compartibility with previous versions with separate interrupt numbers.
///@{
#define IRQN_IEEE_BG_COMMAND_SUSPENDED IRQN_BG_COMMAND_SUSPENDED
#define IRQN_IEEE_TX_FRAME             IRQN_TX_DONE
#define IRQN_IEEE_TX_ACK               IRQN_TX_ACK

#define IRQN_IEEE_RX_FRAME             IRQN_RX_OK
#define IRQN_IEEE_RX_NOK               IRQN_RX_NOK
#define IRQN_IEEE_RX_IGNORED           IRQN_RX_IGNORED
#define IRQN_IEEE_RX_BUF_FULL          IRQN_RX_BUF_FULL
#define IRQN_IEEE_RX_ENTRY_DONE        IRQN_RX_ENTRY_DONE

#define IRQ_IEEE_BG_COMMAND_SUSPENDED  (1U << IRQN_IEEE_BG_COMMAND_SUSPENDED)
#define IRQ_IEEE_TX_FRAME              (1U << IRQN_IEEE_TX_FRAME)
#define IRQ_IEEE_TX_ACK                (1U << IRQN_IEEE_TX_ACK)
#define IRQ_IEEE_RX_FRAME              (1U << IRQN_IEEE_RX_FRAME)
#define IRQ_IEEE_RX_NOK                (1U << IRQN_IEEE_RX_NOK)
#define IRQ_IEEE_RX_IGNORED            (1U << IRQN_IEEE_RX_IGNORED)
#define IRQ_IEEE_RX_BUF_FULL           (1U << IRQN_IEEE_RX_BUF_FULL)
#define IRQ_IEEE_RX_ENTRY_DONE         (1U << IRQN_IEEE_RX_ENTRY_DONE)
///@}



/// \name Radio operation status
/// Radio operation status format:
/// Bits 15:12: Protocol
///             0010: IEEE 802.15.4
/// Bits 11:10: Type
///             00: Not finished
///             01: Done successfully
///             10: Done with error
/// Bits 9:0:   Identifier

/// \name Operation not finished
///@{
#define IEEE_SUSPENDED          0x2001  ///< Operation suspended
///@}
/// \name Operation finished normally
///@{
#define IEEE_DONE_OK            0x2400  ///< Operation ended normally
#define IEEE_DONE_BUSY          0x2401  ///< CSMA-CA operation ended with failure
#define IEEE_DONE_STOPPED       0x2402  ///< Operation stopped after stop command
#define IEEE_DONE_ACK           0x2403  ///< ACK packet received with pending data bit cleared
#define IEEE_DONE_ACKPEND       0x2404  ///< ACK packet received with pending data bit set
#define IEEE_DONE_TIMEOUT       0x2405  ///< Operation ended due to timeout
#define IEEE_DONE_BGEND         0x2406  ///< FG operation ended because necessary background level
                                        ///< operation ended 
#define IEEE_DONE_ABORT         0x2407  ///< Operation aborted by command
///@}
/// \name Operation finished with error
///@{
#define IEEE_ERROR_PAR          0x2800  ///< Illegal parameter
#define IEEE_ERROR_NO_SETUP     0x2801  ///< Operation using Rx or Tx attemted when not in 15.4 mode
#define IEEE_ERROR_NO_FS        0x2802  ///< Operation using Rx or Tx attemted without frequency synth configured
#define IEEE_ERROR_SYNTH_PROG   0x2803  ///< Synthesizer programming failed to complete on time
#define IEEE_ERROR_RXOVF        0x2804  ///< Receiver overflowed during operation
#define IEEE_ERROR_TXUNF        0x2805  ///< Transmitter underflowed during operation
///@}
///@}

#endif
