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
 *         Tickless clock implementation for NXP jn516x.
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *         Atis Elsts <atis.elsts@sics.se>
 *
 */

#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
#include "contiki.h"
#include "sys/energest.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include "rtimer-arch.h"
#include "dev/watchdog.h"


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define CLOCK_TIMER           E_AHI_TIMER_1
#define CLOCK_TIMER_ISR_DEV   E_AHI_DEVICE_TIMER1
/* 16Mhz / 2^10 = 15.625 kHz */
#define CLOCK_PRESCALE 10
/* 8ms tick --> overflow after ~397.7 days */
#define CLOCK_INTERVAL 125
/* Max schedulable number of ticks.
 * Must not be more than:
 *   0xffff / (16'000'000 / (1 << CLOCK_PRESCALE) / CLOCK_SECOND)
 */
#define CLOCK_MAX_SCHEDULABLE_TICKS 520
/* Min guard time an etimer can be scheduled before an rtimer */
#define CLOCK_RTIMER_GUARD_TIME US_TO_RTIMERTICKS(16)
/* Clock tick expressed as rtimer ticks */
#define CLOCK_TICK ((1 << CLOCK_PRESCALE) * CLOCK_INTERVAL)

#define CLOCK_LT(a, b) ((int32_t)((a)-(b)) < 0)

/*---------------------------------------------------------------------------*/
static void
check_etimers(void)
{
  if(etimer_pending()) {
    clock_time_t now = clock_time();
    if(!CLOCK_LT(now, etimer_next_expiration_time())) {
      etimer_request_poll();
    }
  }
  process_nevents();
}
/*---------------------------------------------------------------------------*/
void
clockTimerISR(uint32 u32Device, uint32 u32ItemBitmap)
{
  if(u32Device != CLOCK_TIMER_ISR_DEV) {
    return;
  }

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  check_etimers();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
clock_arch_calibrate(void)
{
  bAHI_SetClockRate(E_AHI_XTAL_32MHZ);

  /* Wait for oscillator to stabilise */
  while(bAHI_GetClkSource() == 1) ;
  while(bAHI_Clock32MHzStable() == 0) ;

  vAHI_OptimiseWaitStates();

  /* Turn on SPI master */
  vREG_SysWrite(REG_SYS_PWR_CTRL, u32REG_SysRead(REG_SYS_PWR_CTRL)
                | REG_SYSCTRL_PWRCTRL_SPIMEN_MASK);
}
/*---------------------------------------------------------------------------*/
void
clock_arch_init(void)
{
  vAHI_TimerEnable(CLOCK_TIMER, CLOCK_PRESCALE, 0, 1, 0);
  vAHI_TimerClockSelect(CLOCK_TIMER, 0, 0);

  vAHI_TimerConfigureOutputs(CLOCK_TIMER, 0, 1);
  vAHI_TimerDIOControl(CLOCK_TIMER, 0);

#if (CLOCK_TIMER == E_AHI_TIMER_0)
  vAHI_Timer0RegisterCallback(clockTimerISR);
#elif (CLOCK_TIMER == E_AHI_TIMER_1)
  vAHI_Timer1RegisterCallback(clockTimerISR);
#endif
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  /* gMAC_u8MaxBuffers = 2; */
#ifdef JENNIC_CHIP_FAMILY_JN516x
  /* Turn off debugger */
  *(volatile uint32 *)0x020000a0 = 0;
#endif

  clock_arch_calibrate();

  /* setup clock mode and interrupt handler */
  clock_arch_init();
}
/*---------------------------------------------------------------------------*/
static uint32_t
clock(void)
{
  /* same as rtimer_arch_now() */
  return u32AHI_TickTimerRead();
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return clock() / CLOCK_TICK;
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 0.0625 us.
 */
void
clock_delay_usec(uint16_t dt)
{
  uint32_t end = clock() + dt;
  /* Note: this does not call watchdog periodic() */
  while(CLOCK_LT(clock(), end));
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 8 us.
 */
void
clock_delay(unsigned int dt)
{
  uint32_t end = clock() + dt * 128;
  while(CLOCK_LT(clock(), end)) {
    watchdog_periodic();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 10 ms.
 *
 */
void
clock_wait(clock_time_t t)
{
  clock_time_t end = clock_time() + t;
  while(CLOCK_LT(clock_time(), end)) {
    watchdog_periodic();
  }
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return clock() / (CLOCK_TICK * CLOCK_CONF_SECOND);
}
/*---------------------------------------------------------------------------*/
clock_t
clock_arch_time_to_etimer(void)
{
  clock_t time_to_etimer;
  if(etimer_pending()) {
    time_to_etimer = etimer_next_expiration_time() - clock_time();
    if(time_to_etimer < 0) {
      time_to_etimer = 0;
    }
  } else {
    /* no active etimers */
    time_to_etimer = (clock_t)-1;
  }
  return time_to_etimer;
}
/*---------------------------------------------------------------------------*/
void
clock_arch_schedule_interrupt(clock_t time_to_etimer, rtimer_clock_t ticks_to_rtimer)
{
  if(time_to_etimer > CLOCK_MAX_SCHEDULABLE_TICKS) {
    time_to_etimer = CLOCK_MAX_SCHEDULABLE_TICKS;
  }

  time_to_etimer *= CLOCK_INTERVAL;

  if(ticks_to_rtimer != (rtimer_clock_t)-1) {
    /* if the next rtimer is close enough to the etimer... */
    rtimer_clock_t ticks_to_etimer = time_to_etimer * (1 << CLOCK_PRESCALE);

#if RTIMER_USE_32KHZ
    ticks_to_rtimer = (uint64_t)ticks_to_rtimer * (F_CPU / 2) / RTIMER_SECOND;
#endif

    if(!CLOCK_LT(ticks_to_rtimer, ticks_to_etimer)
        && CLOCK_LT(ticks_to_rtimer, ticks_to_etimer + CLOCK_RTIMER_GUARD_TIME)) {
      /* ..then schedule the etimer after the rtimer */
      time_to_etimer += 2;
    }
  }

  /* interrupt will not be generated if 0 is passed as the parameter */
  if(time_to_etimer == 0) {
    time_to_etimer = 1;
  }

  vAHI_TimerStartSingleShot(CLOCK_TIMER, 0, time_to_etimer);
}
/*---------------------------------------------------------------------------*/
