/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: print-stats.c,v 1.5 2010/03/09 14:14:36 nvt-se Exp $
 */

/**
 * \file
 *         Convenience function for printing system statistics
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "lib/print-stats.h"

#include "net/rime.h"
#include "sys/energest.h"

#include <stdio.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTA(...) printf(__VA_ARGS__)
#endif

/*---------------------------------------------------------------------------*/
void
print_stats(void)
{
  PRINTA("S %d.%d clock %lu tx %lu rx %lu rtx %lu rrx %lu rexmit %lu acktx %lu noacktx %lu ackrx %lu timedout %lu badackrx %lu toolong %lu tooshort %lu badsynch %lu badcrc %lu contentiondrop %lu sendingdrop %lu lltx %lu llrx %lu\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 clock_seconds(),
	 rimestats.tx, rimestats.rx,
	 rimestats.reliabletx, rimestats.reliablerx,
	 rimestats.rexmit, rimestats.acktx, rimestats.noacktx,
	 rimestats.ackrx, rimestats.timedout, rimestats.badackrx,
	 rimestats.toolong, rimestats.tooshort,
	 rimestats.badsynch, rimestats.badcrc,
	 rimestats.contentiondrop, rimestats.sendingdrop,
	 rimestats.lltx, rimestats.llrx);
#if ENERGEST_CONF_ON
  PRINTA("E %d.%d clock %lu cpu %lu lpm %lu irq %lu gled %lu yled %lu rled %lu tx %lu listen %lu sensors %lu serial %lu\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 clock_seconds(),
	 energest_total_time[ENERGEST_TYPE_CPU].current,
	 energest_total_time[ENERGEST_TYPE_LPM].current,
	 energest_total_time[ENERGEST_TYPE_IRQ].current,
	 energest_total_time[ENERGEST_TYPE_LED_GREEN].current,
	 energest_total_time[ENERGEST_TYPE_LED_YELLOW].current,
	 energest_total_time[ENERGEST_TYPE_LED_RED].current,
	 energest_total_time[ENERGEST_TYPE_TRANSMIT].current,
	 energest_total_time[ENERGEST_TYPE_LISTEN].current,
	 energest_total_time[ENERGEST_TYPE_SENSORS].current,
	 energest_total_time[ENERGEST_TYPE_SERIAL].current);
#endif /* ENERGEST_CONF_ON */
}
/*---------------------------------------------------------------------------*/
