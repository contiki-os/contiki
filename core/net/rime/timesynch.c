/**
 * \addtogroup timesynch
 * @{
 */


/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         A simple time synchronization mechanism
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/rime.h"
#include "net/rime/timesynch.h"
#include <string.h>

#if TIMESYNCH_CONF_ENABLED
static int authority_level;
static rtimer_clock_t offset;

#define TIMESYNCH_CHANNEL  7

struct timesynch_msg {
  uint8_t authority_level;
  uint8_t dummy;
  uint16_t authority_offset;
  uint16_t clock_fine;
  clock_time_t clock_time;
  uint32_t seconds;
  /* We need some padding so that the radio has time to update the
     timestamp at the end of the packet, after the transmission has
     started. */
  uint8_t padding[16];

  /* The timestamp must be the last two bytes. */
  uint16_t timestamp;
};

PROCESS(timesynch_process, "Timesynch process");

#define MIN_INTERVAL CLOCK_SECOND * 8
#define MAX_INTERVAL CLOCK_SECOND * 60 * 5
/*---------------------------------------------------------------------------*/
int
timesynch_authority_level(void)
{
  return authority_level;
}
/*---------------------------------------------------------------------------*/
void
timesynch_set_authority_level(int level)
{
  int old_level = authority_level;

  authority_level = level;

  if(old_level != authority_level) {
    /* Restart the timesynch process to restart with a low
       transmission interval. */
    process_exit(&timesynch_process);
    process_start(&timesynch_process, NULL);
  }
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_time(void)
{
  return RTIMER_NOW() + offset;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_time_to_rtimer(rtimer_clock_t synched_time)
{
  return synched_time - offset;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_rtimer_to_time(rtimer_clock_t rtimer_time)
{
  return rtimer_time + offset;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_offset(void)
{
  return offset;
}
/*---------------------------------------------------------------------------*/
static void
adjust_offset(rtimer_clock_t authoritative_time, rtimer_clock_t local_time)
{
  offset = authoritative_time - local_time;
}
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  struct timesynch_msg msg;

  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));

  /* We check the authority level of the sender of the incoming
       packet. If the sending node has a lower authority level than we
       have, we synchronize to the time of the sending node and set our
       own authority level to be one more than the sending node. */
  if(msg.authority_level < authority_level) {
    adjust_offset(msg.timestamp + msg.authority_offset,
                  packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP));
    timesynch_set_authority_level(msg.authority_level + 1);
  }
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(timesynch_process, ev, data)
{
  static struct etimer sendtimer, intervaltimer;
  static clock_time_t interval;
  struct timesynch_msg msg;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, TIMESYNCH_CHANNEL, &broadcast_call);

  interval = MIN_INTERVAL;

  while(1) {
    etimer_set(&intervaltimer, interval);
    etimer_set(&sendtimer, random_rand() % interval);

    PROCESS_WAIT_UNTIL(etimer_expired(&sendtimer));

    msg.authority_level = authority_level;
    msg.dummy = 0;
    msg.authority_offset = offset;
    msg.clock_fine = clock_fine();
    msg.clock_time = clock_time();
    msg.seconds = clock_seconds();
    msg.timestamp = 0;
    packetbuf_copyfrom(&msg, sizeof(msg));
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
                       PACKETBUF_ATTR_PACKET_TYPE_TIMESTAMP);
    broadcast_send(&broadcast);

    PROCESS_WAIT_UNTIL(etimer_expired(&intervaltimer));
    interval *= 2;
    if(interval >= MAX_INTERVAL) {
      interval = MAX_INTERVAL;
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
timesynch_init(void)
{
  process_start(&timesynch_process, NULL);
}
/*---------------------------------------------------------------------------*/
#endif /* TIMESYNCH_CONF_ENABLED */
/** @} */
