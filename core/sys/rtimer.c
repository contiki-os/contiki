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
 * @(#)$Id: rtimer.c,v 1.1 2007/03/25 17:10:30 adamdunkels Exp $
 */

#include "sys/rtimer.h"
#include "contiki.h"

#define LIST_SIZE 16

static struct rtimer *rtimers[LIST_SIZE];
static u8_t next, firstempty;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
rtimer_init(void)
{
  next = 0;
  firstempty = 0;
  rtimer_arch_init();
}
/*---------------------------------------------------------------------------*/
int
rtimer_set(struct rtimer *rtimer, rt_clock_t time, rt_clock_t duration,
	   void (* func)(struct rtimer *t, void *ptr), void *ptr)
{
  int i;

  PRINTF("rtimer_set time %d\n", time);
  
  /* Check if rtimer queue is full. */
  if(firstempty == (next - 1) % LIST_SIZE) {
    PRINTF("rtimer_set: next %d firstempty %d full\n", next, firstempty);
    return RTIMER_ERR_FULL;
  }
  
  /* Check if it is possible to run this rtimer at the requested
     time. */
  for(i = next; i != firstempty;
      i = (i + 1) % LIST_SIZE) {

    /* XXX: should check a range of time not just the same precise
       moment. */
    if(rtimers[i]->time == time) {
      PRINTF("rtimer_set: next %d firstempty %d time %d == %d\n",
	     next, firstempty, rtimers[i]->time, time);
      return RTIMER_ERR_TIME;
    }
  }
  /* Put the rtimer at the end of the rtimer list. */
  rtimer->time = time;
  rtimers[firstempty] = rtimer;
  PRINTF("rt_post: putting rtimer %s as %d\n", rtimer->name, firstempty);

  firstempty = (firstempty + 1) % LIST_SIZE;

  /*  PRINTF("rt_post: next %d firstempty %d scheduling soon\n",
      next, firstempty);*/

  /* Check if this is the first rtimer on the list. If so, we need to
     run the rt_arch_schedule() function to get the ball rolling. */
  if(firstempty == (next + 1) % LIST_SIZE) {
    
    PRINTF("rtimer_set scheduling %d %s (%d)\n",
	 next, rtimers[next]->name, rtimers[next]->time);
    rtimer
  }
  
  return RTIMER_OK;
}
/*---------------------------------------------------------------------------*/
void
rtimer_run_next(void)
{
  int i, n;
  struct rtimer *t;

  t = rtimers[next];

  /* Increase the pointer to the next rtimer. */
  next = (next + 1) % LIST_SIZE;
  
  /* Run the rtimer. */
  PRINTF("rtimer_run_next running %s\n", t->name);
  t->func(t, t->ptr);

  if(next == firstempty) {
    PRINTF("rtimer_run_next: empty rtimer list\n");
    /* The list is empty, no more rtimers to schedule. */
    return;
  }

  /* Find the next rtimer to run. */
  n = next;
  for(i = next; i != firstempty; i = (i + 1) % LIST_SIZE) {
    PRINTF("rtimer_run_next checking %s (%d) against %s (%d)\n",
	   rtimers[i]->name, rtimers[i]->time,
	   rtimers[n]->name, rtimers[n]->time);
    if(RT_CLOCK_LT(rtimers[i]->time, rtimers[n]->time)) {
      n = i;
    }
  }

  PRINTF("rtimer_run_next next rtimer is %d %s (%d)\n",
	 n, rtimers[n]->name, rtimers[n]->time);
  
  /* Put the next rtimer first in the rtimer list. */
  t = rtimers[next];
  rtimers[next] = rtimers[n];
  rtimers[n] = t;

  PRINTF("rtimer_run_next scheduling %d %s (%d)\n",
	 next, rtimers[next]->name, rtimers[next]->time);

  rtimer_arch_schedule(rtimers[next]->time);
}
/*---------------------------------------------------------------------------*/
