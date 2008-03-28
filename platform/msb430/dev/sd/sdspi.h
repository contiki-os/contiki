
/*
Copyright 2007, Freie Universitaet Berlin. All rights reserved.

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
Berlin, 2007
*/

/**
 * @addtogroup	uart1
 * @ingroup		libsd_spi
 * @{
 */

/**
 * @file	ScatterWeb.Spi.h
 * @ingroup	libsdspi
 * @brief	Serial Peripheral Interface
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @version	$Revision: 1.1 $
 *
 * $Id: sdspi.h,v 1.1 2008/03/28 15:58:44 nvt-se Exp $
 */

#ifndef __SPI_H__
#define __SPI_H__

#ifndef SPI_DMA_READ
#define SPI_DMA_READ	0
#endif
#ifndef SPI_DMA_WRITE
#define SPI_DMA_WRITE	0
#endif
#ifndef SPI_WRITE
#define SPI_WRITE 		1
#endif

/**
 * @brief	Init SPI driver
 */
void sdspi_init(void);

/// chipselect
__inline void sdspi_select();
__inline void sdspi_unselect();

/**
 * @brief	Receive one byte from SPI
 */
uint8_t sdspi_rx(void);

/**
 * @brief 		Send one byte to SPI
 * 
 * @param[in]	c				Byte to send
 */
void sdspi_tx(const uint8_t c);

/**
 * @brief		Read a number of bytes from SPI
 * 
 * @param[in]	pDestination	Pointer to buffer to store bytestream
 * @param[in]	size			Number of bytes to store in pDestination
 * @param[in]	incDest			Increment destination pointer after each byte by one (or not)
 */
void sdspi_read(void *pDestination, const uint16_t size, const bool incDest);

#if SPI_DMA_WRITE
extern uint8_t sdspi_dma_lock;
#endif
#if SPI_DMA_READ || SPI_DMA_WRITE
void sdspi_dma_wait(void);
#endif

#if SPI_WRITE
	/**
	 * @brief		Write a number of bytes to SPI
	 * 
	 * @param[in]	pSource		Pointer to buffer with data to send
	 * @param[in]	size		Number of bytes to send
	 * @param[in]	startToken	First byte to send before starting to send size bytes from pSource
	 * @param[in]	incSource	Increment source pointer after each byte by one (or not)
	 */
void sdspi_write(const void *pSource,
		 const uint16_t size, const int increment);
#endif

/**
 * @brief		Wait a number of clock cycles
 * 
 * @param[in]	clocks		Wait clocks x 8 cycles
 */
void sdspi_idle(const uint16_t clocks);

/**
 * @brief		Read bytes from SPI until token is received
 * 
 * @param[in]	token	Token to wait for
 * @param[in]	timeout	Maximum number of bytes to read
 * @return		true if token received, false otherwise
 */
uint16_t sdspi_wait_token(const uint8_t feed, const uint8_t mask,
			  const uint8_t token, const uint16_t timeout);

#endif /*__SPI_H__*/

/** @} */
