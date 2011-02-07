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
 * $Id: rtimer-arch.c,v 1.10 2010/02/28 21:29:19 dak664 Exp $
 */

/**
 * \file
 *         AVR-specific rtimer code
 *         Currently only works on ATMEGAs that have Timer 3.
 * \author
 *         Fredrik Osterlind <fros@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

/* OBS: 8 seconds maximum time! */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "rtimer-arch.h"

#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega1284P__)
//#error FTH081029 test timer 3
#define ETIMSK TIMSK3
#define ETIFR TIFR3
#define TICIE3 ICIE3

//Has no 'C', so we just set it to B. The code doesn't really use C so this
//is safe to do but lets it compile. Probably should enable the warning if
//it is ever used on other platforms.
//#warning no OCIE3C in timer3 architecture, hopefully it won't be needed!

#define OCIE3C	OCIE3B
#define OCF3C	OCF3B
#endif

#if defined(__AVR_AT90USB1287__) || defined(__AVR_ATmega128RFA1__) 
#define ETIMSK TIMSK3
#define ETIFR TIFR3
#define TICIE3 ICIE3
#endif
/*---------------------------------------------------------------------------*/
#ifdef TCNT3
ISR (TIMER3_COMPA_vect) {
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  ETIMSK &= ~((1 << OCIE3A) | (1 << OCIE3B) | (1 << TOIE3) |
      (1 << TICIE3) | (1 << OCIE3C));

  /* Call rtimer callback */
  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

#else
#error "No Timer3 in rtimer-arch.c"

#endif
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  /* Disable interrupts (store old state) */
  uint8_t sreg;
  sreg = SREG;
  cli ();

#ifdef TCNT3
  /* Disable all timer functions */
  ETIMSK &= ~((1 << OCIE3A) | (1 << OCIE3B) | (1 << TOIE3) |
      (1 << TICIE3) | (1 << OCIE3C));
  /* Write 1s to clear existing timer function flags */
  ETIFR |= (1 << ICF3) | (1 << OCF3A) | (1 << OCF3B) | (1 << TOV3) |
  (1 << OCF3C); 

  /* Default timer behaviour */
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3C = 0;

  /* Reset counter */
  TCNT3 = 0;

  /* Start clock, maximum prescaler */
  TCCR3B |= 5;

#else
#error "No Timer3 in rtimer-arch.c"

#endif

  /* Restore interrupt state */
  SREG = sreg;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  /* Disable interrupts (store old state) */
  uint8_t sreg;
  sreg = SREG;
  cli ();

#ifdef TCNT3
  /* Set compare register */
  OCR3A = t;
  /* Write 1s to clear all timer function flags */
  ETIFR |= (1 << ICF3) | (1 << OCF3A) | (1 << OCF3B) | (1 << TOV3) |
  (1 << OCF3C);
  /* Enable interrupt on OCR3A match */
  ETIMSK |= (1 << OCIE3A);

#else
#error "No Timer3 in rtimer-arch.c"

#endif

  /* Restore interrupt state */
  SREG = sreg;
}
