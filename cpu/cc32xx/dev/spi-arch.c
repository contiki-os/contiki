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
 *         Implementation of SPI architecture for TI CC32xx.
 * \author
 *         Dominik Grauert <dominik.grauert@3bscientific.com>
 */

#include "contiki.h"

#include "hw_ints.h"
#include "hw_types.h"

#include "hw_memmap.h"
#include "hw_mcspi.h"

#include "prcm.h"

#include "rom.h"
#include "rom_map.h"

#include "../../ti-cc3200-sdk/driverlib/src/spi.h"	// Ti SPI driver for cc32xx
#include "spi.h"									// Contiki SPI driver

#include <stdio.h>

#if STARTUP_CONF_VERBOSE
#define ERR_PRINT(x)		printf("Error [%d] at line [%d] in function [%s]\n", x, __LINE__, __FUNCTION__)
#define PRINTF(...) 		printf(__VA_ARGS__)
#define ASSERT_ON_ERROR(e)	{ if(e < 0) { ERR_PRINT(e); return; } }
#else
#define ERR_PRINT(x)
#define PRINTF(...)
#define ASSERT_ON_ERROR(e)
#endif

/*
 * If not differntly defined from the platform, configure the cc32xx SPI with following parameters:
 * • Mode: 4-Pin/Master
 * • Sub mode: 0
 * • Bit Rate: 1 MHz
 * • Chip Select: Software controlled/Active High
 * • Word Length: 8 bits
 */
#ifndef CC32XX_SPI_BITRATE
#define CC32XX_SPI_BITRATE		1000000				// value is in Hz
#endif

#ifndef CC32XX_SPI_MODE
#define CC32XX_SPI_MODE			SPI_MODE_MASTER
#endif

#ifndef CC32XX_SPI_SUBMODE
#define CC32XX_SPI_SUBMODE		SPI_SUB_MODE_0
#endif

#ifndef CC32XX_SPI_CONFIG
#define CC32XX_SPI_CONFIG		(SPI_SW_CTRL_CS | SPI_4PIN_MODE|SPI_TURBO_OFF |	SPI_CS_ACTIVEHIGH |	SPI_WL_8)
#endif

// Enable debug messages
#define DEBUG	1

/*---------------------------------------------------------------------------*/
void
spi_init(void)
{
	/*
	 * Basic Initialization
	 * ====================
	 * (refer to chapter 0.2.6.1 in http://www.ti.com/lit/ug/swru367b/swru367b.pdf)
	 *
	 * (a) Enable the SPI module clock by invoking following API:
	 * This is allready done in pin_mux_config.c
	 */
	/*
	 * (b) Set the pinmux to bring out the SPI signals to the chip boundary at desired location using:
	 * This is allready done in pin_mux_config.c
	 */
	/*
	 * (c) Soft reset the module:
	 */
	MAP_SPIReset(GSPI_BASE);

	/*
	 * Configure the SPI with following parameters:
	 * Mode, Sub mode, Bit Rate, Chip Select, Word Length
	 */
	MAP_SPIConfigSetExpClk(GSPI_BASE,
		MAP_PRCMPeripheralClockGet(PRCM_GSPI),
		CC32XX_SPI_BITRATE,
		CC32XX_SPI_MODE,
		CC32XX_SPI_SUBMODE,
		CC32XX_SPI_CONFIG);
	/*
	 * Enable SPI channel for communication:
	 */
	MAP_SPIEnable(GSPI_BASE);

#if STARTUP_CONF_VERBOSE && DEBUG
	PRINTF("SPI of CC32xx initialized\n");
#endif
}
