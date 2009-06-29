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
 * @addtogroup interfaces
 * @{ */

/**
 * @defgroup uart1 UART1
 * The UART module multiplexes differenct protocol on the MSB's UART1
 * interface. Currently RS232 and SPI are supported.
 * @{
 */

/**
 * \file	Header file for for the MSB430 UART driver.
 * \author	Michael Baar	<baar@inf.fu-berlin.de>
 */

#ifndef MSB430_UART1_H
#define MSB430_UART1_H

#define	UART_RX				RXBUF1
#define	UART_TX				TXBUF1
#define UART_RESET_RX()		do { U1IFG &= ~URXIFG1; } while(0)
#define	UART_RESET_RXTX()	do { U1IFG &= ~(URXIFG1 | UTXIFG1); } while(0)
#define	UART_WAIT_RX()		while((U1IFG & URXIFG1) == 0) { _NOP(); }
#define	UART_WAIT_TX()		while((U1IFG & UTXIFG1) == 0) { _NOP(); } 
#define UART_WAIT_TXDONE()	while((UTCTL1 & TXEPT) == 0) { _NOP(); }

/**
 * @brief Operating state
 */
extern volatile unsigned char uart_mode;
extern volatile unsigned char uart_lockcnt;

/**
 * @name	UART mode flags
 * @{
 */
#define	UART_MODE_RS232		(0x00u)			///< RS232 mode
#define	UART_MODE_SPI		(0x01u)			///< SPI mode
#define UART_MODE_DEFAULT	UART_MODE_RS232
#define UART_NUM_MODES		(UART_MODE_SPI + 1)	///< Highest mode number
#define UART_MODE_RESET		(0xFFu)			///< reset with current settings
/** @} */

#define UART_WAIT_LOCK(x)	((uart_mode != x) && (uart_lockcnt))
#define UART_MODE_IS(x)		(uart_mode == x)

typedef int(*uart_handler_t)(unsigned char);

/**
 * \brief      Initialize the UART module
 *
 *             This function is called from the boot up code to
 *             initalize the UART module.
 */
void uart_init(void);

void uart_set_speed(unsigned, unsigned, unsigned, unsigned);
void uart_set_handler(unsigned, uart_handler_t);
int uart_lock(unsigned);
int uart_lock_wait(unsigned);
int uart_unlock(unsigned);
void uart_set_mode(unsigned);
int uart_get_mode(void);

#endif /* !MSB430_UART1_H */

/** @} */
/** @} */
