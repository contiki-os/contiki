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
 */

/**
 * \file
 *         MSP430-specific 32-bit rtimer code
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "sys/process.h"
#include "dev/watchdog.h"
#include "isr_compat.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

extern volatile unsigned long seconds;
static rtimer_clock_t currently_scheduled;
static void rtimer_arch_schedule_current();

/*---------------------------------------------------------------------------*/
ISR(TIMERA0, timera0)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  watchdog_start();

  if(currently_scheduled == 0) { /* Timer expired */
    rtimer_run_next();
  } else {
    rtimer_arch_schedule_current();
  }

  if(process_nevents() > 0) {
    LPM4_EXIT;
  }

  watchdog_stop();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  dint();

  /* CCR0 interrupt enabled, interrupt occurs when timer equals CCR0. */
  TACCTL0 = CCIE;

  /* Enable interrupts. */
  eint();
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{
  static rtimer_clock_t prev_32 = 0;
  rtimer_clock_t new = 0;

  /* Disable interrupts */
  spl_t s = splhigh();

  uint16_t t1, t2;
  do {
    t1 = TAR;
    t2 = TAR;
  } while(t1 != t2);

  new = (seconds & 0xfffffffe) * RTIMER_ARCH_SECOND + t1;

  if(new < prev_32) {
    splx(s);
    return new + 2 * RTIMER_ARCH_SECOND;
  }

  prev_32 = new;

  /* Re-enable interrupts if necessary */
  splx(s);

  return new;
}
/*---------------------------------------------------------------------------*/
static void
rtimer_arch_schedule_current()
{
  rtimer_clock_t now = RTIMER_NOW();

  if(currently_scheduled == 0) {
    return;
  }

  if(currently_scheduled < now) {
    rtimer_run_next();
    return;
  }

  if((currently_scheduled - now) & 0xffff0000) { /* Cannot be scheduled yet */
    TACCR0 = now + RTIMER_ARCH_SECOND;
  } else { /* Can be scheduled now */
    TACCR0 = currently_scheduled & 0xffff;
    currently_scheduled = 0;
  }

}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{

  PRINTF("rtimer_arch_schedule time %lu\n", t);

  /*if(currently_scheduled == 0)*/ {
    currently_scheduled = t;
  //  TACCR0 = t;
    rtimer_arch_schedule_current();
  }
}
/*---------------------------------------------------------------------------*/
