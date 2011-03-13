/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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

/* Watchdog routines for the AVR */

/* Default timeout of 2 seconds is available on most MCUs */
#ifndef WATCHDOG_CONF_TIMEOUT
#define WATCHDOG_CONF_TIMEOUT WDTO_2S
//#define WATCHDOG_CONF_TIMEOUT WDTO_4S
#endif
 
 /* While balancing start and stop calls is a good idea, an imbalance will cause
  * resets that can take a lot of time to track down.
  * Some low power protocols may do this.
  * The default is no balance; define WATCHDOG_CONF_BALANCE 1 to override.
  */
#ifndef WATCHDOG_CONF_BALANCE
#define WATCHDOG_CONF_BALANCE 0
#endif

#include "dev/watchdog.h"
#include <avr/wdt.h>
#include <avr/interrupt.h>

#if WATCHDOG_CONF_BALANCE
static int stopped = 0;
#endif

/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
	MCUSR&=~(1<<WDRF);
#if WATCHDOG_CONF_BALANCE
	stopped = 0;
#endif
	watchdog_stop();
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
#if WATCHDOG_CONF_BALANCE
	stopped--;
	if(!stopped)
#endif
		wdt_enable(WATCHDOG_CONF_TIMEOUT);
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
#if WATCHDOG_CONF_BALANCE
	if(!stopped)
#endif
		wdt_reset();
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
#if WATCHDOG_CONF_BALANCE
	stopped++;
#endif
	wdt_disable();
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
	cli();
	wdt_enable(WDTO_15MS); //wd on,250ms 
	while(1); //loop
}
/*---------------------------------------------------------------------------*/
