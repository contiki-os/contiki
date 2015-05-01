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
 * \addtogroup cc26xx-rtc
 * @{
 *
 */
/**
 * \file
 * Implementation of the CC26xx AON RTC driver
 */
#include "contiki.h"
#include "sys/energest.h"
#include "rtimer.h"
#include "lpm.h"

#include "ti-lib.h"

#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define cc26xx_rtc_isr(...) AONRTCIntHandler(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* Prototype of a function in clock.c. Called every time the handler fires */
void clock_update(void);
/*---------------------------------------------------------------------------*/
void
cc26xx_rtc_init(void)
{
  uint32_t compare_value;
  bool interrupts_disabled;

  /* Disable and clear interrupts */
  interrupts_disabled = ti_lib_int_master_disable();

  ti_lib_aon_rtc_disable();

  ti_lib_aon_rtc_event_clear(AON_RTC_CH0);
  ti_lib_aon_rtc_event_clear(AON_RTC_CH2);

  /* Setup the wakeup event */
  ti_lib_aon_event_mcu_wake_up_set(AON_EVENT_MCU_WU0, AON_EVENT_RTC0);
  ti_lib_aon_event_mcu_wake_up_set(AON_EVENT_MCU_WU1, AON_EVENT_RTC2);
  ti_lib_aon_rtc_combined_event_config(AON_RTC_CH0 | AON_RTC_CH2);

  /* Configure channel 2 in continuous compare, 128 ticks / sec */
  ti_lib_aon_rtc_inc_value_ch2_set(RTIMER_SECOND / CLOCK_SECOND);
  ti_lib_aon_rtc_mode_ch2_set(AON_RTC_MODE_CH2_CONTINUOUS);
  compare_value = (RTIMER_SECOND / CLOCK_SECOND) +
                  ti_lib_aon_rtc_current_compare_value_get();
  ti_lib_aon_rtc_compare_value_set(AON_RTC_CH2, compare_value);

  /* Enable channel 2 and the RTC */
  ti_lib_aon_rtc_channel_enable(AON_RTC_CH2);
  ti_lib_aon_rtc_enable();

  ti_lib_int_enable(INT_AON_RTC);

  /* Re-enable interrupts */
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
cc26xx_rtc_get_next_trigger()
{
  rtimer_clock_t ch2 = ti_lib_aon_rtc_compare_value_get(AON_RTC_CH2);

  if(HWREG(AON_RTC_BASE + AON_RTC_O_CHCTL) & AON_RTC_CHCTL_CH0_EN) {
    rtimer_clock_t ch0 = ti_lib_aon_rtc_compare_value_get(AON_RTC_CH2);

    return RTIMER_CLOCK_LT(ch0, ch2) ? ch0 : ch2;
  }

  return ch2;
}
/*---------------------------------------------------------------------------*/
void
cc26xx_rtc_schedule_one_shot(uint32_t ticks)
{
  /* Set the channel to fire a one-shot compare event at time==ticks */
  ti_lib_aon_rtc_compare_value_set(AON_RTC_CH0, ticks);
  ti_lib_aon_rtc_channel_enable(AON_RTC_CH0);
}
/*---------------------------------------------------------------------------*/
/* The AON RTC interrupt handler */
void
cc26xx_rtc_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(ti_lib_aon_rtc_event_get(AON_RTC_CH0)) {
    ti_lib_aon_rtc_event_clear(AON_RTC_CH0);
    rtimer_run_next();
  }

  if(ti_lib_aon_rtc_event_get(AON_RTC_CH2)) {
    ti_lib_aon_rtc_event_clear(AON_RTC_CH2);
    clock_update();
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** @} */
