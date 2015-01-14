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
 * \addtogroup cc32xx-uart
 * @{
 *
 * \file
 * Implementation of the cc32xx UART driver
 */
#include "contiki.h"
#include "dev/uart-arch.h"

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"

#include "prcm.h"
#include "rom.h"
#include "rom_map.h"

#include "uart.h"
#include "osi.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if UART_IN_USE(0) && defined(UART0_CONF_BAUD_RATE)
#define UART0_BAUD_RATE		UART0_CONF_BAUD_RATE
#else
#define UART0_BAUD_RATE		0
#error Contiki is configured to use UART0, but its baud rate are not valid
#endif

#if UART_IN_USE(1) && defined(UART1_CONF_BAUD_RATE)
#define UART1_BAUD_RATE 	UART1_CONF_BAUD_RATE
#else
#define UART1_BAUD_RATE 	0
#error Contiki is configured to use UART1, but its baud rate are not valid
#endif

typedef struct {
	uint32_t base;
	uint32_t baud;
	uint32_t pcrm;
	uint32_t nvic_int;
	void (*isr)(void);
} uart_regs_t;

#define UART_ISR(u)  		void uart##u##_isr(void) { uart_isr(u); }
#define UART_ISR_FUNC(u)	uart##u##_isr

// Interrupt service routines
UART_ISR(0)
UART_ISR(1)

/*---------------------------------------------------------------------------*/
static const uart_regs_t uart_regs[UART_INSTANCE_COUNT] = {
	{
		.base = UARTA0_BASE,
		.baud = UART0_BAUD_RATE,
		.pcrm = PRCM_UARTA0,
		.nvic_int = INT_UARTA0,
		.isr = UART_ISR_FUNC(0)
	}, {
		.base = UARTA1_BASE,
		.baud = UART1_BAUD_RATE,
		.pcrm = PRCM_UARTA1,
		.nvic_int = INT_UARTA1,
		.isr = UART_ISR_FUNC(1)
	}
};

static int (* input_handler[UART_INSTANCE_COUNT])(unsigned char c);

/*---------------------------------------------------------------------------*/
void
uart_isr(uint8_t uart)
{
	uint32_t uart_base;
	uint32_t uart_status;
	uint8_t uart_buffer;

	uart_base = uart_regs[uart].base;

	// Get interrupt status
	uart_status = MAP_UARTIntStatus(uart_base, true);

	// Clear the asserted interrupt
	MAP_UARTIntClear(uart_base, uart_status);

	// Check if data is available
	while(MAP_UARTCharsAvail(uart_base))
	{
		// Get value from UART
		uart_buffer = MAP_UARTCharGetNonBlocking(uart_base);

        // To prevent an Overrun Error, we need to flush the FIFO even if we
        // don't have an input_handler.
		if(input_handler[uart] != NULL)
		{
			input_handler[uart](uart_buffer);
		}
	}
}
/*---------------------------------------------------------------------------*/
void
uart_init(uint8_t uart)
{
	const uart_regs_t *regs;

	if (uart >= UART_INSTANCE_COUNT)
	{
		return;
	}

	regs = &uart_regs[uart];

	// Configure UART with baud rate at 8-N-1 operation.
	MAP_UARTConfigSetExpClk(regs->base, MAP_PRCMPeripheralClockGet(regs->pcrm), regs->baud, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // Set UART FIFO level for TX / RX interrupt.
    MAP_UARTFIFOLevelSet(regs->base, UART_FIFO_TX7_8, UART_FIFO_RX4_8);

#ifdef USE_TIRTOS
	// Register interrupt at TI-RTOS
	osi_InterruptRegister(regs->nvic_int, regs->isr, INT_PRIORITY_LVL_1);
#else
	MAP_UARTIntRegister(regs->base, regs->isr);
#endif

	// Enable RX and RX timeout interrupt
	MAP_UARTIntEnable(regs->base, UART_INT_RX | UART_INT_RT);

	// Enable UART
	MAP_UARTEnable(regs->base);
}
/*---------------------------------------------------------------------------*/
void
uart_write_byte(uint8_t uart, uint8_t b)
{
	uint32_t uart_base;

	if(uart >= UART_INSTANCE_COUNT)
	{
		return;
	}

	uart_base = uart_regs[uart].base;

	// Put char with block until the TX FIFO is full
	MAP_UARTCharPut(uart_base, b);
}
/*---------------------------------------------------------------------------*/
void
uart_set_input(uint8_t uart, int (* input)(unsigned char c))
{
	if(uart >= UART_INSTANCE_COUNT)
	{
		return;
	}

	input_handler[uart] = input;
}
/*---------------------------------------------------------------------------*/
