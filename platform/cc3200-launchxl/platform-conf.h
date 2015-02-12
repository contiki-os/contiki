
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
 *         Platform configuration for the wismote platform.
 * \author
 *         Dominik Grauert <dominik.grauert@3bscientific.com>
 */

#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */
// #define PLATFORM_HAS_LEDS   1
// #define PLATFORM_HAS_BUTTON 1

/**
 * \name Device string used on startup
 * @{
 */
#define PLATFORM_STRING "TI CC3200 LaunchpadXL + TI CC2520"
/** @} */

#define CC2520_CS_N_PIN_MASK 		GPIO_PIN_6
#define CC2520_VREG_EN_PIN_MASK 	GPIO_PIN_7
#define CC2520_CONTROL_PIN_MASK 	(CC2520_CS_N_PIN_MASK || CC2520_VREG_EN_PIN_MASK)

#define CC2520_SPI_SET_ENABLED		GPIO_PIN_7
#define CC2520_SPI_SET_POWER_DOWN	GPIO_PIN_6
#define CC2520_SPI_SET_DISABLED		(CC2520_SPI_SET_ENABLED || CC2520_SPI_SET_POWER_DOWN)

/* Seting the base address of the SPI module */
#define LAUNCHPAD_SPI_BASE 		GSPI_BASE

/*
 * SPI bus configuration for the Launchpad
 */

/* SPI input/output registers. */
#define SPI_TXBUF 	HWREG(LAUNCHPAD_SPI_BASE+MCSPI_O_TX0)
#define SPI_RXBUF 	HWREG(LAUNCHPAD_SPI_BASE+MCSPI_O_RX0)

/*  */
#define SPI_WAITFOREOTx() 		while(!(HWREG(LAUNCHPAD_SPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_EOT))

/*
 * Wait for Rx data
 * - Read register status register
 * - Check is data is available
 */
#define SPI_WAITFOREORx() 		while(!(HWREG(LAUNCHPAD_SPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_RXS))

/* Wait for data in input register/FIFO. */
#define SPI_WAITFORTxREADY() 	while(!(HWREG(LAUNCHPAD_SPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_TXS))

/*
 * Enables/disables CC2520 access to the SPI bus (not the bus).
 * (Chip Select)
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
#define CC2520_SPI_IS_ENABLED() 	(!(GPIOA0_BASE&CC2520_CS_N_PIN_MASK))

#endif /* PLATFORM_CONF_H_ */
