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
 */

/**
 * \file
 *         Collect view client code
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/collect-neighbor.h"
#include "net/rime.h"
#include "net/rime/timesynch.h"
#include "collect-view.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
void
collect_view_construct_message(struct collect_view_data_msg *msg,
                               const rimeaddr_t *parent,
                               uint16_t parent_etx,
                               uint16_t current_rtmetric,
                               uint16_t num_neighbors,
                               uint16_t beacon_interval)
{
  static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
  unsigned long cpu, lpm, transmit, listen;


  msg->len = sizeof(struct collect_view_data_msg) / sizeof(uint16_t);
  msg->clock = clock_time();
#if TIMESYNCH_CONF_ENABLED
  msg->timesynch_time = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
  msg->timesynch_time = 0;
#endif /* TIMESYNCH_CONF_ENABLED */

  energest_flush();

  cpu = energest_type_time(ENERGEST_TYPE_CPU) - last_cpu;
  lpm = energest_type_time(ENERGEST_TYPE_LPM) - last_lpm;
  transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_transmit;
  listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last_listen;

  /* Make sure that the values are within 16 bits. If they are larger,
     we scale them down to fit into 16 bits. */
  while(cpu >= 65536ul || lpm >= 65536ul ||
	transmit >= 65536ul || listen >= 65536ul) {
    cpu /= 2;
    lpm /= 2;
    transmit /= 2;
    listen /= 2;
  }

  msg->cpu = cpu;
  msg->lpm = lpm;
  msg->transmit = transmit;
  msg->listen = listen;

  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

  memcpy(&msg->parent, &parent->u8[RIMEADDR_SIZE - 2], 2);
  msg->parent_etx = parent_etx;
  msg->current_rtmetric = current_rtmetric;
  msg->num_neighbors = num_neighbors;
  msg->beacon_interval = beacon_interval;

  memset(msg->sensors, 0, sizeof(msg->sensors));
  collect_view_arch_read_sensors(msg);
}
/*---------------------------------------------------------------------------*/
