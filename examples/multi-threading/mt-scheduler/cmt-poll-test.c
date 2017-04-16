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
 *         Testing mt polling functionality
 *         Example implementation of an contiki event enabled  mt scheduler
 * \author
 *         marcas756 <marcas756@gmail.com>
 */



#include "contiki.h"
#include <stdint.h>
#include <stdlib.h>
#include "cmt.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define NUM_OF_THREADS 5
#define MAX_POLL_DELAY CLOCK_SECOND

static cmt_thread threads[NUM_OF_THREADS];
static cmt_thread poller_thread;


void poller_thread_func(void *data)
{
    PRINTF("THREAD poller started \n");
    while(1)
    {
        cmt_sleep(rand()%MAX_POLL_DELAY);
        cmt_poll(&threads[rand()%NUM_OF_THREADS]);
        PRINTF("THREAD sent poll event \n");
    }

    mt_exit();
}

int get_thread_id(cmt_thread *thread)
{
    return (thread-threads);
}

void test_thread_func(void *data)
{
    int thread_id;

    thread_id = get_thread_id(cmt_current());

    while(1)
    {

        cmt_wait_event_until(cmt_get_ev() == CMT_EVENT_POLL);
        PRINTF("Thread [%d] %p received poll \n",thread_id,cmt_current());
    }

    cmt_exit();
}



/*---------------------------------------------------------------------------*/
PROCESS(smt_example, "smt example");
AUTOSTART_PROCESSES(&smt_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smt_example, ev, data)
{
      static struct etimer et;

      PROCESS_BEGIN();
      {
          int tmp;

          mt_init();
          cmt_init();


          for(tmp = 0; tmp < NUM_OF_THREADS; tmp+=2)
          {
              cmt_start(&threads[tmp],test_thread_func,NULL);
          }

          cmt_start(&poller_thread,poller_thread_func,NULL);

          while(1)
          {
              etimer_set(&et,rand()%MAX_POLL_DELAY);
              PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
              cmt_poll(&threads[rand()%NUM_OF_THREADS]);
              PRINTF("PROCESS sent poll event \n");
          }

      }
      PROCESS_END();
}
/*---------------------------------------------------------------------------*/

