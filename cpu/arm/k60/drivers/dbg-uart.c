/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Debug port initialization for the Mulle platform.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "dbg-uart.h"
#include "config-board.h"
#include "config-clocks.h"
#include "uart.h"
#include "port.h"

/**
 * Initialize debug UART used by printf.
 *
 * \note Undefining BOARD_DEBUG_UART_TX_PIN_PORT will disable printf.
 */
void
dbg_uart_init(void)
{
#ifdef BOARD_DEBUG_UART_TX_PIN_PORT
  /* Enable the clock gate to the TX pin PORT */
  port_module_enable(BOARD_DEBUG_UART_TX_PIN_PORT);
  /* Choose UART TX for the pin mux and disable PORT interrupts on the pin */
  BOARD_DEBUG_UART_TX_PIN_PORT->PCR[BOARD_DEBUG_UART_TX_PIN_NUMBER] =
    PORT_PCR_MUX(BOARD_DEBUG_UART_TX_PIN_MUX);
#endif
#ifdef BOARD_DEBUG_UART_RX_PIN_PORT
  port_module_enable(BOARD_DEBUG_UART_RX_PIN_PORT);

  /* Choose UART RX for the pin mux and disable PORT interrupts on the pin */
  BOARD_DEBUG_UART_RX_PIN_PORT->PCR[BOARD_DEBUG_UART_RX_PIN_NUMBER] =
    PORT_PCR_MUX(BOARD_DEBUG_UART_RX_PIN_MUX);
#endif

  uart_init(BOARD_DEBUG_UART_NUM, 0, BOARD_DEBUG_UART_BAUD);
}
