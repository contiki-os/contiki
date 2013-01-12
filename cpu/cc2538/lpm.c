/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 *
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
 * \addtogroup cc2538-lpm
 * @{
 *
 * \file
 * Implementation of low power modes ofr the cc2538
 */
#include "contiki-conf.h"
#include "sys/energest.h"
#include "sys/process.h"
#include "dev/sys-ctrl.h"
#include "dev/scb.h"
#include "dev/rfcore-xreg.h"
#include "dev/usb-regs.h"
#include "rtimer-arch.h"
#include "reg.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#if ENERGEST_CONF_ON
static unsigned long irq_energest = 0;

#define ENERGEST_IRQ_SAVE(a) do { \
    a = energest_type_time(ENERGEST_TYPE_IRQ); } while(0)
#define ENERGEST_IRQ_RESTORE(a) do { \
    energest_type_set(ENERGEST_TYPE_IRQ, a); } while(0)
#else
#define ENERGEST_IRQ_SAVE(a) do {} while(0)
#define ENERGEST_IRQ_RESTORE(a) do {} while(0)
#endif
/*---------------------------------------------------------------------------*/
/*
 * Deep Sleep thresholds in rtimer ticks (~30.5 usec)
 *
 * If Deep Sleep duration < DEEP_SLEEP_PM1_THRESHOLD, simply enter PM0
 * If duration < DEEP_SLEEP_PM2_THRESHOLD drop to PM1
 * else PM2.
 */
#define DEEP_SLEEP_PM1_THRESHOLD    10
#define DEEP_SLEEP_PM2_THRESHOLD    100
/*---------------------------------------------------------------------------*/
#define assert_wfi() do { asm("wfi"::); } while(0)
/*---------------------------------------------------------------------------*/
#if LPM_CONF_STATS
rtimer_clock_t lpm_stats[3];

#define LPM_STATS_INIT()         do { memset(lpm_stats, 0, sizeof(lpm_stats)); \
  } while(0)
#define LPM_STATS_ADD(pm, val)   do { lpm_stats[pm] += val; } while(0)
#else
#define LPM_STATS_INIT()
#define LPM_STATS_ADD(stat, val)
#endif
/*---------------------------------------------------------------------------*/
/*
 * Remembers what time it was when went to deep sleep
 * This is used when coming out of PM1/2 to adjust the system clock, which
 * stops ticking while in those PMs
 */
static rtimer_clock_t sleep_enter_time;

#define RTIMER_CLOCK_TICK_RATIO (RTIMER_SECOND / CLOCK_SECOND)

void clock_adjust(clock_time_t ticks);
/*---------------------------------------------------------------------------*/
/* Stores the currently specified MAX allowed PM */
static uint8_t max_pm;
/*---------------------------------------------------------------------------*/
/*
 * Routine to put is in PM0. We also need to do some housekeeping if the stats
 * or the energest module is enabled
 */
static void
enter_pm0(void)
{
  ENERGEST_OFF(ENERGEST_TYPE_CPU);
  ENERGEST_ON(ENERGEST_TYPE_LPM);

  /* We are only interested in IRQ energest while idle or in LPM */
  ENERGEST_IRQ_RESTORE(irq_energest);

  /*
   * After PM0 we don't need to adjust the system clock. Thus, saving the time
   * we enter Deep Sleep is only required if we are keeping stats.
   */
  if(LPM_CONF_STATS) {
    sleep_enter_time = RTIMER_NOW();
  }

  assert_wfi();

  /* We reach here when the interrupt context that woke us up has returned */
  LPM_STATS_ADD(0, RTIMER_NOW() - sleep_enter_time);

  /* Remember IRQ energest for next pass */
  ENERGEST_IRQ_SAVE(irq_energest);

  ENERGEST_ON(ENERGEST_TYPE_CPU);
  ENERGEST_OFF(ENERGEST_TYPE_LPM);
}
/*---------------------------------------------------------------------------*/
static void
select_32_mhz_xosc(void)
{
  /* Turn on the 32 MHz XOSC and source the system clock on it. */
  REG(SYS_CTRL_CLOCK_CTRL) &= ~SYS_CTRL_CLOCK_CTRL_OSC;

  /* Wait for the switch to take place */
  while((REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_OSC) != 0);
}
/*---------------------------------------------------------------------------*/
static void
select_16_mhz_rcosc(void)
{
  /*First, make sure there is no ongoing clock source change */
  while((REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_SOURCE_CHANGE) != 0);

  /* Set the System Clock to use the 16MHz RC OSC */
  REG(SYS_CTRL_CLOCK_CTRL) |= SYS_CTRL_CLOCK_CTRL_OSC;

  /* Wait till it's happened */
  while((REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_OSC) == 0);
}
/*---------------------------------------------------------------------------*/
void
lpm_exit()
{
  if((REG(SYS_CTRL_PMCTL) & SYS_CTRL_PMCTL_PM3) == SYS_CTRL_PMCTL_PM0) {
    /* We either just exited PM0 or we were not sleeping in the first place.
     * We don't need to do anything clever */
    return;
  }

  LPM_STATS_ADD(REG(SYS_CTRL_PMCTL) & SYS_CTRL_PMCTL_PM3,
                RTIMER_NOW() - sleep_enter_time);

  /* Adjust the system clock, since it was not counting while we were sleeping
   * We need to convert sleep duration from rtimer ticks to clock ticks and
   * this will cost us some accuracy */
  clock_adjust((clock_time_t)
               ((RTIMER_NOW() - sleep_enter_time) / RTIMER_CLOCK_TICK_RATIO));

  /* Restore system clock to the 32 MHz XOSC */
  select_32_mhz_xosc();

  /* Restore PMCTL to PM0 for next pass */
  REG(SYS_CTRL_PMCTL) = SYS_CTRL_PMCTL_PM0;

  /* Remember IRQ energest for next pass */
  ENERGEST_IRQ_SAVE(irq_energest);

  ENERGEST_ON(ENERGEST_TYPE_CPU);
  ENERGEST_OFF(ENERGEST_TYPE_LPM);
}
/*---------------------------------------------------------------------------*/
void
lpm_enter()
{
  rtimer_clock_t lpm_exit_time;
  rtimer_clock_t duration;

  /*
   * If either the RF or the USB is on, dropping to PM1/2 would equal pulling
   * the rug (32MHz XOSC) from under their feet. Thus, we only drop to PM0.
   * PM0 is also used if max_pm==0
   *
   * Note: USB Suspend/Resume/Remote Wake-Up are not supported. Once the PLL is
   * on, it stays on.
   */
  if((REG(RFCORE_XREG_FSMSTAT0) & RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE) != 0
     || REG(USB_CTRL) != 0 || max_pm == 0) {
    enter_pm0();

    /* We reach here when the interrupt context that woke us up has returned */
    return;
  }

  /*
   * USB PLL was off. Radio was off: Some Duty Cycling in place.
   * rtimers run on the Sleep Timer. Thus, if we have a scheduled rtimer
   * task, a Sleep Timer interrupt will fire and will wake us up.
   * Choose the most suitable PM based on anticipated deep sleep duration
   */
  lpm_exit_time = rtimer_arch_next_trigger();
  duration = lpm_exit_time - RTIMER_NOW();

  if(duration < DEEP_SLEEP_PM1_THRESHOLD || lpm_exit_time == 0) {
    /* Anticipated duration too short or no scheduled rtimer task. Use PM0 */
    enter_pm0();

    /* We reach here when the interrupt context that woke us up has returned */
    return;
  }

  /* If we reach here, we -may- (but may as well not) be dropping to PM1+. We
   * know the USB and RF are off so we can switch to the 16MHz RCOSC. */
  select_16_mhz_rcosc();

  /*
   * Switching the System Clock from the 32MHz XOSC to the 16MHz RC OSC may
   * have taken a while. Re-estimate sleep duration.
   */
  duration = lpm_exit_time - RTIMER_NOW();

  if(duration < DEEP_SLEEP_PM1_THRESHOLD) {
    /*
     * oops... The clock switch took some time and now the remaining sleep
     * duration is too short. Restore the clock source to the 32MHz XOSC and
     * abort the LPM attempt altogether. We can't drop to PM0,
     * we need to yield to main() since we may have events to service now.
     */
    select_32_mhz_xosc();

    return;
  } else if(duration >= DEEP_SLEEP_PM2_THRESHOLD && max_pm == 2) {
    /* Long sleep duration and PM2 is allowed. Use it */
    REG(SYS_CTRL_PMCTL) = SYS_CTRL_PMCTL_PM2;
  } else {
    /*
     * Anticipated duration too short for PM2 but long enough for PM1 and we
     * are allowed to use PM1
     */
    REG(SYS_CTRL_PMCTL) = SYS_CTRL_PMCTL_PM1;
  }

  /* We are only interested in IRQ energest while idle or in LPM */
  ENERGEST_IRQ_RESTORE(irq_energest);
  ENERGEST_OFF(ENERGEST_TYPE_CPU);
  ENERGEST_ON(ENERGEST_TYPE_LPM);

  /* Remember the current time so we can adjust the clock when we wake up */
  sleep_enter_time = RTIMER_NOW();

  /*
   * Last chance to abort entering Deep Sleep.
   *
   * - There is the slight off-chance that a SysTick interrupt fired while we
   *   were trying to make up our mind. This may have raised an event.
   * - The Sleep Timer may have fired
   *
   * Check if there is still a scheduled rtimer task and check for pending
   * events before going to Deep Sleep
   */
  if(process_nevents() || rtimer_arch_next_trigger() == 0) {
    /* Event flag raised or rtimer inactive.
     * Turn on the 32MHz XOSC, restore PMCTL and abort */
    select_32_mhz_xosc();

    REG(SYS_CTRL_PMCTL) = SYS_CTRL_PMCTL_PM0;
  } else {
    /* All clear. Assert WFI and drop to PM1/2. This is now un-interruptible */
    assert_wfi();
  }

  /*
   * We reach here after coming back from PM1/2. The interrupt context that
   * woke us up has returned. lpm_exit() has run, it has switched the system
   * clock source back to the 32MHz XOSC, it has adjusted the system clock,
   * it has restored PMCTL and it has done energest housekeeping
   */
  return;
}
/*---------------------------------------------------------------------------*/
void
lpm_set_max_pm(uint8_t pm)
{
  max_pm = pm > LPM_CONF_MAX_PM ? LPM_CONF_MAX_PM : pm;
}
/*---------------------------------------------------------------------------*/
void
lpm_init()
{
  /*
   * The main loop calls lpm_enter() when we have no more events to service.
   * By default, we will enter PM0 unless lpm_enter() decides otherwise
   */
  REG(SYS_CTRL_PMCTL) = SYS_CTRL_PMCTL_PM0;
  REG(SCB_SYSCTRL) |= SCB_SYSCTRL_SLEEPDEEP;

  max_pm = LPM_CONF_MAX_PM;

  LPM_STATS_INIT();
}
/*---------------------------------------------------------------------------*/
/** @} */
