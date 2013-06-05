/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http:/www.ti.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 * Header for with definitions related to the cc2538 SysTick
 */
/*---------------------------------------------------------------------------*/
#ifndef SYSTICK_H_
#define SYSTICK_H_
/*---------------------------------------------------------------------------*/
/* SysTick Register Definitions */
#define SYSTICK_STCTRL            0xE000E010    /* Control and Status */
#define SYSTICK_STRELOAD          0xE000E014    /* Reload Value */
#define SYSTICK_STCURRENT         0xE000E018    /* Current Value */
#define SYSTICK_STCAL             0xE000E01C    /* SysTick Calibration */
/*---------------------------------------------------------------------------*/
/* Bit Definitions for the STCTRL Register */
#define SYSTICK_STCTRL_COUNT      0x00010000    /* Count Flag */
#define SYSTICK_STCTRL_CLK_SRC    0x00000004    /* Clock Source */
#define SYSTICK_STCTRL_INTEN      0x00000002    /* Interrupt Enable */
#define SYSTICK_STCTRL_ENABLE     0x00000001    /* Enable */

#endif /* SYSTICK_H_ */

/** @} */
