/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 *
 */

/**
 * \file
 *         Implementation of clock architecture for TI CC32xx.
 * \author
 *         Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#include "sys/clock.h"
#include "sys/etimer.h"

#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
#include "osi.h"
#else
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "systick.h"
#endif

#include "clock-arch.h"
#include "rtimer-arch.h"

static volatile clock_time_t clock_arch_tick_count;

/*---------------------------------------------------------------------------*/
void
clock_arch_init(void)
{
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
	// Create task for clock timer tick
	osi_TaskCreate(clock_arch_tick_task, (const signed char * const)"ContikiClock", CLOCK_ARCH_TICKTASK_STACKSIZE, NULL, CLOCK_ARCH_TICKTASK_PRIORITY, NULL);
#else
	// Set SysTick period
	SysTickPeriodSet(CLOCK_ARCH_PRELOAD);

	// Register SysTick ISR
	SysTickIntRegister(&clock_arch_isr);

	// Enable the SysTick Interrupt
	SysTickIntEnable();

	// Enable SysTick
	SysTickEnable();
#endif
}
/*---------------------------------------------------------------------------*/
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
void
clock_arch_tick_task(void *pv_parameters)
{
	// Loop for ever
	while(1)
	{
		clock_arch_update();

		// Thread sleeps for timer tick interval
		osi_Sleep(CLOCK_ARCH_TICK_MS);
	}
}

#else
void
clock_arch_isr(void)
{
	clock_arch_update();
}
#endif

void clock_arch_update(void)
{
	// Increment ticks
	clock_arch_tick_count++;

	// Check if rtimer is pending
	if (rtimer_arch_pending())
	{
		// Proceed rtimer module
		rtimer_arch_request_poll();
	}

	// Check if one clock tick needs increment
	if (clock_arch_tick_count % RTIMER_TO_CLOCK_SECOND == 0)
	{
		// Check if etimer events are pending
		if (etimer_pending())
		{
			// Proceed etimer module
			etimer_request_poll();
		}
	}
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_arch_get_tick_count(void)
{
	return clock_arch_tick_count;
}
/*---------------------------------------------------------------------------*/
void
clock_arch_set_tick_count(clock_time_t t)
{
	clock_arch_tick_count = t;
}
/*---------------------------------------------------------------------------*/
