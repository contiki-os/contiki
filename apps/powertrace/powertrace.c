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
 * $Id: powertrace.c,v 1.6 2010/04/30 07:17:21 adamdunkels Exp $
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
#include "powertrace.h"
#include "net/rime.h"
#include "node-id.h"

#include <stdio.h>

PROCESS(powertrace_process, "Periodic power output");
/*---------------------------------------------------------------------------*/
void
powertrace_print(char *str)
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
  
  printf("%s %lu P %d %lu %lu %lu %lu %lu %lu %lu (radio %d.%02d%% tx %d.%02d%% listen %d.%02d%%)\n",
         str,
         clock_time(), node_id, seqno++,
         cpu, lpm, transmit, listen, idle_transmit, idle_listen,
         (int)((100L * (transmit + listen)) / time),
         (int)((10000L * (transmit + listen) / time) - (100L * (transmit + listen) / time) * 100),
         (int)((100L * transmit) / time),
         (int)((10000L * transmit) / time - (100L * transmit / time) * 100),
         (int)((100L * listen) / time),
         (int)((10000L * listen) / time - (100L * listen / time) * 100));
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
    powertrace_print("");
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
void
powertrace_stop(void)
{
  process_exit(&powertrace_process);
}
/*---------------------------------------------------------------------------*/
#if ! UIP_CONF_IPV6
static void
sniffprint(char *prefix, int seqno)
{
  const rimeaddr_t *sender, *receiver, *esender, *ereceiver;

  sender = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  receiver = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  esender = packetbuf_addr(PACKETBUF_ADDR_ESENDER);
  ereceiver = packetbuf_addr(PACKETBUF_ADDR_ERECEIVER);


  printf("%lu %s %d %u %d %d %d.%d %u %u\n",
         clock_time(),
         prefix,
         rimeaddr_node_addr.u8[0], seqno,
         packetbuf_attr(PACKETBUF_ATTR_CHANNEL),
         packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE),
         esender->u8[0], esender->u8[1],
         packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME),
         packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME));
}
/*---------------------------------------------------------------------------*/
static void
input_sniffer(void)
{
  static int seqno = 0;
  sniffprint("I", seqno++);

  if(packetbuf_attr(PACKETBUF_ATTR_CHANNEL) == 0) {
    int i;
    uint8_t *dataptr;
    
    printf("x %d ", packetbuf_totlen());
    dataptr = packetbuf_hdrptr();
    printf("%02x ", dataptr[0]);
    for(i = 1; i < packetbuf_totlen(); ++i) {
      printf("%02x ", dataptr[i]);
    }
    printf("\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
output_sniffer(void)
{
  static int seqno = 0;
  sniffprint("O", seqno++);
}
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(s, input_sniffer, output_sniffer);
/*---------------------------------------------------------------------------*/
void
powertrace_sniff(powertrace_onoff_t onoff)
{
  switch(onoff) {
  case POWERTRACE_ON:
    rime_sniffer_add(&s);
    break;
  case POWERTRACE_OFF:
    rime_sniffer_remove(&s);
    break;
  }
}
/*---------------------------------------------------------------------------*/
#else /* ! UIP_CONF_IPV6 */
void
powertrace_sniff(powertrace_onoff_t onoff)
{
}
#endif /* ! UIP_CONF_IPV6 */
