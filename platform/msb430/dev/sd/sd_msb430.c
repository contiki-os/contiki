
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
 * @file	ScatterWeb.Sd.MSB430.c
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library initialisation for MSB430
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @date	Jan 2007
 * @version	$Revision: 1.1 $
 * 
 * Replace this file for use on another platform.
 *
 * $Id: sd_msb430.c,v 1.1 2008/03/28 15:58:44 nvt-se Exp $
 */

#include "sd_internals.h"

void
sd_init_platform(void)
{
  sdspi_init();

  P5SEL |= 0x0E;		// 00 00 11 10  -> Dout, Din, Clk = peripheral (now done in UART module)
  P5SEL &= ~0x01;		// 00 00 00 01  -> Cs = I/O
  P5OUT |= 0x01;		// 00 00 00 01  -> Cs = High
  P5DIR |= 0x0D;		// 00 00 11 01  -> Dout, Clk, Cs = output
  P5DIR &= ~0x02;		// 00 00 00 10  -> Din = Input
  P2SEL &= ~0x40;		// 11 00 00 00  -> protect, detect = I/O
  P2DIR &= ~0x40;		// 11 00 00 00  -> protect, detect = input     
}

/**
 * @brief Activate SD Card on SPI Bus
 * \internal
 */
__inline void
sdspi_select()
{
  P5OUT &= ~0x01;		// Card Select
}

__inline void
sdspi_unselect()
{
  UART_WAIT_TXDONE();

  P5OUT |= 0x01;		// Card Deselect
  sdspi_tx(0xFF);
}
