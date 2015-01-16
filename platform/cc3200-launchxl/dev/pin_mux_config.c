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

#ifndef USE_LAUNCHPAD
    //
    // Configure PIN_53 for UART0 UART0_TX
    //
    MAP_PinTypeUART(PIN_53, PIN_MODE_9);

    //
    // Configure PIN_57 for UART0 UART0_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);
#else
    //
    // Configure PIN_55 for UART0 UART0_TX
    //
    MAP_PinTypeUART(PIN_55, PIN_MODE_3);

    //
    // Configure PIN_57 for UART0 UART0_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);
#endif

#ifndef USE_LAUNCHPAD
    //
    // Configure PIN_01 for UART1 UART1_TX
    //
    MAP_PinTypeUART(PIN_01, PIN_MODE_7);

    //
    // Configure PIN_02 for UART1 UART1_RX
    //
    MAP_PinTypeUART(PIN_02, PIN_MODE_7);
#else

#endif

    //
    // Configure PIN_50 for GPIO Input
    //
    MAP_PinTypeGPIO(PIN_50, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, 0x1, GPIO_DIR_MODE_IN);

    //
    // Configure PIN_64 for GPIO Output
    //
    MAP_PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x2, GPIO_DIR_MODE_OUT);
}
