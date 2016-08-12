/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Lee Mitchell
 * Integrated into Contiki by Beshr Al Nahas
 *
 */

#ifndef UARTDRIVER_H
#define UARTDRIVER_H

#include <jendefs.h>
#include "contiki-conf.h"

#define UART_EXTRAS 1

void uart_driver_init(uint8_t uart_dev, uint8_t br, uint8_t * txbuf_data, uint16_t txbuf_size, uint8_t * rxbuf_data, uint16_t rxbuf_size, int (*uart_input_function)(unsigned char c));
void uart_driver_write_buffered(uint8_t uart_dev, uint8_t ch);
void uart_driver_write_with_deadline(uint8_t uart_dev, uint8_t c);
uint8_t uart_driver_read(uint8_t uart_dev, uint8_t *data);
void uart_driver_write_direct(uint8_t uart_dev, uint8_t ch);
void uart_driver_set_input(uint8_t u8Uart, int (*uart_input_function)(unsigned char c));

void uart_driver_rx_handler(uint8_t uart_dev);
void uart_driver_enable_interrupts(uint8_t uart_dev);
void uart_driver_disable_interrupts(uint8_t uart_dev);
int8_t uart_driver_interrupt_is_enabled(uint8_t uart_dev);
void uart_driver_store_interrupts(uint8_t uart_dev);
void uart_driver_restore_interrupts(uint8_t uart_dev);

uint8_t uart_driver_tx_in_progress(uint8_t uart_dev);

#ifdef UART_EXTRAS
void uart_driver_flush(uint8_t uart_dev, bool_t reset_tx, bool_t reset_rx);
#endif

#endif /* UARTDRIVER_H */
