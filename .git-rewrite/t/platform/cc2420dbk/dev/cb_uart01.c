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
 * @(#)$Id: cb_uart01.c,v 1.1 2007/02/02 14:09:06 bg- Exp $
 */

#include <stdio.h>

#include <avr/interrupt.h>

#include "contiki.h"

#include "dev/slip.h"

static int
uart0_putchar(char c, FILE *stream)
{
  while (!(UCSR0A & BV(UDRE0)))
    ;
  UDR0 = c;

  return c;
}

static FILE uart0_stdout =
FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_WRITE);

void
slip_arch_init(unsigned long ubr)
{
  u8_t dummy;
  spl_t s = splhigh();

  UBRR1L = ubr; 
  UBRR1H = ubr >> 8;

  UCSR1C = BV(UCSZ1) | BV(UCSZ0); /* 1 start bit, no parity, 1 stop bit */
  UCSR1B = BV(RXEN) | BV(TXEN) | BV(RXCIE);
  dummy = UDR1;			/* Flush RX buffer. */

  /* And now UART0. */
  UBRR0L = BAUD2UBR(38400); 
  UBRR0H = BAUD2UBR(38400) >> 8;

  UCSR0C = BV(UCSZ1) | BV(UCSZ0); /* 1 start bit, no parity, 1 stop bit */
  UCSR0B = BV(TXEN);		/* TX ONLY!!! */

  splx(s);

  stdout = &uart0_stdout;
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
