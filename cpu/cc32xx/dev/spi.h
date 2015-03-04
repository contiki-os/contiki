/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Basic SPI macros for CC32xx
 * \author
 *         Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#ifndef SPI_H_
#define SPI_H_

#include "hw_ints.h"
#include "hw_types.h"

#include "spi-arch.h"

/* Externals */
extern uint8_t spi_busy;

/* Prototypes */
void spi_flush(void);
void spi_init(void);
uint8_t spi_read(void);
void spi_write(uint8_t data);
void spi_write_fast(uint8_t data);

void spi_wait_tx_ready(void);
void spi_wait_tx_ended(void);

/* SPI buffer dummy's */
#define SPI_RXBUF	spi_get_rxbuf()
#define SPI_TXBUF	spi_txbuf;

/* Define SPI macros used by CC2x20 drivers */
#define SPI_WAITFORTxREADY()	spi_wait_tx_ready()
#define SPI_WAITFORTx_BEFORE() 	SPI_WAITFORTxREADY()
#define SPI_WAITFORTx_AFTER()
#define SPI_WAITFORTx_ENDED()	spi_wait_tx_ended()

/* Write one character to SPI */
#define SPI_WRITE(data)			spi_write(data)

/* Write one character to SPI - will not wait for end
   useful for multiple writes with wait after final */
#define SPI_WRITE_FAST(data)	spi_write(data)

/* Read one character from SPI */
#define SPI_READ(data)			data = spi_read()

/* Flush the SPI read register */
#ifndef SPI_FLUSH
#define SPI_FLUSH()				spi_flush()
#endif

#endif /* SPI_H_ */
