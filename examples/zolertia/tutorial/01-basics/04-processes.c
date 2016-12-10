/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         An example showing how to use the on-board LED and user button
 * \author
 *         Antonio Lignan <alinan@zolertia.com> <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sys/etimer.h"

/* This new library adds the random functions */
#include "lib/random.h"
/*---------------------------------------------------------------------------*/
/* This will generate a random wait time from zero to five seconds */
#define WAIT_INTERVAL		(CLOCK_SECOND * 5)
#define WAIT_TIME		    (random_rand() % (WAIT_INTERVAL))
/*---------------------------------------------------------------------------*/
/* Event timers used in each process */
static struct etimer et1;
static struct etimer et2;
static struct etimer et3;
/*---------------------------------------------------------------------------*/
/* We create an event to be used by process3 to communicate to process1 */
process_event_t event_from_process3;
/*---------------------------------------------------------------------------*/
/* We are going to create three different processes, with its own printable
 * name.  Processes are a great way to run different applications and features
 * in parallel
 */
PROCESS(process1, "Main process");
PROCESS(process2, "Auxiliary process");
PROCESS(process3, "Another auxiliary process");

/* But we are only going to automatically start the first two */
AUTOSTART_PROCESSES(&process1, &process2);
/*---------------------------------------------------------------------------*/
/* Now let's implement the main process, this will start process3 and begin to
 * increment a variable.  When a given value is reached, it will stop process3 
 */
PROCESS_THREAD(process1, ev, data)
{
  PROCESS_BEGIN();

  /* This is a variable to store the data sent by the process3 */
  static uint8_t counter;

  printf("Process 1 started\n");

  /* Use a random value to wait before starting the process */
  etimer_set(&et1, WAIT_TIME);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));

  /* Start the process and send as data our name */
  process_start(&process3, "Process 1");

  /* And wait until process3 sends us a message */

  while(1) {
    /* This protothread waits for any event, we need to check of the event type
     * ourselves
     */
    PROCESS_YIELD();

    /* We are waiting here for an event from process3 */
    if(ev == event_from_process3) {
      counter = *((uint8_t *)data);
      printf("Process 3 has requested shutdown in %u seconds\n", counter);
      etimer_set(&et1, CLOCK_SECOND);
    }

    /* We are waiting here for a timer event */
    if(ev == PROCESS_EVENT_TIMER) {

      /* When the counter reaches zero, kill process3 */
      if(counter <= 0) {
        process_exit(&process3);

      /* Increment the counter value and restart the timer */
      } else {
        printf("Process 3 will be terminated in: %u\n", counter);
        counter--;
        leds_toggle(LEDS_RED);
        etimer_reset(&et1);
      }
    }
  }

  /* This is the end of the process, we tell the system we are done.  Even if
   * we won't reach this due to the "while(...)" we need to include it
   */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* process2 waits until process3 is stopped and then proceed to restart it */
PROCESS_THREAD(process2, ev, data)
{
  PROCESS_BEGIN();

  printf("Process 2 started\n");

  while(1) {
    PROCESS_YIELD();

    /* We are waiting here for process3 being stopped */
    if(ev == PROCESS_EVENT_EXITED) {
      printf("* Process 3 has been stopped by Process 1!\n");
      etimer_set(&et2, CLOCK_SECOND * 5);
    }

    if(ev == PROCESS_EVENT_TIMER) {
      printf("Process 2 is restarting Process 3\n");

      /* Restart process3 and send as data our name */
      process_start(&process3, "Process 2");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* process3 is started first by process1, then when stopped by process1 it will
 * be restarted by process2
 */
PROCESS_THREAD(process3, ev, data)
{
  PROCESS_BEGIN();

  /* Create a pointer to the data, as we are expecting a string we use "char" */
  static char *parent;
  parent = (char * )data;

  /* And a counter */
  static uint8_t counter;

  printf("Process 3 started by %s\n", parent);

  /* We need to allocate a numeric process ID to our process */
  event_from_process3 = process_alloc_event();

  etimer_set(&et3, CLOCK_SECOND);

  counter = 0;

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et3));

    /* Increment the counter, when it reaches a value of five notify process1 */
    counter++;

    /* Blink the LED */
    leds_toggle(LEDS_GREEN);

    if(counter == 10) {
      /* Send a message to process1 and send our current counter value */
      process_post(&process1, event_from_process3, &counter);
    }

    /* Restart the timer */
    etimer_reset(&et3);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
