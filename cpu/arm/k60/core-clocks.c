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
 *         Implementation of K60 clock configuration.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "K60.h"
#include "config-clocks.h"

void core_clocks_init_early(void)
{
    /* System clock initialization, early boot */

    /* Enable clock gate for RTC module */
    /* side note: It is ironic that we need to enable the clock gate for a clock module */
    SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

    /* Reset the RTC status */
    RTC_SR = 0;

    /* Enable RTC clock */
    RTC_CR |= RTC_CR_OSCE_MASK | /* RTC_CR_CLKO_MASK | */
        RTC_CR_SC8P_MASK | RTC_CR_SC4P_MASK; /* enable 12pF load capacitance, might need adjusting.. */
}

/*
 * Arrange so that the core_clocks_init_early() function is called during
 * early init.
 */
void __attribute__((section(".preinit_array"))) (*preinit_core_clocks_init_early[])(void) = {core_clocks_init_early};


void core_clocks_init(void)
{
    /* System clock initialization */

    /* Check that the running CPU revision matches the compiled revision */
    if (SCB_CPUID != K60_EXPECTED_CPUID)
    {
        uint32_t CPUID = SCB_CPUID; /* This is only to ease debugging, type
                                     * "print /x CPUID" in gdb */
        uint32_t SILICON_REVISION = K60_RUNNING_CPU_REVISION + 1;
        (void)CPUID; /* prevents compiler warnings about an unused variable. */
        (void)SILICON_REVISION;

        /* Running on the wrong CPU, the clock initialization is different
         * between silicon revision 1.x and 2.x (LSB of CPUID) */
        /* If you unexpectedly end up on this line when debugging:
         * Rebuild the code using the correct value for K60_CPU_REV */
        __asm("bkpt #99\n");
        while(1);
    }

    /* Set clock prescalers to safe values */
    /*
     * We want to achieve the following clocks:
     * Core/system: <100MHz
     * Bus: <50MHz
     * FlexBus: <50MHz
     * Flash: <25MHz
     *
     * using dividers 1-2-2-4 will obey the above limits when using a 96MHz FLL source.
     */
    SIM_CLKDIV1 = (
        SIM_CLKDIV1_OUTDIV1(CONFIG_CLOCK_K60_SYS_DIV) | /* Core/System clock divider */
        SIM_CLKDIV1_OUTDIV2(CONFIG_CLOCK_K60_BUS_DIV) | /* Bus clock divider */
        SIM_CLKDIV1_OUTDIV3(CONFIG_CLOCK_K60_FB_DIV) | /* FlexBus divider, not used in Mulle */
        SIM_CLKDIV1_OUTDIV4(CONFIG_CLOCK_K60_FLASH_DIV)); /* Flash clock divider */

    /* Select FLL as source (as opposed to PLL) */
    SIM_SOPT2 &= ~(SIM_SOPT2_PLLFLLSEL_MASK);
    /* Use external 32kHz RTC clock as source for OSC32K */
    /* Check this */
    #if K60_CPU_REV == 1
    SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL_MASK;
    #elif K60_CPU_REV == 2
    SIM_SOPT1 = (SIM_SOPT1 & ~(SIM_SOPT1_OSC32KSEL(0b11))) | SIM_SOPT1_OSC32KSEL(0b10);
    #else
    #error Unknown K60 CPU revision
    #endif

    /* Select RTC 32kHz clock as reference clock for the FLL */
    #if K60_CPU_REV == 1
    /* Rev 1 parts */
    SIM_SOPT2 |= SIM_SOPT2_MCGCLKSEL_MASK;
    #elif K60_CPU_REV == 2
    /* Rev 2 parts */
    MCG_C7 = (MCG_C7_OSCSEL_MASK);
    #else
    #error Unknown K60 CPU revision
    #endif

    /* Set range to low frequency input (32kHz) */
    #if K60_CPU_REV == 1
    /* Rev 1 parts */
    MCG_C2 = (MCG_C2_RANGE(0));
    #elif K60_CPU_REV == 2
    /* Rev 2 parts renamed the parameter RANGE -> RANGE0 */
    MCG_C2 = (MCG_C2_RANGE0(0));
    #else
    #error Unknown K60 CPU revision
    #endif

    /* Select the FLL in the PLLS mux */
    MCG_C6 &= ~(MCG_C6_PLLS_MASK);
    while((MCG_S & MCG_S_PLLST_MASK)) {
        /* Make sure the FLL is selected in the PLLS mux */
    }

    /* Select FLL clock as source for the MCGCLKOUT */
    /* Divide clock by 1 */
    /* Select FLL as reference clock for MCG */
    MCG_C1 = MCG_C1_CLKS(0) | MCG_C1_FRDIV(0);
    while((MCG_S & MCG_S_IREFST_MASK) != 0u) {
        /* Check that the source of the FLL reference clock is the external reference clock. */
    }
    while((MCG_S & MCG_S_CLKST_MASK) != 0x00u) {
        /* Wait until external reference clock is selected as MCG output */
    }

    /* Set FLL scalers to yield 96 MHz clock from 32768 Hz reference */
    MCG_C4 = (((CONFIG_CLOCK_K60_FLL_MCG_C4_DMX32 << MCG_C4_DMX32_SHIFT) & MCG_C4_DMX32_MASK) | MCG_C4_DRST_DRS(CONFIG_CLOCK_K60_FLL_MCG_C4_DRST_DRS));
}

