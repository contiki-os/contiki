/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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

/*
 * Machine dependent AVR SLIP routines for UART1.
 */

#include <stdio.h>

#include <avr/interrupt.h>

#include "contiki.h"

#include "dev/slip.h"

static int
slip_putchar(char c, FILE *stream)
{
#define SLIP_END 0300
  static char debug_frame = 0;

  if (!debug_frame) {		/* Start of debug output */
    slip_arch_writeb(SLIP_END);
    slip_arch_writeb('\r'); /* Type debug line == '\r' */
    debug_frame = 1;
  }

  slip_arch_writeb((unsigned char)c);
  
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

static FILE slip_stdout =
FDEV_SETUP_STREAM(slip_putchar, NULL, _FDEV_SETUP_WRITE);

void
slip_arch_init(unsigned long ubr)
{
  uint8_t dummy;
  spl_t s = splhigh();

  UBRR1L = ubr;
  UBRR1H = ubr >> 8;		/* Always zero */

  UCSR1C = BV(UCSZ1) | BV(UCSZ0); /* 1 start bit, no parity, 1 stop bit */
  UCSR1B = BV(RXEN) | BV(TXEN) | BV(RXCIE);
  dummy = UDR1;			/* Flush RX buffer. */

  splx(s);

  stdout = &slip_stdout;
}

void
slip_arch_writeb(unsigned char c)
{
  while (!(UCSR1A & BV(UDRE1)))
    ;
  UDR1 = c;
}

ISR(SIG_UART1_RECV)
{
  /* Should deal with receive errors. */
  slip_input_byte(UDR1);
}
