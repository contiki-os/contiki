
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
 * @date	2007
 * @version	1.0
 */

#include <io.h>
#include <signal.h>
#include <string.h>

#include "contiki-msb430.h"

#define	SPI_IDLE_SYMBOL		0xFF

void
spi_init(void)
{

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

uint8_t_t
spi_rx(void)
{
  UART_RESET_RX();
  UART_TX = SPI_IDLE_SYMBOL;
  UART_WAIT_RX();
  return (UART_RX);
}

void
spi_tx(register const uint8_t_t c)
{
  UART_RESET_RX();
  UART_TX = c;
  UART_WAIT_RX();
}

void
spi_read(void *pDestination, const uint16_t size, const bool incDest)
{
  register uint8_t *p = (uint8_t *) pDestination;
  register uint16_t i;

  for (i = size; i > 0; i--) {
    *p = spi_rx();
    if (incDest)
      p++;
  }
}

void
spi_write(const void *pSource, const uint16_t size, const uint8_t startToken,
	  const bool incSource)
{
  register unsigned char *p = (unsigned char *) pSource;
  register uint16_t i;

  spi_tx(startToken);
  for (i = size; i > 0; i--) {
    spi_tx(*p);
    if (incSource)
      p++;
  }
}

void
spi_idle(register const uint16_t clocks)
{
  register uint16_t i;

  for (i = 0; i < clocks; i++) {
    UART_RESET_RX();
    UART_TX = SPI_IDLE_SYMBOL;
    UART_WAIT_RX();
  }
}

bool
spi_wait_token(const uint8_t token, const uint16_t timeout)
{
  uint16_t i;
  uint8_t rx;

  for (i = 0; i < timeout; i++) {
    rx = spi_rx();
    if (rx == token)
      return TRUE;
  }
  return FALSE;
}
