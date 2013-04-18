/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-char-io cc2538 Character I/O
 *
 * cc2538 CPU-specific functions for debugging and SLIP I/O
 *
 * On the cc2538, character I/O can be directed over USB or UART. This is
 * controlled by a series of configuration directives:
 * - SLIP_ARCH_CONF_USB: Controls the operation of slip-arch.
 * - CC2538_RF_CONF_SNIFFER_USB: Controls the output of the RF driver when
 *                               operating as a sniffer
 * - DBG_CONF_USB: Controls all debugging output
 *
 * Defaults for those defines are set in contiki-conf.h
 * @{
 *
 * \file
 * Header file for the cc2538 Debug I/O module
 */
#ifndef DBG_H_
#define DBG_H_

#include "contiki-conf.h"

#include "usb/usb-serial.h"
/**
 * \brief Print a stream of bytes
 * \param seq A pointer to the stream
 * \param len The number of bytes to print
 * \return The number of printed bytes
 *
 * This function is an arch-specific implementation required by the dbg-io
 * API in cpu/arm/common/dbg-io. It prints a stream of bytes over the
 * peripheral used by the platform.
 */
unsigned int dbg_send_bytes(const unsigned char *seq, unsigned int len);

/**
 * \brief Flushes character output
 *
 *        When debugging is sent over USB, this functions causes the USB
 *        driver to immediately TX the content of output buffers. When
 *        debugging is over UART, this function does nothing.
 *
 *        There is nothing stopping you from using this macro in your code but
 *        normally, you won't have to.
 */
#if DBG_CONF_USB
#define dbg_flush() usb_serial_flush()
#else
#define dbg_flush()
#endif

#endif /* DBG_H_ */

/**
 * @}
 * @}
 */
