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
 *         K60 clock configuration defines.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

/* Base clocks */
#define CPU_XTAL_CLK_HZ                 8000000u /**< Value of the external crystal or oscillator clock frequency in Hz */
#define CPU_XTAL32k_CLK_HZ              32768u   /**< Value of the external 32k crystal or oscillator clock frequency in Hz */
#define CPU_INT_SLOW_CLK_HZ             32768u   /**< Value of the slow internal oscillator clock frequency in Hz  */
#define CPU_INT_FAST_CLK_HZ             4000000u /**< Value of the fast internal oscillator clock frequency in Hz  */
#define DEFAULT_SYSTEM_CLOCK            96000000u /**< Default System clock value */

/**
 * System clock divider setting, the actual hardware register value, see reference manual for details.
 */
#define CONFIG_CLOCK_K60_SYS_DIV 0x00

/**
 * Bus clock divider setting, the actual hardware register value, see reference manual for details
 */
#define CONFIG_CLOCK_K60_BUS_DIV 0x01

/**
 * Flexbus clock divider setting, the actual hardware register value, see reference manual for details
 */
#define CONFIG_CLOCK_K60_FB_DIV 0x01

/**
 * Flash clock divider setting, the actual hardware register value, see reference manual for details
 */
#define CONFIG_CLOCK_K60_FLASH_DIV 0x03

/**
 * FLL parameter DRST DRS in MCG register C4, see reference manual for details
 */
#define CONFIG_CLOCK_K60_FLL_MCG_C4_DRST_DRS 0b11

/**
 * FLL parameter DMX32 in MCG register C4, see reference manual for details
 */
#define CONFIG_CLOCK_K60_FLL_MCG_C4_DMX32 1

/**
 * CPU core frequency resulting from the chosen divisors and multipliers.
 */
#define F_CPU DEFAULT_SYSTEM_CLOCK

/**
 * System frequency resulting from the chosen divisors and multipliers.
 */
#define F_SYS (F_CPU / (CONFIG_CLOCK_K60_SYS_DIV + 1))

/**
 * Bus frequency resulting from the chosen divisors and multipliers.
 */
#define F_BUS (F_CPU / (CONFIG_CLOCK_K60_BUS_DIV + 1))

/**
 * FlexBus frequency resulting from the chosen divisors and multipliers.
 */
#define F_FLEXBUS (F_CPU / (CONFIG_CLOCK_K60_FB_DIV + 1))

/**
 * Flash frequency resulting from the chosen divisors and multipliers.
 */
#define F_FLASH (F_CPU / (CONFIG_CLOCK_K60_FLASH_DIV + 1))

