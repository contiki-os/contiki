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
 * \defgroup cc26xx-wdt CC13xx/CC26xx watchdog timer driver
 *
 * Driver for the CC13xx/CC26xx Watchdog Timer
 *
 * This file is not called watchdog.c because the filename is in use by
 * TI CC26xxware/CC13xxware
 * @{
 *
 * \file
 * Implementation of the CC13xx/CC26xx watchdog driver.
 */
#include "contiki.h"
#include "dev/watchdog.h"
#include "ti-lib.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#ifdef CONTIKI_WATCHDOG_CONF_TIMER_TOP
#define CONTIKI_WATCHDOG_TIMER_TOP CONTIKI_WATCHDOG_CONF_TIMER_TOP
#else
#define CONTIKI_WATCHDOG_TIMER_TOP 0xFFFFF
#endif

#ifdef  CONTIKI_WATCHDOG_CONF_LOCK_CONFIG
#define CONTIKI_WATCHDOG_LOCK_CONFIG CONTIKI_WATCHDOG_CONF_LOCK_CONFIG
#else
#define CONTIKI_WATCHDOG_LOCK_CONFIG 1
#endif

#define LOCK_INTERRUPTS_DISABLED 0x01
#define LOCK_REGISTERS_UNLOCKED  0x02
/*---------------------------------------------------------------------------*/
static uint32_t
unlock_config(void)
{
  uint32_t ret = 0;
  bool int_status;

  if(CONTIKI_WATCHDOG_LOCK_CONFIG) {
    int_status = ti_lib_int_master_disable();

    if(ti_lib_watchdog_lock_state()) {
      ret |= LOCK_REGISTERS_UNLOCKED;
      ti_lib_watchdog_unlock();
    }

    ret |= (int_status) ? (0) : (LOCK_INTERRUPTS_DISABLED);
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
lock_config(uint32_t status)
{
  if(CONTIKI_WATCHDOG_LOCK_CONFIG) {

    if(status & LOCK_REGISTERS_UNLOCKED) {
      ti_lib_watchdog_lock();
    }
    if(status & LOCK_INTERRUPTS_DISABLED) {
      ti_lib_int_master_enable();
    }
  }
}
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
  ti_lib_watchdog_reload_set(CONTIKI_WATCHDOG_TIMER_TOP);
  lock_config(LOCK_REGISTERS_UNLOCKED);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Starts the CC26xx WDT
 */
void
watchdog_start(void)
{
  uint32_t lock_status = unlock_config();

  watchdog_periodic();
  ti_lib_watchdog_reset_enable();

  lock_config(lock_status);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Refreshes the CC26xx WDT
 */
void
watchdog_periodic(void)
{
  ti_lib_watchdog_reload_set(CONTIKI_WATCHDOG_TIMER_TOP);
  ti_lib_watchdog_int_clear();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Stops the WDT such that it won't timeout and cause MCU reset
 */
void
watchdog_stop(void)
{
  uint32_t lock_status = unlock_config();

  ti_lib_watchdog_reset_disable();

  lock_config(lock_status);
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
