/*
 * Copyright (c) 2014, marcas756@gmail.com.
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
 * Author: marcas756 <marcas756@gmail.com>
 *
 */

/**
 * \file
 *         Scheduled cooperative multi threading implementation
 *
 *         - Non preemtive mt-scheduler (context switch during irq not
 *           allowed for this implementation)
 *         - In this implementation the cmt thread is derived from contiki process, thus a thread uses
 *           the contiki process scheduler instead of an own scheduler.
 *         - As a cmt instance is also a process instance, process post/post_sync can be used directly
 *           by processes and cmt_threads to post events to each other.
 *         - This example also shows the close logical relationship between protothreads and
 *           usual cooperative threads. Think twice if you really need context switching or not!
 *         - For this special example it would be an advantage if process functions would use a
 *           void pointer for process passing, to avoid explicit casting on application level.
 *         - It would be an advantage to be able to define thread specific stack sizes to avoid
 *           unnecessary RAM usage. Maybe by using cpu min stack size and multiplier? ... to be discussed
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */


#include "cmt.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static process_event_t cmt_ev;     /*!< Any cmt_thread may access this event id via get func */
static process_data_t cmt_data;    /*!< Any cmt_thread may access this event data via get func */

/*---------------------------------------------------------------------------*/

#define PROCESS_RESOLVE_FUNC_NAME(name) \
        process_thread_##name

PROCESS_THREAD(cmt_thread_handler, ev, data)
{
  /* move event into global scope */
  cmt_ev = ev;
  cmt_data = data;

  PRINTF("cmt: %p receives 0x%0X\n",cmt_current(),cmt_ev);

  PROCESS_BEGIN();

  /* Ensure that we are outside of any other mt_thread context, see cmt_start */
  PROCESS_WAIT_EVENT();

  /* exec thread once to get to first blocking situation */
  mt_start(&(cmt_current()->mt_thread),cmt_current()->function,data);
  mt_exec(&(cmt_current()->mt_thread));

  PRINTF("cmt: %p started\n",cmt_current());

  while((cmt_current()->mt_thread).state != MT_STATE_EXITED)
  {
      PRINTF("cmt: %p blocks  \n",cmt_current());
      PROCESS_WAIT_EVENT();
      mt_exec(&(cmt_current()->mt_thread));
  }


  mt_stop(&(cmt_current()->mt_thread));
  PRINTF("cmt: %p exited\n",cmt_current());

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

process_event_t
cmt_get_ev()
{
    return cmt_ev;
}

process_data_t
cmt_get_data()
{
    return cmt_data;
}

void
cmt_start(struct cmt_thread *thread, void (* function)(void *), void *data)
{

#ifndef PROCESS_CONF_NO_PROCESS_NAMES
    static const char* cmt_any_text = "any cmt";
#endif

    /* First make sure that we don't try to start a process that is
       already running. */
    if(process_is_running((struct process *)thread))
    {
        /* If process is running, we bail out. */
        return;
    }

    thread->function = function;

    thread->process.thread = PROCESS_RESOLVE_FUNC_NAME(cmt_thread_handler);
#ifndef PROCESS_CONF_NO_PROCESS_NAMES
    thread->process.name = (char*)cmt_any_text;
#endif

    PRINTF("cmt: %p starts %p\n",cmt_current(),thread);

    process_start((struct process*)thread,NULL); /* start the cmt_thread_handler */

    /* move out of any cmt context ...
       this allows to cmt_start another thread within a
       cmt_thread context */
    process_post((struct process*)thread,PROCESS_EVENT_NONE,data);
}

void
cmt_pause()
{
    process_post(PROCESS_CURRENT(),PROCESS_EVENT_CONTINUE,NULL);
    cmt_wait_event_until(cmt_get_ev() == PROCESS_EVENT_CONTINUE);
}

void
cmt_sleep(clock_time_t interval)
{
    etimer_set(&cmt_current()->et,interval);
    cmt_wait_event_until(etimer_expired(&cmt_current()->et));
}

void
cmt_join(struct cmt_thread *thread)
{
    /* if we would join on ourself, the cmt will block forever -> not allowed! */
    /* if the joined thread is not running anymore, it can not fire PROCESS_EVENT_EXITED anymore,
       thus we return directly here in this case.*/
    if (cmt_current() == thread || !process_is_running((struct process *)thread)){
        return;
    }

    PRINTF("cmt: %p joined %p\n",cmt_current(),thread);

    /* wait until the joined cmt_thread_handler broadcasts PROCESS_EVENT_EXITED */
    cmt_wait_event_until(
        cmt_get_ev() == PROCESS_EVENT_EXITED &&
        cmt_get_data() == thread);
}








