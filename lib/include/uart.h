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

#ifndef UART_H
#define UART_H

#include <stdint.h>

/* Timer registers are all 16-bit wide with 16-bit access only */
#define UART1_BASE      (0x80005000)
#define UART2_BASE      (0x8000B000)

struct UART_struct {
	union {
		uint32_t CON;
		struct UART_CON {
			uint32_t TXE:1;
			uint32_t RXE:1;
			uint32_t PEN:1;
			uint32_t EP:1;
			uint32_t ST2:1;
			uint32_t SB:1;
			uint32_t CONTX:1;
			uint32_t TXOENB:1;
			uint32_t :2;
			uint32_t XTIM:1;
			uint32_t FCP:1;
			uint32_t FCE:1;
			uint32_t MTXR:1;
			uint32_t MRXR:1;
			uint32_t TST:1;
			uint32_t :16;
		} CONbits;
	};
	union {
		uint32_t STAT;
		struct UART_STAT {
			uint32_t SE:1;
			uint32_t PE:1;
			uint32_t FE:1;
			uint32_t TOE:1;
			uint32_t ROE:1;
			uint32_t RUE:1;
			uint32_t RXRDY:1;
			uint32_t TXRDY:1;
			uint32_t :24;
		} USTATbits;
	};
	union {
		uint32_t DATA;
		struct UART_DATA {
			uint32_t DATA:8;
			uint32_t :24;
		} DATAbits;
	};
	union {
		uint32_t RXCON;
		struct UART_URXCON {
                        uint32_t LVL:6;
			uint32_t :26;
		} RXCONbits;
	};
	union {
		uint32_t TXCON;
		struct UART_TXCON {
			uint32_t LVL:6;
			uint32_t :26;
		} TXCONbits;
	};
	union {
		uint32_t CTS;
		struct UART_CTS {
			uint32_t LVL:5;
			uint32_t :27;
		} CTSbits;
	};
	union {
		uint32_t BR;
		struct UART_BR {
			uint32_t MOD:16;
			uint32_t INC:16;
		} BRbits;
	};
};

static volatile struct UART_struct * const UART1 = (void *) (UART1_BASE);
static volatile struct UART_struct * const UART2 = (void *) (UART2_BASE);

/* Old uart definitions, for compatibility */
#ifndef REG_NO_COMPAT

#define UCON      (0)
/* UCON bits */
#define UCON_SAMP     10
#define UCON_SAMP_8X   0
#define UCON_SAMP_16X  1

#define USTAT     (0x04)
#define UDATA     (0x08)
#define URXCON    (0x0c)
#define UTXCON    (0x10)
#define UCTS      (0x14)
#define UBRCNT    (0x18)

#define UART1_UCON       ((volatile uint32_t *) ( UART1_BASE + UCON   ))
#define UART1_USTAT      ((volatile uint32_t *) ( UART1_BASE + USTAT  ))
#define UART1_UDATA      ((volatile uint32_t *) ( UART1_BASE + UDATA  ))
#define UART1_URXCON     ((volatile uint32_t *) ( UART1_BASE + URXCON ))
#define UART1_UTXCON     ((volatile uint32_t *) ( UART1_BASE + UTXCON ))
#define UART1_UCTS       ((volatile uint32_t *) ( UART1_BASE + UCTS   ))
#define UART1_UBRCNT     ((volatile uint32_t *) ( UART1_BASE + UBRCNT ))

#define UART2_UCON       ((volatile uint32_t *) ( UART2_BASE + UCON   ))
#define UART2_USTAT      ((volatile uint32_t *) ( UART2_BASE + USTAT  ))
#define UART2_UDATA      ((volatile uint32_t *) ( UART2_BASE + UDATA  ))
#define UART2_URXCON     ((volatile uint32_t *) ( UART2_BASE + URXCON ))
#define UART2_UTXCON     ((volatile uint32_t *) ( UART2_BASE + UTXCON ))
#define UART2_UCTS       ((volatile uint32_t *) ( UART2_BASE + UCTS   ))
#define UART2_UBRCNT     ((volatile uint32_t *) ( UART2_BASE + UBRCNT ))

#endif /* REG_NO_COMPAT */

void uart_init(volatile struct UART_struct * uart, uint32_t baud);
void uart_setbaud(volatile struct UART_struct * uart, uint32_t baud);
void uart_flowctl(volatile struct UART_struct * uart, uint8_t on);


/* The mc1322x has a 32 byte hardware FIFO for transmitted characters.
 * Currently it is always filled from a larger RAM buffer. It would be
 * possible to eliminate that overhead by filling directly from a chain
 * of data buffer pointers, but printf's would be not so easy.
 */
#define UART1_TX_BUFFERSIZE 1024
extern volatile uint32_t  u1_tx_head, u1_tx_tail;
void uart1_putc(char c);

/* The mc1322x has a 32 byte hardware FIFO for received characters.
 * If a larger rx buffersize is specified the FIFO will be extended into RAM.
 * RAM transfers will occur on interrupt when the FIFO is nearly full.
 * If a smaller buffersize is specified hardware flow control will be
 * initiated at that FIFO level.
 * Set to 32 for no flow control or RAM buffer.
 */
#define UART1_RX_BUFFERSIZE 128
#if UART1_RX_BUFFERSIZE > 32
extern volatile uint32_t  u1_rx_head, u1_rx_tail;
#define uart1_can_get() ((u1_rx_head!=u1_rx_tail) || (*UART1_URXCON > 0))
#else
#define uart1_can_get() (*UART1_URXCON > 0)
#endif
uint8_t uart1_getc(void);


#define UART2_TX_BUFFERSIZE 1024
extern volatile uint32_t  u2_tx_head, u2_tx_tail;
void uart2_putc(char c);

#define UART2_RX_BUFFERSIZE 128
#if UART2_RX_BUFFERSIZE > 32
extern volatile uint32_t  u2_rx_head, u2_rx_tail;
#define uart2_can_get() ((u2_rx_head!=u2_rx_tail) || (*UART2_URXCON > 0))
#else
#define uart2_can_get() (*UART2_URXCON > 0)
#endif
uint8_t uart2_getc(void);

#endif

