/*
 * Copyright (c) 2013, University of Michigan.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 */
/**
 * \addtogroup cc2538-spi
 * @{
 *
 * \file
 * Implementation of the cc2538 SPI peripheral
 */
#include "contiki.h"
#include "reg.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"
#include "spi-arch.h"

/**
 * \brief Initialize the SPI bus.
 *
 * This SPI init() function uses the following #defines to set the pins:
 *    CC2538_SPI_CLK_PORT_NUM    CC2538_SPI_CLK_PIN_NUM
 *    CC2538_SPI_MOSI_PORT_NUM   CC2538_SPI_MOSI_PIN_NUM
 *    CC2538_SPI_MISO_PORT_NUM   CC2538_SPI_MISO_PIN_NUM
 *    CC2538_SPI_SEL_PORT_NUM    CC2538_SPI_SEL_PIN_NUM
 *
 * This sets the SPI data width to 8 bits and the mode to Freescale mode 3.
 */
void
spi_init(void)
{
  /* Enable the SSI peripheral */
  REG(SYS_CTRL_RCGCSSI) |= 1;

  /* Start by disabling the peripheral before configuring it */
  REG(SSI0_BASE + SSI_CR1) = 0;

  /* Set the IO clock as the SSI clock */
  REG(SSI0_BASE + SSI_CC) = 1;

  /* Set the mux correctly to connect the SSI pins to the correct GPIO pins */
  ioc_set_sel(CC2538_SPI_CLK_PORT_NUM, CC2538_SPI_CLK_PIN_NUM, IOC_PXX_SEL_SSI0_CLKOUT);
  ioc_set_sel(CC2538_SPI_MOSI_PORT_NUM, CC2538_SPI_MOSI_PIN_NUM, IOC_PXX_SEL_SSI0_TXD);
  REG(IOC_SSIRXD_SSI0) = (CC2538_SPI_MISO_PORT_NUM * 8) + CC2538_SPI_MISO_PIN_NUM;
  ioc_set_sel(CC2538_SPI_SEL_PORT_NUM, CC2538_SPI_SEL_PIN_NUM, IOC_PXX_SEL_SSI0_FSSOUT);

  /* Put all the SSI gpios into peripheral mode */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_CLK_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_CLK_PIN_NUM));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_MOSI_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_MOSI_PIN_NUM));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_MISO_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_MISO_PIN_NUM));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_SEL_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_SEL_PIN_NUM));

  /* Disable any pull ups or the like */
  ioc_set_over(CC2538_SPI_CLK_PORT_NUM, CC2538_SPI_CLK_PIN_NUM, IOC_OVERRIDE_DIS);
  ioc_set_over(CC2538_SPI_MOSI_PORT_NUM, CC2538_SPI_MOSI_PIN_NUM, IOC_OVERRIDE_DIS);
  ioc_set_over(CC2538_SPI_MISO_PORT_NUM, CC2538_SPI_MISO_PIN_NUM, IOC_OVERRIDE_DIS);
  ioc_set_over(CC2538_SPI_SEL_PORT_NUM, CC2538_SPI_SEL_PIN_NUM, IOC_OVERRIDE_DIS);

  /* Configure the clock */
  REG(SSI0_BASE + SSI_CPSR) = 2;

  /* Put the ssi in motorola SPI mode with 8 bit data */
  REG(SSI0_BASE + SSI_CR0) = SSI_CR0_SPH_M | SSI_CR0_SPO_M | (7);

  /* Enable the SSI */
  REG(SSI0_BASE + SSI_CR1) |= SSI_CR1_SSE;

  /* Clear the RX FIFO */
  SPI_WAITFOREORx();
}
/** @} */
