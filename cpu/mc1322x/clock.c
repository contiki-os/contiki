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

#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include <sys/rtimer.h>

#include "contiki-conf.h"
#include "mc1322x.h"

#define MAX_TICKS (~((clock_time_t)0) / 2)

static volatile clock_time_t current_clock = 0;

volatile unsigned long seconds = 0;

static struct rtimer rt_clock;

/* the typical clock things like incrementing current_clock and etimer checks */
/* are performed as a periodically scheduled rtimer */
void
rt_do_clock(struct rtimer *t, void *ptr)
{
	rtimer_set(t, RTIMER_TIME(t) + (rtc_freq/CLOCK_CONF_SECOND) , 1,
						 (rtimer_callback_t)rt_do_clock, ptr);

	current_clock++;

	if((current_clock % CLOCK_CONF_SECOND) == 0) {
		seconds++;
	}

	if(etimer_pending() &&
		 (etimer_next_expiration_time() - current_clock - 1) > MAX_TICKS) {
		etimer_request_poll();
	}

}


/* RTC MUST have been already setup by mc1322x init */
void
clock_init(void)
{
	rtimer_set(&rt_clock, RTIMER_NOW() + rtc_freq/CLOCK_CONF_SECOND, 1, (rtimer_callback_t)rt_do_clock, NULL);
}

clock_time_t
clock_time(void)
{
	return current_clock;
}

unsigned long
clock_seconds(void)
{
	return seconds;
}

void
clock_wait(clock_time_t t)
{
	clock_time_t endticks = current_clock + t;
	while ((signed long)(current_clock - endticks) < 0) {;}
}
/*---------------------------------------------------------------------------*/
/*
 * Delay the CPU for up to 65535 microseconds.
 * Use the 250KHz MACA clock for longer delays to avoid interrupt effects.
 * However that can't be used if the radio is being power cycled!
 */
void
clock_delay_usec(uint16_t howlong)
{
	if(howlong<2) return;
#if 0
	if(howlong>400) {
		volatile register uint32_t i=*MACA_CLK+howlong/4;
		while (i > *MACA_CLK) ;
		return;
	}
#endif
	 /* These numbers at 25MHz, gcc -Os */
		volatile register uint32_t i=4000*howlong/2301;
		while(--i);
}
/*---------------------------------------------------------------------------*/
/*
 * Delay the CPU for up to 65535 milliseconds. The watchdog is NOT disabled.
 */
void
clock_delay_msec(uint16_t howlong)
{
	while(howlong--) clock_delay_usec(1000);
}
/*---------------------------------------------------------------------------*/
/*
 * Legacy delay. The original clock_delay for the msp430 used a granularity
 * of 2.83 usec. This approximates that delay for values up to 1456 usec.
 * (The largest core call in leds.c uses 400).
 */
void
clock_delay(unsigned int howlong)
{
	if(howlong--) return;
	clock_delay_usec((283*howlong)/100);
}
/*---------------------------------------------------------------------------*/
/*
 * Adjust clock ticks after a cpu sleep.
 */
void clock_adjust_ticks(clock_time_t howmany) {
/* Add seconds */
	seconds+=howmany/CLOCK_CONF_SECOND;
/* Handle tick overflow */
	if(((current_clock % CLOCK_CONF_SECOND) + (howmany % CLOCK_CONF_SECOND)) >= CLOCK_CONF_SECOND) seconds++;
/* Add ticks */
	current_clock+=howmany;
}
