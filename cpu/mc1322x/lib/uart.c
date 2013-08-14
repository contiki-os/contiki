/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details. 
 *
 *
 */

#include <mc1322x.h>
#include <stdint.h>

#define MOD 9999
#define CLK 24000000
#define DIV 16 /* uart->CON.XTIM = 0 is 16x oversample (datasheet is incorrect) */

void uart_setbaud(volatile struct UART_struct * uart, uint32_t baud) {
	uint64_t inc;

	/* baud rate eqn from reference manual */
	/* multiply by an additional 10 to do a fixed point round later */
	inc = ((uint64_t) baud * DIV * MOD * 10 / CLK ) - 10 ;
	/* add 5 and divide by 10 to get a rounding */
	inc = (inc + 5) / 10;

        /* UART must be disabled to set the baudrate */
	uart->CONbits.TXE = 0;
	uart->CONbits.RXE = 0;

	uart->BR = ( (uint16_t)inc << 16 ) | MOD;

	uart->CONbits.XTIM = 0;
	uart->CONbits.TXE = 1;
	uart->CONbits.RXE = 1;
}

void uart_flowctl(volatile struct UART_struct * uart, uint8_t on) {
	if (on) {
		if( uart == UART1 ) {
			/* CTS and RTS directions */
			GPIO->PAD_DIR_SET.U1CTS = 1;
			GPIO->PAD_DIR_RESET.U1RTS = 1;
			/* function select to uart */
			GPIO->FUNC_SEL.U1CTS = 1;
			GPIO->FUNC_SEL.U1RTS = 1;
		} else { 
			/* UART 2 */
			/* CTS and RTS directions */
			GPIO->PAD_DIR_SET.U2CTS = 1;
			GPIO->PAD_DIR_RESET.U2RTS = 1;
			/* function select to uart */
			GPIO->FUNC_SEL.U2CTS = 1;
			GPIO->FUNC_SEL.U2RTS = 1;
		}
		/* enable flow control */
		uart->CONbits.FCE = 1;
	} else {
		/* off */
		/* disable flow control */
		uart->CONbits.FCE = 0;
		if( uart == UART1 ) {
			/* CTS and RTS to inputs */
			GPIO->PAD_DIR_RESET.U1CTS = 1;
			GPIO->PAD_DIR_RESET.U1RTS = 1;
			/* function select to gpio */
			GPIO->FUNC_SEL.U1CTS = 3;
			GPIO->FUNC_SEL.U1RTS = 3;
		} else { 
			/* UART 2 */
			/* CTS and RTS to inputs */
			GPIO->PAD_DIR_RESET.U2CTS = 1;
			GPIO->PAD_DIR_RESET.U2RTS = 1;
			/* function select to gpio */
			GPIO->FUNC_SEL.U2CTS = 3;
			GPIO->FUNC_SEL.U2RTS = 3;
		}
	}
}

void uart_init(volatile struct UART_struct * uart, uint32_t baud) {
	/* enable the uart so we can set the gpio mode */
	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	uart->CONbits = (struct UART_CON) {
		.TXE = 1,
		.RXE = 1,
	};

	/* interrupt when there are this number or more bytes free in the TX buffer*/
	uart->TXCON = 16;

	if( uart == UART1 ) {
		/* TX and RX directions */
		GPIO->PAD_DIR_SET.U1TX = 1;
		GPIO->PAD_DIR_RESET.U1RX = 1;
		
		/* set func sel to UART */
		GPIO->FUNC_SEL.U1TX = 1;
		GPIO->FUNC_SEL.U1RX = 1;

#if UART1_RX_BUFFERSIZE > 32
		*UART1_UCON = (1 << 0) | (1 << 1) ;	/* enable receive, transmit, and both interrupts */
		*UART1_URXCON = 30;					/* interrupt when fifo is nearly full */
		u1_rx_head = 0; u1_rx_tail = 0;
#elif UART1_RX_BUFFERSIZE < 32			/* enable receive, transmit, flow control, disable rx interrupt */
		*UART1_UCON = (1 << 0) | (1 << 1) | (1 << 12) | (1 << 14); 
		*UART1_UCTS = UART1_RX_BUFFERSIZE;  /* drop cts when tx buffer at trigger level */
		*GPIO_FUNC_SEL1 = ( (0x01 << (0*2)) | (0x01 << (1*2)) ); /* set GPIO17-16 to UART1 CTS and RTS */
#else 
		*UART1_UCON = (1 << 0) | (1 << 1) | (1 << 14); /* enable receive, transmit, disable rx interrupt */
#endif

		u1_tx_head = 0; u1_tx_tail = 0;

		/* tx and rx interrupts are enabled in the UART by default */
		/* see status register bits 13 and 14 */
		/* enable UART1 interrupts in the interrupt controller */
		enable_irq(UART1);

	} else {
		/* UART2 */
		/* TX and RX directions */
		GPIO->PAD_DIR_SET.U2TX = 1;
		GPIO->PAD_DIR_RESET.U1RX = 1;
		
		/* set func sel to UART */
		GPIO->FUNC_SEL.U2TX = 1;
		GPIO->FUNC_SEL.U2RX = 1;

#if UART2_RX_BUFFERSIZE > 32
		*UART2_UCON = (1 << 0) | (1 << 1) ;	/* enable receive, transmit, and both interrupts */
		*UART2_URXCON = 30;					/* interrupt when fifo is nearly full */
		u2_rx_head = 0; u2_rx_tail = 0;
#elif UART2_RX_BUFFERSIZE < 32			/* enable receive, transmit, disable flow control, disable rx interrupt */
		*UART2_UCON = (1 << 0) | (1 << 1) | (0 << 12) | (1 << 14);
		*UART2_UCTS = UART2_RX_BUFFERSIZE;  /* drop cts when tx buffer at trigger level */
		*GPIO_FUNC_SEL1 = ( (0x01 << (0*2)) | (0x01 << (1*2)) ); /* set GPIO17-16 to UART2 CTS and RTS */
#else 
		*UART2_UCON = (1 << 0) | (1 << 1) | (1 << 14); /* enable receive, transmit, disable rx interrupt */
#endif

		u2_tx_head = 0; u2_tx_tail = 0;

		enable_irq(UART2);
	}

	uart_setbaud(uart, baud);

}

