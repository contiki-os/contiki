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
 * \addtogroup cc26xx-clocks
 * @{
 *
 * \defgroup cc26xx-wdt CC26xx watchdog timer driver
 *
 * Driver for the CC26xx Watchdog Timer
 *
 * This file is not called watchdog.c because the filename is in use by
 * TI CC26xxware
 * @{
 *
 * \file
 * Implementation of the cc26xx watchdog driver.
 */
#include "watchdog.h"
#include "ti-lib.h"
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialises the CC26xx WDT
 *
 * Simply sets the reload counter to a default value. The WDT is not started
 * yet. To start it, watchdog_start() must be called.
 */
void
watchdog_init(void)
{
  ti_lib_watchdog_reload_set(0xFFFFF);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Starts the CC26xx WDT
 */
void
watchdog_start(void)
{
  ti_lib_watchdog_reset_enable();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Refreshes the CC26xx WDT
 */
void
watchdog_periodic(void)
{
  ti_lib_watchdog_int_clear();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Stub function to satisfy API requirements
 */
void
watchdog_stop(void)
{
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Manually trigger a WDT reboot
 */
void
watchdog_reboot(void)
{
  watchdog_start();
  while(1);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
