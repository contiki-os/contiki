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
 * @file	ScatterWeb.Spi.c
 * @ingroup	libsdspi
 * @brief	Serial Peripheral Interface for SD library
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @version	$Revision: 1.2 $
 *
 * $Id: sdspi.c,v 1.2 2008/03/28 23:03:05 nvt-se Exp $
 */

#include <msp430x16x.h>
#include "contiki-msb430.h"

#if SPI_DMA_WRITE
uint8_t sdspi_dma_lock;
#endif

#ifndef U1IFG
#define U1IFG			IFG2
#endif

#define	SPI_IDLE_SYMBOL		0xFF

void
sdspi_init(void)
{
#if SPI_DMA_WRITE
  sdspi_dma_lock = FALSE;
#endif

  /* The 16-bit value of UxBR0+UxBR1 is the division factor of the USART clock
   * source, BRCLK. The maximum baud rate that can be generated in master
   * mode is BRCLK/2. The maximum baud rate that can be generated in slave
   * mode is BRCLK. The modulator in the USART baud rate generator is not used
   * for SPI mode and is recommended to be set to 000h. The UCLK frequency is
   * given by:
   * Baud rate = BRCLK / UxBR             with UxBR= [UxBR1, UxBR0]
   */
  uart_set_speed(UART_MODE_SPI, 0x02, 0x00, 0x00);
}

uint8_t
sdspi_rx(void)
{
  UART_TX = SPI_IDLE_SYMBOL;
  UART_WAIT_RX();
  return (UART_RX);
}

void
sdspi_tx(register const uint8_t c)
{
  UART_TX = c;
  UART_WAIT_TXDONE();
}

#if SPI_DMA_READ || SPI_DMA_WRITE
void
sdspi_dma_wait(void)
{
  while (DMA0CTL & DMAEN) {
    _NOP();
  }				// Wait until a previous transfer is complete
}
#endif

void
sdspi_read(void *pDestination, const uint16_t size, const bool incDest)
{
#if SPI_DMA_READ
  sdspi_dma_wait();

  UART_RESET_RXTX();		// clear interrupts

  // Configure the DMA transfer   
  DMA0SA = (uint16_t) & UART_RX;	// source DMA address
  DMA0DA = (uint16_t) pDestination;	// destination DMA address
  DMA0SZ = size;		// number of bytes to be transferred
  DMA1SA = (uint16_t) & UART_TX;	// source DMA address (constant 0xff)
  DMA1DA = DMA1SA;		// destination DMA address
  DMA1SZ = size - 1;		// number of bytes to be transferred
  DMACTL0 = DMA0TSEL_9 | DMA1TSEL_9;	// trigger is UART1 receive for both DMA0 and DMA1
  DMA0CTL = DMADT_0 |		// Single transfer mode
    DMASBDB |			// Byte mode
    DMADSTINCR0 | DMADSTINCR1 |	// Increment destination
    DMAEN;			// Enable DMA
  if (!incDest) {
    DMA0CTL &= ~(DMADSTINCR0 | DMADSTINCR1);
  }

  DMA1CTL = DMADT_0 |		// Single transfer mode 
    DMASBDB |			// Byte mode
    DMAEN;			// Enable DMA 

  UART_TX = SPI_IDLE_SYMBOL;	// Initiate transfer by sending the first byte
  sdspi_dma_wait();

#else
  register uint8_t *p = (uint8_t *) pDestination;
  register uint16_t i = size;

  do {
    UART_TX = SPI_IDLE_SYMBOL;
    UART_WAIT_RX();
    *p = UART_RX;
    if (incDest) {
      p++;
    }
    i--;
  } while (i);
#endif
}

#if SPI_WRITE
void
sdspi_write(const void *pSource, const uint16_t size, const int increment)
{
#if SPI_DMA_WRITE
  sdspi_dma_wait();

  UART_RESET_RXTX();		// clear interrupts

  // Configure the DMA transfer
  DMA0SA = ((uint16_t) pSource) + 1;	// source DMA address
  DMA0DA = (uint16_t) & UART_TX;	// destination DMA address
  DMA0SZ = size - 1;		// number of bytes to be transferred
  DMACTL0 = DMA0TSEL_9;		// trigger is UART1 receive
  DMA0CTL = DMADT_0 |		// Single transfer mode
    DMASBDB |			// Byte mode
    DMASRCINCR_3 |		// Increment source
    DMAEN;			// Enable DMA
  if (increment == 0) {
    DMA0CTL &= ~DMASRCINCR_3;
  }

  sdspi_dma_lock = TRUE;
  SPI_TX = ((uint8_t *) pSource)[0];
#else
  register uint8_t *p = (uint8_t *) pSource;
  register uint16_t i = size;

  do {
    UART_TX = *p;
    UART_WAIT_TXDONE();
    UART_RX;
    p += increment;
    i--;
  } while (i);
#endif
}
#endif

void
sdspi_idle(register const uint16_t clocks)
{
  register uint16_t i = clocks;

  do {
    UART_TX = SPI_IDLE_SYMBOL;
    UART_WAIT_RX();
    UART_RX;
    i--;
  } while (i);
}


uint16_t
sdspi_wait_token(const uint8_t feed, const uint8_t mask,
		 const uint8_t token, const uint16_t timeout)
{
  uint16_t i = 0;
  uint8_t rx;

  do {
    UART_TX = feed;
    UART_WAIT_RX();
    rx = UART_RX;
    i++;
  } while (((rx & mask) != token) && (i < timeout));
  return i;
}
