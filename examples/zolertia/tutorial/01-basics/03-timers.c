/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         An example showing how to use the on-board LED and user button
 * \author
 *         Antonio Lignan <alinan@zolertia.com> <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
/* This is the main contiki header, it should be included always */
#include "contiki.h"

/* This is the standard input/output C library, used to print information to the
 * console, amongst other features
 */
#include <stdio.h>

/* And this includes the on-board LEDs */
#include "dev/leds.h"

/* The event timer library */
#include "sys/etimer.h"

/* The seconds timer library */
#include "sys/stimer.h"

/* The regular timer library */
#include "sys/timer.h"

/* The callback timer library */
#include "sys/ctimer.h"

/* The "real-time" timer library */
#include "sys/rtimer.h"
/*---------------------------------------------------------------------------*/
/* The following are the structures used when you need to include a timer, it
 * serves to keep the timer information.
 */
static struct timer  nt;
static struct stimer st;
static struct etimer et;
static struct ctimer ct;
static struct rtimer rt;
/*---------------------------------------------------------------------------*/
/* We first declare the name or our process, and create a friendly printable
 * name
 */
PROCESS(test_timers_process, "Timers example process");

/* And we tell the system to start this process as soon as the device is done
 * initializing
 */
AUTOSTART_PROCESSES(&test_timers_process);
/*---------------------------------------------------------------------------*/
/* This is an example of functions called by the rtimer real-time timer.  Notice
 * the argument of the function is "void *ptr", meaning you can pass any type
 * of data as pointer
 */
static void
rtimer_callback_example(struct rtimer *timer, void *ptr)
{
  uint32_t *rtimer_ticks = ptr;
  printf("rtimer, now: \t%ld\n", *rtimer_ticks);

  /* We can restart the ctimer and keep the counting going */
  (*rtimer_ticks)++;
  ctimer_restart(&ct);
}
/*---------------------------------------------------------------------------*/
/* This is an example of functions called by the ctimer callback timer.  Notice
 * the argument of the function is "void *ptr", meaning you can pass any type
 * of data as pointer
 */
static void
ctimer_callback_example(void *ptr)
{
  uint32_t *ctimer_ticks = ptr;
  printf("ctimer, now: \t%ld\n", *ctimer_ticks);

  /* The real timer allows execution of real-time tasks (with predictable
   * execution times).
   * The function RTIMER_NOW() is used to get the current system time in ticks
   * and RTIMER_SECOND specifies the number of ticks per second.
   */

  (*ctimer_ticks)++;
  rtimer_set(&rt, RTIMER_NOW() + RTIMER_SECOND, 0,
             rtimer_callback_example, ctimer_ticks);
}
/*---------------------------------------------------------------------------*/
/* We declared the process in the above step, now we are implementing the
 * process.  A process communicates with other processes and, thats why we
 * include as arguments "ev" (event type) and "data" (information associated
 * with the event, so when we receive a message from another process, we can
 * check which type of event is, and depending of the type what data should I
 * receive
 */
PROCESS_THREAD(test_timers_process, ev, data)
{
  /* Every process start with this macro, we tell the system this is the start 
   * of the thread
   */
  PROCESS_BEGIN();

  /* Variables inside a thread should be declared as static */
  static uint32_t ticks = 0;

  /* This is the most basic timer, it doesn't trigger any expiration event so
   * you need to check if it has expired.  The "CLOCK_SECOND" constant is
   * platform-dependant, and corresponds to the number of "ticks" in a second.
   * For periods less than a second, use this timer
   */
  timer_set(&nt, CLOCK_SECOND);

  /* To test the timer we increase the "ticks" variable each time the code
   * inside the loop, the "timer_expired(...)" function checks if the timer has
   * expired.
   */
  while(!timer_expired(&nt)) {
    ticks++;
  }

  printf("timer, ticks: \t%ld\n", ticks);

  /* The seconds timer already uses seconds as a time base, so there's no need
   * to use the "CLOCK_SECOND" constant.
   */
  stimer_set(&st, 1);

  ticks = 0;

  while(!stimer_expired(&st)) {
    ticks++;
  }

  printf("stimer, ticks: \t%ld\n", ticks);

  /* The etimer is the most used timer, as it triggers an event when it expires,
   * thus we can carry on without having to manually check for expiration, but
   * only wait for an event.  As learned in previous example, we can hold the
   * process until the event happens
   */
  ticks = 0;
  etimer_set(&et, CLOCK_SECOND * 2);

  ticks++;
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
  printf("etimer, now: \t%ld\n", ticks);

  /* The callback timer triggers a given function when the timer expires.  It
   * takes as parameters the ctimer structure, the time period, a function to
   * invoke when it expires, and alternatively a pointer to data
   */

  ticks++;
  ctimer_set(&ct, CLOCK_SECOND * 2, ctimer_callback_example, &ticks);  

  /* And we keep the process halt while we wait for the callback timer to
   * expire.
   */

  while(1) {
    PROCESS_YIELD();
  }

  /* This is the end of the process, we tell the system we are done.  Even if
   * we won't reach this due to the "while(...)" we need to include it
   */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
