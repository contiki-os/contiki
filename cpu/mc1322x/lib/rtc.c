/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details.
 *
 *
 */

#include <mc1322x.h>
#include <stdlib.h>
#include "rtc.h"

/* Define USE_32KHZ in board.h to start and use the 32 KHz
   oscillator, otherwise the 2 KHz ring oscillator is used. */

int rtc_freq = 0;
static int __use_32khz = 0;

/* Init RTC */
void rtc_init_osc(int use_32khz)
{
	__use_32khz = use_32khz;

	if (use_32khz)
	{
		uint32_t old;

		/* You have to hold its hand with this one */
		/* once you start the 32KHz crystal it can only be
		 * stopped with a reset (hard or soft). */

		/* first, disable the ring osc */
		CRM->RINGOSC_CNTLbits.ROSC_EN = 0;

		/* enable the 32kHZ crystal */
		CRM->XTAL32_CNTLbits.XTAL32_EN = 1;

		/* set the XTAL32_EXISTS bit */
		/* the datasheet says to do this after you check that RTC_COUNT
		   is changing, but it is not correct; it needs to be set first */
		CRM->SYS_CNTLbits.XTAL32_EXISTS = 1;

		old = CRM->RTC_COUNT;
		while (CRM->RTC_COUNT == old)
			continue;

                /* RTC has started up */
		rtc_freq = 32000;
	}
	else
	{
		/* Enable ring osc */
		CRM->RINGOSC_CNTLbits.ROSC_EN = 1;
		CRM->XTAL32_CNTLbits.XTAL32_EN = 0;

		/* Set default tune values from datasheet */
		CRM->RINGOSC_CNTLbits.ROSC_CTUNE = 0x6;
		CRM->RINGOSC_CNTLbits.ROSC_FTUNE = 0x17;

		/* Trigger calibration */
		rtc_calibrate();
	}
}

uint32_t __rtc_try(int loading, int timeout)
{
	/* Total loading is
	     ctune * 1000 fF + ftune * 160 fF
	   ctune = 0-15
	   ftune = 0-31
	   max = 19960 fF
	*/

#define RTC_LOADING_MIN 0
#define RTC_LOADING_MAX 19960

	/* The fine tune covers a range larger than a single coarse
	   step.  Check all coarse steps within the fine tune range to
	   find the optimal CTUNE, FTUNE pairs. */
#define CTUNE_MAX 15
#define FTUNE_MAX 31
#define CSTEP 1000
#define FSTEP 160
#define MAX_F (FSTEP*FTUNE_MAX)  /* actually lcm(CSTEP,FSTEP) would be better,
				    but in this case it's basically the same */
	int ctune;
	int ftune;
	int ctune_start = (loading - MAX_F) / CSTEP;
	int ctune_end = loading / CSTEP;
	int best_err = loading, best_ctune = 0, best_ftune = 0;

	uint32_t count;

	if (ctune_start < 0) ctune_start = 0;
	if (ctune_end > CTUNE_MAX) ctune_end = CTUNE_MAX;

	for (ctune = ctune_start; ctune <= ctune_end; ctune++)
	{
		int this_loading, this_err;

		ftune = ((loading - (ctune * CSTEP)) + (FSTEP / 2)) / FSTEP;
		if (ftune < 0) ftune = 0;
		if (ftune > FTUNE_MAX) ftune = FTUNE_MAX;

		this_loading = ctune * CSTEP + ftune * FSTEP;
		this_err = abs(this_loading - loading);
		if (this_err < best_err) {
			best_err = this_err;
			best_ctune = ctune;
			best_ftune = ftune;
		}
	}

//	printf("requested loading %d, actual loading %d\r\n", loading,
//		  best_ctune * CSTEP + best_ftune * FSTEP);

	/* Run the calibration */
	CRM->RINGOSC_CNTLbits.ROSC_CTUNE = best_ctune;
	CRM->RINGOSC_CNTLbits.ROSC_FTUNE = best_ftune;
	CRM->CAL_CNTLbits.CAL_TIMEOUT = timeout;
	CRM->STATUSbits.CAL_DONE = 1;
	CRM->CAL_CNTLbits.CAL_EN = 1;
	while (CRM->STATUSbits.CAL_DONE == 0)
		continue;

	/* Result should ideally be close to (REF_OSC * (timeout / 2000)) */
	count = CRM->CAL_COUNT;
	if (count == 0) count = 1;  /* avoid divide by zero problems */
	return count;
}

/* Calibrate the ring oscillator */
void rtc_calibrate(void)
{
	/* Just bisect a few times.  Our best tuning accuracy is about
	   1/500 of the full scale, so doing this 8-9 times is about
	   as accurate as we can get */
	int i;
	int low = RTC_LOADING_MIN, high = RTC_LOADING_MAX;
	int mid;
	uint32_t count;

	if (__use_32khz) {
		rtc_freq = 32000;
		return;
	}

#define TIMEOUT 100  /* 50 msec per attempt */

	for (i = 0; i < 16; i++)
	{
		mid = (low + high) / 2;
		count = __rtc_try(mid, TIMEOUT);
		// careful about overflow
		rtc_freq = REF_OSC / ((count + TIMEOUT/2) / TIMEOUT);

		if (rtc_freq > 2048)
			low = mid;  // increase loading
		else
			high = mid; // decrease loading
	}

//	printf("RTC calibrated to %d Hz\r\n", rtc_freq);
}


/* Delay for the specified number of milliseconds by polling RTC */
void rtc_delay_ms(uint32_t msec)
{
	uint32_t start;

	start = CRM->RTC_COUNT;
	while ((CRM->RTC_COUNT - start) < ((msec * rtc_freq) / 1000))
		continue;
}
