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
 * @(#)$Id: clock.c,v 1.25 2010/04/04 12:29:50 adamdunkels Exp $
 */

#include "contiki-conf.h"
#include "sys/energest.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include "rtimer-arch.h"
#include "isr_compat.h"

#include "dev/leds.h"

#define INTERVAL (RTIMER_ARCH_SECOND / CLOCK_SECOND)

#define MAX_TICKS (~((clock_time_t)0) / 2)

static volatile unsigned long seconds;

static volatile clock_time_t count = 0;
/* last_tar is used for calculating clock_fine, last_ccr might be better? */
static unsigned short last_tar = 0;
/*---------------------------------------------------------------------------*/
ISR(TIMER1_A1, timera1)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(TA1IV == 2) {

    /* HW timer bug fix: Interrupt handler called before TR==CCR.
     * Occurrs when timer state is toggled between STOP and CONT. */
    while(TA1CTL & MC1 && TA1CCR1 - TA1R == 1);

    /* Make sure interrupt time is future */
    do {
      /*      TACTL &= ~MC1;*/
      TA1CCR1 += INTERVAL;
      /*      TACTL |= MC1;*/
      ++count;

      /* Make sure the CLOCK_CONF_SECOND is a power of two, to ensure
         that the modulo operation below becomes a logical and and not
         an expensive divide. Algorithm from Wikipedia:
         http://en.wikipedia.org/wiki/Power_of_two */
#if (CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND - 1)) != 0
#error CLOCK_CONF_SECOND must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change CLOCK_CONF_SECOND in contiki-conf.h.
#endif
      if(count % CLOCK_CONF_SECOND == 0) {
        ++seconds;
        energest_flush();
      }
    } while((TA1CCR1 - TA1R) > INTERVAL);

    last_tar = TA1R;

    if(etimer_pending() &&
       (etimer_next_expiration_time() - count - 1) > MAX_TICKS) {
      etimer_request_poll();
      LPM4_EXIT;
    }

  }
  /*  if(process_nevents() >= 0) {
    LPM4_EXIT;
    }*/

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  clock_time_t t1, t2;
  do {
    t1 = count;
    t2 = count;
  } while(t1 != t2);
  return t1;
}
/*---------------------------------------------------------------------------*/
void
clock_set(clock_time_t clock, clock_time_t fclock)
{
  TA1R = fclock;
  TA1CCR1 = fclock + INTERVAL;
  count = clock;
}
/*---------------------------------------------------------------------------*/
int
clock_fine_max(void)
{
  return INTERVAL;
}
/*---------------------------------------------------------------------------*/
unsigned short
clock_fine(void)
{
  unsigned short t;
  /* Assign last_tar to local varible that can not be changed by interrupt */
  t = last_tar;
  /* perform calc based on t, TAR will not be changed during interrupt */
  return (unsigned short) (TA1R - t);
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  dint();

  /* Select SMCLK (2.4576MHz), clear TAR */
  /* TACTL = TASSEL1 | TACLR | ID_3; */

  /* Select ACLK 32768Hz clock, divide by 2 */
/*   TA1CTL = TASSEL0 | TACLR | ID_1; */

#if INTERVAL==32768/CLOCK_SECOND
  TA1CTL = TASSEL0 | TACLR;
#elif INTERVAL==16384/CLOCK_SECOND
  TA1CTL = TASSEL0 | TACLR | ID_1;
#else
#error NEED TO UPDATE clock.c to match interval!
#endif

  /* Initialize ccr1 to create the X ms interval. */
  /* CCR1 interrupt enabled, interrupt occurs when timer equals CCR1. */
  TA1CCTL1 = CCIE;

  /* Interrupt after X ms. */
  TA1CCR1 = INTERVAL;

  /* Start Timer_A in continuous mode. */
  TA1CTL |= MC1;

  count = 0;

  /* Enable interrupts. */
  eint();

}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 2.83 us.
 */
void
clock_delay(unsigned int i)
{
  /*
   * This means that delay(i) will delay the CPU for CONST + 3x
   * cycles. On a 2.4756 CPU, this means that each i adds 1.22us of
   * delay.
   *
   * do {
   *   --i;
   * } while(i > 0);
   */
#ifdef __IAR_SYSTEMS_ICC__
  asm("add #-1, r12");
  asm("jnz $-2");
#else
#ifdef __GNUC__
  asm("add #-1, r15");
  asm("jnz $-2");
#else
  do {
    asm("nop");
    --i;
  } while(i > 0);
#endif /* __GNUC__ */
#endif /* __IAR_SYSTEMS_ICC__ */
}
/*---------------------------------------------------------------------------*/
#ifdef __GNUC__
void
__delay_cycles(unsigned long c)
{
  c /= 4;
  asm("add #-1, r15");
  asm("jnz $-2");
}
#endif /* __GNUC__ */
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 10 ms.
 *
 */
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{

}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  unsigned long t1, t2;
  do {
    t1 = seconds;
    t2 = seconds;
  } while(t1 != t2);
  return t1;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
clock_counter(void)
{
  return TA1R;
}
/*---------------------------------------------------------------------------*/
