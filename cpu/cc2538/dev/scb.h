/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-scb cc2538 System Control Block
 * @{
 *
 * \file
 * Header file for the System Control Block (SCB)
 */
#ifndef SCB_H_
#define SCB_H_

#define SCB_CPUID              0xE000ED00 /**< CPU ID Base */
#define SCB_INTCTRL            0xE000ED04 /**< Interrupt Control and State */
#define SCB_VTABLE             0xE000ED08 /**< Vector Table Offset */
#define SCB_APINT              0xE000ED0C /**< Application Interrupt and Reset Control */
#define SCB_SYSCTRL            0xE000ED10 /**< System Control */
#define SCB_CFGCTRL            0xE000ED14 /**< Configuration and Control */
#define SCB_SYSPRI1            0xE000ED18 /**< System Handler Priority 1 */
#define SCB_SYSPRI2            0xE000ED1C /**< System Handler Priority 2 */
#define SCB_SYSPRI3            0xE000ED20 /**< System Handler Priority 3 */
#define SCB_SYSHNDCTRL         0xE000ED24 /**< System Handler Control and State */
#define SCB_FAULTSTAT          0xE000ED28 /**< Configurable Fault Status */
#define SCB_HFAULTSTAT         0xE000ED2C /**< Hard Fault Status */
#define SCB_DEBUG_STAT         0xE000ED30 /**< Debug Status Register */
#define SCB_MMADDR             0xE000ED34 /**< Memory Management Fault Address */
#define SCB_FAULT_ADDR         0xE000ED38 /**< Bus Fault Address */
/*---------------------------------------------------------------------------*/
/** \name VTABLE register bits
 * @{
 */
#define SCB_VTABLE_BASE        0x20000000 /**< Vector Table Base */
#define SCB_VTABLE_OFFSET_M    0x1FFFFE00 /**< Vector Table Offset */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SCB_SYSCTRL register bits
 * @{
 */
#define SCB_SYSCTRL_SEVONPEND  0x00000010  /**< Wake up on pending */
#define SCB_SYSCTRL_SLEEPDEEP  0x00000004  /**< Deep sleep enable */
#define SCB_SYSCTRL_SLEEPEXIT  0x00000002  /**< Sleep on ISR exit */
/** @} */
/*---------------------------------------------------------------------------*/

#endif /* SCB_H_ */

/**
 * @}
 * @}
 */
