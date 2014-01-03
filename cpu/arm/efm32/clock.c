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


#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include <core_cm3.h>
#include <stdio.h>

#include "em_cmu.h"
#include "burtc.h"

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

void SysTick_Handler(void)
{
  current_clock++;
  if(etimer_pending() && etimer_next_expiration_time() <= current_clock) {
    etimer_request_poll();
    /* printf("%d,%d\n", clock_time(),etimer_next_expiration_time  	()); */
  }

  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }

}

#ifdef WITH_PM
/*---------------------------------------------------------------------------*/
int clock_suspend(void)
{
	// Disable systick
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return 0;
}

int clock_resume(void)
{
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return 0;
}

static pm_ops_struct clock_pm_ops = {
	.suspend = clock_suspend,
	.resume = clock_resume,
};
#endif

/*---------------------------------------------------------------------------*/

// ticks = 1ms
void
clock_init()
{
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    printf("ERROR: SysTick_Config failed\n");
    while(1);
  }

#ifdef WITH_PM
  power_register(&clock_pm_ops);
#endif
}

// Return numbers of ticks (ms)
clock_time_t
clock_time(void)
{
  return current_clock;
}

unsigned long
clock_seconds(void)
{
  return current_seconds;
}

void clock_set_seconds(unsigned long sec)
{
	current_seconds = sec;
}

/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 0.3us (at 14MHz)
 */
void
clock_delay(unsigned int delay)
{
  while(delay--) {
    __NOP();
  }
}

/**
 * Delay a given number of microseconds.
 * \param dt   How many microseconds to delay.
 *
 * \note Interrupts could increase the delay by a variable amount.
 */
void clock_delay_usec(uint16_t dt)
{
	  clock_delay(dt*3);
}

/**
 * Delay a given number of milliseconds.
 * \param dt   How many milliseconds to delay.
 *
 * \note Interrupts could increase the delay by a variable amount.
 */
void clock_delay_msec(uint16_t dt)
{
      clock_delay(dt*3000);
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 ms.
 */
void
clock_wait(clock_time_t delay)
{
	clock_time_t start;

	start = clock_time();
	while(clock_time() - start < (clock_time_t)delay)
	{
		// Wait for interrupt until next systick IRQ to save power
		//__WFI();
		__NOP();
	}
}


/*---------------------------------------------------------------------------*/
/**
 * Set time, e.g. to a standard epoch for an absolute date/time.
 * This will set bottime to value in seconds when the system was booted.
 * Boottime is later used by clock_get_time.
 */
void
clock_set_time(unsigned long sec)
{
	// TODO
	burtc_set_offset(sec);
//	/* Cancel adjustments and set the new time */
//	adjcompare = 0;
//	boottime = sec - seconds;
}
/*---------------------------------------------------------------------------*/
