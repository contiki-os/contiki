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

void rtc_isr(void) {
	PRINTF("rtc_wu_irq\n\r");
	PRINTF("now is %u\n", rtimer_arch_now());
	disable_rtc_wu();
	disable_rtc_wu_irq();
	rtimer_run_next();
	clear_rtc_wu_evt();
}

void
rtimer_arch_init(void)
{
}

void
rtimer_arch_schedule(rtimer_clock_t t)
{
	volatile uint32_t now;
	now = rtimer_arch_now();
	PRINTF("rtimer_arch_schedule time %u; now is %u\n", t,now);

#if 1
/* If specified time is always in the future, counter can wrap without harm */
	*CRM_RTC_TIMEOUT = t - now;
#else
/* Immediate interrupt if specified time is before current time. This may also
   happen on counter overflow. */
	if(now>t) {
		*CRM_RTC_TIMEOUT = 1;
	} else {
		*CRM_RTC_TIMEOUT = t - now;
	}
#endif

	clear_rtc_wu_evt();
	enable_rtc_wu();
	enable_rtc_wu_irq();
	PRINTF("rtimer_arch_schedule CRM_RTC_TIMEOUT is %u\n", *CRM_RTC_TIMEOUT);
}
