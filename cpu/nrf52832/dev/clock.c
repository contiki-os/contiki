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
 * \addtogroup nrf52832-dev Device drivers
 * @{
 *
 * \addtogroup nrf52832-clock Clock driver
 * @{
 *
 * \file
 *         Software clock implementation for the nRF52.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 *
 */
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_config.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "contiki.h"
#include "platform-conf.h"

/*---------------------------------------------------------------------------*/
const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(PLATFORM_RTC_INSTANCE_ID); /**< RTC instance used for platform clock */
/*---------------------------------------------------------------------------*/
static volatile uint32_t ticks;
void clock_update(void);

#define TICKS (RTC1_CONFIG_FREQUENCY/CLOCK_CONF_SECOND)

/**
 * \brief Function for handling the RTC0 interrupts
 * \param int_type Type of interrupt to be handled
 */
static void
rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
  if (int_type == NRF_DRV_RTC_INT_TICK) {
    clock_update();
  }
}

#ifndef SOFTDEVICE_PRESENT
/** \brief Function starting the internal LFCLK XTAL oscillator.
 */
static void
lfclk_config(void)
{
  ret_code_t err_code = nrf_drv_clock_init(NULL);
  APP_ERROR_CHECK(err_code);

  nrf_drv_clock_lfclk_request();
}
#endif

/**
 * \brief Function initialization and configuration of RTC driver instance.
 */
static void
rtc_config(void)
{
  uint32_t err_code;

  //Initialize RTC instance
  err_code = nrf_drv_rtc_init(&rtc, NULL, rtc_handler);
  APP_ERROR_CHECK(err_code);

  //Enable tick event & interrupt
  nrf_drv_rtc_tick_enable(&rtc, true);

  //Power on RTC instance
  nrf_drv_rtc_enable(&rtc);
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  ticks = 0;
#ifndef SOFTDEVICE_PRESENT
  lfclk_config();
#endif
  rtc_config();
}
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  return (clock_time_t)(ticks & 0xFFFFFFFF);
}
/*---------------------------------------------------------------------------*/
void
clock_update(void)
{
  ticks++;
  if (etimer_pending()) {
    etimer_request_poll();
  }
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  return (unsigned long)ticks/CLOCK_CONF_SECOND;
}
/*---------------------------------------------------------------------------*/
void
clock_wait(clock_time_t i)
{
  clock_time_t start;
  start = clock_time();
  while (clock_time() - start < (clock_time_t)i) {
    __WFE();
  }
}
/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t dt)
{
  nrf_delay_us(dt);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Obsolete delay function but we implement it here since some code
 * still uses it
 */
void
clock_delay(unsigned int i)
{
  clock_delay_usec(i);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
