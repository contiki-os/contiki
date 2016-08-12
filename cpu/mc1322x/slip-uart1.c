/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
 *
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
 * This file is part of the Contiki OS.
 *
 *
 */

/*
 * Machine dependent mc1322x SLIP routines for UART1.
 */

#include "contiki.h"
#include "dev/slip.h"

#include "mc1322x.h"

/*---------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  uart1_putc(c);
}
/*---------------------------------------------------------------------------*/
/*
 * The serial line is used to transfer IP packets using slip. To make
 * it possible to send debug output over the same line we send debug
 * output as slip frames (i.e delimeted by SLIP_END).
 *
 */
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV4
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
#endif /* NETSTACK_CONF_WITH_IPV4 */
/*---------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port and the SLIP driver.
 *
 */
void
slip_arch_init(unsigned long ubr)
{
  uart1_set_input(slip_input_byte);
}
/*---------------------------------------------------------------------------*/
