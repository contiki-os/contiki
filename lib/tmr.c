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
#include "tmr.h"

/* Initialize timer.  This just counts and interrupts, doesn't drive an output.
   timer_num = 0, 1, 2, 3
   rate = desired rate in Hz,
   enable_int = whether to enable an interrupt on every cycle
   Returns actual timer rate. */
uint32_t timer_setup_ex(int timer_num, uint32_t rate, int enable_int)
{
	uint32_t actual_rate;
	volatile struct TMR_struct *timer = TMR_ADDR(timer_num);
	int log_divisor = 0;
	uint32_t period;
	
	/* Turn timer off */
	TMR0->ENBL &= ~(1 << timer_num);

	/* Calculate optimal rate */
	for (log_divisor = 0; log_divisor < 8; log_divisor++)
	{
		int denom = (rate * (1 << log_divisor));
		period = (REF_OSC + denom/2) / denom;
		if (period <= 65535)
			break;
	}
	if (log_divisor >= 8)
	{
		period = 65535;
		log_divisor = 7;
	}

	if (period < 2) period = 2;

	actual_rate = REF_OSC / (period * (1 << log_divisor));

	/* Set up timer */
	
	timer->LOAD = 0;
	timer->CMPLD1 = (period - 1);
	timer->COMP1 = timer->CMPLD1;
	timer->CNTR = timer->LOAD;
	timer->SCTRL = 0;
	timer->CSCTRLbits = (struct TMR_CSCTRL) {
		.CL1 = 0x01,	// Reload COMP1 when COMP1 matches
	};
	timer->CTRLbits = (struct TMR_CTRL) {
		.COUNT_MODE = 1,		// Count rising edge of primary source
		.PRIMARY_CNT_SOURCE = 8 + log_divisor,	// Peripheral clock divided by (divisor)
		.LENGTH = 1,			// At compare, reset to LOAD
	};

	TMR0->ENBL |= (1 << timer_num);

	if (enable_int) {
		enable_irq(TMR);
		timer->SCTRLbits.TCFIE = 1;
	}

	return actual_rate;
}
