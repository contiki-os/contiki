/** \addtogroup servreghack
 * @{ */

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
 *         Implementation of the servreg-hack application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "net/uip.h"

#include "net/uip-ds6.h"

#include "servreg-hack.h"

#include <stdio.h>

struct servreg_hack_registration {
  struct servreg_hack_registration *next;

  struct timer timer;
  uip_ipaddr_t addr;
  servreg_hack_id_t id;
  uint8_t seqno;
};


#define MAX_REGISTRATIONS 16

LIST(others_services);
LIST(own_services);

MEMB(registrations, struct servreg_hack_registration, MAX_REGISTRATIONS);

PROCESS(servreg_hack_process, "Service regstry hack");

#define PERIOD_TIME 120 * CLOCK_SECOND

#define NEW_REG_TIME 10 * CLOCK_SECOND

#define MAX_BUFSIZE 2 + 80

#define UDP_PORT 61616

#define LIFETIME 10 * 60 * CLOCK_SECOND

#define SEQNO_LT(a, b) ((signed char)((a) - (b)) < 0)

static struct etimer sendtimer;

static uint8_t started = 0;

/*---------------------------------------------------------------------------*/
/* Go through the list of registrations and remove those that are too
   old. */
static void
purge_registrations(void)
{
  struct servreg_hack_registration *t;

  for(t = list_head(own_services);
      t != NULL;
      t = list_item_next(t)) {
    if(timer_expired(&t->timer)) {
      t->seqno++;
      timer_set(&t->timer, LIFETIME / 2);
    }
  }

  for(t = list_head(others_services);
      t != NULL;
      t = list_item_next(t)) {
    if(timer_expired(&t->timer)) {
      list_remove(others_services, t);
      memb_free(&registrations, t);
      t = list_head(others_services);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
servreg_hack_init(void)
{
  if(started == 0) {
    list_init(others_services);
    list_init(own_services);
    memb_init(&registrations);

    process_start(&servreg_hack_process, NULL);
    started = 1;
  }
}
/*---------------------------------------------------------------------------*/
void
servreg_hack_register(servreg_hack_id_t id, const uip_ipaddr_t *addr)
{
  servreg_hack_item_t *t;
  struct servreg_hack_registration *r;
  /* Walk through list, see if we already have a service ID
     registered. If not, allocate a new registration and put it on our
     list. If we cannot allocate a service registration, we reuse one
     from the service registrations made by others. */

  servreg_hack_init();

  for(t = list_head(own_services);
      t != NULL;
      t = list_item_next(t)) {
    if(servreg_hack_item_id(t) == id) {
      return;
    }
  }

  r = memb_alloc(&registrations);
  if(r == NULL) {
    printf("servreg_hack_register: error, could not allocate memory, should reclaim another registration but this has not been implemented yet.\n");
    return;
  }
  r->id = id;
  r->seqno = 1;
  uip_ipaddr_copy(&r->addr, addr);
  timer_set(&r->timer, LIFETIME / 2);
  list_push(own_services, r);


  PROCESS_CONTEXT_BEGIN(&servreg_hack_process);
  etimer_set(&sendtimer, random_rand() % (NEW_REG_TIME));
  PROCESS_CONTEXT_END(&servreg_hack_process);

}
/*---------------------------------------------------------------------------*/
servreg_hack_item_t *
servreg_hack_list_head(void)
{
  purge_registrations();
  return list_head(others_services);
}
/*---------------------------------------------------------------------------*/
servreg_hack_id_t
servreg_hack_item_id(servreg_hack_item_t *item)
{
  return ((struct servreg_hack_registration *)item)->id;
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
servreg_hack_item_address(servreg_hack_item_t *item)
{
  return &((struct servreg_hack_registration *)item)->addr;
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
servreg_hack_lookup(servreg_hack_id_t id)
{
  servreg_hack_item_t *t;

  servreg_hack_init();

  purge_registrations();

  for(t = servreg_hack_list_head(); t != NULL; t = list_item_next(t)) {
    if(servreg_hack_item_id(t) == id) {
      return servreg_hack_item_address(t);
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_reg(const uip_ipaddr_t *owner, servreg_hack_id_t id, uint8_t seqno)
{
  servreg_hack_item_t *t;
  struct servreg_hack_registration *r;

  /* Walk through list, see if we already have a service ID
     registered. If so, we do different things depending on the seqno
     of the update: if the seqno is older than what we have, we
     discard the incoming registration. If the seqno is newer than
     what we have, we reset the lifetime timer of the current
     registration.

     If we did not have the service registered already, we allocate a
     new registration and put it on our list. If we cannot allocate a
     service registration, we discard the incoming registration (for
     now - we might later choose to discard the oldest registration
     that we have). */

  for(t = servreg_hack_list_head();
      t != NULL;
      t = list_item_next(t)) {
    if(servreg_hack_item_id(t) == id) {
      r = t;
      if(SEQNO_LT(r->seqno, seqno)) {
        r->seqno = seqno;
        timer_set(&r->timer, LIFETIME);

        /* Put item first on list, so that subsequent lookups will
           find this one. */
        list_remove(others_services, r);
        list_push(others_services, r);
      }
      return;
    }
  }

  r = memb_alloc(&registrations);
  if(r == NULL) {
    printf("servreg_hack_register: error, could not allocate memory, should reclaim another registration but this has not been implemented yet.\n");
    return;
  }
  r->id = id;
  r->seqno = 1;
  uip_ipaddr_copy(&r->addr, owner);
  timer_set(&r->timer, LIFETIME);
  list_add(others_services, r);
}
/*---------------------------------------------------------------------------*/
/*
 * The structure of UDP messages:
 *
 *  +-------------------+-------------------+
 *  |  Numregs (1 byte) |   Flags (1 byte)  |
 *  +-------------------+-------------------+-------------------+
 *  | IP addr (16 bytes)| 3 regs (3 bytes)  |  Seqno (1 byte)   |
 *  +-------------------+-------------------+-------------------+
 *  | IP addr (16 bytes)| 3 regs (3 bytes)  |  Seqno (1 byte)   |
 *  +-------------------+-------------------+-------------------+
 *  |        ...        |       ...         |       ...         |
 *  +-------------------+-------------------+-------------------+
 */

#define MSG_NUMREGS_OFFSET   0
#define MSG_FLAGS_OFFSET     1
#define MSG_ADDRS_OFFSET     2

#define MSG_IPADDR_SUBOFFSET 0
#define MSG_REGS_SUBOFFSET   16
#define MSG_SEQNO_SUBOFFSET  19

#define MSG_ADDRS_LEN        20

/*---------------------------------------------------------------------------*/
static void
send_udp_packet(struct uip_udp_conn *conn)
{
  int numregs;
  uint8_t buf[MAX_BUFSIZE];
  int bufptr;
  servreg_hack_item_t *t;

  buf[MSG_FLAGS_OFFSET]   = 0;

  numregs = 0;
  bufptr = MSG_ADDRS_OFFSET;
  
  for(t = list_head(own_services);
      (bufptr + MSG_ADDRS_LEN <= MAX_BUFSIZE) && t != NULL;
      t = list_item_next(t)) {

    uip_ipaddr_copy((uip_ipaddr_t *)&buf[bufptr + MSG_IPADDR_SUBOFFSET],
                    servreg_hack_item_address(t));
    buf[bufptr + MSG_REGS_SUBOFFSET] =
      servreg_hack_item_id(t);
    buf[bufptr + MSG_REGS_SUBOFFSET + 1] =
      buf[bufptr + MSG_REGS_SUBOFFSET + 2] = 0;
    buf[bufptr + MSG_SEQNO_SUBOFFSET] = ((struct servreg_hack_registration *)t)->seqno;

    bufptr += MSG_ADDRS_LEN;
    ++numregs;
  }

  for(t = servreg_hack_list_head();
      (bufptr + MSG_ADDRS_LEN <= MAX_BUFSIZE) && t != NULL;
      t = list_item_next(t)) {
    uip_ipaddr_copy((uip_ipaddr_t *)&buf[bufptr + MSG_IPADDR_SUBOFFSET],
                    servreg_hack_item_address(t));
    buf[bufptr + MSG_REGS_SUBOFFSET] =
      servreg_hack_item_id(t);
    buf[bufptr + MSG_REGS_SUBOFFSET + 1] =
      buf[bufptr + MSG_REGS_SUBOFFSET + 2] = 0;
    buf[bufptr + MSG_SEQNO_SUBOFFSET] = ((struct servreg_hack_registration *)t)->seqno;

    bufptr += MSG_ADDRS_LEN;
    ++numregs;
  }
  /*  printf("send_udp_packet numregs %d\n", numregs);*/
  buf[MSG_NUMREGS_OFFSET] = numregs;

  if(numregs > 0) {
    /*    printf("Sending buffer len %d\n", bufptr);*/
    uip_udp_packet_send(conn, buf, bufptr);
  }
}
/*---------------------------------------------------------------------------*/
static void
parse_incoming_packet(const uint8_t *buf, int len)
{
  int numregs;
  int flags;
  int i;
  int bufptr;

  numregs = buf[MSG_NUMREGS_OFFSET];
  flags   = buf[MSG_FLAGS_OFFSET];

  /*  printf("parse_incoming_packet Numregs %d flags %d\n", numregs, flags);*/

  bufptr = MSG_ADDRS_OFFSET;
  for(i = 0; i < numregs; ++i) {
    handle_incoming_reg((uip_ipaddr_t *)&buf[bufptr + MSG_IPADDR_SUBOFFSET],
                        buf[bufptr + MSG_REGS_SUBOFFSET],
                        buf[bufptr + MSG_SEQNO_SUBOFFSET]);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(servreg_hack_process, ev, data)
{
  static struct etimer periodic;
  static struct uip_udp_conn *outconn, *inconn;
  PROCESS_BEGIN();

  /* Create outbound UDP connection. */
  outconn = udp_broadcast_new(UIP_HTONS(UDP_PORT), NULL);
  udp_bind(outconn, UIP_HTONS(UDP_PORT));

  /* Create inbound UDP connection. */
  inconn = udp_new(NULL, UIP_HTONS(UDP_PORT), NULL);
  udp_bind(inconn, UIP_HTONS(UDP_PORT));

  etimer_set(&periodic, PERIOD_TIME);
  etimer_set(&sendtimer, random_rand() % (PERIOD_TIME));
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER && data == &periodic) {
      etimer_reset(&periodic);
      etimer_set(&sendtimer, random_rand() % (PERIOD_TIME));
    } else if(ev == PROCESS_EVENT_TIMER && data == &sendtimer) {
      send_udp_packet(outconn);
    } else if(ev == tcpip_event) {
      parse_incoming_packet(uip_appdata, uip_datalen());
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
