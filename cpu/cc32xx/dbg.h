/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \addtogroup cc32xx
 * @{
 *
 * \defgroup cc32xx-char-io cc32xx Character I/O
 *
 * cc32xx CPU-specific functions for debugging and SLIP I/O
 *
 * @{
 *
 * \file
 * Header file for the cc32xx Debug I/O module
 */
#ifndef DBG_H_
#define DBG_H_

#include "contiki-conf.h"

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
#define dbg_flush()

#endif /* DBG_H_ */

/**
 * @}
 * @}
 */
