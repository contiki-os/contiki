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

/**
 * \file
 *         Implementation of the architecture-agnostic parts of the real-time timer module.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/**
 * \addtogroup rt
 * @{
 */

#include "sys/rtimer.h"
#include "contiki.h"

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static struct rtimer *volatile next_rtimer;
/*---------------------------------------------------------------------------*/
/*--------------------------MULTIPLE ACCESS RTIMER---------------------------*/
/*---------------------------------------------------------------------------*/
#if RTIMER_MULTIPLE_ACCESS
#include "sys/isr-control.h"
/*---------------------------------------------------------------------------*/
static void
schedule_rtimer_isr_safe(struct rtimer *rtimer)
{
  rtimer_clock_t min;
  if(rtimer) {
    min = RTIMER_NOW() + RTIMER_MINIMAL_SAFE_SCHEDULE;
    if(RTIMER_CLOCK_LT(rtimer->time, min)) {
      rtimer_arch_schedule(min);
    } else {
      rtimer_arch_schedule(rtimer->time);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
add_to_queue(struct rtimer *rtimer, rtimer_clock_t time,
             rtimer_callback_t func, void *ptr)
{
  int ret = RTIMER_OK;
  struct rtimer *iter;

  isr_state_t state = atomic_section_enter();

  if(rtimer->state != RTIMER_READY) {
    ret = RTIMER_ERR_ALREADY_SCHEDULED;
    goto finish;
  }

  rtimer->func = func;
  rtimer->ptr = ptr;
  rtimer->time = time;

  if(next_rtimer == NULL || RTIMER_CLOCK_LT(rtimer->time, next_rtimer->time)) {
    schedule_rtimer_isr_safe(rtimer);

    rtimer->next = next_rtimer;
    next_rtimer = rtimer;
    rtimer->state = RTIMER_QUEUED;
    goto finish;
  }

  iter = next_rtimer;
  while(iter->next != NULL) {
    if(!RTIMER_CLOCK_LT(iter->next->time, rtimer->time)) {
      break;
    }
    iter = iter->next;
  }

  rtimer->next = iter->next;
  iter->next = rtimer;
  rtimer->state = RTIMER_QUEUED;

finish:
  atomic_section_exit(state);

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
remove_from_queue(struct rtimer *rtimer)
{
  int ret = RTIMER_OK;
  struct rtimer *iter;

  isr_state_t state = atomic_section_enter();

  if(rtimer->state == RTIMER_READY) {
    ret = RTIMER_ERR_NOT_SCHEDULED;
  } else if(rtimer->state == RTIMER_RUNNING) {
    /* too late to cancel */
    ret = RTIMER_ERR_TOO_LATE;
  } else if(rtimer == next_rtimer) {
    next_rtimer = next_rtimer->next;
    schedule_rtimer_isr_safe(next_rtimer);
  } else {
    iter = next_rtimer;
    while(iter->next != NULL) {
      if(iter->next == rtimer) {
        iter->next = iter->next->next;
        break;
      }
      iter = iter->next;
    }
  }

  rtimer->state = RTIMER_READY;

  atomic_section_exit(state);

  return ret;
}
/*---------------------------------------------------------------------------*/
static struct rtimer *
pop_expired_from_queue(void)
{
  isr_state_t state;
  rtimer_clock_t now;

  struct rtimer *last = NULL;
  struct rtimer *ret = NULL;
  struct rtimer *iter;

  state = atomic_section_enter();

  now = RTIMER_NOW();

  iter = next_rtimer;

  while(iter != NULL) {
    if(RTIMER_CLOCK_LT(now, iter->time)) {
      break;
    }
    /* mark as running so that no-one can cancel */
    iter->state = RTIMER_RUNNING;
    last = iter;
    iter = iter->next;
  }

  if(last != NULL) {
    ret = next_rtimer;
    last->next = NULL;
    next_rtimer = iter;

    schedule_rtimer_isr_safe(next_rtimer);
  }

  atomic_section_exit(state);

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
func_call_list(volatile struct rtimer *list)
{
  struct rtimer *next;
  rtimer_callback_t cb;
  void *ptr;

  while(list != NULL) {

    next = list->next;
    cb = list->func;
    ptr = list->ptr;

    /* we've made list volatile because after setting to RTIMER_READY,
       other threads of execution are allowed to modify the rtimer. The
       volatile qualifier prvents reordering the above reads through the write
       to state. A memory barrier put here would be better, but we don't have
       this functionality in Contiki at this time. */
    list->state = RTIMER_READY;

    cb((struct rtimer *)list, ptr);

    list = next;
  }
}
#else
/*---------------------------------------------------------------------------*/
/*---------------------------FALLBACK DEFINITIONS----------------------------*/
/*---------------------------------------------------------------------------*/
#define add_to_queue(a, b, c, d) (0)
#define remove_from_queue(a)     (0)
#define pop_expired_from_queue() (NULL)
#define func_call_list(a)        {}
#endif
/*---------------------------------------------------------------------------*/
/*---------------------------SINGLE ACCESS RTIMER----------------------------*/
/*---------------------------------------------------------------------------*/
static int
add_single_access(struct rtimer *rtimer)
{
  int first = 0;

  if(next_rtimer == NULL) {
    first = 1;
  }

  next_rtimer = rtimer;

  if(first == 1) {
    rtimer_arch_schedule(rtimer->time);
  }

  return RTIMER_OK;
}
/*---------------------------------------------------------------------------*/
static int
remove_single_access(struct rtimer *rtimer)
{
  if(rtimer == next_rtimer) {
    next_rtimer = NULL;
    return RTIMER_OK;
  }

  return RTIMER_ERR_NOT_SCHEDULED;
}
/*---------------------------------------------------------------------------*/
static struct rtimer *
remove_expired_single_access(void)
{
  struct rtimer *t = next_rtimer;

  if(next_rtimer == NULL) {
    return NULL;
  }

  next_rtimer = NULL;

  return t;
}
/*---------------------------------------------------------------------------*/
static void
func_call_single(struct rtimer *t)
{
  if(t != NULL) {
    t->func(t, t->ptr);
  }
}
/*---------------------------------------------------------------------------*/
/*-----------------------------PUBLIC INTERFACE------------------------------*/
/*---------------------------------------------------------------------------*/
void
rtimer_init(void)
{
  if(RTIMER_MULTIPLE_ACCESS) {
    PRINTF("rtimer is multiple access\n");
  }
  rtimer_arch_init();
}
/*---------------------------------------------------------------------------*/
int
rtimer_cancel(struct rtimer *rtimer)
{
  if(RTIMER_MULTIPLE_ACCESS) {
    return remove_from_queue(rtimer);
  } else {
    return remove_single_access(rtimer);
  }
}
/*---------------------------------------------------------------------------*/
int
rtimer_set(struct rtimer *rtimer, rtimer_clock_t time,
           rtimer_clock_t duration, rtimer_callback_t func, void *ptr)
{
  PRINTF("rtimer_set time %lu\n", time);
  if(RTIMER_MULTIPLE_ACCESS) {
    return add_to_queue(rtimer, time, func, ptr);
  } else {
    rtimer->func = func;
    rtimer->ptr = ptr;
    rtimer->time = time;

    return add_single_access(rtimer);
  }
}
/*---------------------------------------------------------------------------*/
void
rtimer_run_next(void)
{
  struct rtimer *t;

  if(RTIMER_MULTIPLE_ACCESS) {
    t = pop_expired_from_queue();
    func_call_list(t);
  } else {
    t = remove_expired_single_access();
    func_call_single(t);
    if(next_rtimer) {
      rtimer_arch_schedule(next_rtimer->time);
    }
  }
}
/*---------------------------------------------------------------------------*/

/** @}*/
