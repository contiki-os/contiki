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
 *         Atis Elsts <atis.elsts@sics.se>
 */

#include "sys/rtimer.h"
#include "sys/clock.h"
#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
#include <MicroSpecific.h>
#include "dev/watchdog.h"
#include "sys/energest.h"
#include "sys/process.h"

#if !RTIMER_USE_32KHZ

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define RTIMER_TIMER_ISR_DEV  E_AHI_DEVICE_TICK_TIMER

static volatile rtimer_clock_t scheduled_time;
static volatile uint8_t has_next;

void
rtimer_arch_run_next(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
  uint32_t delta;

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
  delta = u32AHI_TickTimerRead() - scheduled_time;
  if(delta >> 28 == 0) {
    /* run scheduled */
    has_next = 0;
    watchdog_start();
    rtimer_run_next();
    process_nevents();
  } else {
    /* No match. Schedule again. */
    vAHI_TickTimerIntEnable(1);
    vAHI_TickTimerInterval(scheduled_time);
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  /* Initialise tick timer to run continuously */
  vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
  vAHI_TickTimerIntEnable(0);
  vAHI_TickTimerRegisterCallback(rtimer_arch_run_next);
  vAHI_TickTimerWrite(0);
  vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_CONT);

  /* enable wakeup timers, but keep interrupts disabled */
  vAHI_WakeTimerEnable(WAKEUP_TIMER, FALSE);
  vAHI_WakeTimerEnable(TICK_TIMER, FALSE);
  /* count down from zero (2, as values 0 and 1 must not be used) */
  vAHI_WakeTimerStartLarge(TICK_TIMER, 2);

  (void)u32AHI_Init();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_reinit(rtimer_clock_t sleep_start, rtimer_clock_t sleep_ticks)
{
  uint64_t t;
  /* Initialise tick timer to run continuously */
  vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
  vAHI_TickTimerIntEnable(0);
  /* set the highest priority for the rtimer interrupt */
  vAHI_InterruptSetPriority(MICRO_ISR_MASK_TICK_TMR, 15);
  vAHI_TickTimerRegisterCallback(rtimer_arch_run_next);
  WAIT_FOR_EDGE(t);
  vAHI_TickTimerWrite(sleep_start + sleep_ticks);
  vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_CONT);

  /* call pending interrupts */
  u32AHI_Init();

  if(has_next) {
    vAHI_TickTimerIntPendClr();
    vAHI_TickTimerIntEnable(1);
    vAHI_TickTimerInterval(scheduled_time);
  }
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
  has_next = 1;
  scheduled_time = t;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_time_to_rtimer(void)
{
  rtimer_clock_t now = RTIMER_NOW();
  if(has_next) {
    return scheduled_time >= now ? scheduled_time - now : 0;
  }
  /* if no wakeup is scheduled yet return maximum time */
  return (rtimer_clock_t)-1;
}
/*---------------------------------------------------------------------------*/
#endif /* !RTIMER_USE_32KHZ */
