/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/**
 * \addtogroup nrf52832-dev Device drivers
 * @{
 *
 * \addtogroup nrf52832-watchdog Watchdog driver
 * @{
 *
 * \file
 *         Contiki compatible watchdog driver implementation.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 */
#include <nrf_drv_wdt.h>
#include "app_error.h"
#include "contiki-conf.h"

static nrf_drv_wdt_channel_id wdt_channel_id;
static uint8_t wdt_initialized = 0;

/**
 * \brief WDT events handler.
 */
static void wdt_event_handler(void)
{
    LEDS_OFF(LEDS_MASK);
}

/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  ret_code_t err_code;
  err_code = nrf_drv_wdt_init(NULL, &wdt_event_handler);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_wdt_channel_alloc(&wdt_channel_id);
  APP_ERROR_CHECK(err_code);
  wdt_initialized = 1;
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  if(wdt_initialized) {
    nrf_drv_wdt_enable();
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  if(wdt_initialized) {
    nrf_drv_wdt_channel_feed(wdt_channel_id);
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  NVIC_SystemReset();
}
/**
 * @}
 * @}
 */
