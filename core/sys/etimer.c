/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**
 * \addtogroup etimer
 * @{
 */

/**
 * \file
 * Event timer library implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

#include "contiki-conf.h"

#include "sys/etimer.h"
#include "sys/timer.h"
#include "sys/process.h"

#include <stdio.h>

static struct etimer *timerlist;
static clock_time_t next_expiration;

PROCESS(etimer_process, "Event timer");

#define DEBUG 0

#if defined(DEBUG) && (DEBUG)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while(0)
#endif
/*---------------------------------------------------------------------------*/
static void
update_next_expiration(void)
{
  if(timerlist != NULL) {
    next_expiration = etimer_expiration_time(timerlist);
  } else {
    next_expiration = 0;
  }
}
/*---------------------------------------------------------------------------*/
static void
post_expiries(void)
{
  struct etimer *tmp;
  int ret;

  while(timerlist != NULL) {

    if(!timer_expired(&timerlist->timer)) {
      /* since the list is sorted, this means we've processed all expired
         timers already */
      break;
    }

    ret = process_post(timerlist->p, PROCESS_EVENT_TIMER, timerlist);

    if(ret != PROCESS_ERR_OK) {
      /* the event queue is full; we will try again later */
      PRINTF("etimer: PROCESS_ERR %d\n", ret);
      etimer_request_poll();
      break;
    }

    timerlist->p = PROCESS_NONE;

    tmp = timerlist;
    timerlist = tmp->next;
    tmp->next = NULL;
  }

  update_next_expiration();
}
/*---------------------------------------------------------------------------*/
static void
remove_process(struct process *p)
{
  struct etimer **itr;

  itr = &timerlist;

  while(*itr) {
    if((*itr)->p == p) {
      *itr = (*itr)->next;
    } else {
      itr = &((*itr)->next);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(etimer_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_POLL) {
      post_expiries();
    } else if(ev == PROCESS_EVENT_EXITED) {
      remove_process(data);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
etimer_request_poll(void)
{
  process_poll(&etimer_process);
}
/*---------------------------------------------------------------------------*/
static void
add_timer(struct etimer *timer)
{
  clock_time_t now = clock_time();
  struct process *proc = PROCESS_CURRENT();
  struct etimer *t_this;
  struct etimer *t_last;

  if(proc == PROCESS_NONE) {
    /* don't add an etimer with no process */
    PRINTF("etimer: proc is NONE\n");
    return;
  }

  etimer_request_poll();

  if(timer->p != PROCESS_NONE) {
    etimer_stop(timer);
  }

  timer->p = proc;

  if(timerlist == NULL || etimer_lte(timer, timerlist, now)) {
    timer->next = timerlist;
    timerlist = timer;
    update_next_expiration();
    return;
  }

  t_last = timerlist;
  for(t_this = timerlist->next; t_this != NULL; t_this = t_this->next) {
    if(etimer_lte(timer, t_this, now)) {
      break;
    }
    t_last = t_this;
  }

  t_last->next = timer;
  timer->next = t_this;
}
/*---------------------------------------------------------------------------*/
void
etimer_set(struct etimer *et, clock_time_t interval)
{
  timer_set(&et->timer, interval);
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void
etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval)
{
  timer_reset(&et->timer);
  et->timer.interval = interval;
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void
etimer_reset(struct etimer *et)
{
  timer_reset(&et->timer);
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void
etimer_restart(struct etimer *et)
{
  timer_restart(&et->timer);
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
void
etimer_adjust(struct etimer *et, int timediff)
{
  et->timer.start += timediff;
  add_timer(et);
}
/*---------------------------------------------------------------------------*/
int
etimer_expired(struct etimer *et)
{
  return et->p == PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/
clock_time_t
etimer_expiration_time(struct etimer *et)
{
  return et->timer.start + et->timer.interval;
}
/*---------------------------------------------------------------------------*/
clock_time_t
etimer_start_time(struct etimer *et)
{
  return et->timer.start;
}
/*---------------------------------------------------------------------------*/
int
etimer_pending(void)
{
  return timerlist != NULL;
}
/*---------------------------------------------------------------------------*/
clock_time_t
etimer_next_expiration_time(void)
{
  return etimer_pending() ? next_expiration : 0;
}
/*---------------------------------------------------------------------------*/
const struct timer *
etimer_next_to_expire(void)
{
  return timerlist ? &(timerlist->timer) : NULL;
}
/*---------------------------------------------------------------------------*/
void
etimer_stop(struct etimer *et)
{
  struct etimer *t;

  /* First check if et is the first event timer on the list. */
  if(et == timerlist) {
    timerlist = timerlist->next;
    update_next_expiration();
  } else {
    /* Else walk through the list and try to find the item before the
       et timer. */
    for(t = timerlist; t != NULL && t->next != et; t = t->next);

    if(t != NULL) {
      /* We've found the item before the event timer that we are about
         to remove. We point the items next pointer to the event after
         the removed item. */
      t->next = et->next;
    }
  }

  /* Remove the next pointer from the item to be removed. */
  et->next = NULL;
  /* Set the timer as expired */
  et->p = PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/
/** @} */
