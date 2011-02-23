/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
 *
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
 * This file is part of the Contiki OS.
 *
 *
 */

#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include "dev/leds.h"

#include "contiki-conf.h"
#include "mc1322x.h"

#include "contiki-conf.h"

#define MAX_TICKS (~((clock_time_t)0) / 2)

static volatile clock_time_t current_clock = 0;

volatile unsigned long seconds = 0;

#define TCF  15
#define TCF1 4
#define TCF2 5

void
clock_init()
{
	/* timer setup */
	/* CTRL */
#define COUNT_MODE 1      /* use rising edge of primary source */
#define PRIME_SRC  0xf    /* Perip. clock with 128 prescale (for 24Mhz = 187500Hz)*/
#define SEC_SRC    0      /* don't need this */
#define ONCE       0      /* keep counting */
#define LEN        1      /* count until compare then reload with value in LOAD */
#define DIR        0      /* count up */
#define CO_INIT    0      /* other counters cannot force a re-initialization of this counter */
#define OUT_MODE   0      /* OFLAG is asserted while counter is active */

	*TMR_ENBL = 0;                     /* tmrs reset to enabled */
	*TMR0_SCTRL = 0;
	*TMR0_CSCTRL =0x0040;
	*TMR0_LOAD = 0;                    /* reload to zero */
	*TMR0_COMP_UP = 1875;             /* trigger a reload at the end */
	*TMR0_CMPLD1 = 1875;              /* compare 1 triggered reload level, 10HZ maybe? */
	*TMR0_CNTR = 0;                    /* reset count register */
	*TMR0_CTRL = (COUNT_MODE<<13) | (PRIME_SRC<<9) | (SEC_SRC<<7) | (ONCE<<6) | (LEN<<5) | (DIR<<4) | (CO_INIT<<3) | (OUT_MODE);
	*TMR_ENBL = 0xf;                   /* enable all the timers --- why not? */

	enable_irq(TMR);

}

void tmr0_isr(void) {
	if(bit_is_set(*TMR(0,CSCTRL),TCF1)) {
		current_clock++;
		if((current_clock % CLOCK_CONF_SECOND) == 0) {
			seconds++;
#if BLINK_SECONDS
			leds_toggle(LEDS_GREEN);
#endif
		}
		
		if(etimer_pending() &&
		   (etimer_next_expiration_time() - current_clock - 1) > MAX_TICKS) {
 			etimer_request_poll();
 		}


		/* clear the compare flags */
		clear_bit(*TMR(0,SCTRL),TCF);                
		clear_bit(*TMR(0,CSCTRL),TCF1);                
		clear_bit(*TMR(0,CSCTRL),TCF2);                
		return;
	} else {
		/* this timer didn't create an interrupt condition */
		return;
	}
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

unsigned long
clock_seconds(void)
{
	return seconds;
}

/* clock delay from cc2430 */
/* I don't see any documentation about how this routine is suppose to behave */
void
clock_delay(unsigned int len)
{
  unsigned int i;
  for(i = 0; i< len; i++) {
	  asm("nop");
  }
}
