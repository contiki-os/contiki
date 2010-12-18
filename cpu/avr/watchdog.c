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
 * @(#)$Id: watchdog.c,v 1.3 2010/12/18 20:51:11 dak664 Exp $
 */

 /* Dummy watchdog routines for the Raven 1284p */
#include "dev/watchdog.h"
#include <avr/wdt.h>
#include <avr/interrupt.h>

static int stopped = 0;

/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
	MCUSR&=~(1<<WDRF);
	stopped = 0;
	watchdog_stop();
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
	stopped--;
//	if(!stopped)
		wdt_enable(WDTO_2S);
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
//	if(!stopped)
		wdt_reset();
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
//	stopped++;
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
