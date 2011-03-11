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
#include "pwm.h"

static struct {
	uint32_t period;
	uint32_t guard;
	uint32_t pad_forced;
} pwm_info[4];

static inline void pad_set_output(int timer_num) { // set to output (when in GPIO mode)
	switch (timer_num) {
	case 0: GPIO->DATA_SEL.TMR0_PIN = 1; GPIO->PAD_DIR.TMR0_PIN = 1; break;
	case 1: GPIO->DATA_SEL.TMR1_PIN = 1; GPIO->PAD_DIR.TMR1_PIN = 1; break;
	case 2: GPIO->DATA_SEL.TMR2_PIN = 1; GPIO->PAD_DIR.TMR2_PIN = 1; break;
	case 3: GPIO->DATA_SEL.TMR3_PIN = 1; GPIO->PAD_DIR.TMR3_PIN = 1; break;
	default: break;
	}
}

static inline void pad_set_zero(int timer_num) { // set to zero in GPIO mode
	switch (timer_num) {
	case 0: GPIO->DATA_RESET.TMR0_PIN = 1; GPIO->FUNC_SEL.TMR0_PIN = 0; break;
	case 1: GPIO->DATA_RESET.TMR1_PIN = 1; GPIO->FUNC_SEL.TMR1_PIN = 0; break;
	case 2: GPIO->DATA_RESET.TMR2_PIN = 1; GPIO->FUNC_SEL.TMR2_PIN = 0; break;
	case 3: GPIO->DATA_RESET.TMR3_PIN = 1; GPIO->FUNC_SEL.TMR3_PIN = 0; break;
	default: break;
	}
}

static inline void pad_set_one(int timer_num) { // set to one in GPIO mode
	switch (timer_num) {
	case 0: GPIO->DATA_SET.TMR0_PIN = 1; GPIO->FUNC_SEL.TMR0_PIN = 0; break;
	case 1: GPIO->DATA_SET.TMR1_PIN = 1; GPIO->FUNC_SEL.TMR1_PIN = 0; break;
	case 2: GPIO->DATA_SET.TMR2_PIN = 1; GPIO->FUNC_SEL.TMR2_PIN = 0; break;
	case 3: GPIO->DATA_SET.TMR3_PIN = 1; GPIO->FUNC_SEL.TMR3_PIN = 0; break;
	default: break;
	}
}

static inline void pad_set_normal(int timer_num) { // set to TMR OFLAG output
	switch (timer_num) {
	case 0: GPIO->FUNC_SEL.TMR0_PIN = 1; break;
	case 1: GPIO->FUNC_SEL.TMR1_PIN = 1; break;
	case 2: GPIO->FUNC_SEL.TMR2_PIN = 1; break;
	case 3: GPIO->FUNC_SEL.TMR3_PIN = 1; break;
	default: break;
	}
}

/* Initialize PWM output.
   timer_num = 0, 1, 2, 3
   rate = desired rate in Hz,
   duty = desired duty cycle.  0=always off, 65536=always on.
   enable_timer = whether to actually run the timer, versus just configuring it
   Returns actual PWM rate. */
uint32_t pwm_init_ex(int timer_num, uint32_t rate, uint32_t duty, int enable_timer)
{
	uint32_t actual_rate;
	volatile struct TMR_struct *timer = TMR_ADDR(timer_num);
	int log_divisor = 0;
	uint32_t period, guard;

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

	/* Guard value (for safely changing duty cycle) should be
	   about 32 CPU clocks.  Calculate how many timer counts that
	   is, based on prescaler */
	guard = 32 >> log_divisor;
	if (guard < 2) guard = 2;

	/* Period should be about 50% longer than guard */
	if (period < ((guard * 3) / 2))
		period = guard + 4;

	/* Store period, guard, actual rate */
	pwm_info[timer_num].period = period;
	pwm_info[timer_num].guard = guard;
	actual_rate = REF_OSC / (period * (1 << log_divisor));

	/* Set up timer */
	pwm_duty_ex(timer_num, duty); // sets CMPLD1, LOAD
	timer->SCTRLbits = (struct TMR_SCTRL) {
		.OEN = 1, 	// drive OFLAG
	};
	timer->CSCTRLbits = (struct TMR_CSCTRL) {
		.CL1 = 0x01,	// Reload COMP1 when COMP1 matches
	};
	timer->COMP1 = timer->CMPLD1;
	timer->CNTR = timer->LOAD;
	timer->CTRLbits = (struct TMR_CTRL) {
		.COUNT_MODE = 1,		// Count rising edge of primary source
		.PRIMARY_CNT_SOURCE = 8 + log_divisor,	// Peripheral clock divided by (divisor)
		.LENGTH = 1,			// At compare, reset to LOAD
		.OUTPUT_MODE = 6,		// Set on COMP1, clear on rollover
	};

	pad_set_output(timer_num);
	pad_set_normal(timer_num);

	if (enable_timer) {
		TMR0->ENBL |= (1 << timer_num);
	}

//	printf("pwm timer %d, addr %p, requested rate %d, actual rate: %d, period %d, guard %d, divisor %d\r\n",
//		  timer_num, timer, rate, actual_rate, period, guard, 1 << log_divisor);

	return actual_rate;
}

/* Change duty cycle.  Safe to call at any time.
   timer_num = 0, 1, 2, 3
   duty = desired duty cycle.  0=always off, 65536=always on.
*/
void pwm_duty_ex(int timer_num, uint32_t duty)
{
	uint16_t comp1, load;
	volatile struct TMR_struct *timer = TMR_ADDR(timer_num);
	uint32_t period = pwm_info[timer_num].period;

	duty = (duty * period + 32767) / 65536;

	/* We don't use the "variable PWM" mode described in the datasheet because
	   there's no way to reliably change the duty cycle without potentially
	   changing the period for one cycle, which will cause phase drifts.

	   Instead, we use the "Set on compare, clear on rollover" output mode:

	   waveform:    |_________|          |----------|
	   counter:     0       COMP1      LOAD        65535

	   The low portion of the wave is COMP1 cycles long.  The
	   compare changes the counter to LOAD, and so the high
	   portion is (65536 - LOAD) cycles long.

	   Now, we just have to make sure we're not about to hit COMP1
	   before we change LOAD and COMPLD1.  If (COMP1 - CNTR) is less
	   than GUARD cycles, we wait for it to reload before changing.
	*/

	if (duty == 0) {
		pad_set_zero(timer_num);
		pwm_info[timer_num].pad_forced = 1;
		return;
	}

	if (duty >= period) {
		pad_set_one(timer_num);
		pwm_info[timer_num].pad_forced = 1;
		return;
	}

	if (pwm_info[timer_num].pad_forced) {
		pad_set_normal(timer_num);
		pwm_info[timer_num].pad_forced = 0;
	}

	comp1 = (period - duty) - 1;
	load = (65536 - duty);

	/* Disable interrupts */
	uint32_t old_INTCNTL = ITC->INTCNTL;
	ITC->INTCNTL = 0;

	if (TMR0->ENBL & (1 << timer_num))
	{
		/* Timer is enabled, so use the careful approach.
		   Implemented in ASM so we can be sure of the cycle
		   count */
		uint32_t tmp1, tmp2;
		asm volatile (//".arm \n\t"
			      "1: \n\t"
			      "ldrh %[tmp1], %[comp] \n\t"		// load COMP1
			      "ldrh %[tmp2], %[count] \n\t"		// load CNTR
			      "sub %[tmp1], %[tmp1], %[tmp2] \n\t"	// subtract
			      "lsl %[tmp1], %[tmp1], #16 \n\t"		// clear high bits
			      "lsr %[tmp1], %[tmp1], #16 \n\t"
			      "cmp %[tmp1], %[guard] \n\t"		// compare to GUARD
			      "bls 1b \n\t"				// if less, goto 1

			      "strh %[ld1], %[cmpld] \n\t"		// store CMPLD1
			      "strh %[ld2], %[load] \n\t"		// store LOAD
			      : /* out */
				[tmp1] "=&l" (tmp1),
				[tmp2] "=&l" (tmp2),
				[cmpld] "=m" (timer->CMPLD1),
				[load] "=m" (timer->LOAD)
			      : /* in */
				[comp] "m" (timer->COMP1),
				[count] "m" (timer->CNTR),
				[ld1] "l" (comp1),
				[ld2] "l" (load),
				[guard] "l" (pwm_info[timer_num].guard)
			      : "memory"
		);
	} else {
		/* Just set it directly, timer isn't running */
		timer->CMPLD1 = comp1;
		timer->LOAD = load;
	}

	/* Re-enable interrupts */
	ITC->INTCNTL = old_INTCNTL;
}
