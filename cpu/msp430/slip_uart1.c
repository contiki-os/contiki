/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: slip_uart1.c,v 1.7 2007/05/22 21:00:27 adamdunkels Exp $
 */

/*
 * Machine dependent MSP430 SLIP routines for UART1.
 */

#include <io.h>
#include <signal.h>

#include "contiki.h"

#include "dev/slip.h"

void
slip_arch_writeb(unsigned char c)
{
  /* Loop until the transmission buffer is available. */
  while ((IFG2 & UTXIFG1) == 0);

  /* Transmit the data. */
  TXBUF1 = c;
}

/*
 * The serial line is used to transfer IP packets using slip. To make
 * it possible to send debug output over the same line we send debug
 * output as slip frames (i.e delimeted by SLIP_END).
 *
 */
int
putchar(int c)
{
#define SLIP_END 0300
  static char debug_frame = 0;

  if (!debug_frame) {		/* Start of debug output */
    slip_arch_writeb(SLIP_END);
    slip_arch_writeb('\r');	/* Type debug line == '\r' */
    debug_frame = 1;
  }

  slip_arch_writeb((char)c);
  
  /*
   * Line buffered output, a newline marks the end of debug output and
   * implicitly flushes debug output.
   */
  if (c == '\n') {
    slip_arch_writeb(SLIP_END);
    debug_frame = 0;
  }

  return c;
}

/**
 * Initalize the RS232 port and the SLIP driver.
 *
 */
void
slip_arch_init(unsigned long ubr)
{
  /* RS232 */
  P3DIR &= ~0x80;			/* Select P37 for input (UART1RX) */
  P3DIR |= 0x40;			/* Select P36 for output (UART1TX) */
  P3SEL |= 0xC0;			/* Select P36,P37 for UART1{TX,RX} */

  UCTL1 = SWRST | CHAR;                 /* 8-bit character, UART mode */

#if 0
   U1RCTL &= ~URXEIE; /* even erroneous characters trigger interrupts */
#endif

  UTCTL1 = SSEL1;                       /* UCLK = MCLK */

  UBR01 = ubr;
  UBR11 = ubr >> 8;		/* always zero */
  /*
   * UMCTL1 values calculated using
   * http://mspgcc.sourceforge.net/baudrate.html and are not
   * complete. Also the table assumes that F_CPU = 2,457,600 Hz.
   */
  switch (ubr) {
  case BAUD2UBR(115200):
    UMCTL1 = 0x4a;
    break;
  case BAUD2UBR(57600):
    UMCTL1 = 0x5b;
    break;
  case BAUD2UBR(19600):
    UMCTL1 = 0x4a;
    break;
  default:
    UMCTL1 = 0x00;
  }

  ME2 &= ~USPIE1;			/* USART1 SPI module disable */
  ME2 |= (UTXE1 | URXE1);               /* Enable USART1 TXD/RXD */

  UCTL1 &= ~SWRST;

  /* XXX Clear pending interrupts before enable!!! */

  IE2 |= URXIE1;                        /* Enable USART1 RX interrupt  */
}

interrupt(UART1RX_VECTOR)
__uart1_intr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* Check status register for receive errors. */
  if(URCTL1 & RXERR) {
    volatile unsigned dummy;
    dummy = RXBUF1;   /* Clear error flags by forcing a dummy read. */
  } else {
    if(slip_input_byte(RXBUF1)) {
      LPM4_EXIT;
    }
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
