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
 *         Board configuration defines for Mulle platform.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "adc.h"

/**
 * CPU silicon revision (some registers are moved or added between revisions 1 and 2)
 */
#if !defined(MULLE_BOARD_SERIAL_NUMBER)
/* Default to revision 1 unless the serial number is specified in the build. */
#define K60_CPU_REV 1
#elif defined(MULLE_BOARD_SERIAL_NUMBER) && \
    (MULLE_BOARD_SERIAL_NUMBER >= 200) && \
    (MULLE_BOARD_SERIAL_NUMBER <= 219)
/* Only Mulles with serial numbers 200 through 219 have revision 1.x silicon
 * (revision 1.4, 4N30D mask set), see the sticker on the CPU top on the Mulle */
#define K60_CPU_REV 1
#else
/* Newer boards have the revision 2 silicon */
#define K60_CPU_REV 2
#endif

/**
 * Voltage reference high for ADC computations (millivolts).
 */
#define MULLE_ADC_VREFH_MILLIVOLTS 3300u

/**
 * Voltage reference low for ADC computations (millivolts).
 */
#define MULLE_ADC_VREFL_MILLIVOLTS 0u

/**
 * Total span of ADC measurement (millivolts).
 */
#define MULLE_ADC_VREFHL_SCALE_MILLIVOLTS ((MULLE_ADC_VREFH_MILLIVOLTS) - (MULLE_ADC_VREFL_MILLIVOLTS))

/**
 * Which channel should perform Vbat measurements
 */
#define MULLE_ADC_VBAT_ADC_NUM 1

#define MULLE_ADC_VBAT_CHANNEL ADC_CH_DAD0

#define MULLE_ADC_VCHR_ADC_NUM 1

#define MULLE_ADC_VCHR_CHANNEL ADC_CH_AD19

/**
 * UART module used for debug printf.
 */
#define BOARD_DEBUG_UART_BASE_PTR UART1_BASE_PTR

/**
 * Baud rate of debug UART.
 */
#define BOARD_DEBUG_UART_BAUD 115200

/**
 * PORT module containing the TX pin of the debug UART.
 */
#define BOARD_DEBUG_UART_TX_PIN_PORT PORTC_BASE_PTR

/**
 * PORT module containing the RX pin of the debug UART.
 */
#define BOARD_DEBUG_UART_RX_PIN_PORT PORTC_BASE_PTR

/**
 * Pin number within the PORT module of the TX pin of the debug UART.
 */
#define BOARD_DEBUG_UART_TX_PIN_NUMBER 4

/**
 * Pin number within the PORT module of the RX pin of the debug UART.
 */
#define BOARD_DEBUG_UART_RX_PIN_NUMBER 3

/**
 * Function number in the PORT mux for the TX pin of the debug UART.
 */
#define BOARD_DEBUG_UART_TX_PIN_MUX 3

/**
 * Function number in the PORT mux for the RX pin of the debug UART.
 */
#define BOARD_DEBUG_UART_RX_PIN_MUX 3

