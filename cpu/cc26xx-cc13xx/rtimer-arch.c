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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-rtimer
 * @{
 *
 * \file
 * Implementation of the arch-specific rtimer functions for the CC13xx/CC26xx
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/energest.h"
#include "sys/rtimer.h"
#include "cpu.h"
#include "dev/soc-rtc.h"

#include "ti-lib.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief We don't need to do anything special here. The RTC is initialised
 * elsewhere
 */
void
rtimer_arch_init(void)
{
  return;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Schedules an rtimer task to be triggered at time t
 * \param t The time when the task will need executed.
 *
 * \e t is an absolute time, in other words the task will be executed AT
 * time \e t, not IN \e t rtimer ticks.
 *
 * This function schedules a one-shot event with the AON RTC.
 *
 * This functions converts \e to a value suitable for the AON RTC.
 */
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  /* Convert the rtimer tick value to a value suitable for the AON RTC */
  soc_rtc_schedule_one_shot(AON_RTC_CH0, t);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current real-time clock time
 * \return The current rtimer time in ticks
 *
 * The value is read from the AON RTC counter and converted to a number of
 * rtimer ticks
 *
 */
rtimer_clock_t
rtimer_arch_now()
{
  return ti_lib_aon_rtc_current_compare_value_get();
}
/*---------------------------------------------------------------------------*/
/** @} */
