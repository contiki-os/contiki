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
 *         Scheduled multi threading library
 *
 *         - Non preemtive mt-scheduler (context switch during irq not
 *           allowed for this implementation)
 *         - sending events from a contiki-process to a smt-thread works by
 *           using smt_post in the contiki-process
 *         - sending events from a smt-thread to a smt-thread works by
 *           using smt_post source smt_thread
 *         - sending events from a smt-thread to a contiki-process works by
 *           using process_post/process_post_synch in the smt-thread
 *         - smt-thread can be sent to bed with smt_sleep. This does only block
 *           the smt-thread calling sleep, not the whole system.
 *         - an smt-thread has to exit on its own. Killing the thread from outside
 *           is not possible
 *         - A broadcast of an event to all smt-threads can be achieved by setting the
 *           target thread to SMT_BROADCAST when invoking smt_post.
 *         - Don't forget to call mt_init() and smt_init() before using this lib.
 *         - polling threads from irq or main thread by using smt_poll()
 *
 *
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */

#include "smt.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


static struct mt_thread *smt_list = NULL;  /*!< list holding active mt_threads */
process_event_t smt_ev;     /*!< Any mt_thread may access this event id */
process_data_t smt_data;    /*!< Any mt_thread may access this event data */

MEMB(memb_smt_event_ext,struct smt_event_ext_t,SMT_MAX_EVENTS);

PROCESS(mt_scheduler_process, "smt process");

void
smt_start(struct mt_thread *thread, void (* function)(void *), void *data)
{
    mt_thread *iter;

    /* First make sure that we don't try to start a thread that is
       already running. */
    for(iter = smt_list; iter; iter = iter->next)
    {
        /* If we found the thread on the  thread list, we bail out. */
        if (iter == thread) return;
    }

    PRINTF("smt: initial exec of %p \n", thread);
    mt_start(thread,function,data);
    mt_exec(thread); /* exec thread once to get to first blocking situation */

    if(thread->state == MT_STATE_EXITED) /* thread exited ... thread did not wait for any event */
        return;

    /* Put on the thread list.*/
    thread->next = smt_list;
    smt_list = thread;
    PRINTF("smt: added %p to scheduler \n", thread);
}

/* !!! only smt_post is allowed to send a process event to mt_scheduler_process !!! */
int
smt_post(struct mt_thread *thread, process_event_t ev, process_data_t data)
{
    struct smt_event_ext_t *smt_event_ext;

    smt_event_ext = memb_alloc(&memb_smt_event_ext);

    if (!smt_event_ext) return SMT_ERR_FULL;

    smt_event_ext->target = thread;
    smt_event_ext->data = data;
    PRINTF("smt: post ev 0x%X to mt %p\n",ev,thread);
    return process_post(&mt_scheduler_process, ev, smt_event_ext);
}

/* smt_post_sync ... difficult! ... tbd */

void
smt_init(void)
{
    PRINTF("smt: process start\n");
    process_start(&mt_scheduler_process,NULL);
}

static void
smt_wakeup_call(void *data)
{
    mt_thread *iter, *thread;

    thread = data;

    PRINTF("smt: wake up call for %p\n", thread);

    mt_exec(thread);

    if (thread->state != MT_STATE_EXITED)
        return;

    mt_stop(thread);

    /* remove thread from thread list */
    iter = smt_list;
    if(iter == thread)
    {
        smt_list = thread->next;
        return;
    }

    while(iter->next)
    {
        if(iter->next == thread)
        {
            iter->next = thread->next;
            return;
        }

        iter = iter->next;
    }
}


void
smt_sleep(clock_time_t interval)
{
    struct ctimer ct;

    ctimer_set(&ct,interval,smt_wakeup_call,mt_current());
    PRINTF("smt: %p goes to bed\n", mt_current());
    /* ignore events delivered by scheduler while waiting for ctimer */
    smt_wait_event_until(ctimer_expired(&ct));
    PRINTF("smt: %p wakes up\n", mt_current());
}

void
smt_poll(mt_thread *thread)
{
    PRINTF("smt: poll requested for %p \n", thread);
    thread->needspoll = 1; /* irq-save if called from main thread? ... same question for process_poll */
    process_poll(&mt_scheduler_process);
}

void
smt_do_poll()
{
    mt_thread *iter, *prev = NULL;

    /* move event into global scope */
    smt_ev = SMT_EVENT_POLL;
    smt_data = NULL;

    /* for all threads in list ...*/
    iter = smt_list;
    while(iter)
    {
        if(iter->needspoll)
        {
            iter->needspoll = 0; /* irq-save? ... same question for process_do_poll */

            mt_exec(iter); /* ... deliver event */

            if (iter->state == MT_STATE_EXITED)
            {
                mt_stop(iter);

                /* remove thread from list */
                if(prev) prev->next = iter->next;
                    else  smt_list = iter->next;
            }
            else
            {
                prev = iter;
            }

            iter = iter->next;
        }
        else
        {
            prev = iter;
            iter = iter->next;
        }
    }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(mt_scheduler_process, ev, data)
{

  PROCESS_BEGIN();

  while(1)
  {
      PROCESS_WAIT_EVENT();
      {
          mt_thread *target;

          /* thread poll events. may only be raised by smt_poll() */
          if (ev == SMT_EVENT_POLL)
          {
              smt_do_poll();
              //continue;
          }

          /* check if event has been created by using smt_post */
          if(!memb_inmemb(&memb_smt_event_ext,data))
              continue;

          /* move event into global scope */
          smt_ev = ev;
          smt_data = ((struct smt_event_ext_t*)data)->data;
          target = ((struct smt_event_ext_t*)data)->target;

          /* handle addressed event */
          if(target)
          {
              mt_thread *iter, *prev = NULL;

              /* search for thread in list */
              iter = smt_list;
              while(iter)
              {
                  /* found thread */
                  if(iter == target)
                  {
                      mt_exec(target); /* deliver event */

                      if (target->state == MT_STATE_EXITED)
                      {
                          mt_stop(target);

                          /* remove thread from list */
                          if(prev) prev->next = target->next;
                              else smt_list = target->next;
                      }

                      break;

                  }
                  prev = iter;
                  iter = iter->next;
              }
          }
          else /* handle broadcast event */
          {
              mt_thread *iter,*prev = NULL;

              /* for all threads in list ...*/
              iter = smt_list;
              while(iter)
              {
                    mt_exec(iter); /* ... deliver event */

                    if (iter->state == MT_STATE_EXITED)
                    {
                        mt_stop(iter);

                        /* remove thread from list */
                        if(prev) prev->next = iter->next;
                            else  smt_list = iter->next;
                    }
                    else
                    {
                        prev = iter;
                    }

                    iter = iter->next;
              }
          }

          /* free the extended event data allocated by smt_post */
          memb_free(&memb_smt_event_ext,data);
      }
  }

  /* exit ... tbd */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
