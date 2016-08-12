/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include "contiki.h"
#include "sys/etimer.h"
#include "sys/stimer.h"
#include "sys/timer.h"
#include "sys/rtimer.h"

PROCESS(process1, "ETimer x Timer x STimer Process");
PROCESS(process2, "CTimer Process 2");
PROCESS(process3, "RTimer Process 3");
AUTOSTART_PROCESSES(&process1, &process2, &process3);

static int counter_etimer;
static int counter_timer;
static int counter_stimer;
static int counter_ctimer;
static int counter_rtimer;
static struct timer timer_timer;
static struct stimer timer_stimer;
static struct ctimer timer_ctimer;
static struct rtimer timer_rtimer;
static rtimer_clock_t timeout_rtimer = RTIMER_SECOND / 2;

void
do_timeout1()
{
  counter_etimer++;
  if(timer_expired(&timer_timer)) {
    counter_timer++;
  }

  if(stimer_expired(&timer_stimer)) {
    counter_stimer++;
  }

  printf("\nProcess 1: %s", counter_timer == counter_etimer
         && counter_timer == counter_stimer ? "SUCCESS" : "FAIL");
}
/*---------------------------------------------------------------------------*/
void
do_timeout2()
{
  ctimer_reset(&timer_ctimer);
  printf("\nProcess 2: CTimer callback called");
  counter_ctimer++;
}
/*---------------------------------------------------------------------------*/
void
do_timeout3(struct rtimer *timer, void *ptr)
{
  counter_rtimer++;

  printf("\nProcess 3: RTimer callback called");

  /* Re-arm rtimer */
  rtimer_set(&timer_rtimer, RTIMER_NOW() + timeout_rtimer, 0, do_timeout3,
             NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(process1, ev, data)
{
  static struct etimer timer_etimer;

  PROCESS_BEGIN();

  while(1) {
    timer_set(&timer_timer, 3 * CLOCK_SECOND);
    stimer_set(&timer_stimer, 3);
    etimer_set(&timer_etimer, 3 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    do_timeout1();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(process2, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    ctimer_set(&timer_ctimer, 5 * CLOCK_SECOND, do_timeout2, NULL);
    PROCESS_YIELD();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(process3, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    rtimer_set(&timer_rtimer, RTIMER_NOW() + timeout_rtimer, 0,
               do_timeout3, NULL);
    PROCESS_YIELD();
  }

  PROCESS_END();
}
