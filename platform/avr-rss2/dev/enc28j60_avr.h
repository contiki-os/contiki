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

#ifndef ENC28J60_AVR_H
#define ENC28J60_AVR_H

#define SPI_DDR  DDRB   /* Data Direction Register: Port B */
#define SPI_PORT PORTB  /* Serial Peripheral Interface */
#define SPI_MOSI 2      /* PB2: Master Out Slave In */
#define SPI_MISO 3      /* PB3: Master In Slave out */
#define SPI_SCK  1      /* PB1: Serial Clock */

#define CS_SPI_DDR  DDRD   /* Data Direction Register: Port B */
#define CS_SPI_PORT PORTD  /* Serial Peripheral Interface */
#define SPI_CS   6      /* PD6: OW2_PIN, Chip Select */

/* AVR specific's for enc28j60 */

void enc28j60_arch_spi_init(void);
uint8_t enc28j60_arch_spi_write(uint8_t data);
uint8_t enc28j60_arch_spi_read(void);
void enc28j60_arch_spi_select(void);
void enc28j60_arch_spi_deselect(void);

#endif /* ENC28J60_AVR_H */
