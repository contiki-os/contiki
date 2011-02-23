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

#include "mc1322x.h"

#define CRM_DEBUG 1
#if CRM_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

uint32_t cal_rtc_secs;      /* calibrated 2khz rtc seconds */

void sleep(uint32_t opts, uint32_t mode)
{

	/* the maca must be off before going to sleep */
	/* otherwise the mcu will reboot on wakeup */
//	maca_off();
	*CRM_SLEEP_CNTL = opts;
	*CRM_SLEEP_CNTL = (opts | mode);

	/* wait for the sleep cycle to complete */
	while(!bit_is_set(*CRM_STATUS,0)) { continue; }
        /* write 1 to sleep_sync --- this clears the bit (it's a r1wc bit) and powers down */
	set_bit(*CRM_STATUS,0);

	/* now we are asleep */
	/* and waiting for something to wake us up */
	/* you did tell us how to wake up right? */

	/* waking up */
	while(!bit_is_set(*CRM_STATUS,0)) { continue; }
        /* write 1 to sleep_sync --- this clears the bit (it's a r1wc bit) and finishes the wakeup */
	set_bit(*CRM_STATUS,0);

	/* you may also need to do other recovery */
	/* such as interrupt handling */
	/* peripheral init */
	/* and turning the radio back on */

}

/* turn on the 32kHz crystal */
/* once you start the 32xHz crystal it can only be stopped with a reset (hard or soft) */
void enable_32khz_xtal(void) 
{
	static volatile uint32_t rtc_count;
        PRINTF("enabling 32kHz crystal\n\r");
	/* first, disable the ring osc */
	ring_osc_off();
	/* enable the 32kHZ crystal */
	xtal32_on();

        /* set the XTAL32_EXISTS bit */
        /* the datasheet says to do this after you've check that RTC_COUNT is changing */
        /* the datasheet is not correct */
	xtal32_exists();

	/* now check that the crystal starts */
	/* this blocks until it starts */
	/* it would be better to timeout and return an error */
	rtc_count = *CRM_RTC_COUNT;
	PRINTF("waiting for xtal\n\r");
	while(*CRM_RTC_COUNT == rtc_count) {
		continue;
	}
	/* RTC has started up */
	PRINTF("32kHZ xtal started\n\r");

}

void cal_ring_osc(void)
{
	uint32_t cal_factor;
	PRINTF("performing ring osc cal\n\r");
	PRINTF("crm_status: 0x%0x\n\r",*CRM_STATUS);
	PRINTF("sys_cntl: 0x%0x\n\r",*CRM_SYS_CNTL); 
	*CRM_CAL_CNTL = (1<<16) | (20000); 
	while((*CRM_STATUS & (1<<9)) == 0);
	PRINTF("ring osc cal complete\n\r");
	PRINTF("cal_count: 0x%0x\n\r",*CRM_CAL_COUNT); 
	cal_factor = (REF_OSC*1000) / *CRM_CAL_COUNT;
	cal_rtc_secs = (NOMINAL_RING_OSC_SEC * cal_factor)/100;
	PRINTF("cal factor: %d\n\r", cal_factor);
	PRINTF("hib_wake_secs: %d\n\r", cal_rtc_secs);      
	set_bit(*CRM_STATUS,9);
}
