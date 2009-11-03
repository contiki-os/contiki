/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 *	SD driver implementation using SPI.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef SD_ARCH_H
#define SD_ARCH_H

#include "msb430-uart1.h"

#ifndef U1IFG
#define U1IFG		IFG2
#endif /* U1IFG */

#define MS_DELAY(x) clock_delay(354 * (x))

/* Machine-dependent macros. */
#define LOCK_SPI()		do {				\
				  if(!uart_lock(UART_MODE_SPI))	{\
				    return 0;			\
				  }				\
				} while(0)
#define UNLOCK_SPI()		do {				\
				  uart_unlock(UART_MODE_SPI);	\
				} while(0)

#define SD_CONNECTED()		!(P2IN & 0x40)
#define LOWER_CS()		(P5OUT &= ~0x01)
#define RAISE_CS()		do {				\
				  UART_WAIT_TXDONE();		\
				  P5OUT |= 0x01;		\
				  UART_TX = SPI_IDLE;		\
				  UART_WAIT_TXDONE();		\
				} while(0)

/* Configuration parameters. */
#define SD_TRANSACTION_ATTEMPTS		512
#define SD_READ_RESPONSE_ATTEMPTS	8
#define SD_READ_BLOCK_ATTEMPTS		2

int sd_arch_init(void);
void sd_arch_spi_write(int c);
void sd_arch_spi_write_block(uint8_t *bytes, int amount);
unsigned sd_arch_spi_read(void);

#endif /* !SD_ARCH_H */
