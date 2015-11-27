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
 * $Id: tdma_mac.c,v 1.8 2010/06/14 19:19:17 adamdunkels Exp $
 *      tdma.c,v 0.1 2015/11/02 14:57, Mao Ngo SUTD 
 */

#include "contiki.h"
#include "net/mac/tdma.h"
#include "net/packetbuf.h"
#include "net/ipv4/uip-fw.h"
#include "sys/rtimer.h"
#include "net/rime/rime.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "dev/leds.h"
#include "node-id.h"

#include <string.h>
#include <stdio.h>

#include "net/netstack.h"

#define DEBUG1 1
#define DEBUG 1
#if DEBUG
#define DLEDS_ON(x) leds_on(x)
#define DLEDS_OFF(x) leds_off(x)
#define DLEDS_TOGGLE(x) leds_toggle(x)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define DLEDS_ON(x)
#define DLEDS_OFF(x)
#define DLEDS_TOGGLE(x)
#define PRINTF(...)
#endif

/* TDMA configuration */
#define NR_SLOTS 3
#define SLOT_LENGTH (RTIMER_SECOND/3)
#define GUARD_PERIOD (RTIMER_SECOND/12)

#define MY_SLOT (*id % NR_SLOTS)
#define PERIOD_LENGTH RTIMER_SECOND

/* Buffers */
#define NUM_PACKETS 8
uint8_t lastqueued = 0;
uint8_t nextsend = 0;
uint8_t freeslot = 0;
struct queuebuf* data[NUM_PACKETS];
int id[NUM_PACKETS];

static struct rtimer rtimer;
uint8_t timer_on = 0;

static const struct radio_driver *radio;
static void (* receiver_callback)(const struct mac_driver *);
static int id_counter = 0;
static int sent_counter = 0;

/*---------------------------------------------------------------------------*/
static char
transmitter(struct rtimer *t, void *ptr)
{
  int r;
  rtimer_clock_t now, rest, period_start, slot_start;

  /* Calculate slot start time */
  now = RTIMER_NOW();
  rest = now % PERIOD_LENGTH;
  period_start = now - rest;
  slot_start = period_start + MY_SLOT*SLOT_LENGTH;

  /* Check if we are inside our slot */
  if(now < slot_start ||
      now > slot_start + SLOT_LENGTH - GUARD_PERIOD) {
    PRINTF("TIMER We are outside our slot: %u != [%u,%u]\n", now, slot_start, slot_start + SLOT_LENGTH);
    while(now > slot_start + SLOT_LENGTH - GUARD_PERIOD) {
      slot_start += PERIOD_LENGTH;
    }

    PRINTF("TIMER Rescheduling until %u\n", slot_start);
    r = rtimer_set(&rtimer, slot_start, 1,
        (void (*)(struct rtimer *, void *))transmitter, NULL);
    if(r) {
      PRINTF("TIMER Error #1: %d\n", r);
    }

    return 1;
  }

  /* Transmit queued packets */
  while(nextsend != freeslot) {
    PRINTF("RADIO Transmitting packet #%i\n", id[nextsend]);
    if(!radio->send(queuebuf_dataptr(data[nextsend]),
		    queuebuf_datalen(data[nextsend]))) {
      sent_counter++;
      PRINTF("RADIO Transmit OK for #%i, total=%i\n", id[nextsend], sent_counter);
      DLEDS_TOGGLE(LEDS_GREEN);
    } else {
      PRINTF("RADIO Transmit failed for #%i, total=%i\n", id[nextsend], sent_counter);
      DLEDS_TOGGLE(LEDS_RED);
    }

    nextsend = (nextsend + 1) % NUM_PACKETS;

    /* Recalculate new slot */
    if(RTIMER_NOW() > slot_start + SLOT_LENGTH - GUARD_PERIOD) {
      PRINTF("TIMER No more time to transmit\n");
      break;
    }
  }

  /* Calculate time of our next slot */
  slot_start += PERIOD_LENGTH;
  PRINTF("TIMER Rescheduling until %u\n", slot_start);
  r = rtimer_set(&rtimer, slot_start, 1,
      (void (*)(struct rtimer *, void *))transmitter, NULL);
  if(r) {
    PRINTF("TIMER Error #2: %d\n", r);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
send(void)
{
  int r;
  id_counter++;

  /* Clean up already sent packets */
  while(lastqueued != nextsend) {
    PRINTF("BUFFER Cleaning up packet #%i\n", id[lastqueued]);
    queuebuf_free(data[lastqueued]);
    data[lastqueued] = NULL;

    lastqueued = (lastqueued + 1) % NUM_PACKETS;
  }

  if((freeslot + 1) % NUM_PACKETS == lastqueued) {
    PRINTF("BUFFER Buffer full, dropping packet #%i\n", (id_counter+1));
    return UIP_FW_DROPPED;
  }

  /* Allocate queue buf for packet */
  data[freeslot] = queuebuf_new_from_packetbuf();
  id[freeslot] = id_counter;
  if(data[freeslot] == NULL) {
    PRINTF("BUFFER Queuebuffer full, dropping packet #%i\n", id[freeslot]);
    return UIP_FW_DROPPED;
  }
  PRINTF("BUFFER Wrote packet #%i to buffer \n", id[freeslot]);

  freeslot = (freeslot + 1) % NUM_PACKETS;

  if(!timer_on) {
    PRINTF("TIMER Starting timer\n");
    r = rtimer_set(&rtimer, RTIMER_NOW() + RTIMER_SECOND, 1,
        (void (*)(struct rtimer *, void *))transmitter, NULL);
    if(r) {
      PRINTF("TIMER Error #3: %d\n", r);
    } else {
      timer_on = 1;
    }
  }

  return UIP_FW_OK; /* TODO Return what? */
}
/*---------------------------------------------------------------------------*/
static void
input(const struct radio_driver *d)
//input(void)
{
  receiver_callback(&tdma_driver);
}
/*---------------------------------------------------------------------------*/
static int
read(void)
{
  DLEDS_TOGGLE(LEDS_RED);
  int len;
  packetbuf_clear();
  len = radio->read(packetbuf_dataptr(), PACKETBUF_SIZE);
  packetbuf_set_datalen(len);
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
  return NETSTACK_RDC.on();
  DLEDS_TOGGLE(LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return NETSTACK_RDC.off(keep_radio_on);
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
init(const struct radio_driver *d)
{
  int i;
  for(i = 0; i < NUM_PACKETS; i++) {
    data[i] = NULL;
  }

  radio = d;
  if (DEBUG1) {
  //radio->set_receive_function(input);
  //radio->channel_check_interval();
  } else {
  //set_receive_function(input);
  }
  //DLEDS_TOGGLE(LEDS_ALL);

  radio->on();

  return &tdma_driver;
}

static unsigned short
channel_check_interval(void)
{
  if(NETSTACK_RDC.channel_check_interval) {
    return NETSTACK_RDC.channel_check_interval();
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
const struct mac_driver tdma_driver = {
    "TDMA MAC",
    init,
    send,
    read,
    on,
    off,
    channel_check_interval
    //set_receive_function,
};
