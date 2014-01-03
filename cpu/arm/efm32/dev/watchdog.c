/*
 * Copyright (c) 2013, Kerlink
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
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup efm32-devices
 *
 * @{
 */

/**
 * \file
 *         EFM32 Watchdog driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include <stdbool.h>
#include "contiki.h"
#include "dev/watchdog.h"
#include "clock.h"
#include "em_cmu.h"
#include "em_wdog.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  // Use Software Reset instead of watchdog
  NVIC_SystemReset();
}

#ifndef NO_WATCHDOG
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  WDOG_Enable(true);
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
  WDOG_Enable(false);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief
 *   Feed the watchdog.
 *
 * \details
 *   When the watchdog is activated, it must be fed (ie clearing the counter)
 *   before it reaches the defined timeout period. Otherwise, the watchdog
 *   will generate a reset.
 */
void
watchdog_periodic(void)
{
  /* This function is called periodically to reset the watchdog
     timer. */
  WDOG_Feed();
}

#ifdef CONFIG_PM
/*---------------------------------------------------------------------------*/
int watchdog_suspend(void)
{
	watchdog_stop();
	return 0;
}
/*---------------------------------------------------------------------------*/
/*
 * LPOsc was stopped during PM8, restart watchdog
 */
int watchdog_resume(void)
{
	watchdog_start();
	return 0;
}
/*---------------------------------------------------------------------------*/

static pm_ops_struct watchdog_pm_ops = {
	.suspend = watchdog_suspend,
	.resume = watchdog_resume,
};
#endif

/*---------------------------------------------------------------------------*/
/**
 * \brief   Initialize watchdog
 *
 * This function initialize efm32 internal watchdog for max period : 256s
 *
 * \details
 * It also locks configuration :
 *  after initialization, Software can't disable watchdog or the mother clock
 *  Only next reset will reset configuration.
 */
void
watchdog_init(void)
{
  // Default is max period (256s)
  WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_CORELE, true);

#ifdef CONFIG_PM
  // Allow counting PM2 and PM3
  init.em2Run = true;
  init.em3Run = true;
#endif

#ifndef NO_WATCHDOG
  // Lock configuration
  init.lock = true;
  init.swoscBlock = true;
#endif

  WDOG_Init(&init);
}
#else
void watchdog_init(void) { return; }
void watchdog_start(void){ return; }
void watchdog_stop(void){ return; }
void watchdog_periodic(void){ return; }

#endif

/** @} */
