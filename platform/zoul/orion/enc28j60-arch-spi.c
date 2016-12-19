/*
 * Copyright (c) 2014, CETIC.
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zolertia-orion-ethernet-router
 * @{
 *
 * \defgroup zolertia-eth-arch-spi Zolertia ENC28J60 SPI arch
 *
 * ENC28J60 eth-gw SPI arch specifics
 * @{
 *
 * \file
 * eth-gw SPI arch specifics
 */
/*---------------------------------------------------------------------------*/
#include "spi-arch.h"
#include "spi.h"
#include "dev/gpio.h"
/*---------------------------------------------------------------------------*/
#define RESET_PORT  GPIO_PORT_TO_BASE(ETH_RESET_PORT)
#define RESET_BIT   GPIO_PIN_MASK(ETH_RESET_PIN)
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_init(void)
{
  spix_init(ETH_SPI_INSTANCE);
  spix_cs_init(ETH_SPI_CSN_PORT, ETH_SPI_CSN_PIN);
  spix_set_mode(ETH_SPI_INSTANCE, SSI_CR0_FRF_MOTOROLA, 0, 0, 8);
  GPIO_SOFTWARE_CONTROL(RESET_PORT, RESET_BIT);
  GPIO_SET_OUTPUT(RESET_PORT, RESET_BIT);
  GPIO_SET_INPUT(RESET_PORT, RESET_BIT);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_select(void)
{
  SPIX_CS_CLR(ETH_SPI_CSN_PORT, ETH_SPI_CSN_PIN);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_deselect(void)
{
  SPIX_CS_SET(ETH_SPI_CSN_PORT, ETH_SPI_CSN_PIN);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_write(uint8_t output)
{
  SPIX_WAITFORTxREADY(ETH_SPI_INSTANCE);
  SPIX_BUF(ETH_SPI_INSTANCE) = output;
  SPIX_WAITFOREOTx(ETH_SPI_INSTANCE);
  SPIX_WAITFOREORx(ETH_SPI_INSTANCE);
  uint32_t dummy = SPIX_BUF(ETH_SPI_INSTANCE);
  (void) dummy;
}
/*---------------------------------------------------------------------------*/
uint8_t
enc28j60_arch_spi_read(void)
{
  SPIX_WAITFORTxREADY(ETH_SPI_INSTANCE);
  SPIX_BUF(ETH_SPI_INSTANCE) = 0;
  SPIX_WAITFOREOTx(ETH_SPI_INSTANCE);
  SPIX_WAITFOREORx(ETH_SPI_INSTANCE);
  return SPIX_BUF(ETH_SPI_INSTANCE);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

