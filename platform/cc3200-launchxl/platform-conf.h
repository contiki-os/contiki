/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 * 3. Neither the name of the Institute nor the names of its contributors
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
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 * 	Platform configuration for the cc3200-launchxl platform.
 */

#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_mcspi.h"

#include "gpio.h"

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */
#define PLATFORM_HAS_LEDS   1
// #define PLATFORM_HAS_BUTTON 1

/**
 * LED port definitions
 *  @{
 */
#define LEDS_CONF_GREEN  	GPIO_PIN_3
#define LEDS_CONF_YELLOW  	GPIO_PIN_2
#define LEDS_CONF_RED  		GPIO_PIN_1
/** @} */

/**
 * \name Device string used on startup
 * @{
 */
#define PLATFORM_STRING "TI CC3200 LaunchpadXL + TI CC2520"
/** @} */

/**
 * UIP fall back interface definitions
 * @{
 */
#undef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE ip64_uip_fallback_interface
/** @} */

/**
 * CC2520 SPI related definitions
 * @{
 */
#define CC2520_CS_N_PIN_MASK 		GPIO_PIN_6
#define CC2520_VREG_EN_PIN_MASK 	GPIO_PIN_7
#define CC2520_CONTROL_PIN_MASK 	(CC2520_CS_N_PIN_MASK | CC2520_VREG_EN_PIN_MASK)

#define CC2520_SPI_SET_ENABLED		GPIO_PIN_7
#define CC2520_SPI_SET_POWER_DOWN	GPIO_PIN_6
#define CC2520_SPI_SET_DISABLED		(CC2520_SPI_SET_ENABLED | CC2520_SPI_SET_POWER_DOWN)

/*
 * SPI bus configuration for the CC3200 LaunchPad-XL
 */

/* SPI input/output registers. */
#define SPI_TXBUF 	HWREG(GSPI_BASE+MCSPI_O_TX0)
#define SPI_RXBUF 	HWREG(GSPI_BASE+MCSPI_O_RX0)

/*
 * Wait for end of transmission
 */
#define SPI_WAITFOREOTx() 		while(!(HWREG(GSPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_EOT))

/*
 * Wait for RX data
 */
#define SPI_WAITFOREORx() 		while(!(HWREG(GSPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_RXS))

/*
 * Wait for data could be transmitted.
 */
#define SPI_WAITFORTxREADY() 	while(!(HWREG(GSPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_TXS))

/*
 * Enable / Disable CC2520 access to the SPI bus.
 */

 /* ENABLE CSn (active low) */
#define CC2520_SPI_ENABLE()     															\
	do { 																					\
		MAP_GPIOPinWrite(GPIOA0_BASE, CC2520_CONTROL_PIN_MASK, CC2520_SPI_SET_ENABLED);		\
	} while(0)

 /* DISABLE CSn (active low) */
#define CC2520_SPI_DISABLE()    															\
	do { 																					\
		MAP_GPIOPinWrite(GPIOA0_BASE, CC2520_CONTROL_PIN_MASK, CC2520_SPI_SET_DISABLED);	\
	} while(0)

 /* is CSn == 0? */
#define CC2520_SPI_IS_ENABLED()  (!((MAP_GPIOPinRead(GPIOA0_BASE, CC2520_CS_N_PIN_MASK) & CC2520_CS_N_PIN_MASK) ? 1 : 0))
/** @} */

#endif /* PLATFORM_CONF_H_ */
