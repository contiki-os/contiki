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
 * \file
 *         EFM32 RTimer implementation (based on RTC module)
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include "rtimer-arch.h"
#include "sys/energest.h"
#include "sys/rtimer.h"
#include "sys/process.h"
#include "dev/watchdog.h"

#include "em_rtc.h"
#include "em_cmu.h"


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


// https://github.com/contiki-os/contiki/wiki/Timers#wiki-The_Rtimer_Library

void RTC_IRQHandler(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  // Find reason of IRQ

  if(RTC_IntGet() & RTC_IF_COMP0)
  {
    watchdog_start();
    rtimer_run_next();
    watchdog_stop();
    // disable interrupt
    RTC_IntDisable(RTC_IF_COMP0);
  }
  else
  {
    PRINTF("%s: unknown reason for RTC interrupt\r\n",__func__);
  }

  // Clear interrupts
  RTC_IntClear(_RTC_IF_MASK);

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  RTC_Init_TypeDef init = RTC_INIT_DEFAULT;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  /* Don't use prescaler to have highest precision */
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_1);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false; /* Start disabled to reset counter */
  init.debugRun = false;
  init.comp0Top = false; /* Don't Reset Count on comp0 */
  RTC_Init(&init);

  /* Disable interrupt generation from RTC0 */
  RTC_IntDisable(_RTC_IF_MASK);

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);

  /* Start RTC counter */
  RTC_Enable(true);
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void)
{
	return RTC_CounterGet();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  // Program Alarm
  RTC_IntClear(RTC_IF_COMP0);

  // Enable Alarm 0 preventing overflow !!!
  RTC_CompareSet(0, (t & 0xFFFFFF));
  PRINTF("Now %u, comp0 = %u\r\n", rtimer_arch_now(), t);

  RTC_IntEnable(RTC_IF_COMP0);
}
/*---------------------------------------------------------------------------*/



/* Test Functions */
#if 0
static struct rtimer task;
static volatile uint8_t sem = 0;
void callback(struct rtimer *t, void *ptr)
{
  printf("Rtimer callback \r\n");
  sem=1;
}

static void test_rtimer(void)
{
	while(1)
	{
		printf("Now = %u\r\n",RTIMER_NOW());
		rtimer_set(&task, RTIMER_NOW() + RTIMER_SECOND*2, RTIMER_SECOND, &callback, NULL);
		PROCESS_WAIT_UNTIL(sem);
		sem=0;
	}
}

#endif
