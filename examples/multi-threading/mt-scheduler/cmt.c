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

#include "cmt.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static struct cmt_thread *cmt_list = NULL;  /*!< list holding active cmt threads */
process_event_t cmt_ev;     /*!< Any cmt_thread may access this event id */
process_data_t cmt_data;    /*!< Any cmt_thread may access this event data */

MEMB(memb_cmt_event_ext,struct cmt_event_ext_t,CMT_MAX_EVENTS);

PROCESS(cmt_scheduler_process, "cmt process");


process_event_t cmt_get_ev()
{
    return cmt_ev;
}

process_data_t cmt_get_data()
{
    return cmt_data;
}

void
cmt_start(struct cmt_thread *thread, void (* function)(void *), void *data)
{
    struct cmt_thread *iter;

    /* First make sure that we don't try to start a thread that is
       already running. */
    for(iter = cmt_list; iter; iter = iter->next)
    {
        /* If we found the thread on the  thread list, we bail out. */
        if (iter == thread) return;
    }

    PRINTF("cmt: initial exec of %p \n", thread);
    mt_start((struct mt_thread*)thread,function,data);
    mt_exec((struct mt_thread*)thread); /* exec thread once to get to first blocking situation */

    if(((struct mt_thread*)thread)->state == MT_STATE_EXITED)
    {
        /* thread exited ... thread did not wait for any event */
        mt_stop((struct mt_thread*)thread);
        return;
    }

    /* Put on the thread list.*/
    thread->next = cmt_list;
    cmt_list = thread;
    PRINTF("cmt: added %p to scheduler \n", thread);
}

/* !!! only smt_post is allowed to send a process event to mt_scheduler_process !!! */
int
cmt_post(struct cmt_thread *thread, process_event_t ev, process_data_t data)
{
    struct cmt_event_ext_t *cmt_event_ext;

    cmt_event_ext = memb_alloc(&memb_cmt_event_ext);

    if (!cmt_event_ext) return CMT_ERR_FULL;

    cmt_event_ext->target = thread;
    cmt_event_ext->data = data;
    PRINTF("cmt: post ev 0x%X to mt %p\n",ev,thread);
    return process_post(&cmt_scheduler_process, ev, cmt_event_ext);
}

/* smt_post_sync ... difficult! ... tbd */

void
cmt_init(void)
{
    PRINTF("smt: process start\n");
    process_start(&cmt_scheduler_process,NULL);
}

static void
cmt_wakeup_call(void *data)
{
    cmt_thread *iter, *thread;

    thread = data;

    PRINTF("cmt: wake up call for %p\n", thread);

    /* move event into global scope */
    cmt_ev = PROCESS_EVENT_CONTINUE;
    cmt_data = NULL;

    mt_exec((struct mt_thread*)thread);

    if (((struct mt_thread*)thread)->state != MT_STATE_EXITED)
        return;

    /* thread exited ... thread did not wait for any event */

    mt_stop((struct mt_thread*)thread);

    /* remove thread from thread list */
    iter = cmt_list;
    if(iter == thread)
    {
        cmt_list = thread->next;
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
cmt_sleep(clock_time_t interval)
{
    cmt_thread *thread = cmt_current();
    ctimer_set(&(thread->ct),interval,cmt_wakeup_call,thread);
    PRINTF("cmt: %p goes to bed\n", thread);
    /* ignore events delivered by scheduler while waiting for ctimer */
    cmt_wait_event_until((cmt_get_ev() == PROCESS_EVENT_CONTINUE) && ctimer_expired(&(thread->ct)));
    PRINTF("cmt: %p wakes up\n", thread);
}

void
cmt_poll(cmt_thread *thread)
{
    PRINTF("cmt: poll requested for %p \n", thread);
    thread->needspoll = 1; /* irq-save if called from main thread? ... same question for process_poll */
    process_poll(&cmt_scheduler_process);
}

void
cmt_do_poll()
{
    cmt_thread *iter, *prev = NULL;

    /* move event into global scope */
    cmt_ev = CMT_EVENT_POLL;
    cmt_data = NULL;

    /* for all threads in list ...*/
    iter = cmt_list;
    while(iter)
    {
        if(iter->needspoll)
        {
            iter->needspoll = 0; /* irq-save? ... same question for process_do_poll */

            mt_exec((struct mt_thread*)iter); /* ... deliver event */

            if (((struct mt_thread*)iter)->state == MT_STATE_EXITED)
            {
                mt_stop((struct mt_thread*)iter);

                /* remove thread from list */
                if(prev) prev->next = iter->next;
                    else  cmt_list = iter->next;
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

void
cmt_pause()
{
    if (!cmt_post(cmt_current(),CMT_EVENT_CONTINUE,NULL))
        return; /* to less resources to pause! */

    cmt_wait_event_until(cmt_get_ev() == CMT_EVENT_CONTINUE);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(cmt_scheduler_process, ev, data)
{

  PROCESS_BEGIN();

  while(1)
  {
      PROCESS_WAIT_EVENT();
      {
          cmt_thread *target;

          /* thread poll events. may only be raised by smt_poll() */
          if (ev == CMT_EVENT_POLL)
          {
              cmt_do_poll();
              continue;
          }

          /* check if event has been created by using smt_post */
          if(!memb_inmemb(&memb_cmt_event_ext,data))
              continue;

          /* move event into global scope */
          cmt_ev = ev;
          cmt_data = ((struct cmt_event_ext_t*)data)->data;
          target = ((struct cmt_event_ext_t*)data)->target;

          /* handle addressed event */
          if(target)
          {
              cmt_thread *iter, *prev = NULL;

              /* search for thread in list */
              iter = cmt_list;
              while(iter)
              {
                  /* found thread */
                  if(iter == target)
                  {
                      mt_exec((struct mt_thread*)target); /* deliver event */

                      if (((struct mt_thread*)target)->state == MT_STATE_EXITED)
                      {
                          mt_stop((struct mt_thread*)target);

                          /* remove thread from list */
                          if(prev) prev->next = target->next;
                              else cmt_list = target->next;
                      }

                      break;

                  }
                  prev = iter;
                  iter = iter->next;
              }
          }
          else /* handle broadcast event */
          {
              cmt_thread *iter,*prev = NULL;

              /* for all threads in list ...*/
              iter = cmt_list;
              while(iter)
              {
                    mt_exec((struct mt_thread*)iter); /* ... deliver event */

                    if (((struct mt_thread*)iter)->state == MT_STATE_EXITED)
                    {
                        mt_stop((struct mt_thread*)iter);

                        /* remove thread from list */
                        if(prev) prev->next = iter->next;
                            else  cmt_list = iter->next;
                    }
                    else
                    {
                        prev = iter;
                    }

                    iter = iter->next;
              }
          }

          /* free the extended event data allocated by smt_post */
          memb_free(&memb_cmt_event_ext,data);
      }
  }

  /* exit ... tbd :*/
  /* - stop active cmt threads */
  /* - clear cmt_list */
  /* - reinit memb */
  /* - cleanup/remove pending cmt events in process event queue */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
