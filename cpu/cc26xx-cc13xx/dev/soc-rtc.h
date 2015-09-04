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
 * \defgroup cc26xx-clocks CC13xx/CC26xx clock and timer subsystem
 *
 * For the CC13xx/CC26xx cpu we use the AON RTC as the basis for all clocks and
 * timers
 *
 * We use two of the aviable AON RTC channels. Channel 0 is used by the rtimer
 * sub-system. Channel 1 is used by the system clock and the LPM module.
 *
 * The RTC runs in all power modes except 'shutdown'
 *
 * @{
 *
 * \defgroup cc13xx-cc26xx-rtc CC13xx/CC26xx AON RTC driver
 *
 * Underpins the platform's software clocks and timers
 *
 * @{
 * \file
 * Header file for the CC13xx/CC26xx AON RTC driver
 */
#ifndef SOC_RTC_H_
#define SOC_RTC_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "rtimer.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialise the CC13XX/CC26XX AON RTC module
 *
 * This timer configures AON RTC channels.
 *
 * This function must be called before clock_init() and rtimer_init()
 */
void soc_rtc_init(void);

/**
 * \brief Return the time of the next scheduled rtimer event
 * \return The time at which the next rtimer event is due to fire
 *
 * This function will check both AON RTC channels and will only take CH0's
 * compare into account if the channel is actually enabled
 */
rtimer_clock_t soc_rtc_get_next_trigger(void);

/**
 * \brief Schedule an AON RTC channel 0 one-shot compare event
 * \param channel AON_RTC_CH0 or AON_RTC_CH1
 * \param t The time when the event will be fired. This is an absolute
 *          time, in other words the event will fire AT time \e t,
 *          not IN \e t ticks
 *
 * Channel AON_RTC_CH0 is reserved for the rtimer. AON_RTC_CH1 is reserved
 * for the system clock.
 *
 * User applications should not use this function. User applications should
 * instead use Contiki's timer-related libraries
 */
void soc_rtc_schedule_one_shot(uint32_t channel, uint32_t t);
/*---------------------------------------------------------------------------*/
#endif /* SOC_RTC_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
