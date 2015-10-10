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
 *         Clock implementation for NXP jn516x.
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
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

/**
 * TickTimer will be used for RTIMER
 * E_AHI_TIMER_1 will be used for ticking
 **/

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static volatile unsigned long seconds = 0;
static volatile uint8_t ticking = 0;
static volatile clock_time_t clock_ticks = 0;
/* last_tar is used for calculating clock_fine */

#define CLOCK_TIMER           E_AHI_TIMER_1
#define CLOCK_TIMER_ISR_DEV    E_AHI_DEVICE_TIMER1
/* 16Mhz / 2^7 = 125Khz */
#define CLOCK_PRESCALE 7
/* 10ms tick --> overflow after ~981/2 days */
#define CLOCK_INTERVAL (125 * 10)
#define MAX_TICKS (CLOCK_INTERVAL)
/*---------------------------------------------------------------------------*/
void
clockTimerISR(uint32 u32Device, uint32 u32ItemBitmap)
{
  if(u32Device != CLOCK_TIMER_ISR_DEV) {
    return;
  }
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  watchdog_start();

  clock_ticks++;
  if(clock_ticks % CLOCK_CONF_SECOND == 0) {
    ++seconds;
    energest_flush();
  }
  if(etimer_pending() && (etimer_next_expiration_time() - clock_ticks - 1) > MAX_TICKS) {
    etimer_request_poll();
    /* TODO exit low-power mode */
  }
  if(process_nevents() >= 0) {
    /* TODO exit low-power mode */
  }

  watchdog_stop();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static void
clock_timer_init(void)
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
  clock_ticks = 0;
  vAHI_TimerStartRepeat(CLOCK_TIMER, 0, CLOCK_INTERVAL);
  ticking = 1;
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
  /* system controller interrupts callback is disabled
   * -- Only wake Interrupts --
   */
  vAHI_SysCtrlRegisterCallback(0);

  /* schedule clock tick interrupt */
  clock_timer_init();
  rtimer_init();
  (void)u32AHI_Init();

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
clock_time_t
clock_time(void)
{
  clock_time_t t1, t2;
  do {
    t1 = clock_ticks;
    t2 = clock_ticks;
  } while(t1 != t2);
  return t1;
}
/*---------------------------------------------------------------------------*/
void
clock_set(clock_time_t clock, clock_time_t fclock)
{
  clock_ticks = clock;
}
/*---------------------------------------------------------------------------*/
int
clock_fine_max(void)
{
  return CLOCK_INTERVAL;
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 0.0625 us.
 */
void
clock_delay_usec(uint16_t dt)
{
  volatile uint32_t t = u32AHI_TickTimerRead();
#define RTIMER_MAX_TICKS 0xffffffff
  /* beware of wrapping */
  if(RTIMER_MAX_TICKS - t < dt) {
    while(u32AHI_TickTimerRead() < RTIMER_MAX_TICKS && u32AHI_TickTimerRead() != 0) ;
    dt -= RTIMER_MAX_TICKS - t;
    t = 0;
  }
  while(u32AHI_TickTimerRead() - t < dt) {
    watchdog_periodic();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 8 us.
 */
void
clock_delay(unsigned int i)
{
  volatile uint32_t t = u16AHI_TimerReadCount(CLOCK_TIMER);
  /* beware of wrapping */
  if(MAX_TICKS - t < i) {
    while(u16AHI_TimerReadCount(CLOCK_TIMER) < MAX_TICKS && u16AHI_TimerReadCount(CLOCK_TIMER) != 0) ;
    i -= MAX_TICKS - t;
    t = 0;
  }
  while(u16AHI_TimerReadCount(CLOCK_TIMER) - t < i) {
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
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)t) {
    watchdog_periodic();
  }
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  seconds = sec;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  unsigned long t1, t2;
  do {
    t1 = seconds;
    t2 = seconds;
  } while(t1 != t2);
  return t1;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
clock_counter(void)
{
  return rtimer_arch_now();
}
