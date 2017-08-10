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
#include "driverlib/setup_rom.h"

#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief We don't need to do anything special here. The RTC is initialised
 * elsewhere
 */
void
rtimer_arch_init(void)
{
#if RTIMER_ARCH_SECOND != RTIMER_ARCH_SECOND_NORM
    //* normal RTC setup counts with 32kHz LF for RTIMER_ARCH_SECOND_NORM in 1 sec
    const unsigned long RTIMER_ARCH_NORM_INC = 0x800000ul;
    //* need increment that counts about RTIMER_ARCH_SECOND in 1sec with same LF freq
    const unsigned long RTIMER_ARCH_USE_INC  = ((uint64_t)RTIMER_ARCH_NORM_INC*RTIMER_ARCH_SECOND)/RTIMER_ARCH_SECOND_NORM;
    uint32_t pd_status = ti_lib_aon_wuc_power_status_get();
    bool aux_on = (pd_status & AONWUC_AUX_POWER_ON) != 0; //AON_WUC_PWRSTAT_AUX_BUS_CONNECTED
    if (!aux_on){

        // Force AUX on and enable clocks
        // At this point both AUX and AON should have been reset to 0x0.
        HWREG(AON_WUC_BASE + AON_WUC_O_AUXCTL) = AON_WUC_AUXCTL_AUX_FORCE_ON;
        // Wait for power on on the AUX domain
        while( ! ( HWREGBITW( AON_WUC_BASE + AON_WUC_O_PWRSTAT, AON_WUC_PWRSTAT_AUX_PD_ON_BITN )));
    }
    SetupSetAonRtcSubSecInc(RTIMER_ARCH_USE_INC);
    if (!aux_on){
        // Allow AUX to power down
        bool aux_down = true;//(pd_status & AONWUC_AUX_POWER_DOWN)!= 0;
        AUXWUCPowerCtrl( (aux_down)? AUX_WUC_POWER_DOWN : AUX_WUC_POWER_OFF );
    }
#endif
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
