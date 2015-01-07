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
 *         Clock implementation for TI CC32xx.
 * \author
 *         Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#include "sys/clock.h"
#include "clock-arch.h"

#include "rom.h"
#include "rom_map.h"
#include "utils.h"

/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
	// Call architecture specific clock initialize
	clock_arch_init();
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
	// Return architecture specific clock count
	return clock_arch_get_tick_count();
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
	// Return architecture specific clock seconds
	return (clock_arch_get_tick_count() / CLOCK_ARCH_TICK_COUNT);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
	// Update architecture specific clock seconds
	clock_arch_set_tick_count((clock_time_t)sec * CLOCK_ARCH_TICK_COUNT);
}
/*---------------------------------------------------------------------------*/
void
clock_wait(clock_time_t t)
{
	clock_time_t start;

	start = clock_time();
	while(clock_time() - start < (clock_time_t)t);
}
/*---------------------------------------------------------------------------*/
void clock_delay_usec(uint16_t dt)
{
	// Call delay from driver lib
	MAP_UtilsDelay(USEC_TO_LOOP(dt));
}
/*---------------------------------------------------------------------------*/
