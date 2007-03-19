/**
 * \defgroup rt Real-time task scheduling
 *
 * The real-time module handles the scheduling and execution of
 * real-time tasks (with predictible execution times).
 *
 * @{
 */

/**
 * \file
 *         Header file for the real-time module.
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
 * @(#)$Id: rt.h,v 1.1 2007/03/19 00:16:13 adamdunkels Exp $
 */
#ifndef __RT_H__
#define __RT_H__

typedef unsigned short rt_clock_t;
#define RT_CLOCK_LT(a,b)     ((signed short)((a)-(b)) < 0)

/**
 * \brief      Initialize the real-time scheduler.
 *
 *             This function initializes the real-time scheduler and
 *             must be called at boot-up, before any other functions
 *             from the real-time scheduler is called.
 */
void rt_init(void);

/**
 * \brief      Repressentation of a real-time task
 *
 *             This structure represents a real-time task and is used
 *             by the real-time module and the architecture specific
 *             support module for the real-time module.
 */
struct rt_task {
  char *name;
  rt_clock_t time;
  void (* func)(struct rt_task *t, void *ptr);
  unsigned char prio;
  void *ptr;
};

/**
 * \brief      Declare a real-time task.
 * \param name The name of the task state variable.
 * \param func The function implementing the real-time task.
 * \param prio The priority of the task.
 * \param ptr  An opaque pointer that is passed to the real-time task
 *             when it is executed.
 *
 *             This macro declares a real-time task.
 *
 * \hideinitializer
 */
#define RT_TASK(name, func, prio, ptr) { name, 0, func, prio, ptr }

enum {
  RT_OK,
  RT_ERR_FULL,
  RT_ERR_TIME,
};

/**
 * \brief      Post a real-time task.
 * \param task A pointer to the task variable previously declared with RT_TASK().
 * \param time The time when the task is to be executed.
 * \return     Non-zero (true) if the task could be scheduled, zero
 *             (false) if the task could not be scheduled.
 *
 *             This function schedules a real-time task a specified
 *             time in the future.
 *
 */
int rt_post(struct rt_task *task, rt_clock_t time, rt_clock_t duration);

/**
 * \brief      Execute the next real-time task and schedule the next task, if any
 *
 *             This function is called by the architecture dependent
 *             code to execute and schedule the next real-time task.
 *
 */
void rt_task_run(void);

/**
 * \brief      Get the current clock time
 * \return     The current time
 *
 *             This function returns what the real-time module thinks
 *             is the current time. The current time is used to set
 *             the timeouts for real-time tasks.
 *
 * \hideinitializer
 */
#define RT_NOW() rt_arch_now()

/**
 * \brief      Get the time that a task last was executed
 * \param task The task
 * \return     The time that a task last was executed
 *
 *             This function returns the time that the task was last
 *             executed. This typically is used to get a periodic
 *             execution of a task without clock drift.
 *
 * \hideinitializer
 */
#define RT_TASK_TIME(task) ((task)->time)

void rt_arch_init(void);
void rt_arch_schedule(rt_clock_t t);
rt_clock_t rt_arch_now(void);


#include "rt-arch.h"

#endif /* __RT_H__ */

/** @} */
