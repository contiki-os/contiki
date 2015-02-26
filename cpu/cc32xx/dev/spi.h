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

#include "hw_memmap.h"
#include "hw_mcspi.h"

#include "prcm.h"

#include "rom.h"
#include "rom_map.h"

#include "../../ti-cc3200-sdk/driverlib/src/spi.h"	// Ti SPI driver for cc32xx
#include "spi-arch.h"

extern uint8_t spi_busy;

/* SPI buffer dummy's */
#define SPI_RXBUF	spi_get_rxbuf()
#define SPI_TXBUF	spi_txbuf;

/* Define SPI macros used by CC2x20 drivers */
#define SPI_WAITFORTxREADY()
#define SPI_WAITFORTx_BEFORE() 	SPI_WAITFORTxREADY()
#define SPI_WAITFORTx_AFTER()
#define SPI_WAITFORTx_ENDED()	do { while(!(HWREG(GSPI_BASE + MCSPI_O_CH0STAT) & MCSPI_CH0STAT_EOT)); } while(0)

/* Prototypes */
void spi_init(void);

/* Write one character to SPI */
#define SPI_WRITE(data)                         				\
  do {															\
	spi_txbuf = data;											\
	MAP_SPITransfer(GSPI_BASE, &spi_txbuf, &spi_rxbuf, 1, 0);	\
  } while(0)

/* Write one character to SPI - will not wait for end
   useful for multiple writes with wait after final */
#define SPI_WRITE_FAST(data)                         			\
do {															\
	spi_txbuf = data;											\
	MAP_SPITransfer(GSPI_BASE, &spi_txbuf, &spi_rxbuf, 1, 0);	\
} while(0)

/* Read one character from SPI */
#define SPI_READ(data)   										\
do {															\
	spi_txbuf = 0;												\
	MAP_SPITransfer(GSPI_BASE, &spi_txbuf, &spi_rxbuf, 1, 0);	\
	data = spi_get_rxbuf();										\
} while(0)

/* Flush the SPI read register */
#ifndef SPI_FLUSH
#define SPI_FLUSH() \
  do {              \
  } while(0);
#endif

#endif /* SPI_H_ */
