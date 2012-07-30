/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark of Advancare,SL
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
 *         A quick program for testing a generic relay device connected in the
 *         phidget port
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */

#include <stdio.h>
#include "contiki.h"
#include "dev/relay-phidget.h"


#if 1
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#if 0
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif


#define RELAY_INTERVAL (CLOCK_SECOND)

PROCESS(test_process, "Relay test process");
AUTOSTART_PROCESSES(&test_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
static uint8_t status;

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

  /* Selects P6.7 as control pin of the relay module */
  relay_enable(7);

  while(1) {
    etimer_set(&et, RELAY_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    
    status = relay_toggle();
    PRINTF("Relay [%d]\n", status);
  }
  PROCESS_END();
}
