/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: powertrace.c,v 1.1 2010/02/20 14:15:45 adamdunkels Exp $
 */

/**
 * \file
 *         Powertrace: periodically print out power consumption
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "sys/compower.h"
#include "node-id.h"

#include <stdio.h>

PROCESS(powertrace_process, "Periodic power output");
/*---------------------------------------------------------------------------*/
static void
print_power(void)
{
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  static uint32_t last_idle_transmit, last_idle_listen;

  static uint32_t cpu, lpm, transmit, listen;
  static uint32_t idle_transmit, idle_listen;

  static uint32_t seqno;
  
  uint32_t time;

  energest_flush();
  
  cpu = energest_type_time(ENERGEST_TYPE_CPU) - last_cpu;
  lpm = energest_type_time(ENERGEST_TYPE_LPM) - last_lpm;
  transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_transmit;
  listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last_listen;
  idle_transmit = compower_idle_activity.transmit - last_idle_transmit;
  idle_listen = compower_idle_activity.listen - last_idle_listen;

  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  last_idle_listen = compower_idle_activity.listen;
  last_idle_transmit = compower_idle_activity.transmit;

  time = cpu + lpm;
  
  printf("P %d %lu %lu %lu %lu %lu %lu %lu (radio %d.%02d%% tx %d.%02d%% listen %d.%02d%%)\n",
         node_id, seqno++,
         cpu, lpm, transmit, listen, idle_transmit, idle_listen,
         (int)((100L * (transmit + listen)) / time),
         (int)((10000L * (transmit + listen) / time) - (100L * (transmit + listen)) / time),
         (int)((100L * (transmit)) / time),
         (int)((10000L * (transmit)) / time - (100L * (transmit)) / time),
         (int)((100L * (listen)) / time),
         (int)((10000L * (listen)) / time - (100L * (listen)) / time));
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(powertrace_process, ev, data)
{
  static struct etimer periodic;
  clock_time_t *period;
  PROCESS_BEGIN();

  period = data;

  if(period == NULL) {
    PROCESS_EXIT();
  }
  etimer_set(&periodic, *period);

  while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    etimer_reset(&periodic);
    print_power();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
powertrace_start(clock_time_t period)
{
  process_start(&powertrace_process, (void *)&period);
}
/*---------------------------------------------------------------------------*/
