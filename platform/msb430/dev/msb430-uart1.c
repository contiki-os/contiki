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
 * @file		ScatterWeb.Uart.c
 * @addtogroup	interfaces
 * @brief		UART interface
 * @author		Michael Baar	<baar@inf.fu-berlin.de>
 *
 * UART switch for RS232 and SPI protocols on UART1 written for
 * ScatterWeb MSB boards. Compatible to ScatterWeb EOS,
 * ScatterWeb Bootload and Contiki.
 */

#include <io.h>
#include <signal.h>
#include <string.h>
#include "dev/msb430-uart1.h"
#include "msp430.h"
#include "dev/lpm.h"

#ifndef U1ME
#define U1ME	ME2
#endif

void _uart_configure(unsigned char mode);
void _uart_set_mode(unsigned char mode);

volatile unsigned char 		uart_mode = UART_MODE_RESET;
volatile unsigned char		uart_lockcnt = 0;
volatile uint8_t uart_edge = 0;

static unsigned char		_uart_speed_br0[UART_NUM_MODES];
static unsigned char		_uart_speed_br1[UART_NUM_MODES];
static unsigned char		_uart_speed_bmn[UART_NUM_MODES];
static fp_uart_handler		_uart_handler[UART_NUM_MODES] = {NULL, NULL};

/*---------------------------------------------------------------------------*/
void
uart_set_speed(unsigned char mode, unsigned char ubr0,
		unsigned char ubr1, unsigned char umctl)
{
  // store setting
  _uart_speed_br0[mode] = ubr0;		// baudrate
  _uart_speed_br1[mode] = ubr1;		// baudrate
  _uart_speed_bmn[mode] = umctl;	// modulation

  // reconfigure, if mode active
  if (uart_mode == mode)
    _uart_configure(mode);
}
/*---------------------------------------------------------------------------*/
void
uart_set_handler(unsigned char mode, fp_uart_handler fpHandler)
{
  // store setting
  _uart_handler[mode] = fpHandler;
  if (mode == uart_mode) {
    if (fpHandler == NULL)
      IE2 &= ~URXIE1;			// Disable USART1 RX interrupt
    else
      IE2 |= URXIE1;			// Enable USART1 RX interrupt
  }
}
/*---------------------------------------------------------------------------*/
int
uart_lock(unsigned char mode)
{
  // already locked?
  if ((mode != uart_mode) && (uart_lockcnt)) {
    return 0;
  } else {
    // increase lock count
    uart_lockcnt++;
    // switch mode (if neccessary)
    _uart_set_mode(mode);
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
int
uart_unlock(unsigned char mode)
{
  /*
  Do we wan't strict checking?
  if( (uart_lockcnt == 0) || (mode != uart_mode) )
    return false;
  */

  // decrement lock
  if (uart_lockcnt > 0) {
    uart_lockcnt--;

    // if no more locks, switch back to default mode
    if (uart_lockcnt == 0) {
      _uart_set_mode(UART_MODE_DEFAULT);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
_uart_configure(unsigned char mode)
{
  _DINT();					// disable interrupts

  UART_WAIT_TXDONE();							
// wait till all buffered data has been transmitted

  // configure
  if (mode == UART_MODE_RS232) {
    P5OUT |= 0x01;							
    // unselect SPI
    P3SEL |= 0xC0;							
    // select rs232
    // to RS232 mode
    UCTL1 = SWRST | CHAR;		// 8-bit character
    UTCTL1 |= SSEL1;			// UCLK = MCLK
    // activate
    U1ME |= UTXE1 | URXE1;		// Enable USART1 TXD/RXD		
  } else if( mode == UART_MODE_SPI ) {
    P3SEL &= ~0xC0;			// unselect RS232
    // to SPI mode
    UCTL1 = SWRST | CHAR | SYNC | MM;	// 8-bit SPI Master
    /*
     * SMCLK, 3-pin mode, clock idle low, data valid on
     * rising edge, UCLK delayed
     */
    UTCTL1 |= CKPH | SSEL1 | SSEL0 | STC;	// activate
    U1ME |= USPIE1;				// Enable USART1 SPI
  }

  // restore speed settings
  UBR01  = _uart_speed_br0[mode];	// set baudrate
  UBR11  = _uart_speed_br1[mode];			
  UMCTL1 = _uart_speed_bmn[mode];	// set modulation

  UCTL1 &= ~SWRST;			// clear reset flag
  _EINT();				// enable interrupts
}
/*---------------------------------------------------------------------------*/
void
_uart_set_mode(unsigned char mode)
{
  // do nothing if mode already set
  if (mode == uart_mode)
    return;

  IE2 &= ~(URXIE1 | UTXIE1);		// disable irq
  _uart_configure(mode);		// configure uart parameters
  uart_mode = mode;
	
  if (_uart_handler[mode] != NULL)
    IE2 |= URXIE1;			// Enable USART1 RX interrupt
}
/*---------------------------------------------------------------------------*/
int
uart_get_mode(void)
{
  return uart_mode;
}
/*---------------------------------------------------------------------------*/
interrupt(UART1RX_VECTOR)_uart_rx(void)
{
  fp_uart_handler handler = _uart_handler[uart_mode];
  int c;
	
  if (!(IFG2 & URXIFG1)) {
    // If start edge detected, toggle & return	
    uart_edge = 1;
    U1TCTL &= ~URXSE;
    U1TCTL |= URXSE;
    _BIC_SR_IRQ(LPM3_bits);
    return;
  }
  uart_edge = 0;
  if (!(URCTL1 & RXERR)) {
    c = UART_RX;
    if (handler(c)) {
      _BIC_SR_IRQ(LPM3_bits);
    }
  } else {
    // read out the char to clear the I-flags, etc.
    c = UART_RX;
  }
}
