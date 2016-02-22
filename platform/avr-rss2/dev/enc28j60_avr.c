/*
 * Copyright (c) 2012-2013, Robert Olsson <robert@radio-sensors.com>
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <avr/pgmspace.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "dev/watchdog.h"
#include "contiki.h"
#include "i2c.h"
#include <compat/twi.h>
#include <stdio.h>
#include <string.h>
#include "enc28j60_avr.h"

#include <util/delay_basic.h>
#define delay_us(us)   (_delay_loop_2(1 + (us * F_CPU) / 4000000UL))

void
enc28j60_arch_spi_init(void)
{
  CS_SPI_DDR |= (1 << SPI_CS);
  CS_SPI_PORT |= (1 << SPI_CS);

  SPI_DDR |= (1 << SPI_MOSI) | (1 << SPI_SCK);
  SPI_DDR &= ~(1 << SPI_MISO);
  SPI_PORT &= ~(1 << SPI_MOSI);
  SPI_PORT &= ~(1 << SPI_SCK);
  SPCR = (1 << SPE) | (1 << MSTR);
  /* SPSR |= (1<<SPI2X); */
}
uint8_t
enc28j60_arch_spi_write(uint8_t data)
{
  SPDR = data;
  while(!(SPSR & (1 << SPIF))) ;
  return SPDR;
}
uint8_t
enc28j60_arch_spi_read(void)
{
  SPDR = 0xAA; /* dummy */
  while(!(SPSR & (1 << SPIF))) ;
  return SPDR;
}
void
enc28j60_arch_spi_select(void)
{
  CS_SPI_PORT &= ~(1 << SPI_CS);
  delay_us(1000);
}
void
enc28j60_arch_spi_deselect(void)
{
  CS_SPI_PORT |= (1 << SPI_CS);
}
