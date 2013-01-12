/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 * \defgroup cc2538-lpm cc2538 Low Power Modes
 *
 * Driver for the cc2538 power modes
 * @{
 *
 * \file
 * Header file with register, macro and function declarations for the cc2538
 * low power module
 */
#ifndef LPM_H_
#define LPM_H_

#include "contiki-conf.h"
#include "rtimer.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \name LPM stats
 *
 * Maintains a record of how many rtimer ticks spent in each Power Mode.
 * Mainly used for debugging the module
 * @{
 */
#if LPM_CONF_STATS
extern rtimer_clock_t lpm_stats[3];

/**
 * \brief Read the time spent in a PM in rtimer ticks
 * \param pm The pm as a value in [0,2]
 */
#define LPM_STATS_GET(pm)        lpm_stats[pm]
#else
#define LPM_STATS_GET(pm)
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Constants to be used as arguments to lpm_set_max_pm()
 * @{
 */
#define LPM_PM0           0
#define LPM_PM1           1
#define LPM_PM2           2
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialise the LPM module
 */
void lpm_init(void);

/**
 * \brief Drop to Deep Sleep
 *
 * This function triggers a sequence to enter Deep Sleep. The sequence involves
 * determining the most suitable PM and switching the system clock source to
 * the 16MHz if required. If the energest module is enabled, the sequence also
 * performs some simple energest calculations.
 *
 * Broadly speaking, this function will be called from the main loop when all
 * events have been serviced. This functions aims to be clever enough in order
 * to be able to choose between PMs 0/1/2 depending on chip status and
 * anticipated sleep duration. This choice is made subject to configuration
 * restrictions and subject to restrictions imposed by calls to
 * lpm_set_max_pm().
 *
 * This PM selection heuristic has the following primary criteria:
 * - Is the RF off?
 * - Is the USB PLL off?
 * - Is the Sleep Timer scheduled to fire an interrupt?
 *
 * If the answer to any of those questions is no, we will drop to PM0 and
 * will wake up to any interrupt. Best case scenario (if nothing else happens),
 * we will idle until the next SysTick in no more than 1000/CLOCK_SECOND ms
 * (7.8125ms).
 *
 * If all can be answered with 'yes', we can drop to PM1/2 knowing that the
 * Sleep Timer will wake us up. Depending on the estimated deep sleep duration
 * and the max PM allowed by user configuration, we select the most efficient
 * Power Mode to drop to. If the duration is too short, we simply IDLE in PM0.
 *
 * Dropping to PM1/2 requires a switch to the 16MHz OSC. We have the option of
 * letting the SoC do this for us automatically. However, if an interrupt fires
 * during this automatic switch, we will need to re-assert WFI. To avoid this
 * complexity, we perform the switch to the 16MHz OSC manually in software and
 * we assert WFI after the transition has been completed. This gives us a
 * chance to bail out if an interrupt fires or an event is raised during the
 * transition. If nothing happens, dropping to PM1+ is un-interruptible and
 * with a deterministic duration. When we wake up, we switch back to the 32MHz
 * OSC manually before handing control back to main. This is implemented in
 * lpm_exit(), which will always be called from within the Sleep Timer ISR
 * context.
 *
 * \sa main(), rtimer_arch_next_trigger(), lpm_exit(), lpm_set_max_pm()
 */
void lpm_enter(void);

/**
 * \brief Perform an 'Exit Deep Sleep' sequence
 *
 * This routine is called from within the context of the ISR that caused us to
 * come out of PM1/2. It performs a wake up sequence to make sure the 32MHz OSC
 * is back on and the system clock is sourced on it.
 *
 * While in PMs 1 and 2, the system clock stops ticking. This functions adjusts
 * it when we wake up.
 *
 * We always exit PM1/2 as a result of a scheduled rtimer task or a GPIO
 * interrupt. This may lead to other parts of the code trying to use the RF,
 * so we need to switch the clock source \e before said code gets executed.
 *
 * \sa lpm_enter(), rtimer_isr()
 */
void lpm_exit(void);

/**
 * \brief Prevent the SoC from dropping to a PM higher than \e max_pm
 * \param pm The highest PM we are allowed to enter, specified as a
 *        number in [0, 2]
 *
 * Defines for the \e pm argument are LPM_PMx.
 *
 * This function can be used by software in situations where some power
 * modes are undesirable. If, for example, an application needs to avoid PM2,
 * it would call lpm_set_max_pm(LPM_PM1).
 * If an application wants to avoid PM1 as well, it would call
 * lpm_set_max_pm(LPM_PM0)
 *
 * PM0 can not be disabled at runtime. Use LPM_CONF_ENABLE to disable LPM
 * support altogether
 *
 * \note If the value of argument \e pm is greater than the value of the
 *       LPM_CONF_MAX_PM configuration directive, LPM_CONF_MAX_PM is used. Thus
 *       if LPM_CONF_MAX_PM==1, calling lpm_set_max_pm(LPM_PM2) would
 *       result in a maximum PM set to 1 and all subsequent Deep Sleeps would
 *       be limited to either PM0 or PM1.
 *
 * \sa lpm_enter()
 */
void lpm_set_max_pm(uint8_t pm);
/*---------------------------------------------------------------------------*/
/* Disable the entire module if required */
#if LPM_CONF_ENABLE==0
#define lpm_init()
#define lpm_enter()
#define lpm_exit()
#define lpm_set_max_pm(...)
#endif

#endif /* LPM_H_ */

/**
 * @}
 * @}
 */
