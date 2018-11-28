/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
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
 * This file is part of the Contiki operating system.
 */
/**
 * \addtogroup saker
 *
 * @{
 *
 * \defgroup saker-enc28j60 Ethernet chip driver arch implementation
 *
 * The arch file for the standard contiki enc28j60 ethernet chip driver. The chip
 * is addressed via SPI.
 *
 * @{
 *
 * \file
 *      Implementation of enc28j60.h
 */

#include "enc28j60.h"

#include "spi-arch.h"
#include "dev/gpio.h"
#include "dev/ioc.h"

/*---------------------------------------------------------------------------*/
static void
gpio_set_input(uint8_t PORT, uint8_t PIN)
{
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(PORT), GPIO_PIN_MASK(PIN));
  ioc_set_over(PORT, PIN, IOC_OVERRIDE_DIS);
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(PORT), GPIO_PIN_MASK(PIN));
}
/*---------------------------------------------------------------------------*/
static void
gpio_set_output(uint8_t PORT, uint8_t PIN)
{
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(PORT), GPIO_PIN_MASK(PIN));
  ioc_set_over(PORT, PIN, IOC_OVERRIDE_DIS);
  GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(PORT), GPIO_PIN_MASK(PIN));
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_init(void)
{
  /* CS_N output high */
  gpio_set_output(ENC28J60_CS_N_PORT, ENC28J60_CS_N_PIN);
  enc28j60_arch_spi_deselect();
  /* INT_N input */
  gpio_set_input(ENC28J60_INT_N_PORT, ENC28J60_INT_N_PIN);
  /* RESET_N output */
  gpio_set_output(ENC28J60_RESET_N_PORT, ENC28J60_RESET_N_PIN);
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(ENC28J60_RESET_N_PORT),
               GPIO_PIN_MASK(ENC28J60_RESET_N_PIN));
  /* Set up SPI */
  spix_init(ENC28J60_SPI_INSTANCE);
  spix_set_mode(ENC28J60_SPI_INSTANCE, SSI_CR0_FRF_MOTOROLA, 0, 0, 8);
}
/*---------------------------------------------------------------------------*/
uint8_t
enc28j60_arch_spi_write(uint8_t c)
{
  ENC28J60_SPI_WAITFORTxREADY();
  ENC28J60_SPI_TXBUF = c;
  ENC28J60_SPI_WAITFOREORx();
  return ENC28J60_SPI_RXBUF;
}
/*---------------------------------------------------------------------------*/
uint8_t
enc28j60_arch_spi_read(void)
{
  ENC28J60_SPI_WAITFORTxREADY();
  ENC28J60_SPI_TXBUF = 0;
  ENC28J60_SPI_WAITFOREORx();
  return ENC28J60_SPI_RXBUF;
}
/*---------------------------------------------------------------------------*/
int
enc28j60_arch_spi_rw_byte(uint8_t c)
{
  ENC28J60_SPI_WAITFORTxREADY();
  ENC28J60_SPI_TXBUF = c;
  ENC28J60_SPI_WAITFOREORx();
  return ENC28J60_SPI_RXBUF;
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_select(void)
{
  GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ENC28J60_CS_N_PORT),
               GPIO_PIN_MASK(ENC28J60_CS_N_PIN));
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_deselect(void)
{
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(ENC28J60_CS_N_PORT),
               GPIO_PIN_MASK(ENC28J60_CS_N_PIN));
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
