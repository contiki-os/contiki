/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
 *
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
 * This file is part of the Contiki OS.
 *
 *
 */

/**
 * \file
 *         mc1322x-specific rtimer code
 * \author
 *         Mariano Alvira <mar@devl.org>
 */

#include <signal.h>

/* contiki */
#include "sys/energest.h"
#include "sys/rtimer.h"

/* mc1322x */
#include "utils.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static uint32_t last_rtc;

void
rtc_isr(void)
{
	/* see note in table 5-13 of the reference manual: it takes at least two RTC clocks for the EVT bit to clear */
	if ((CRM->RTC_COUNT - last_rtc) <= 2) {
		CRM->STATUS = ~0; /* Clear all events */

//		CRM->STATUSbits.RTC_WU_EVT = 1;
		return;
	}

	last_rtc = CRM->RTC_COUNT;

  /* Clear all events (for paranoia) */
	/* clear RTC event flag (for paranoia)*/
//	CRM->STATUSbits.RTC_WU_EVT = 1;
	CRM->STATUS = ~0; 

	rtimer_run_next();

}

void
rtimer_arch_init(void)
{
	last_rtc = CRM->RTC_COUNT;
	/* enable timeout interrupts */
	/* RTC WU is the periodic RTC timer */
	/* TIMER WU is the wakeup timers (clocked from the RTC source) */
	/* it does not appear you can have both enabled at the same time */
	CRM->WU_CNTLbits.RTC_WU_EN = 1;
	CRM->WU_CNTLbits.RTC_WU_IEN = 1;
	enable_irq(CRM);
}

void
rtimer_arch_schedule(rtimer_clock_t t)
{
	volatile uint32_t now;
	now = rtimer_arch_now();
	PRINTF("rtimer_arch_schedule time %u; now is %u\n", t, now);

/* Immediate interrupt if specified time is before current time. This may also
   happen on counter overflow. */
	if(now > t) {
		CRM->RTC_TIMEOUT = 1;
	} else {
		CRM->RTC_TIMEOUT = t - now;
	}
}

void 
rtimer_arch_sleep(rtimer_clock_t howlong)
{
	CRM->WU_CNTLbits.TIMER_WU_EN = 1;
	CRM->WU_CNTLbits.RTC_WU_EN = 0;
	CRM->WU_TIMEOUT = howlong;

	/* the maca must be off before going to sleep */
	/* otherwise the mcu will reboot on wakeup */
	maca_off();

	CRM->SLEEP_CNTLbits.DOZE = 0;
	CRM->SLEEP_CNTLbits.RAM_RET = 3;
	CRM->SLEEP_CNTLbits.MCU_RET = 1;
	CRM->SLEEP_CNTLbits.DIG_PAD_EN = 1;
	CRM->SLEEP_CNTLbits.HIB = 1;

  /* wait for the sleep cycle to complete */
  while((*CRM_STATUS & 0x1) == 0) { continue; }
  /* write 1 to sleep_sync --- this clears the bit (it's a r1wc bit) and powers down */
  *CRM_STATUS = 1;

  /* asleep */

  /* wait for the awake cycle to complete */
  while((*CRM_STATUS & 0x1) == 0) { continue; }
  /* write 1 to sleep_sync --- this clears the bit (it's a r1wc bit) and finishes wakeup */
  *CRM_STATUS = 1;

	CRM->WU_CNTLbits.TIMER_WU_EN = 0;
	CRM->WU_CNTLbits.RTC_WU_EN = 1;

	/* reschedule clock ticks */
	clock_init();
	clock_adjust_ticks((CRM->WU_COUNT*CLOCK_CONF_SECOND)/rtc_freq);
}


