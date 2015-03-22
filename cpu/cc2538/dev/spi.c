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
/*---------------------------------------------------------------------------*/
/* Map the old port / pin names to the new ones for compatibility reasons */
#if (SPI_DEFAULT_INSTANCE==0)
#ifdef SPI_CLK_PORT
#define SPI0_CLK_PORT					SPI_CLK_PORT
#endif
#ifdef SPI_CLK_PIN
#define SPI0_CLK_PIN					SPI_CLK_PIN
#endif
#ifdef SPI_MOSI_PORT
#define SPI0_TX_PORT					SPI_MOSI_PORT
#endif
#ifdef SPI_MOSI_PIN
#define SPI0_TX_PIN						SPI_MOSI_PIN
#endif
#ifdef SPI_MISO_PORT
#define SPI0_RX_PORT					SPI_MISO_PORT
#endif
#ifdef SPI_MISO_PIN
#define SPI0_RX_PIN						SPI_MISO_PIN
#endif
#elif (SPI_DEFAULT_INSTANCE==1)
#ifdef SPI_CLK_PORT
#define SPI1_CLK_PORT					SPI_CLK_PORT
#endif
#ifdef SPI_CLK_PIN
#define SPI1_CLK_PIN					SPI_CLK_PIN
#endif
#ifdef SPI_MOSI_PORT
#define SPI1_TX_PORT					SPI_MOSI_PORT
#endif
#ifdef SPI_MOSI_PIN
#define SPI1_TX_PIN						SPI_MOSI_PIN
#endif
#ifdef SPI_MISO_PORT
#define SPI1_RX_PORT					SPI_MISO_PORT
#endif
#ifdef SPI_MISO_PIN
#define SPI1_RX_PIN						SPI_MISO_PIN
#endif
#else
#error "Invalid SPI instance. Valid values are 0 or 1"
#endif
/*---------------------------------------------------------------------------*/
/* Check port / pin settings for SPI0 and provide default values for spi_cfg */
#ifndef SPI0_CLK_PORT
#define SPI0_CLK_PORT					(-1)
#endif
#ifndef SPI0_CLK_PIN
#define SPI0_CLK_PIN					(-1)
#endif
#if SPI0_CLK_PORT >= 0 && SPI0_CLK_PIN < 0 || \
    SPI0_CLK_PORT < 0 && SPI0_CLK_PIN >= 0
#error "Both SPI0_CLK_PORT and SPI0_CLK_PIN must be valid or invalid"
#endif

#ifndef SPI0_TX_PORT
#define SPI0_TX_PORT					(-1)
#endif
#ifndef SPI0_TX_PIN
#define SPI0_TX_PIN						(-1)
#endif
#if SPI0_TX_PORT >= 0 && SPI0_TX_PIN < 0 || \
    SPI0_TX_PORT < 0 && SPI0_TX_PIN >= 0
#error "Both SPI0_TX_PORT and SPI0_TX_PIN must be valid or invalid"
#endif

#ifndef SPI0_RX_PORT
#define SPI0_RX_PORT					(-1)
#endif
#ifndef SPI0_RX_PIN
#define SPI0_RX_PIN						(-1)
#endif
#if SPI0_RX_PORT >= 0 && SPI0_RX_PIN < 0 || \
    SPI0_RX_PORT < 0 && SPI0_RX_PIN >= 0
#error "Both SPI0_RX_PORT and SPI0_RX_PIN must be valid or invalid"
#endif

/* Here we check that either all or none of the ports are defined. As
   we did already check that both ports + pins are either defined or
   not for every pin, this means that we can check for an incomplete 
   configuration by only looking at the port defines */
#if (SPI0_CLK_PORT < 0 && SPI0_TX_PORT < 0 && SPI0_RX_PORT >= 0) ||  \
	(SPI0_CLK_PORT < 0 && SPI0_TX_PORT >= 0 && SPI0_RX_PORT < 0) ||  \
	(SPI0_CLK_PORT < 0 && SPI0_TX_PORT >= 0 && SPI0_RX_PORT >= 0) || \
	(SPI0_CLK_PORT >= 0 && SPI0_TX_PORT < 0 && SPI0_RX_PORT < 0) ||  \
	(SPI0_CLK_PORT >= 0 && SPI0_TX_PORT < 0 && SPI0_RX_PORT >= 0) || \
	(SPI0_CLK_PORT >= 0 && SPI0_TX_PORT >= 0 && SPI0_RX_PORT < 0) ||
#error "SPI0 port / pin definition incomplete"
#endif
/*---------------------------------------------------------------------------*/
/* Check port / pin settings for SPI1 and provide default values for spi_cfg */
#ifndef SPI1_CLK_PORT
#define SPI1_CLK_PORT					(-1)
#endif
#ifndef SPI1_CLK_PIN
#define SPI1_CLK_PIN					(-1)
#endif
#if SPI1_CLK_PORT >= 0 && SPI1_CLK_PIN < 0 || \
    SPI1_CLK_PORT < 0 && SPI1_CLK_PIN >= 0
#error "Both SPI1_CLK_PORT and SPI1_CLK_PIN must be valid or invalid"
#endif

#ifndef SPI1_TX_PORT
#define SPI1_TX_PORT					(-1)
#endif
#ifndef SPI1_TX_PIN
#define SPI1_TX_PIN					   	(-1)
#endif
#if SPI1_TX_PORT >= 0 && SPI1_TX_PIN < 0 || \
    SPI1_TX_PORT < 0 && SPI1_TX_PIN >= 0
#error "Both SPI1_TX_PORT and SPI1_TX_PIN must be valid or invalid"
#endif

#ifndef SPI1_RX_PORT
#define SPI1_RX_PORT				   	(-1)
#endif
#ifndef SPI1_RX_PIN
#define SPI1_RX_PIN					   	(-1)
#endif
#if SPI1_RX_PORT >= 0 && SPI1_RX_PIN < 0 || \
    SPI1_RX_PORT < 0 && SPI1_RX_PIN >= 0
#error "Both SPI1_RX_PORT and SPI1_RX_PIN must be valid or invalid"
#endif

/* Here we check that either all or none of the ports are defined. As
   we did already check that both ports + pins are either defined or
   not for every pin, this means that we can check for an incomplete 
   configuration by only looking at the port defines */
#if (SPI1_CLK_PORT < 0 && SPI1_TX_PORT < 0 && SPI1_RX_PORT >= 0) ||  \
	(SPI1_CLK_PORT < 0 && SPI1_TX_PORT >= 0 && SPI1_RX_PORT < 0) ||  \
	(SPI1_CLK_PORT < 0 && SPI1_TX_PORT >= 0 && SPI1_RX_PORT >= 0) || \
	(SPI1_CLK_PORT >= 0 && SPI1_TX_PORT < 0 && SPI1_RX_PORT < 0) ||  \
	(SPI1_CLK_PORT >= 0 && SPI1_TX_PORT < 0 && SPI1_RX_PORT >= 0) || \
	(SPI1_CLK_PORT >= 0 && SPI1_TX_PORT >= 0 && SPI1_RX_PORT < 0) ||
#error "SPI1 port / pin definition incomplete"
#endif
/*---------------------------------------------------------------------------*/
typedef struct
{
	int8_t port_num;
	int8_t pin_num;
} spi_pad_t;
typedef struct {
	uint32_t base;
	uint32_t ioc_ssirxd_ssi;
	uint32_t ioc_pxx_sel_ssi_clkout;
	uint32_t ioc_pxx_sel_ssi_txd;
	spi_pad_t clk;
	spi_pad_t txd;
	spi_pad_t rxd;
} spi_cfg_t;
/*---------------------------------------------------------------------------*/
static const spi_cfg_t spi_regs[SSI_INSTANCE_COUNT] = {
	{
		.base = SSI0_BASE,
		.ioc_ssirxd_ssi = IOC_SSIRXD_SSI0,
		.ioc_pxx_sel_ssi_clkout = IOC_PXX_SEL_SSI0_CLKOUT,
		.ioc_pxx_sel_ssi_txd = IOC_PXX_SEL_SSI0_TXD,
		.clk = {SPI0_CLK_PORT, SPI0_CLK_PIN},
		.tx = {SPI0_TX_PORT, SPI0_TX_PIN},
		.rx = {SPI0_RX_PORT, SPI0_RX_PIN}
	}, {
		.base = SSI1_BASE,
		.ioc_ssirxd_ssi = IOC_SSIRXD_SSI1,
		.ioc_pxx_sel_ssi_clkout = IOC_PXX_SEL_SSI1_CLKOUT,
		.ioc_pxx_sel_ssi_txd = IOC_PXX_SEL_SSI1_TXD,
		.clk = {SPI1_CLK_PORT, SPI1_CLK_PIN},
		.tx = {SPI1_TX_PORT, SPI1_TX_PIN},
		.rx = {SPI1_RX_PORT, SPI1_RX_PIN}
	}
};
/*---------------------------------------------------------------------------*/
void spix_init(uint8_t instance)
{
	const spi_cfg_t *cfg; 
	
	if (instance >= SSI_INSTANCE_COUNT) {
		return;
	}

	spix_enable(instance);

	cfg = &spi_cfg[instance];

	if (cfg->clk.port < 0)
	{ 
		/* Port / pin configuration invalid. We checked for completeness
		   above. If clk.port is < 0, this means that all other defines are
		   < 0 as well */
		return;
	}

	/* Start by disabling the peripheral before configuring it */
	REG(cfg->base + SSI_CR1) = 0;
	
	/* Set the IO clock as the SSI clock */
	REG(cfg->base + SSI_CC) = 1;

	/* Set the mux correctly to connect the SSI pins to the correct GPIO pins */
	ioc_set_sel(cfg->clk.port, 
				cfg->clk.pin,
				cfg->ioc_pxx_sel_ssi_clkout);
	ioc_set_sel(cfg->tx.port, 
				cfg->tx.pin,
				cfg->ioc_pxx_sel_ssi_txd);
	REG(cfg->ioc_ssirxd_ssi) = (cfg->rx.port * 8) + cfg->rx.pin;
	
	/* Put all the SSI gpios into peripheral mode */
	GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(cfg->clk.port),
							GPIO_PIN_MASK(cfg->clk.pin));
	GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(cfg->tx.port),
							GPIO_PIN_MASK(cfg->tx.pin));
	GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(cfg->rx.port),
							GPIO_PIN_MASK(cfg->rx.pin));
	
	/* Disable any pull ups or the like */
	ioc_set_over(cfg->clk.port, cfg->clk.pin, IOC_OVERRIDE_DIS);
	ioc_set_over(cfg->tx.port, cfg->tx.pin, IOC_OVERRIDE_DIS);
	ioc_set_over(cfg->rx.port, cfg->rx.pin, IOC_OVERRIDE_DIS);

	/* Configure the clock */
	REG(cfg->base + SSI_CPSR) = 2;
	
	/* 
	 * Configure the default SPI options.
	 *   mode:  Motorola frame format
	 *   clock: High when idle
	 *   data:  Valid on rising edges of the clock
	 *   bits:  8 byte data
	 */
	REG(cfg->base + SSI_CR0) = SSI_CR0_SPH | SSI_CR0_SPO | (0x07);
	
	/* Enable the SSI */
	REG(cfg->base + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spix_enable(uint8_t instance)
{
	if (instance >= SSI_INSTANCE_COUNT) {
		return;
	}

	/* Enable the clock for the SSI peripheral */
	if (instance == 0) {
		/* Enable the clock for the SSI peripheral */
		REG(SYS_CTRL_RCGCSSI) |= 1;
	} else {
		REG(SYS_CTRL_RCGCSSI) |= 2;
	}
}
/*---------------------------------------------------------------------------*/
void
spix_disable(uint8_t instance)
{
	if (instance >= SSI_INSTANCE_COUNT) {
		return;
	}

	/* Gate the clock for the SSI peripheral */
	if (instance == 0) {
		REG(SYS_CTRL_RCGCSSI) &= ~1;
	} else {
		REG(SYS_CTRL_RCGCSSI) &= ~2;
	}
}
/*---------------------------------------------------------------------------*/
void spix_set_mode(uint8_t instance,
				   uint32_t frame_format, 
				   uint32_t clock_polarity, 
				   uint32_t clock_phase, 
				   uint32_t data_size)
{
	const spi_cfg_t *cfg; 
	
	if (instance >= SSI_INSTANCE_COUNT) {
		return;
	}

	cfg = &spi_cfg[instance];

	/* Disable the SSI peripheral to configure it */
	REG(cfg->base + SSI_CR1) = 0;

	/* Configure the SSI options */
	REG(cfg->base + SSI_CR0) = clock_phase | 
		clock_polarity | 
		frame_format | 
		(data_size - 1);

	/* Re-enable the SSI */
	REG(cfg->base + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void spix_cs_init(uint8_t port, uint8_t pin)
{
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port), 
						GPIO_PIN_MASK(pin));
  ioc_set_over(port, pin, IOC_OVERRIDE_DIS);
  GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
}

/** @} */
