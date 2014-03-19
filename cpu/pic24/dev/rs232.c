/*
 * Copyright (c) 2012, Alex Barclay.
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
 *
 *
 * Author: Alex Barclay <alex@planet-barclay.com>
 *
 */

#include <p33Fxxxx.h>

#define STDIN   0
#define STDOUT  1
#define STDERR  2

#include "rs232.h"

void
rs232_init(int baud)
{
  int i;
  _DOZE = 0b000;
  _RP0R = 0b00011; /* Map U1TX to RP1 */

  U1STAbits.UTXEN = 0; /* Enable UART Tx */
  U1MODEbits.UARTEN = 0; /* Enable UART */

#ifdef NOBOOTLOADER
  U1MODEbits.STSEL = 0; /* 1-stop bit */
  U1MODEbits.PDSEL = 0; /* No Parity, 8-data bits */
  U1MODEbits.ABAUD = 0; /* Auto-Baud Disabled */
  U1MODEbits.BRGH = 0; /* Low Speed mode */
  U1BRG = 64; /* BAUD Rate Setting for 9600=259, 38400=64 */
#endif

  U1STAbits.UTXISEL0 = 0; /* Interrupt after one Tx character is transmitted */
  U1STAbits.UTXISEL1 = 0;

  U1MODEbits.UARTEN = 1; /* Enable UART */
  U1STAbits.UTXEN = 1; /* Enable UART Tx */

  /* wait at least 104 usec (1/9600) before sending first char */
  for(i = 0; i < 16384; i++) {
    Nop();
  }
}
int __attribute__((__weak__, __section__(".libc")))
write(int handle, void *buffer, unsigned int len)
{
  int i = 0;
  switch(handle) {
  case STDOUT:
  case STDERR:
    while(i < len) putchar(((char *)buffer)[i++]);
    break;
  }
  return len;    /* number of characters written */
}
int
putchar(int c)
{
  while(U1STAbits.UTXBF) ;
  U1TXREG = (char)c;

  return c;
}
int
puts(const char *s)
{
  while(*s != '\0') {
    putchar(*s++); /* Transmit one character */
  }
  putchar('\n');

  return 1;
}
