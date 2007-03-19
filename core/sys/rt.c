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
 * @(#)$Id: rt.c,v 1.1 2007/03/19 00:16:13 adamdunkels Exp $
 */

#include "sys/rt.h"
#include "contiki.h"

#define LIST_SIZE 16

static struct rt_task *tasks[LIST_SIZE];
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
rt_init(void)
{
  next = 0;
  firstempty = 0;
  rt_arch_init();
}
/*---------------------------------------------------------------------------*/
int
rt_post(struct rt_task *task, rt_clock_t time, rt_clock_t duration)
{
  int i;

  PRINTF("rt_post time %d\n", time);
  
  /* Check if task queue is full. */
  if(firstempty == (next - 1) % LIST_SIZE) {
    PRINTF("rt_post: next %d firstempty %d full\n", next, firstempty);
    return RT_ERR_FULL;
  }
  
  /* Check if it is possible to run this task at the requested
     time. */
  for(i = next; i != firstempty;
      i = (i + 1) % LIST_SIZE) {

    /* XXX: should check a range of time not just the same precise
       moment. */
    if(tasks[i]->time == time) {
      PRINTF("rt_post: next %d firstempty %d time %d == %d\n",
	     next, firstempty, tasks[i]->time, time);
      return RT_ERR_TIME;
    }
  }
  /* Put the task at the end of the task list. */
  task->time = time;
  tasks[firstempty] = task;
  PRINTF("rt_post: putting task %s as %d\n", task->name, firstempty);

  firstempty = (firstempty + 1) % LIST_SIZE;

  /*  PRINTF("rt_post: next %d firstempty %d scheduling soon\n",
      next, firstempty);*/

  /* Check if this is the first task on the list. If so, we need to
     run the rt_arch_schedule() function to get the ball rolling. */
  if(firstempty == (next + 1) % LIST_SIZE) {
    
    PRINTF("rt_post scheduling %d %s (%d)\n",
	 next, tasks[next]->name, tasks[next]->time);
    rt_arch_schedule(time);
  }
  
  return RT_OK;
}
/*---------------------------------------------------------------------------*/
void
rt_task_run(void)
{
  int i, n;
  struct rt_task *t;

  t = tasks[next];

  /* Increase the pointer to the next task. */
  next = (next + 1) % LIST_SIZE;
  
  /* Run the task. */
  PRINTF("rt_task_run running %s\n", t->name);
  t->func(t, t->ptr);

  if(next == firstempty) {
    PRINTF("rt_task_run: empty task list\n");
    /* The list is empty, no more tasks to schedule. */
    return;
  }

  /* Find the next task to run. */
  n = next;
  for(i = next; i != firstempty; i = (i + 1) % LIST_SIZE) {
    PRINTF("rt_task_run checking %s (%d) against %s (%d)\n",
	   tasks[i]->name, tasks[i]->time,
	   tasks[n]->name, tasks[n]->time);
    if(tasks[i]->prio >= tasks[n]->prio &&
       RT_CLOCK_LT(tasks[i]->time, tasks[n]->time)) {
      n = i;
    }
  }

  PRINTF("rt_task_run next task is %d %s (%d)\n",
	 n, tasks[n]->name, tasks[n]->time);
  
  /* Put the next task first in the task list. */
  t = tasks[next];
  tasks[next] = tasks[n];
  tasks[n] = t;

  PRINTF("rt_task_run scheduling %d %s (%d)\n",
	 next, tasks[next]->name, tasks[next]->time);

  rt_arch_schedule(tasks[next]->time);
}
/*---------------------------------------------------------------------------*/
