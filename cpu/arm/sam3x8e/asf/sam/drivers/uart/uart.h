/**
 * \file
 *
 * \brief Universal Asynchronous Receiver Transceiver (UART) driver for SAM.
 *
 * Copyright (c) 2011-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include "compiler.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/*! \brief Option list for UART peripheral initialization */
typedef struct sam_uart_opt {
	//! MCK for UART
	uint32_t ul_mck;
	//! Expected baud rate
	uint32_t ul_baudrate;
	//! Initialize value for UART mode register
	uint32_t ul_mode;
	//! Configure channel mode (Normal, Automatic, Local_loopback or Remote_loopback)
	uint32_t ul_chmode;
} sam_uart_opt_t;

uint32_t uart_init(Uart *p_uart, const sam_uart_opt_t *p_uart_opt);
void uart_enable_tx(Uart *p_uart);
void uart_disable_tx(Uart *p_uart);
void uart_reset_tx(Uart *p_uart);
void uart_enable_rx(Uart *p_uart);
void uart_disable_rx(Uart *p_uart);
void uart_reset_rx(Uart *p_uart);
void uart_enable(Uart *p_uart);
void uart_disable(Uart *p_uart);
void uart_reset(Uart *p_uart);
void uart_enable_interrupt(Uart *p_uart, uint32_t ul_sources);
void uart_disable_interrupt(Uart *p_uart, uint32_t ul_sources);
uint32_t uart_get_interrupt_mask(Uart *p_uart);
uint32_t uart_get_status(Uart *p_uart);
uint32_t uart_is_tx_ready(Uart *p_uart);
uint32_t uart_is_tx_empty(Uart *p_uart);
uint32_t uart_is_rx_ready(Uart *p_uart);
uint32_t uart_is_rx_buf_end(Uart *p_uart);
uint32_t uart_is_tx_buf_end(Uart *p_uart);
uint32_t uart_is_rx_buf_full(Uart *p_uart);
uint32_t uart_is_tx_buf_empty(Uart *p_uart);
uint32_t uart_write(Uart *p_uart, const uint8_t uc_data);
uint32_t uart_read(Uart *p_uart, uint8_t *puc_data);
Pdc *uart_get_pdc_base(Uart *p_uart);

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond

#endif /* UART_H_INCLUDED */
