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
#include "spi-arch.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"

/* Default: Motorola mode 3 with 8-bit data words */
#ifndef SPI_CONF_PHASE
#define SPI_CONF_PHASE           SSI_CR0_SPH
#endif
#ifndef SPI_CONF_POLARITY
#define SPI_CONF_POLARITY        SSI_CR0_SPO
#endif
#ifndef SPI_CONF_DATA_SIZE
#define SPI_CONF_DATA_SIZE       8
#endif

#if SPI_CONF_DATA_SIZE < 4 || SPI_CONF_DATA_SIZE > 16
#error SPI_CONF_DATA_SIZE must be set between 4 and 16 inclusive.
#endif

/**
 * \brief Initialize the SPI bus.
 *
 * This SPI init() function uses the following #defines to set the pins:
 *    CC2538_SPI_CLK_PORT_NUM    CC2538_SPI_CLK_PIN_NUM
 *    CC2538_SPI_MOSI_PORT_NUM   CC2538_SPI_MOSI_PIN_NUM
 *    CC2538_SPI_MISO_PORT_NUM   CC2538_SPI_MISO_PIN_NUM
 *    CC2538_SPI_SEL_PORT_NUM    CC2538_SPI_SEL_PIN_NUM
 *
 * This sets the mode to Motorola SPI with the following format options:
 *    SPI_CONF_PHASE:            0 or SSI_CR0_SPH
 *    SPI_CONF_POLARITY:         0 or SSI_CR0_SPO
 *    SPI_CONF_DATA_SIZE:        4 to 16 bits
 */
void
spi_init(void)
{
  spi_enable();

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

  /* Put the ssi in Motorola SPI mode using the provided format options */
  REG(SSI0_BASE + SSI_CR0) = SPI_CONF_PHASE | SPI_CONF_POLARITY | (SPI_CONF_DATA_SIZE - 1);

  /* Enable the SSI */
  REG(SSI0_BASE + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spi_enable(void)
{
  /* Enable the clock for the SSI peripheral */
  REG(SYS_CTRL_RCGCSSI) |= 1;
}
/*---------------------------------------------------------------------------*/
void
spi_disable(void)
{
  /* Gate the clock for the SSI peripheral */
  REG(SYS_CTRL_RCGCSSI) &= ~1;
}
/** @} */
