/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-clocks CC26xx clock and timer subsystem
 *
 * For the CC26xx cpu we use the AON RTC as the basis for all clocks and timers
 *
 * We configure the AON RTC's channel 2 to run in continuous mode, generating
 * 128 interrupts / second. In continuous mode, the next compare event is
 * scheduled by the hardware automatically; the events are equidistant and
 * this also means we don't need the overhead of re-scheduling within the
 * interrupt handler
 *
 * For rtimers, we use the RTC's channel 0 in one-shot compare mode. When the
 * compare event fires, we call rtimer_run_next
 *
 * The RTC runs in all power modes except 'shutdown'
 *
 * \sa cpu/cc26xx/clock.c cpu/cc26xx/rtimer-arch.c
 * @{
 *
 * \defgroup cc26xx-rtc CC26xx AON RTC driver
 *
 * Underpins the platform's software clocks and timers
 *
 * @{
 * \file
 * Header file for the CC26XX AON RTC driver
 */
#ifndef CC26XX_RTC_H_
#define CC26XX_RTC_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "rtimer.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialise the CC26XX AON RTC module
 *
 * This timer configures the AON RTC's channel 2 to run in continuous mode
 * This function must be called before clock_init() and rtimer_init()
 */
void cc26xx_rtc_init(void);

/**
 * \brief Return the time of the next scheduled rtimer event
 * \return The time at which the next rtimer event is due to fire
 *
 * This function will check both AON RTC channels and will only take CH0's
 * compare into account if the channel is actually enabled
 */
rtimer_clock_t cc26xx_rtc_get_next_trigger(void);

/**
 * \brief Schedule an AON RTC channel 0 one-shot compare event
 * \param t The time when the event will be fired. This is an absolute
 *          time, in other words the event will fire AT time \e t,
 *          not IN \e t ticks
 */
void cc26xx_rtc_schedule_one_shot(uint32_t t);
/*---------------------------------------------------------------------------*/
#endif /* CC26XX_RTC_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
