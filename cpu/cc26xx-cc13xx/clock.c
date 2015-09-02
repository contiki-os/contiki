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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup platform
 * @{
 *
 * \defgroup cc26xx-platforms TI CC26xx-powered Platforms
 * @{
 *
 * \defgroup cc26xx The TI CC26xx and CC13xx CPUs
 *
 * This group documents the TI CC26xx and CC13xx CPUs. The two CPU families are
 * very similar, with the main difference being related to radio capability.
 *
 * Documentation in this group should be considered to be applicable to both
 * families, unless explicitly stated otherwise.
 *
 * @{
 *
 * \addtogroup cc26xx-clocks
 * @{
 *
 * \defgroup cc26xx-software-clock Software Clock
 *
 * Implementation of the clock module for the CC26xx and CC13xx.
 *
 * The software clock uses the facilities provided by the AON RTC driver
 * @{
 *
 * \file
 * Software clock implementation for the TI CC13xx/CC26xx
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "ti-lib.h"
/*---------------------------------------------------------------------------*/
static volatile uint64_t count;
/*---------------------------------------------------------------------------*/
static void
power_domain_on(void)
{
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);
  while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH) !=
        PRCM_DOMAIN_POWER_ON);
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  count = 0;

  /*
   * Here, we configure GPT0 Timer A, which we subsequently use in
   * clock_delay_usec
   *
   * We need to access registers, so firstly power up the PD and then enable
   * the clock to GPT0.
   */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH) !=
     PRCM_DOMAIN_POWER_ON) {
    power_domain_on();
  }

  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Disable both GPT0 timers */
  HWREG(GPT0_BASE + GPT_O_CTL) &= ~(GPT_CTL_TAEN | GPT_CTL_TBEN);

  /*
   * We assume that the clock is running at 48MHz, we use GPT0 Timer A,
   * one-shot, countdown, prescaled by 48 gives us 1 tick per usec
   */
  ti_lib_timer_configure(GPT0_BASE,
                         TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT);

  /* Global config: split pair (2 x 16-bit wide) */
  HWREG(GPT0_BASE + GPT_O_CFG) = TIMER_CFG_SPLIT_PAIR >> 24;

  /*
   * Pre-scale value 47 pre-scales by 48
   *
   * ToDo: The theoretical value here should be 47 (to provide x48 prescale)
   * However, 49 seems to give results much closer to the desired delay
   */
  ti_lib_timer_prescale_set(GPT0_BASE, TIMER_B, 49);

  /* GPT0 / Timer B: One shot, PWM interrupt enable */
  HWREG(GPT0_BASE + GPT_O_TBMR) =
        ((TIMER_CFG_B_ONE_SHOT >> 8) & 0xFF) | GPT_TBMR_TBPWMIE;
}
/*---------------------------------------------------------------------------*/
static void
update_clock_variable(void)
{
  uint32_t aon_rtc_secs_now;
  uint32_t aon_rtc_secs_now2;
  uint16_t aon_rtc_ticks_now;

  do {
    aon_rtc_secs_now = HWREG(AON_RTC_BASE + AON_RTC_O_SEC);
    aon_rtc_ticks_now = HWREG(AON_RTC_BASE + AON_RTC_O_SUBSEC) >> 16;
    aon_rtc_secs_now2 = HWREG(AON_RTC_BASE + AON_RTC_O_SEC);
  } while(aon_rtc_secs_now != aon_rtc_secs_now2);

  /* Convert AON RTC ticks to clock tick counter */
  count = (aon_rtc_secs_now * CLOCK_SECOND) + (aon_rtc_ticks_now >> 9);
}
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  update_clock_variable();

  return (clock_time_t)(count & 0xFFFFFFFF);
}
/*---------------------------------------------------------------------------*/
void
clock_update(void)
{
  update_clock_variable();

  if(etimer_pending()) {
    etimer_request_poll();
  }
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  bool interrupts_disabled;
  uint32_t secs_now;

  interrupts_disabled = ti_lib_int_master_disable();

  secs_now = ti_lib_aon_rtc_sec_get();

  /* Re-enable interrupts */
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  return (unsigned long)secs_now;
}
/*---------------------------------------------------------------------------*/
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t len)
{
  uint32_t clock_status;

  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH) !=
     PRCM_DOMAIN_POWER_ON) {
    power_domain_on();
  }

  clock_status = HWREG(PRCM_BASE + PRCM_O_GPTCLKGR) & PRCM_GPIOCLKGR_CLK_EN;

  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  ti_lib_timer_load_set(GPT0_BASE, TIMER_B, len);
  ti_lib_timer_enable(GPT0_BASE, TIMER_B);

  /*
   * Wait for TBEN to clear. CC26xxware does not provide us with a convenient
   * function, hence the direct register access here
   */
  while(HWREG(GPT0_BASE + GPT_O_CTL) & GPT_CTL_TBEN);

  if(clock_status == 0) {
    ti_lib_prcm_peripheral_run_disable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());
  }
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
 * @}
 * @}
 */
