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
 *         Implementation of the Contiki real-time module rt for TI CC32xx
 * \author
 *         Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#include "sys/rtimer.h"
#include "rtimer-arch.h"

static volatile rtimer_clock_t rtimer_arch_wakeup_time;

/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
	// Set initial time
	rtimer_arch_wakeup_time = rtimer_arch_now();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_request_poll(void)
{
	// Run next timer
	rtimer_run_next();
}
/*---------------------------------------------------------------------------*/
int
rtimer_arch_pending(void)
{
	// Check if timer is expired
	if (rtimer_arch_now() >= rtimer_arch_wakeup_time)
	{
		return 1;
	}

	return 0;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
	// Save next wake up time
	rtimer_arch_wakeup_time = t;
}
/*---------------------------------------------------------------------------*/
