/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 *
 */

/**
 * \file
 *         RTIMER for NXP jn516x
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 */

#include "sys/rtimer.h"
#include "sys/clock.h"
#include "sys/process.h"
#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
#include "dev/watchdog.h"
#include "sys/energest.h"

#define RTIMER_TIMER_ISR_DEV  E_AHI_DEVICE_TICK_TIMER

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static volatile uint32_t compare_time;
static volatile uint32_t last_expired_time;

void
rtimer_arch_run_next(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
  uint32_t delta, temp;
  if(u32DeviceId != RTIMER_TIMER_ISR_DEV) {
    return;
  }
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  vAHI_TickTimerIntPendClr();
  vAHI_TickTimerIntEnable(0);
  /*
   * compare register is only 28bits wide so make sure the upper 4bits match
   * the set compare point
   */
  delta = u32AHI_TickTimerRead() - compare_time;
  if(0 == (delta >> 28)) {
    /* compare_time might change after executing rtimer_run_next()
     * as some process might schedule the timer
     */
    temp = compare_time;

    /* run scheduled */
    watchdog_start();
    rtimer_run_next();

    if(process_nevents() > 0) {
      /* TODO exit low-power mode */
    }

    watchdog_stop();
    last_expired_time = temp;
  } else {
    /* No match. Schedule again. */
    vAHI_TickTimerIntEnable(1);
    vAHI_TickTimerInterval(compare_time);
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  /* Initialise tick timer to run continuously */
  vAHI_TickTimerIntEnable(0);
  vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
  last_expired_time = compare_time = 0;
  vAHI_TickTimerWrite(0);
  vAHI_TickTimerRegisterCallback(rtimer_arch_run_next);
  vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_CONT);
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{
  return u32AHI_TickTimerRead();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  PRINTF("rtimer_arch_schedule time %lu\n", t);
  vAHI_TickTimerIntPendClr();
  vAHI_TickTimerIntEnable(1);
  vAHI_TickTimerInterval(t);
  compare_time = t;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_get_time_until_next_wakeup(void)
{
  rtimer_clock_t now = RTIMER_NOW();
  rtimer_clock_t next_wakeup = compare_time;
  if(bAHI_TickTimerIntStatus()) {
    return next_wakeup >= now ? next_wakeup - now : 0;
    /* if no wakeup is scheduled yet return maximum time */
  }
  return (rtimer_clock_t)-1;
}
/*---------------------------------------------------------------------------*/
