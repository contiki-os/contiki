
/*
Copyright 2006, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2006
*/

/**
 * @defgroup	libsdspi	Serial Peripheral Interface
 * @ingroup		libsd
 */

/**
 * @file	ScatterWeb.Spi.h
 * @ingroup	libsdspi
 * @brief	Serial Peripheral Interface
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @date	2006
 * @version	1.0
 */

#ifndef SPI_H_
#define SPI_H_

#ifndef SPI_DMA_READ
#define SPI_DMA_READ	0
#endif
#ifndef SPI_DMA_WRITE
#define SPI_DMA_WRITE	0
#endif
#ifndef SPI_WRITE
#define SPI_WRITE 		1
#endif

void spi_init(void);

/// Configure UART1 for SPI mode
void spi_enable(void);

/// Receive one byte from SPI
uint8_t spi_rx(void);

/// Send one byte to SPI
void spi_tx(const uint8_t c);

/// Read a number of bytes from SPI
void spi_read(void *pDestination, const uint16_t size, const bool incDest);

#if SPI_DMA_WRITE
extern uint8_t spi_dma_lock;
#endif
#if SPI_DMA_READ || SPI_DMA_WRITE
void spi_dma_wait();
#endif

#if SPI_WRITE
/// Write a number of bytes to SPI
void spi_write(const void *pSource,
	       const uint16_t size,
	       const uint8_t startToken, const bool incSource);
#endif

/// Wait a number of clock cycles
void spi_idle(const uint16_t clocks);

/// Read chars until token is received
bool spi_wait_token(const uint8_t token, const uint16_t timeout);

#endif /*SPI_H_ */
