/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 */
/**
 * \file
 *         A quick program for testing a reed sensor.
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */

#include <stdio.h>
#include "contiki.h"
#include "reed-sensor.h"
#define REED_READ_INTERVAL (CLOCK_SECOND / 4)
#define REED_EXAMPLE_EVENT  1
/*---------------------------------------------------------------------------*/
PROCESS(test_process, "Reed test process");
AUTOSTART_PROCESSES(&test_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(reed_sensor);
#if REED_EXAMPLE_EVENT
  reed_sensor.configure(REED_SENSOR_MODE, REED_SENSOR_EVENT_MODE);
#else
  etimer_set(&et, REED_READ_INTERVAL);
#endif

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      printf("Reed poll status [%d]\n", reed_sensor.value(REED_SENSOR_VAL));
      etimer_restart(&et);
    } else if(ev == reed_sensor_event_changed) {
      printf("Reed sensor event --> %d\n", (*((int *)data)));
    }
  }
  PROCESS_END();
}
