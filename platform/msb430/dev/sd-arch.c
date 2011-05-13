/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *	Architecture-dependent functions for SD over SPI.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "msb430-uart1.h"
#include "sd-arch.h"
#include <io.h>

#define SPI_IDLE	0xff

int
sd_arch_init(void)
{
  P2SEL &= ~64;
  P2DIR &= ~64;

  P5SEL |= 14;
  P5SEL &= ~1;
  P5OUT |= 1;
  P5DIR |= 13;
  P5DIR &= ~2;

  uart_set_speed(UART_MODE_SPI, 2, 0, 0);

  return 0;
}


void
sd_arch_spi_write(int c)
{
  UART_TX = c;
  UART_WAIT_TXDONE();
}

void
sd_arch_spi_write_block(uint8_t *bytes, int amount)
{
  int i;
  volatile char dummy;

  for(i = 0; i < amount; i++) {
    UART_TX = bytes[i];
    UART_WAIT_TXDONE();
    UART_WAIT_RX();
    dummy = UART_RX;
  }
}


unsigned
sd_arch_spi_read(void)
{
  if((U1IFG & URXIFG1) == 0) {
    UART_TX = SPI_IDLE;
    UART_WAIT_RX();
  }
  return UART_RX;
}
