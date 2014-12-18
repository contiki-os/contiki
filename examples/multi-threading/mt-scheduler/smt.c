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


struct mt_thread *smt_list = NULL;  /*!< list holding active mt_threads */
struct mt_thread *current;  /*!< currently running mt_thread */
process_event_t smt_ev;     /*!< Any mt_thread may access this event id */
process_data_t smt_data;    /*!< Any mt_thread may access this event data */

MEMB(memb_smt_event_ext,struct smt_event_ext_t,SMT_MAX_EVENTS); /*!< OOOUCH!  */

PROCESS(mt_scheduler_process, "smt process");


void smt_start(struct mt_thread *thread, void (* function)(void *), void *data)
{
    mt_thread *iter;

    if(!thread) return;

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
int smt_post(struct mt_thread *thread, process_event_t ev, process_data_t data)
{
    struct smt_event_ext_t *smt_event_ext;

    smt_event_ext = memb_alloc(&memb_smt_event_ext); /* OOOUCH!  */

    if (!smt_event_ext) return SMT_ERR_FULL;

    smt_event_ext->target = thread;
    smt_event_ext->data = data;
    PRINTF("smt: post ev 0x%X to mt %p\n",ev,thread);
    return process_post(&mt_scheduler_process, ev, smt_event_ext);
}



void smt_init()
{
    PRINTF("smt: process start\n");
    process_start(&mt_scheduler_process,NULL);
}

void smt_wakeup_call(void *data)
{
    PRINTF("smt: wake up call for %p\n", ((struct smt_event_ext_t*)data)->target);
    /* mt_exec((struct mt_thread*)data); */
    process_post(&mt_scheduler_process,PROCESS_EVENT_TIMER,data);
}


void smt_sleep(clock_time_t interval)
{
    struct ctimer ct;
    struct smt_event_ext_t smt_event_ext;

    smt_event_ext.target = mt_current();

    ctimer_set(&ct,interval,smt_wakeup_call,&smt_event_ext);
    PRINTF("smt: %p goes to bed\n", smt_event_ext.target);
    smt_wait_event_until(ctimer_expired(&ct));
    PRINTF("smt: %p wakes up\n", smt_event_ext.target);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(mt_scheduler_process, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
      PROCESS_WAIT_EVENT();
      {
          mt_thread *iter, *target;

          target = ((struct smt_event_ext_t*)data)->target;

          /* Try to find the thread the event is addressed to */
          for(iter = smt_list; iter; iter = iter->next)
          {
              /* If we found the thread on the  thread list,
                 we deliver the event to the thread */
              if (iter == target)
              {
                  /* move event into global scope */
                  smt_ev = ev;
                  smt_data = ((struct smt_event_ext_t*)data)->data;

                  mt_exec(target); /* deliver event to thread */

                  /* event delivered ... postprocessing ... */

                  if(target->state == MT_STATE_EXITED) /* Event caused the thread to exit */
                  {

                        mt_stop(target);

                        /* remove thread from thread list */
                        if(target == smt_list) {
                            smt_list = smt_list->next;
                        } else {
                            for(iter = smt_list; iter; iter = iter->next) {
                                if(iter->next == target) {
                                    iter->next = target->next;
                                    break;
                                }
                            }
                        }

                        PRINTF("smt: removed %p from scheduler \n", target);
                  }


                  break; /* event delivered ... break loop thread finder */
              }
          }
      }

      /* free the extended event data allocated by smt_post*/
      memb_free(&memb_smt_event_ext,data); /* OUCH! */


  }

  mtarch_remove();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
