/*
 * Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 * \addtogroup nrf52832
 * @{
 *
 * \file
 *      Implementation of the architecture dependent rtimer functions for the nRF52
 *
 * \author
 *      Wojciech Bober <wojciech.bober@nordicsemi.no>
 */
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "app_error.h"
#include "contiki.h"
#include "platform-conf.h"

static const nrf_drv_timer_t timer = NRF_DRV_TIMER_INSTANCE(PLATFORM_TIMER_INSTANCE_ID); /**< Timer instance used for rtimer */

/**
 * \brief Handler for timer events.
 *
 * \param event_type type of an event that should be handled
 * \param p_context opaque data pointer passed from nrf_drv_timer_init()
 */
static void
timer_event_handler(nrf_timer_event_t event_type, void* p_context)
{
  switch (event_type) {
    case NRF_TIMER_EVENT_COMPARE1:
      rtimer_run_next();
      break;

    default:
      //Do nothing.
      break;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize platform rtimer
 */
void
rtimer_arch_init(void)
{
  ret_code_t err_code = nrf_drv_timer_init(&timer, NULL, timer_event_handler);
  APP_ERROR_CHECK(err_code);
  nrf_drv_timer_enable(&timer);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Schedules an rtimer task to be triggered at time t
 * \param t The time when the task will need executed.
 *
 * \e t is an absolute time, in other words the task will be executed AT
 * time \e t, not IN \e t rtimer ticks.
 *
 * This function schedules a one-shot event with the nRF RTC.
 */
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  nrf_drv_timer_compare(&timer, NRF_TIMER_CC_CHANNEL1, t, true);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current real-time clock time
 * \return The current rtimer time in ticks
 *
 */
rtimer_clock_t
rtimer_arch_now()
{
  return nrf_drv_timer_capture(&timer, NRF_TIMER_CC_CHANNEL0);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
