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
 * $Id: csma.c,v 1.8 2010/02/23 20:42:45 nifi Exp $
 */

/**
 * \file
 *         A MAC 
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#define CSMA_CONF_REXMIT 1

#include "net/mac/csma.h"
#include "net/rime/packetbuf.h"
#include "net/rime/queuebuf.h"
#include "net/rime/ctimer.h"

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

struct queued_packet {
  struct queued_packet *next;
  struct queuebuf *buf;
  struct ctimer retransmit_timer;
  mac_callback_t sent;
  void *cptr;
  uint8_t transmissions;
};

#define MAX_RETRANSMITS 4

#define MAX_QUEUED_PACKETS 8
MEMB(packet_memb, struct queued_packet, MAX_QUEUED_PACKETS);


/*---------------------------------------------------------------------------*/
#if CSMA_CONF_REXMIT
static void
free_packet(struct queued_packet *q)
{
  queuebuf_free(q->buf);
  memb_free(&packet_memb, q);
}

static void retransmit_packet(void *ptr);

static void
packet_sent(void *ptr, int status, int num_transmissions)
{
  struct queued_packet *q = ptr;
  clock_time_t time = 0;
  mac_callback_t sent;
  void *cptr;
  int num_tx;
  
  sent = q->sent;
  cptr = q->cptr;
  num_tx = q->transmissions;
  
  if(status != MAC_TX_OK) {
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
  
    time = NETSTACK_RDC.channel_check_interval();
    if(time == 0) {
      time = CLOCK_SECOND;
    }
    time = time + (random_rand() % (q->transmissions * 3 * time));
    
    if(q->transmissions - 1 < MAX_RETRANSMITS) {
      ctimer_set(&q->retransmit_timer, time,
                 retransmit_packet, q);
    } else {
      PRINTF("csma: drop after %d\n", q->transmissions);
      free_packet(q);
      mac_call_sent_callback(sent, cptr, status, num_tx);
    }
  } else {
    PRINTF("csma: rexmit ok %d\n", q->transmissions);
    free_packet(q);
    mac_call_sent_callback(sent, cptr, status, num_tx);
  }
}

static void
retransmit_packet(void *ptr)
{
  struct queued_packet *q = ptr;

  queuebuf_to_packetbuf(q->buf);
  q->transmissions++;
  NETSTACK_RDC.send(packet_sent, q);
}
/*---------------------------------------------------------------------------*/
static void
sent_packet_1(void *ptr, int status, int num_transmissions)
{
  struct queued_packet *q = ptr;
  clock_time_t time;
  rimeaddr_t receiver;
  
  rimeaddr_copy(&receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));

  if(rimeaddr_cmp(&receiver, &rimeaddr_null)) {
    PRINTF("broadcast/");
  }
  if(status != MAC_TX_OK) {
    switch(status) {
    case MAC_TX_COLLISION:
      PRINTF("csma: collision\n");
      break; 
    case MAC_TX_NOACK:
      PRINTF("csma: noack\n");
      break;
   default:
      PRINTF("csma: err %d\n", status);
    }
  } else {
    PRINTF("csma: ok\n");
  }
  
  /* Check if we saw a collission, and if we have a queuebuf with the
     packet available. Only retransmit unicast packets. Retransmit
     only once, for now. */
  if((status == MAC_TX_COLLISION || status == MAC_TX_NOACK) &&
     !rimeaddr_cmp(&receiver, &rimeaddr_null)) {
    /* If the packet couldn't be sent because of a collision or the
         lack of an ACK, we let the other transmissions get through
         before we try again. */
    time = NETSTACK_RDC.channel_check_interval();
    if(time == 0) {
      time = CLOCK_SECOND;
    }
    time = time + (random_rand() % (3 * time));

    ctimer_set(&q->retransmit_timer, time,
	       retransmit_packet, q);
  } else {
    mac_callback_t sent;
    void *cptr;
    int num_tx;
    
    sent = q->sent;
    cptr = q->cptr;
    num_tx = q->transmissions;
    free_packet(q);
    mac_call_sent_callback(sent, cptr, status, num_tx);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct queued_packet *q;

  /* Remember packet for later. */
  q = memb_alloc(&packet_memb);
  if(q != NULL) {
    q->buf = queuebuf_new_from_packetbuf();
    if(q != NULL) {
      q->transmissions = 1;
      q->sent = sent;
      q->cptr = ptr;
      NETSTACK_RDC.send(sent_packet_1, q);
      return;
    }
    memb_free(&packet_memb, q);
  }
  PRINTF("csma: could not allocate queuebuf, will drop if collision or noack\n");
  NETSTACK_RDC.send(sent, ptr);
}
#else /* CSMA_CONF_REXMIT */
static void
send_packet(mac_callback_t sent, void *ptr)
{
  NETSTACK_RDC.send(NULL, NULL);
}
#endif /* CSMA_CONF_REXMIT */
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
