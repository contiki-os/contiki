/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 * \file
 *          UART0 drivers
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *
 */

#ifndef __UART0_H__
#define __UART0_H__

#include <PeripheralRegs.h>
#include "contiki-conf.h"
#include "uart-driver.h"

#define UART_DEFAULT_RX_BUFFER_SIZE 2047
#if UART_XONXOFF_FLOW_CTRL
#define UART_DEFAULT_TX_BUFFER_SIZE 64
#else
#define UART_DEFAULT_TX_BUFFER_SIZE 1281
#endif
#ifdef UART_CONF_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE UART_CONF_TX_BUFFER_SIZE
#else
#define UART_TX_BUFFER_SIZE UART_DEFAULT_TX_BUFFER_SIZE
#endif
#ifdef UART_CONF_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE UART_CONF_RX_BUFFER_SIZE
#else
#define UART_RX_BUFFER_SIZE UART_DEFAULT_RX_BUFFER_SIZE
#endif
void uart0_set_input(int (*input)(unsigned char c));
void uart0_writeb(unsigned char c);
void uart0_init(unsigned char br);

#define uart0_write_direct(c) uart_driver_write_direct(E_AHI_UART_0, (c))
#define uart0_disable_interrupts() uart_driver_disable_interrupts(E_AHI_UART_0)
#define uart0_enable_interrupts() uart_driver_enable_interrupts(E_AHI_UART_0)
#define uart0_restore_interrupts() uart_driver_restore_interrupts(E_AHI_UART_0)
#define uart0_store_interrupts() uart_driver_store_interrupts(E_AHI_UART_0)

uint8_t uart0_active(void);

#endif
