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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-uart cc2538 UART
 *
 * Driver for the cc2538 UART controller
 * @{
 *
 * \file
 * Header file for the cc2538 UART driver
 */
#ifndef UART_ARCH_H_
#define UART_ARCH_H_

#include "contiki.h"

#include <stdint.h>

/** \name UART functions
 * @{
 */

/** \brief Initialises the UART controller, configures I/O control
 * and interrupts
 * \param uart The UART instance to use (0 to \c UART_INSTANCE_COUNT - 1)
 */
void uart_init(uint8_t uart);

/** \brief Sends a single character down the UART
 * \param uart The UART instance to use (0 to \c UART_INSTANCE_COUNT - 1)
 * \param b The character to transmit
 */
void uart_write_byte(uint8_t uart, uint8_t b);

/** \brief Assigns a callback to be called when the UART receives a byte
 * \param uart The UART instance to use (0 to \c UART_INSTANCE_COUNT - 1)
 * \param input A pointer to the function
 */
void uart_set_input(uint8_t uart, int (* input)(unsigned char c));


#define uart1_set_input(f) uart_set_input(UART1_CONF_UART, f)

/** @} */

#endif /* UART_ARCH_H_ */
