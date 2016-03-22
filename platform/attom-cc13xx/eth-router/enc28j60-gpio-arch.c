/*
 * Copyright (c) 2016, Atto Engenharia de Sistemas - http://attosistemas.com.br/
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup attom-cc13xx-eth-router-peripherals
 * @{
 *
 * \file
 * Implementation to the ENC28j60 I/O connections on the AttoM Ethernet Router
 * with a TI CC1310.
 */
/*---------------------------------------------------------------------------*/
#include "clock.h"
#include "ti-lib.h"
/*---------------------------------------------------------------------------*/
/* Delay in us */
#define DELAY 10
/*---------------------------------------------------------------------------*/
inline static void
delay(void)
{
  clock_delay_usec(DELAY);
}
/*---------------------------------------------------------------------------*/
static uint8_t
enc28j60_arch_spi_write_byte(uint8_t output)
{
  int i;
  uint8_t input;
  input = 0;

  for(i = 0; i < 8; i++) {
    /* Write data on MOSI pin */
    if(output & 0x80) {
      ti_lib_gpio_pin_write(BOARD_SPI_ETHERNET_MOSI, 1);
    } else {
      ti_lib_gpio_pin_clear(BOARD_SPI_ETHERNET_MOSI);
    }
    output <<= 1;

    /* Set clock high  */
    ti_lib_gpio_pin_write(BOARD_SPI_ETHERNET_SCK, 1);
    delay();

    /* Read data from MISO pin */
    input <<= 1;
    if(ti_lib_gpio_pin_read(BOARD_SPI_ETHERNET_MISO) != 0) {
      input |= 0x1;
    }

    /* Set clock low */
    ti_lib_gpio_pin_clear(BOARD_SPI_ETHERNET_SCK);
    delay();
  }
  return input;
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_init(void)
{
  /* GPIO pin configuration */

  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_ETHERNET_MISO);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_SPI_ETHERNET_MOSI);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_SPI_ETHERNET_SCK);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_SPI_ETHERNET_CS);

  /* Default output to clear chip select */
  ti_lib_gpio_pin_write(BOARD_SPI_ETHERNET_CS, 1);

  /* The CLK is active low, we set it high when we aren't using it. */
  ti_lib_gpio_pin_clear(BOARD_SPI_ETHERNET_SCK);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_select(void)
{
  ti_lib_gpio_pin_write(BOARD_SPI_ETHERNET_CS, 0);
  delay();
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_deselect(void)
{
  ti_lib_gpio_pin_write(BOARD_SPI_ETHERNET_CS, 1);
}
/*---------------------------------------------------------------------------*/
uint8_t
enc28j60_arch_spi_read(void)
{
  return enc28j60_arch_spi_write_byte(0);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_write(uint8_t output)
{
  enc28j60_arch_spi_write_byte(output);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
