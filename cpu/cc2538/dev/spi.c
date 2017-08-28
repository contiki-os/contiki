/*
 * Copyright (c) 2013, University of Michigan.
 *
 * Copyright (c) 2015, Weptech elektronik GmbH
 * Author: Ulf Knoblich, ulf.knoblich@weptech.de
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
#include "sys/cc.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"
/*---------------------------------------------------------------------------*/
/* Check port / pin settings for SPI0 and provide default values for spi_cfg */
#ifndef SPI0_CLK_PORT
#define SPI0_CLK_PORT         (-1)
#endif
#ifndef SPI0_CLK_PIN
#define SPI0_CLK_PIN          (-1)
#endif
#if SPI0_CLK_PORT >= 0 && SPI0_CLK_PIN < 0 || \
  SPI0_CLK_PORT < 0 && SPI0_CLK_PIN >= 0
#error Both SPI0_CLK_PORT and SPI0_CLK_PIN must be valid or invalid
#endif

#ifndef SPI0_TX_PORT
#define SPI0_TX_PORT          (-1)
#endif
#ifndef SPI0_TX_PIN
#define SPI0_TX_PIN           (-1)
#endif
#if SPI0_TX_PORT >= 0 && SPI0_TX_PIN < 0 || \
  SPI0_TX_PORT < 0 && SPI0_TX_PIN >= 0
#error Both SPI0_TX_PORT and SPI0_TX_PIN must be valid or invalid
#endif

#ifndef SPI0_RX_PORT
#define SPI0_RX_PORT          (-1)
#endif
#ifndef SPI0_RX_PIN
#define SPI0_RX_PIN           (-1)
#endif
#if SPI0_RX_PORT >= 0 && SPI0_RX_PIN < 0 || \
  SPI0_RX_PORT < 0 && SPI0_RX_PIN >= 0
#error Both SPI0_RX_PORT and SPI0_RX_PIN must be valid or invalid
#endif

/* Here we check that either all or none of the ports are defined. As
   we did already check that both ports + pins are either defined or
   not for every pin, this means that we can check for an incomplete
   configuration by only looking at the port defines */
/* If some SPI0 pads are valid */
#if SPI0_CLK_PORT >= 0 || SPI0_TX_PORT >= 0 || SPI0_RX_PORT >= 0
/* but not all */
#if SPI0_CLK_PORT < 0 || SPI0_TX_PORT < 0 || SPI0_RX_PORT < 0
#error Some SPI0 pad definitions are invalid
#endif
#define SPI0_PADS_VALID
#endif
/*---------------------------------------------------------------------------*/
/* Check port / pin settings for SPI1 and provide default values for spi_cfg */
#ifndef SPI1_CLK_PORT
#define SPI1_CLK_PORT         (-1)
#endif
#ifndef SPI1_CLK_PIN
#define SPI1_CLK_PIN          (-1)
#endif
#if SPI1_CLK_PORT >= 0 && SPI1_CLK_PIN < 0 || \
  SPI1_CLK_PORT < 0 && SPI1_CLK_PIN >= 0
#error Both SPI1_CLK_PORT and SPI1_CLK_PIN must be valid or invalid
#endif

#ifndef SPI1_TX_PORT
#define SPI1_TX_PORT          (-1)
#endif
#ifndef SPI1_TX_PIN
#define SPI1_TX_PIN           (-1)
#endif
#if SPI1_TX_PORT >= 0 && SPI1_TX_PIN < 0 || \
  SPI1_TX_PORT < 0 && SPI1_TX_PIN >= 0
#error Both SPI1_TX_PORT and SPI1_TX_PIN must be valid or invalid
#endif

#ifndef SPI1_RX_PORT
#define SPI1_RX_PORT          (-1)
#endif
#ifndef SPI1_RX_PIN
#define SPI1_RX_PIN           (-1)
#endif
#if SPI1_RX_PORT >= 0 && SPI1_RX_PIN < 0 || \
  SPI1_RX_PORT < 0 && SPI1_RX_PIN >= 0
#error Both SPI1_RX_PORT and SPI1_RX_PIN must be valid or invalid
#endif

/* If some SPI1 pads are valid */
#if SPI1_CLK_PORT >= 0 || SPI1_TX_PORT >= 0 || SPI1_RX_PORT >= 0
/* but not all */
#if SPI1_CLK_PORT < 0 || SPI1_TX_PORT < 0 || SPI1_RX_PORT < 0
#error Some SPI1 pad definitions are invalid
#endif
#define SPI1_PADS_VALID
#endif

#ifdef SPI_DEFAULT_INSTANCE
#if SPI_DEFAULT_INSTANCE == 0
#ifndef SPI0_PADS_VALID
#error SPI_DEFAULT_INSTANCE is set to SPI0, but its pads are not valid
#endif
#elif SPI_DEFAULT_INSTANCE == 1
#ifndef SPI1_PADS_VALID
#error SPI_DEFAULT_INSTANCE is set to SPI1, but its pads are not valid
#endif
#endif
#endif

#if (SPI0_CPRS_CPSDVSR & 1) == 1 || SPI0_CPRS_CPSDVSR < 2 || SPI0_CPRS_CPSDVSR > 254
#error SPI0_CPRS_CPSDVSR must be an even number between 2 and 254
#endif

#if (SPI1_CPRS_CPSDVSR & 1) == 1 || SPI1_CPRS_CPSDVSR < 2 || SPI1_CPRS_CPSDVSR > 254
#error SPI1_CPRS_CPSDVSR must be an even number between 2 and 254
#endif
/*---------------------------------------------------------------------------*/
/*
 * Clock source from which the baud clock is determined for the SSI, according
 * to SSI_CC.CS.
 */
#define SSI_SYS_CLOCK   SYS_CTRL_SYS_CLOCK
/*---------------------------------------------------------------------------*/
typedef struct {
  int8_t port;
  int8_t pin;
} spi_pad_t;
typedef struct {
  uint32_t base;
  uint32_t ioc_ssirxd_ssi;
  uint32_t ioc_pxx_sel_ssi_clkout;
  uint32_t ioc_pxx_sel_ssi_txd;
  uint8_t ssi_cprs_cpsdvsr;
  spi_pad_t clk;
  spi_pad_t tx;
  spi_pad_t rx;
} spi_regs_t;
/*---------------------------------------------------------------------------*/
static const spi_regs_t spi_regs[SSI_INSTANCE_COUNT] = {
  {
    .base = SSI0_BASE,
    .ioc_ssirxd_ssi = IOC_SSIRXD_SSI0,
    .ioc_pxx_sel_ssi_clkout = IOC_PXX_SEL_SSI0_CLKOUT,
    .ioc_pxx_sel_ssi_txd = IOC_PXX_SEL_SSI0_TXD,
    .ssi_cprs_cpsdvsr = SPI0_CPRS_CPSDVSR,
    .clk = { SPI0_CLK_PORT, SPI0_CLK_PIN },
    .tx = { SPI0_TX_PORT, SPI0_TX_PIN },
    .rx = { SPI0_RX_PORT, SPI0_RX_PIN }
  }, {
    .base = SSI1_BASE,
    .ioc_ssirxd_ssi = IOC_SSIRXD_SSI1,
    .ioc_pxx_sel_ssi_clkout = IOC_PXX_SEL_SSI1_CLKOUT,
    .ioc_pxx_sel_ssi_txd = IOC_PXX_SEL_SSI1_TXD,
    .ssi_cprs_cpsdvsr = SPI1_CPRS_CPSDVSR,
    .clk = { SPI1_CLK_PORT, SPI1_CLK_PIN },
    .tx = { SPI1_TX_PORT, SPI1_TX_PIN },
    .rx = { SPI1_RX_PORT, SPI1_RX_PIN }
  }
};
/*---------------------------------------------------------------------------*/
/* Deprecated function call provided for compatibility reasons */
#ifdef SPI_DEFAULT_INSTANCE
void
spi_init(void)
{
  spix_init(SPI_DEFAULT_INSTANCE);
}
#endif /* #ifdef SPI_DEFAULT_INSTANCE */
/*---------------------------------------------------------------------------*/
void
spix_init(uint8_t spi)
{
  const spi_regs_t *regs;

  if(spi >= SSI_INSTANCE_COUNT) {
    return;
  }

  regs = &spi_regs[spi];

  if(regs->clk.port < 0) {
    /* Port / pin configuration invalid. We checked for completeness
       above. If clk.port is < 0, this means that all other defines are
       < 0 as well */
    return;
  }

  spix_enable(spi);

  /* Start by disabling the peripheral before configuring it */
  REG(regs->base + SSI_CR1) = 0;

  /* Set the system clock as the SSI clock */
  REG(regs->base + SSI_CC) = 0;

  /* Set the mux correctly to connect the SSI pins to the correct GPIO pins */
  ioc_set_sel(regs->clk.port,
              regs->clk.pin,
              regs->ioc_pxx_sel_ssi_clkout);
  ioc_set_sel(regs->tx.port,
              regs->tx.pin,
              regs->ioc_pxx_sel_ssi_txd);
  REG(regs->ioc_ssirxd_ssi) = (regs->rx.port * 8) + regs->rx.pin;

  /* Put all the SSI gpios into peripheral mode */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->clk.port),
                          GPIO_PIN_MASK(regs->clk.pin));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->tx.port),
                          GPIO_PIN_MASK(regs->tx.pin));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->rx.port),
                          GPIO_PIN_MASK(regs->rx.pin));

  /* Disable any pull ups or the like */
  ioc_set_over(regs->clk.port, regs->clk.pin, IOC_OVERRIDE_DIS);
  ioc_set_over(regs->tx.port, regs->tx.pin, IOC_OVERRIDE_DIS);
  ioc_set_over(regs->rx.port, regs->rx.pin, IOC_OVERRIDE_DIS);

  /* Configure the clock */
  REG(regs->base + SSI_CPSR) = regs->ssi_cprs_cpsdvsr;

  /*
   * Configure the default SPI options.
   *   mode:  Motorola frame format
   *   clock: High when idle
   *   data:  Valid on rising edges of the clock
   *   bits:  8 byte data
   */
  REG(regs->base + SSI_CR0) = SSI_CR0_SPH | SSI_CR0_SPO | (0x07);

  /* Enable the SSI */
  REG(regs->base + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spix_enable(uint8_t spi)
{
  if(spi >= SSI_INSTANCE_COUNT) {
    return;
  }
  REG(SYS_CTRL_RCGCSSI) |= (1 << spi);
}
/*---------------------------------------------------------------------------*/
void
spix_disable(uint8_t spi)
{
  if(spi >= SSI_INSTANCE_COUNT) {
    return;
  }
  REG(SYS_CTRL_RCGCSSI) &= ~(1 << spi);
}
/*---------------------------------------------------------------------------*/
void
spix_set_mode(uint8_t spi,
              uint32_t frame_format,
              uint32_t clock_polarity,
              uint32_t clock_phase,
              uint32_t data_size)
{
  const spi_regs_t *regs;

  if(spi >= SSI_INSTANCE_COUNT) {
    return;
  }

  regs = &spi_regs[spi];

  /* Disable the SSI peripheral to configure it */
  REG(regs->base + SSI_CR1) = 0;

  /* Configure the SSI options */
  REG(regs->base + SSI_CR0) = clock_phase |
    clock_polarity |
    frame_format |
    (data_size - 1);

  /* Re-enable the SSI */
  REG(regs->base + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spix_set_clock_freq(uint8_t spi, uint32_t freq)
{
  const spi_regs_t *regs;
  uint64_t div;
  uint32_t scr;

  if(spi >= SSI_INSTANCE_COUNT) {
    return;
  }

  regs = &spi_regs[spi];

  /* Disable the SSI peripheral to configure it */
  REG(regs->base + SSI_CR1) = 0;

  /* Configure the SSI serial clock rate */
  if(!freq) {
    scr = 255;
  } else {
    div = (uint64_t)regs->ssi_cprs_cpsdvsr * freq;
    scr = (SSI_SYS_CLOCK + div - 1) / div;
    scr = MIN(MAX(scr, 1), 256) - 1;
  }
  REG(regs->base + SSI_CR0) = (REG(regs->base + SSI_CR0) & ~SSI_CR0_SCR_M) |
                              scr << SSI_CR0_SCR_S;

  /* Re-enable the SSI */
  REG(regs->base + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spix_cs_init(uint8_t port, uint8_t pin)
{
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port),
                        GPIO_PIN_MASK(pin));
  ioc_set_over(port, pin, IOC_OVERRIDE_DIS);
  GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
}
/** @} */
