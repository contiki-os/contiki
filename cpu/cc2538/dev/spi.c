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
 * Implementation of the cc2538 SPI peripheral driver
 */
#include "contiki.h"
#include "reg.h"
#include "spi-arch.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"

#define SPI_CLK_PORT_BASE        GPIO_PORT_TO_BASE(SPI_CLK_PORT)
#define SPI_CLK_PIN_MASK         GPIO_PIN_MASK(SPI_CLK_PIN)
#define SPI_MOSI_PORT_BASE       GPIO_PORT_TO_BASE(SPI_MOSI_PORT)
#define SPI_MOSI_PIN_MASK        GPIO_PIN_MASK(SPI_MOSI_PIN)
#define SPI_MISO_PORT_BASE       GPIO_PORT_TO_BASE(SPI_MISO_PORT)
#define SPI_MISO_PIN_MASK        GPIO_PIN_MASK(SPI_MISO_PIN)

/**
 * \brief Initialize the SPI bus.
 *
 * This SPI init() function uses the following defines to set the pins:
 *    SPI_CLK_PORT               SPI_CLK_PIN
 *    SPI_MOSI_PORT              SPI_MOSI_PIN
 *    SPI_MISO_PORT              SPI_MISO_PIN
 *
 * This sets the mode to Motorola SPI with the following format options:
 *    Clock phase:               1; data captured on second (rising) edge
 *    Clock polarity:            1; clock is high when idle
 *    Data size:                 8 bits
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
  ioc_set_sel(SPI_CLK_PORT, SPI_CLK_PIN, IOC_PXX_SEL_SSI0_CLKOUT);
  ioc_set_sel(SPI_MOSI_PORT, SPI_MOSI_PIN, IOC_PXX_SEL_SSI0_TXD);
  REG(IOC_SSIRXD_SSI0) = (SPI_MISO_PORT * 8) + SPI_MISO_PIN;

  /* Put all the SSI gpios into peripheral mode */
  GPIO_PERIPHERAL_CONTROL(SPI_CLK_PORT_BASE, SPI_CLK_PIN_MASK);
  GPIO_PERIPHERAL_CONTROL(SPI_MOSI_PORT_BASE, SPI_MOSI_PIN_MASK);
  GPIO_PERIPHERAL_CONTROL(SPI_MISO_PORT_BASE, SPI_MISO_PIN_MASK);

  /* Disable any pull ups or the like */
  ioc_set_over(SPI_CLK_PORT, SPI_CLK_PIN, IOC_OVERRIDE_DIS);
  ioc_set_over(SPI_MOSI_PORT, SPI_MOSI_PIN, IOC_OVERRIDE_DIS);
  ioc_set_over(SPI_MISO_PORT, SPI_MISO_PIN, IOC_OVERRIDE_DIS);

  /* Configure the clock */
  REG(SSI0_BASE + SSI_CPSR) = 2;

  /* Configure the default SPI options.
   *   mode:  Motorola frame format
   *   clock: High when idle
   *   data:  Valid on rising edges of the clock
   *   bits:  8 byte data
   */
  REG(SSI0_BASE + SSI_CR0) = SSI_CR0_SPH | SSI_CR0_SPO | (0x07);

  /* Enable the SSI */
  REG(SSI0_BASE + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spi_cs_init(uint8_t port, uint8_t pin)
{
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  ioc_set_over(port, pin, IOC_OVERRIDE_DIS);
  GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
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
/*---------------------------------------------------------------------------*/
void
spi_set_mode(uint32_t frame_format, uint32_t clock_polarity,
             uint32_t clock_phase, uint32_t data_size)
{
  /* Disable the SSI peripheral to configure it */
  REG(SSI0_BASE + SSI_CR1) = 0;

  /* Configure the SSI options */
  REG(SSI0_BASE + SSI_CR0) = clock_phase | clock_polarity | frame_format | (data_size - 1);

  /* Re-enable the SSI */
  REG(SSI0_BASE + SSI_CR1) |= SSI_CR1_SSE;
}
/** @} */
