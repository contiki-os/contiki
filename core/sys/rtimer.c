/**
 * \addtogroup rt
 * @{
 */

/**
 * \file
 *         Implementation of the architecture-agnostic parts of the real-time timer module.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */


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
 */

#include <stdbool.h>

#include "sys/rtimer.h"
#include "contiki.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static struct rtimer *next_rtimer;
static volatile bool locked = 0;	/* timer list is locked */
static volatile bool deferred = 0;	/* run a timer after unlocking */


/*---------------------------------------------------------------------------*/
void
rtimer_init(void)
{
  rtimer_arch_init();
}
/*---------------------------------------------------------------------------*/
static void
schedule_locked(rtimer_clock_t time)
{
    rtimer_arch_schedule(time);

    /*
     * The behaviour of rtimer_arch_schedule when the time is in the past,
     * equal to the present, or changes from future to past/present while
     * rtimer_arch_schedule is running is undefined. In particular, no timer
     * interrupt may be generated in such cases, causing the timer to expire
     * in the distant future or possibly never at all.
     *
     * We therefore check whether the expiration time is still in the future
     * after rtimer_arch_schedule. If not, the timer may still be pending and
     * we have to process it "manually" (through run_deferred).
     *
     * Since schedule_locked is run while "locked" is set, we're guaranteed
     * that "deferred" will be checked later on.
     */
    if(!RTIMER_CLOCK_LT(RTIMER_NOW(), time)) {
      deferred = 1;
    }
}
/*---------------------------------------------------------------------------*/
static int
set_locked(struct rtimer *rtimer, rtimer_clock_t time,
	   rtimer_callback_t func, void *ptr)
{
  struct rtimer **anchor;

  /*
   * RTIMER_ERR_ALREADY_SCHEDULED in rtimer.h suggests we should fail if the
   * timer is already scheduled. However, the original implementation allows
   * timers to be rescheduled with impunity, so we maintain de facto
   * compatibility.
   */
  for(anchor = &next_rtimer; *anchor; anchor = &(*anchor)->next) {
    if(*anchor == rtimer) {
      *anchor = rtimer->next;
      break;
    }
  }
  rtimer->time = time;
  rtimer->func = func;
  rtimer->ptr = func;
  rtimer->cancel = 0;

  for(anchor = &next_rtimer; *anchor; anchor = &(*anchor)->next) {
    if(!RTIMER_CLOCK_LT((*anchor)->time, time)) {
      break;
    }
  }
  rtimer->next = *anchor;
  *anchor = rtimer;

  if(next_rtimer == rtimer) {
    schedule_locked(time);
  }
  return RTIMER_OK;
}
/*---------------------------------------------------------------------------*/
static void
next_timer_locked(void)
{
  rtimer_clock_t now = RTIMER_NOW();
  struct rtimer *t;
  bool need_sched = 0;

  while(next_rtimer && !RTIMER_CLOCK_LT(now, next_rtimer->time)) {
    t = next_rtimer;   
    next_rtimer = t->next;
    if(!t->cancel) {
      t->func(t, t->ptr);
      need_sched |= t != next_rtimer;
    }
  }
  if(next_rtimer && need_sched) {
    schedule_locked(next_rtimer->time);
  }
}
/*---------------------------------------------------------------------------*/
static void
run_deferred(void)
{
  while(deferred) {
    locked = 1;
    deferred = 0;
    next_timer_locked();
    locked = 0;
  }
}
/*---------------------------------------------------------------------------*/
int
rtimer_set(struct rtimer *rtimer, rtimer_clock_t time,
	   rtimer_clock_t duration,
	   rtimer_callback_t func, void *ptr)
{
  int res;

  PRINTF("rtimer_set time %d\n", time);

  if(locked) {
    res = set_locked(rtimer, time, func, ptr);
  } else {
    locked = 1;
    res = set_locked(rtimer, time, func, ptr);
    locked = 0;
    run_deferred();
  }
  return res;
}
/*---------------------------------------------------------------------------*/
void
rtimer_cancel(struct rtimer *rtimer)
{
  rtimer->cancel = 1;
}
/*---------------------------------------------------------------------------*/
void
rtimer_run_next(void)
{
  if(locked) {
    deferred = 1;
    return;
  }

  /* lock just to make sure ... */
  locked = 1;
  next_timer_locked();
  locked = 0;
  run_deferred();
}
/*---------------------------------------------------------------------------*/
