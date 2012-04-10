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

/* Track flow through rtimer interrupts*/
#if DEBUGFLOWSIZE&&0
extern uint8_t debugflowsize,debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if (debugflowsize<(DEBUGFLOWSIZE-1)) debugflow[debugflowsize++]=c
#else
#define DEBUGFLOW(c)
#endif

/*---------------------------------------------------------------------------*/
#if defined(TCNT3) && RTIMER_ARCH_PRESCALER
ISR (TIMER3_COMPA_vect) {
  DEBUGFLOW('/');
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* Disable rtimer interrupts */
  ETIMSK &= ~((1 << OCIE3A) | (1 << OCIE3B) | (1 << TOIE3) |
      (1 << TICIE3) | (1 << OCIE3C));

#if RTIMER_CONF_NESTED_INTERRUPTS
  /* Enable nested interrupts. Allows radio interrupt during rtimer interrupt. */
  /* All interrupts are enabled including recursive rtimer, so use with caution */
  sei();
#endif

  /* Call rtimer callback */
  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  DEBUGFLOW('\\');
}

#elif RTIMER_ARCH_PRESCALER
#warning "No Timer3 in rtimer-arch.c - using Timer1 instead"
ISR (TIMER1_COMPA_vect) {
  DEBUGFLOW('/');
  TIMSK &= ~((1<<TICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));

  rtimer_run_next();
  DEBUGFLOW('\\');
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
  DEBUGFLOW(':');
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

#if RDC_CONF_MCU_SLEEP
/*---------------------------------------------------------------------------*/
void
rtimer_arch_sleep(rtimer_clock_t howlong)
{
/* Deep Sleep for howlong rtimer ticks. This will stop all timers except
 * for TIMER2 which can be clocked using an external crystal.
 * Unfortunately this is an 8 bit timer; a lower prescaler gives higher
 * precision but smaller maximum sleep time.
 * Here a maximum 128msec (contikimac 8Hz channel check sleep) is assumed.
 * The rtimer and system clocks are adjusted to reflect the sleep time.
 */
#include <avr/sleep.h>
#include <dev/watchdog.h>
uint32_t longhowlong;
#if AVR_CONF_USE32KCRYSTAL
/* Save TIMER2 configuration if clock.c is using it */
    uint8_t savedTCNT2=TCNT2, savedTCCR2A=TCCR2A, savedTCCR2B = TCCR2B, savedOCR2A = OCR2A;
#endif
    cli();
	watchdog_stop();
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);

/* Set TIMER2 clock asynchronus from external source, CTC mode */
    ASSR |= (1 << AS2);
    TCCR2A =(1<<WGM21);
/* Set prescaler and TIMER2 output compare register */
#if 0    //Prescale by 1024 -   32 ticks/sec, 8 seconds max sleep
    TCCR2B =((1<<CS22)|(1<<CS21)|(1<<CS20));
	longhowlong=howlong*32UL; 
#elif 0  // Prescale by 256 -  128 ticks/sec, 2 seconds max sleep
	TCCR2B =((1<<CS22)|(1<<CS21)|(0<<CS20));
	longhowlong=howlong*128UL;
#elif 0  // Prescale by 128 -  256 ticks/sec, 1 seconds max sleep
	TCCR2B =((1<<CS22)|(0<<CS21)|(1<<CS20));
	longhowlong=howlong*256UL;
#elif 0  // Prescale by  64 -  512 ticks/sec, 500 msec max sleep
	TCCR2B =((1<<CS22)|(0<<CS21)|(0<<CS20));
	longhowlong=howlong*512UL;
#elif 1  // Prescale by  32 - 1024 ticks/sec, 250 msec max sleep
	TCCR2B =((0<<CS22)|(1<<CS21)|(1<<CS20));
	longhowlong=howlong*1024UL;
#elif 0  // Prescale by   8 - 4096 ticks/sec, 62.5 msec max sleep
	TCCR2B =((0<<CS22)|(1<<CS21)|(0<<CS20));
	longhowlong=howlong*4096UL;
#else    // No Prescale -    32768 ticks/sec, 7.8 msec max sleep
	TCCR2B =((0<<CS22)|(0<<CS21)|(1<<CS20));
	longhowlong=howlong*32768UL;
#endif
	OCR2A = longhowlong/RTIMER_ARCH_SECOND;

/* Reset timer count, wait for the write (which assures TCCR2x and OCR2A are finished) */
    TCNT2 = 0; 
    while(ASSR & (1 << TCN2UB));

/* Enable TIMER2 output compare interrupt, sleep mode and sleep */
    TIMSK2 |= (1 << OCIE2A);
    SMCR |= (1 <<  SE);
	sei();
	ENERGEST_OFF(ENERGEST_TYPE_CPU);
	if (OCR2A) sleep_mode();
	  //...zzZZZzz...Ding!//

/* Disable sleep mode after wakeup, so random code cant trigger sleep */
    SMCR  &= ~(1 << SE);

/* Adjust rtimer ticks if rtimer is enabled. TIMER3 is preferred, else TIMER1 */
#if RTIMER_ARCH_PRESCALER
#ifdef TCNT3
    TCNT3 += howlong;
#else
    TCNT1 += howlong;
#endif
#endif
	ENERGEST_ON(ENERGEST_TYPE_CPU);

#if AVR_CONF_USE32KCRYSTAL
/* Restore clock.c configuration */
    cli();
    TCCR2A = savedTCCR2A;
    TCCR2B = savedTCCR2B;
    OCR2A  = savedOCR2A;
    TCNT2  = savedTCNT2;
    sei();
#else
/* Disable TIMER2 interrupt */
    TIMSK2 &= ~(1 << OCIE2A);
#endif
    watchdog_start();

/* Adjust clock.c for the time spent sleeping */
	longhowlong=CLOCK_CONF_SECOND;
	longhowlong*=howlong;
    clock_adjust_ticks(longhowlong/RTIMER_ARCH_SECOND);

}
#if !AVR_CONF_USE32KCRYSTAL
/*---------------------------------------------------------------------------*/
/* TIMER2 Interrupt service */

ISR(TIMER2_COMPA_vect)
{
//    TIMSK2 &= ~(1 << OCIE2A);       //Just one interrupt needed for waking
}
#endif /* !AVR_CONF_USE32KCRYSTAL */
#endif /* RDC_CONF_MCU_SLEEP */

