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
 *         Defaults to Timer3 for those ATMEGAs that have it.
 *         If Timer3 not present Timer1 will be used.
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

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega644__)
#define TIMSK TIMSK1
#define TICIE1 ICIE1
#define TIFR TIFR1
#endif

/*---------------------------------------------------------------------------*/
#if defined(TCNT3) && RTIMER_ARCH_PRESCALER
ISR (TIMER3_COMPA_vect) {
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  ETIMSK &= ~((1 << OCIE3A) | (1 << OCIE3B) | (1 << TOIE3) |
      (1 << TICIE3) | (1 << OCIE3C));

  /* Call rtimer callback */
  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

#elif RTIMER_ARCH_PRESCALER
#warning "No Timer3 in rtimer-arch.c - using Timer1 instead"
ISR (TIMER1_COMPA_vect) {
  TIMSK &= ~((1<<TICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));

  rtimer_run_next();
}

#endif
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
#if RTIMER_ARCH_PRESCALER
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

#if RTIMER_ARCH_PRESCALER==1024
  TCCR3B |= 5;
#elif RTIMER_ARCH_PRESCALER==256
  TCCR3B |= 4;
#elif RTIMER_ARCH_PRESCALER==64
  TCCR3B |= 3;
#elif RTIMER_ARCH_PRESCALER==8
  TCCR3B |= 2;
#elif RTIMER_ARCH_PRESCALER==1
  TCCR3B |= 1;
#else
#error Timer3 PRESCALER factor not supported.
#endif

#elif RTIMER_ARCH_PRESCALER
  /* Leave timer1 alone if PRESCALER set to zero */
  /* Obviously you can not then use rtimers */

  TIMSK &= ~((1<<TICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));
  TIFR |= (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1);

  /* Default timer behaviour */
  TCCR1A = 0;
  TCCR1B = 0;

  /* Reset counter */
  TCNT1 = 0;

  /* Start clock */
#if RTIMER_ARCH_PRESCALER==1024
  TCCR1B |= 5;
#elif RTIMER_ARCH_PRESCALER==256
  TCCR1B |= 4;
#elif RTIMER_ARCH_PRESCALER==64
  TCCR1B |= 3;
#elif RTIMER_ARCH_PRESCALER==8
  TCCR1B |= 2;
#elif RTIMER_ARCH_PRESCALER==1
  TCCR1B |= 1;
#else
#error Timer1 PRESCALER factor not supported.
#endif

#endif /* TCNT3 */

  /* Restore interrupt state */
  SREG = sreg;
#endif /* RTIMER_ARCH_PRESCALER */
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
#if RTIMER_ARCH_PRESCALER
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

#elif RTIMER_ARCH_PRESCALER
  /* Set compare register */
  OCR1A = t;
  TIFR |= (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1);
  TIMSK |= (1 << OCIE1A);

#endif

  /* Restore interrupt state */
  SREG = sreg;
#endif /* RTIMER_ARCH_PRESCALER */
}
