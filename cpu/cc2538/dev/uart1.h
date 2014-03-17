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
 * \defgroup cc2538-uart cc2538 UART
 *
 * Driver for the cc2538 UART controller
 * @{
 *
 * \file
 * Header file for the cc2538 UART driver
 */
#ifndef UART_1_H_
#define UART_1_H_

#include "contiki.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name UART base addresses
 * @{
 */
#define UART_1_BASE           0x4000D000
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Baud rate defines
 *
 * Used in uart_init() to set the values of UART_IBRD and UART_FBRD in order to
 * achieve some standard baud rates. These defines assume that the UART is
 * clocked at 16MHz and that Clock Div is 16 (UART_CTL:HSE clear)
 * @{
 */
#if UART_1_CONF_BAUD_RATE==9600
#define UART_1_CONF_IBRD UART_IBRD_9600
#define UART_1_CONF_FBRD UART_FBRD_9600
#elif UART_1_CONF_BAUD_RATE==38400
#define UART_1_CONF_IBRD UART_IBRD_38400
#define UART_1_CONF_FBRD UART_FBRD_38400
#elif UART_1_CONF_BAUD_RATE==57600
#define UART_1_CONF_IBRD UART_IBRD_57600
#define UART_1_CONF_FBRD UART_FBRD_57600
#elif UART_1_CONF_BAUD_RATE==115200
#define UART_1_CONF_IBRD UART_IBRD_115200
#define UART_1_CONF_FBRD UART_FBRD_115200
#elif UART_1_CONF_BAUD_RATE==230400
#define UART_1_CONF_IBRD UART_IBRD_230400
#define UART_1_CONF_FBRD UART_FBRD_230400
#elif UART_1_CONF_BAUD_RATE==460800
#define UART_1_CONF_IBRD UART_IBRD_460800
#define UART_1_CONF_FBRD UART_FBRD_460800
#else /* Bail out with an error unless the user provided custom values */
#if !(defined UART_1_CONF_IBRD && defined UART_1_CONF_FBRD)
#error "UART baud rate misconfigured and custom IBRD/FBRD values not provided"
#error "Check the value of UART_1_CONF_BAUD_RATE in contiki-conf.h or project-conf.h"
#error "Supported values are 9600, 38400, 57600, 115200, 230400 and 460800."
#error "Alternatively, you can provide custom values for "
#error "UART_1_CONF_IBRD and UART_1_CONF_FBRD"
#endif
#endif

#define BAUD2UBR(x) (x) /* This macro exists only because some examples require it. */

/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART functions
 * @{
 */

/** \brief Initialises the UART controller, configures I/O control
 * and interrupts */
void uart1_init(void);

/** \brief Sends a single character down the UART
 * \param b The character to transmit
 */
void uart1_write_byte(uint8_t b);

/** \brief Assigns a callback to be called when the UART receives a byte
 * \param input A pointer to the function
 */
void uart1_set_input(int (* input)(unsigned char c));

/** @} */

#endif /* UART_1_H_ */

/**
 * @}
 * @}
 */
