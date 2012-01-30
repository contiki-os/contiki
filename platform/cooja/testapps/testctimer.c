/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: testctimer.c,v 1.3 2010/06/14 18:58:45 adamdunkels Exp $
 */

#include "contiki.h"
#include "sys/ctimer.h"

#include <stdio.h>

PROCESS(test_ctimer_process, "Callback timer test process");
AUTOSTART_PROCESSES(&test_ctimer_process);
/*---------------------------------------------------------------------------*/
static struct ctimer ct;
static uint16_t counter = 0;
/*---------------------------------------------------------------------------*/
static void
callback(void *ptr)
{
  counter++;
  printf("Callback function called at time %lu (counter=%i)\n", clock_time(), counter);
  ctimer_set(&ct, CLOCK_SECOND/2, callback, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_ctimer_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Starting callback timer test process (counter=%i)\n", counter);

  ctimer_set(&ct, CLOCK_SECOND/2, callback, NULL);

  PROCESS_END();
}
