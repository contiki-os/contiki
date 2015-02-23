//*****************************************************************************
// rom_pin_mux_config.c
//
// configure the device pins for different signals
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

// This file was automatically generated on 08.01.2015 at 11:28:29
// by TI PinMux version 3.0.396 
//
//*****************************************************************************

#include "pin_mux_config.h" 
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "gpio.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"

//*****************************************************************************
void PinMuxConfig(void)
{
    //
    // Enable Peripheral Clocks 
    //
    MAP_PRCMPeripheralClkEnable(PRCM_ADC, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_58 for ADC0 ADC_CH1
    //
    MAP_PinTypeADC(PIN_58, PIN_MODE_255);

    //
    // Configure PIN_59 for ADC0 ADC_CH2
    //
    MAP_PinTypeADC(PIN_59, PIN_MODE_255);

    //
    // Configure PIN_60 for ADC0 ADC_CH3
    //
    MAP_PinTypeADC(PIN_60, PIN_MODE_255);

#ifdef USE_LAUNCHPAD
    //
    // Configure PIN_03 (UART0_TX on LaunchPad, P1.4) for UART0_TX
    //
    MAP_PinTypeUART(PIN_03, PIN_MODE_7);

    //
    // Configure PIN_04 (UART0_RX on LaunchPad, P1.3) for UART0_RX
    //
    MAP_PinTypeUART(PIN_04, PIN_MODE_7);
#else
    //
    // Configure PIN_53 (AUD_CLK on LaunchPad, P3.8) for UART0_TX
    //
    MAP_PinTypeUART(PIN_53, PIN_MODE_9);

    //
    // Configure PIN_57 (ADC_CH0 on LaunchPad, P3.3) for UART0_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);
#endif

#ifdef USE_LAUNCHPAD
    //
    // Configure PIN_55 (Connected to FTDI on LaunchPad) for UART1_TX
    //
    MAP_PinTypeUART(PIN_55, PIN_MODE_6);

    //
    // Configure PIN_57 (Connected to FTDI on LaunchPad) for UART1_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_6);
#else
    //
    // Configure PIN_01 (I2C_SCL on LaunchPad, P1.9) for UART1_TX
    //
    MAP_PinTypeUART(PIN_01, PIN_MODE_7);

    //
    // Configure PIN_02 (I2C_SDA on LaunchPad, P1.10) for UART1_RX
    //
    MAP_PinTypeUART(PIN_02, PIN_MODE_7);
#endif

    //
    // Configure PIN_50 for GPIO Input
    //
    MAP_PinTypeGPIO(PIN_50, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, 0x1, GPIO_DIR_MODE_IN);

    //
    // Configure PIN_64 for GPIO Output || Configure GPIO09 [PIN64] for LED (red)
    //
    PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x2, GPIO_DIR_MODE_OUT);

#ifdef USE_LAUNCHPAD
    //
    // Configure PIN_01 for GPIO Output || Configure GPIO10 [PIN1] for LED (yellow)
    //
    PinTypeGPIO(PIN_01, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x4, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_02 for GPIO Output || Configure GPIO11 [PIN2] for LED (green)
    //
    PinTypeGPIO(PIN_02, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x8, GPIO_DIR_MODE_OUT);
#endif

    //
	// Configure PIN_08 for SPI0 GSPI_CS
	//
	MAP_PinTypeSPI(PIN_08, PIN_MODE_7);

	//
	// Configure PIN_05 for SPI0 GSPI_CLK
	//
	MAP_PinTypeSPI(PIN_05, PIN_MODE_7);

	//
	// Configure PIN_06 for SPI0 GSPI_MISO
	//
	MAP_PinTypeSPI(PIN_06, PIN_MODE_7);

	//
	// Configure PIN_07 for SPI0 GSPI_MOSI
	//
	MAP_PinTypeSPI(PIN_07, PIN_MODE_7);
}
