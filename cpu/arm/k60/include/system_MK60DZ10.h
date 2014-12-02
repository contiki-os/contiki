/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 * \brief Device specific configuration file for MK60DZ10 (header file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device clocks and initializes the
 * oscillator that is part of the microcontroller device.
 */

#ifndef SYSTEM_MK60DZ10_H_
#define SYSTEM_MK60DZ10_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Current core clock frequency
 *
 * MCGOUTCLK divided by OUTDIV1 clocks the ARM Cortex-M4 core.
 */
extern uint32_t SystemCoreClock;

/**
 * \brief Current system clock frequency
 *
 * MCGOUTCLK divided by OUTDIV1 clocks the crossbar switch and bus masters
 * directly connected to the crossbar. In addition, this clock is used for UART0
 * and UART1.
 */
extern uint32_t SystemSysClock;

/**
 * \brief Current bus clock frequency
 *
 * MCGOUTCLK divided by OUTDIV2 clocks the bus slaves and peripherals (excluding
 * memories).
 */
extern uint32_t SystemBusClock;

/**
 * \brief Current FlexBus clock frequency
 *
 * MCGOUTCLK divided by OUTDIV3 clocks the external FlexBus interface.
 */
extern uint32_t SystemFlexBusClock;

/**
 * \brief Current flash clock frequency
 *
 * MCGOUTCLK divided by OUTDIV4 clocks the flash memory.
 */
extern uint32_t SystemFlashClock;


/**
 * \brief Setup the microcontroller system.
 *
 * Typically this function configures the oscillator (PLL) that is part of the
 * microcontroller device. For systems with variable clock speed it also updates
 * the variable SystemCoreClock. SystemInit is called from startup_device file.
 */
void SystemInit(void);

/**
 * \brief Updates all of the SystemCoreClock variables.
 *
 * It must be called whenever the core clock is changed during program
 * execution. SystemCoreClockUpdate() evaluates the clock register settings and
 * calculates the current core clock.
 */
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #if !defined(SYSTEM_MK60DZ10_H_) */
