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
 * $Id: csma.c,v 1.19 2010/06/16 10:08:30 nifi Exp $
 */

/**
 * \file
 *         A Carrier Sense Multiple Access (CSMA) MAC layer
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/mac/csma.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"

#include "sys/ctimer.h"

#include "lib/random.h"

#include "net/netstack.h"

#include "lib/list.h"
#include "lib/memb.h"

#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

#ifndef CSMA_MAX_MAC_TRANSMISSIONS
#ifdef CSMA_CONF_MAX_MAC_TRANSMISSIONS
#define CSMA_MAX_MAC_TRANSMISSIONS CSMA_CONF_MAX_MAC_TRANSMISSIONS
#else
#define CSMA_MAX_MAC_TRANSMISSIONS 1
#endif /* CSMA_CONF_MAX_MAC_TRANSMISSIONS */
#endif /* CSMA_MAX_MAC_TRANSMISSIONS */

#if CSMA_MAX_MAC_TRANSMISSIONS < 1
#error CSMA_CONF_MAX_MAC_TRANSMISSIONS must be at least 1.
#error Change CSMA_CONF_MAX_MAC_TRANSMISSIONS in contiki-conf.h or in your Makefile.
#endif /* CSMA_CONF_MAX_MAC_TRANSMISSIONS < 1 */

struct queued_packet {
  struct queued_packet *next;
  struct queuebuf *buf;
  struct ctimer retransmit_timer;
  mac_callback_t sent;
  void *cptr;
  uint8_t transmissions, max_transmissions;
  uint8_t collisions, deferrals;
};

#define MAX_QUEUED_PACKETS 8
MEMB(packet_memb, struct queued_packet, MAX_QUEUED_PACKETS);

static void packet_sent(void *ptr, int status, int num_transmissions);

/*---------------------------------------------------------------------------*/
static void
retransmit_packet(void *ptr)
{
  struct queued_packet *q = ptr;

  queuebuf_to_packetbuf(q->buf);
  PRINTF("csma: resending number %d\n", q->transmissions);
  NETSTACK_RDC.send(packet_sent, q);
}
/*---------------------------------------------------------------------------*/
static void
free_packet(struct queued_packet *q)
{
  queuebuf_free(q->buf);
  memb_free(&packet_memb, q);
  ctimer_stop(&q->retransmit_timer);
}
/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_transmissions)
{
  struct queued_packet *q = ptr;
  clock_time_t time = 0;
  mac_callback_t sent;
  void *cptr;
  int num_tx;

  switch(status) {
  case MAC_TX_OK:
  case MAC_TX_NOACK:
    q->transmissions++;
    break;
  case MAC_TX_COLLISION:
    q->collisions++;
    break;
  case MAC_TX_DEFERRED:
    q->deferrals++;
    break;
  }

  sent = q->sent;
  cptr = q->cptr;
  num_tx = q->transmissions;
  
  if(status == MAC_TX_COLLISION ||
     status == MAC_TX_NOACK) {

    /* If the transmission was not performed because of a collision or
       noack, we must retransmit the packet. */
    
    switch(status) {
    case MAC_TX_COLLISION:
      PRINTF("csma: rexmit collision %d\n", q->transmissions);
      break; 
    case MAC_TX_NOACK:
      PRINTF("csma: rexmit noack %d\n", q->transmissions);
      break;
    default:
      PRINTF("csma: rexmit err %d, %d\n", status, q->transmissions);
    }

    /* The retransmission time must be proportional to the channel
       check interval of the underlying radio duty cycling layer. */
    time = NETSTACK_RDC.channel_check_interval();

    /* If the radio duty cycle has no channel check interval (i.e., it
       does not turn the radio off), we make the retransmission time
       proportional to the configured MAC channel check rate. */
    if(time == 0) {
      time = CLOCK_SECOND / MAC_CHANNEL_CHECK_RATE;
    }

    /* The retransmission time uses a linear backoff so that the
       interval between the transmissions increase with each
       retransmit. */
    time = time + (random_rand() % ((q->transmissions + 1) * 3 * time));
    
    if(q->transmissions < q->max_transmissions) {
      PRINTF("csma: retransmitting with time %lu\n", time);
      ctimer_set(&q->retransmit_timer, time,
                 retransmit_packet, q);
    } else {
      PRINTF("csma: drop after %d\n", q->transmissions);
      free_packet(q);
      mac_call_sent_callback(sent, cptr, status, num_tx);
    }
  } else if(status == MAC_TX_OK) {
    PRINTF("csma: rexmit ok %d\n", q->transmissions);
    free_packet(q);
    mac_call_sent_callback(sent, cptr, status, num_tx);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct queued_packet *q;
  static uint16_t seqno;
  
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, seqno++);
  
  /* Remember packet for later. */
  q = memb_alloc(&packet_memb);
  if(q != NULL) {
    q->buf = queuebuf_new_from_packetbuf();
    if(q != NULL) {
      if(packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS) == 0) {
        /* Use default configuration for max transmissions */
        q->max_transmissions = CSMA_MAX_MAC_TRANSMISSIONS;
      } else {
        q->max_transmissions =
          packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS);
      }
      q->transmissions = 0;
      q->collisions = 0;
      q->deferrals = 0;
      q->sent = sent;
      q->cptr = ptr;
      NETSTACK_RDC.send(packet_sent, q);
      return;
    }
    memb_free(&packet_memb, q);
  }
  PRINTF("csma: could not allocate queuebuf, will drop if collision or noack\n");
  NETSTACK_RDC.send(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  NETSTACK_NETWORK.input();
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return NETSTACK_RDC.on();
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return NETSTACK_RDC.off(keep_radio_on);
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  if(NETSTACK_RDC.channel_check_interval) {
    return NETSTACK_RDC.channel_check_interval();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  memb_init(&packet_memb);
}
/*---------------------------------------------------------------------------*/
const struct mac_driver csma_driver = {
  "CSMA",
  init,
  send_packet,
  input_packet,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
