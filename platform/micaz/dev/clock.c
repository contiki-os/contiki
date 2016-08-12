/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 * @(#)$$
 */

#include "sys/clock.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile clock_time_t count, scount;
static volatile unsigned long seconds;

/*---------------------------------------------------------------------------*/
ISR(TIMER0_COMP_vect)
{
  count++;
  if(++scount == CLOCK_SECOND) {
    scount = 0;
    seconds++;
  }
  if(etimer_pending()) {
    etimer_request_poll();
  }
}

/*---------------------------------------------------------------------------*/
void 
clock_init(void)
{
  /* Disable interrupts*/
  cli();

  /* Disable compare match interrupts and overflow interrupts. */
  TIMSK &= ~( _BV(TOIE0) | _BV(OCIE0) );

  /** 
   * set Timer/Counter0 to be asynchronous 
   * from the CPU clock with a second external 
   * clock(32,768kHz) driving it.
   */
  ASSR |= _BV(AS0);

  /*
   * Set timer control register:
   * - prescale: 32 (CS00 and CS01)
   * - counter reset via comparison register (WGM01)
   */
  TCCR0 = _BV(CS00) | _BV(CS01) | _BV(WGM01);

  /* Set counter to zero */
  TCNT0 = 0;

  /*
   * 128 clock ticks per second.
   * 32,768 = 32 * 8 * 128
   */
  OCR0 = 8;

  /* Clear interrupt flag register */
  TIFR = 0x00;

  /**
   * Wait for TCN0UB, OCR0UB, and TCR0UB.
   *
   */
  while(ASSR & 0x07);

  /* Raise interrupt when value in OCR0 is reached. */
  TIMSK |= _BV(OCIE0);

  count = 0;

  /* enable all interrupts*/
  sei();

}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  clock_time_t tmp;
  do {
    tmp = count;
  } while(tmp != count);
  return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of TODO
 */
void
clock_delay(unsigned int i)
{
  for (; i > 0; i--) {		/* Needs fixing XXX */
    unsigned j;
    for (j = 50; j > 0; j--)
      asm volatile("nop");
  }
}

/*---------------------------------------------------------------------------*/
/*
 * Wait for a multiple of 1 / 128 sec = 7.8125 ms.
 *
 */
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < i);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
    // TODO
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  unsigned long tmp;
  do {
    tmp = seconds;
  } while(tmp != seconds);
  return tmp;
}
/*---------------------------------------------------------------------------*/
