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

#include "rom.h"
#include "rom_map.h"

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
#define UIP_FALLBACK_INTERFACE 	ip64_uip_fallback_interface
/** @} */

/*
 * SPI bus configuration for the CC3200 LaunchPad-XL
 * @{
 */

/* SPI input/output registers. */
#define SPI_TXBUF 				HWREG(GSPI_BASE+MCSPI_O_TX0)
#define SPI_RXBUF 				HWREG(GSPI_BASE+MCSPI_O_RX0)

/*
 * Wait for end of transmission
 */
#define SPI_WAITFOREOTx() 		do { printf("Before SPI_WAITFOREOTx\n"); while(!(HWREG(GSPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_EOT)); printf("After SPI_WAITFOREOTx\n"); } while(0)

/*
 * Wait for RX data
 */
#define SPI_WAITFOREORx() 		do { printf("Before SPI_WAITFOREORx\n"); while(!(HWREG(GSPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_RXS)); printf("After SPI_WAITFOREORx\n");  } while(0)

/*
 * Wait for data could be transmitted.
 */
#define SPI_WAITFORTxREADY() 	do { printf("Before SPI_WAITFORTxREADY\n"); while(!(HWREG(GSPI_BASE+MCSPI_O_CH0STAT)&MCSPI_CH0STAT_TXS)); printf("After SPI_WAITFORTxREADY\n"); } while(0)

/** @} */

/**
 * Include TI CC2520 architecture dependent functions
 */
#include "dev/spi-arch.h"
#include "cc2520-arch.h"
#include "cc2520_const.h"
#include "clock-arch.h"

/**
 * CC2520 SPI related definitions
 * @{
 */
#define CC2520_CONF_SYMBOL_LOOP_COUNT	USEC_TO_LOOP(326)

/* GPIO06 - Output: SPI Chip Select (CS_N) */
#define CC2520_CSN_PORT_BASE      		GPIOA0_BASE
#define CC2520_CSN_PIN            		GPIO_PIN_6

/* GPIO07 - Output: VREG_EN to CC2520 */
#define CC2520_VREG_PORT_BASE	   		GPIOA0_BASE
#define CC2520_VREG_PIN           		GPIO_PIN_7

/*
 * CC2520 pin status emulation
 */
#define CC2520_FIFOP_IS_1 		(cc2520_arch_getreg(CC2520_FSMSTAT1) & CC2520_FSMSTAT1_FIFOP)
#define CC2520_FIFO_IS_1 		(cc2520_arch_getreg(CC2520_FSMSTAT1) & CC2520_FSMSTAT1_FIFO)
#define CC2520_CCA_IS_1 		(cc2520_arch_getreg(CC2520_FSMSTAT1) & CC2520_FSMSTAT1_CCA)
#define CC2520_SFD_IS_1   		(cc2520_arch_getreg(CC2520_FSMSTAT1) & CC2520_FSMSTAT1_SFD)

/* CC2520 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()		MAP_GPIOPinWrite(CC2520_VREG_PORT_BASE, CC2520_VREG_PIN, CC2520_VREG_PIN);
#define SET_VREG_INACTIVE()     MAP_GPIOPinWrite(CC2520_VREG_PORT_BASE, CC2520_VREG_PIN, 0);

/* The CC2520 reset pin. */
#define SET_RESET_INACTIVE()   	clock_delay(5)
#define SET_RESET_ACTIVE()   	clock_delay(5)

/* CC2520 dummy external interrupt. */
#define CC2520_FIFOP_INT_INIT()
#define CC2520_ENABLE_FIFOP_INT()
#define CC2520_DISABLE_FIFOP_INT()
#define CC2520_CLEAR_FIFOP_INT()

/*
 * Use GPIO06 for controlling chip select (CSn)
 * otherwise SPI_CS of CC32xx controller is used
 */
#define CC2520_USE_CSN_GPIO		0

/*
 * Enable / Disable CC2520 access to the SPI bus.
 *
 * ENABLE CSn (active low)
 */
#if CC2520_USE_CSN_GPIO
#define CC2520_SPI_ENABLE()     	do { MAP_GPIOPinWrite(CC2520_CSN_PORT_BASE, CC2520_CSN_PIN, 0); clock_delay(5); } while(0)
#else
#define CC2520_SPI_ENABLE()			do { spi_cs_enable(); clock_delay(5); } while(0)
#endif

/* DISABLE CSn (active low) */
#if CC2520_USE_CSN_GPIO
#define CC2520_SPI_DISABLE()    	do { MAP_GPIOPinWrite(CC2520_CSN_PORT_BASE, CC2520_CSN_PIN, CC2520_CSN_PIN); clock_delay(5); } while(0)
#else
#define CC2520_SPI_DISABLE()		do { spi_cs_disable(); clock_delay(5); } while(0)
#endif

/** @} */

#endif /* PLATFORM_CONF_H_ */
