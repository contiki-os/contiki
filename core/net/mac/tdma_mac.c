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
 * $Id: tdma_mac.c,v 1.1 2007/08/31 13:42:22 fros4943 Exp $
 */

#include "contiki.h"
#include "net/mac/tdma_mac.h"
#include "net/rime/rimebuf.h"
#include "net/uip-fw.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "node-id.h"

#include <string.h>
#include <stdio.h>

static const struct radio_driver *radio;
static void (* receiver_callback)(const struct mac_driver *);
static int id_counter = 0;

#define DEBUG 1
#if DEBUG
/*#include "printf2log.h"*/ /* XXX COOJA specifics */
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* TDMA configuration */
#define MAX_BUFFERED_PACKETS 6

#define NR_SLOTS 10
#define SLOT_LENGTH (CLOCK_SECOND) /* Slot length */
#define GUARD_PERIOD (CLOCK_SECOND / 2) /* Approx. packet transmission time */

#define MY_SLOT (node_id % NR_SLOTS)
#define PERIOD_LENGTH (SLOT_LENGTH*NR_SLOTS)

enum {
  EVENT_PACKET_BUFFERED
};

struct buf_packet {
  struct buf_packet *next;
  struct queuebuf *queued_packet;
  int id;
  struct etimer etimer;
};

LIST(buf_packet_list);
MEMB(buf_packet_mem, struct buf_packet, MAX_BUFFERED_PACKETS);

PROCESS(tdma_process, "TDMA process");

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tdma_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();

    if(ev == EVENT_PACKET_BUFFERED || ev == PROCESS_EVENT_TIMER) {
      struct buf_packet *packet;

      /* Locate packet */
      for(packet = list_head(buf_packet_list); packet != NULL; packet = packet->next) {
        if (etimer_expired(&packet->etimer)) {
          int ret, rest, period_start, my_next_slot;
          clock_time_t now;

          /* Calculate time of our next slot */
          now = clock_time();
          rest = now % PERIOD_LENGTH;
          period_start = now - rest;
          my_next_slot = period_start + MY_SLOT*SLOT_LENGTH;

          /* Transmit if inside our slot, with enough time left */
          if ((now == my_next_slot) ||
              (now > my_next_slot && (now - my_next_slot) < (SLOT_LENGTH-GUARD_PERIOD)))
          {
            PRINTF("SCHEDULE We can deliver right now (%i), slot = (%i <-> %i)\n", now, my_next_slot, my_next_slot + SLOT_LENGTH);
          }
          else
          {
            if (now > my_next_slot) {
              my_next_slot += PERIOD_LENGTH;
            }

            int distance = my_next_slot - now;
            etimer_set(&packet->etimer, distance);
            PRINTF("SCHEDULE Packet #%i rescheduled at %i to %i\n", packet->id, clock_time(), my_next_slot);
            continue;
          }

          /* Clear Rime buffer */
          rimebuf_clear();

          /* Restore packet from buffer */
          if(packet->queued_packet != NULL) {
            queuebuf_to_rimebuf(packet->queued_packet);
            queuebuf_free(packet->queued_packet);
          }

          /* Send packet */
          PRINTF("TDMA_PROC Packet #%i send starting at %i\n", packet->id, clock_time());
          ret = radio->send(rimebuf_hdrptr(), rimebuf_totlen());


          if (ret == UIP_FW_OK) {
            /* Remove transmitted packet */
            list_remove(buf_packet_list, packet);
            memb_free(&buf_packet_mem, packet);
            PRINTF("BUFFER Packet removed, memory freed\n");
            PRINTF("TDMA_PROC Packet #%i sent at %i\n", packet->id, clock_time());
          } else {
            /* Reschedule packet */
            etimer_set(&packet->etimer, SLOT_LENGTH);
            PRINTF("TDMA_PROC Packet #%i forced rescheduled\n", packet->id);
          }
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
send(void)
{
  struct buf_packet *packet;

  packet = (struct buf_packet *)memb_alloc(&buf_packet_mem);
  if (packet == NULL) {
    PRINTF("BUFFER No memory available, packet dropped\n");
    return UIP_FW_DROPPED;
  }

  /* Add to buffered packets list */
  list_add(buf_packet_list, packet);
  PRINTF("BUFFER Packet added, memory allocated\n");

  /* Store packet data */
  packet->queued_packet = queuebuf_new_from_rimebuf();
  if (packet->queued_packet == NULL) {
    list_remove(buf_packet_list, packet);
    memb_free(&buf_packet_mem, packet);
    PRINTF("BUFFER No queue memory available, packet dropped\n");
    return UIP_FW_DROPPED;
  }

  PROCESS_CONTEXT_BEGIN(&tdma_process);
  etimer_set(&packet->etimer, 0);
  PROCESS_CONTEXT_END(&tdma_process);

  /* TODO ID only used for debugging */
  packet->id = id_counter++;

  /* Clear Rime buffer */
  rimebuf_clear();

  process_post(&tdma_process, EVENT_PACKET_BUFFERED, NULL);

  return UIP_FW_OK; /* TODO Return what? */

}
/*---------------------------------------------------------------------------*/
static void
input(const struct radio_driver *d)
{
  receiver_callback(&tdma_mac_driver);
}
/*---------------------------------------------------------------------------*/
static int
read(void)
{
  int len;
  rimebuf_clear();
  len = radio->read(rimebuf_dataptr(), RIMEBUF_SIZE);
  rimebuf_set_datalen(len);
  return len;
}
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return radio->on();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  return radio->off();
}
/*---------------------------------------------------------------------------*/
void
tdma_mac_init(const struct radio_driver *d)
{
  memb_init(&buf_packet_mem);
  list_init(buf_packet_list);
  process_start(&tdma_process, NULL);

  radio = d;
  radio->set_receive_function(input);
  radio->on();
}
/*---------------------------------------------------------------------------*/
const struct mac_driver tdma_mac_driver = {
  send,
  read,
  set_receive_function,
  on,
  off,
};
