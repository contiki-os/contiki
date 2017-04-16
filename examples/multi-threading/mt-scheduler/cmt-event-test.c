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
 *         Example implementation of an contiki event enabled  cooperative mt scheduler
 *
 *         2 processes(p3,p4), 2 mt_threads (t1,t2)
 *
 *         t1 -> smt_post     -> t2
 *         t2 -> process_post -> p3
 *         p3 -> process_post -> p4
 *         p4 -> smt_post     -> t1
 *
 *         missing: test for post_sync
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */



#include "contiki.h"
#include <stdint.h>
#include "cmt.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#define TEST_EVENT_ID 22
#define SLEEP_TIME (CLOCK_SECOND/1)

uint8_t token = 0;

cmt_thread t1,t2;
PROCESS(p3, "p3");
PROCESS(p4, "p4");

void t1_thread_func (void* data)
{

    while(1)
    {
        uint8_t* ev_data;

        cmt_wait_event_until(cmt_get_ev() == TEST_EVENT_ID);

        ev_data = cmt_get_data();

        PRINTF("t1 received %d\n", *ev_data);
        (*ev_data)+=1;

        cmt_sleep(SLEEP_TIME);

        cmt_post(&t2,TEST_EVENT_ID,ev_data);
        PRINTF("t1 sent %d\n", *ev_data);
    }

    cmt_exit();
}

void t2_thread_func (void* data)
{

    while(1)
    {
        uint8_t* ev_data;

        cmt_wait_event_until(cmt_get_ev() == TEST_EVENT_ID);

        ev_data = cmt_get_data();

        PRINTF("t2 received %d\n", *ev_data);
        (*ev_data)+=1;

        cmt_sleep(SLEEP_TIME);

        process_post(&p3,TEST_EVENT_ID,ev_data);
        PRINTF("t2 sent %d\n", *ev_data);
    }

    cmt_exit();
}

PROCESS_THREAD(p3, ev, data)
{
    static struct etimer et;
    static uint8_t *ev_data;

    PROCESS_BEGIN();

    while(1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == TEST_EVENT_ID);

        ev_data = data;

        PRINTF("p3 received %d\n", *ev_data);
        (*ev_data)+=1;

        etimer_set(&et,SLEEP_TIME);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        process_post(&p4,TEST_EVENT_ID,ev_data);
        PRINTF("p3 sent %d\n", *ev_data);
    }

    PROCESS_END();
}

PROCESS_THREAD(p4, ev, data)
{
    static struct etimer et;
    static uint8_t *ev_data;

    PROCESS_BEGIN();

    while(1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == TEST_EVENT_ID);

        ev_data = data;

        PRINTF("p4 received %d\n", *ev_data);
        (*ev_data)+=1;

        etimer_set(&et,SLEEP_TIME);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        cmt_post(&t1,TEST_EVENT_ID,ev_data);
        PRINTF("p4 sent %d\n", *ev_data);
    }

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS(smt_example, "smt example");
AUTOSTART_PROCESSES(&smt_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smt_example, ev, data)
{

  PROCESS_BEGIN();

  mt_init();
  cmt_init();

  cmt_start(&t1,t1_thread_func,NULL);
  cmt_start(&t2,t2_thread_func,NULL);

  process_start(&p3,NULL);
  process_start(&p4,NULL);

  // start the endless event loop
  cmt_post(&t1,TEST_EVENT_ID,&token);


  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

