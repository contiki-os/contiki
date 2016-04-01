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
 *         A Carrier Sense Multiple Access (CSMA) MAC layer
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/mac/csma.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"

#include "sys/ctimer.h"
#include "sys/clock.h"

#include "lib/random.h"

#include "net/netstack.h"

#include "lib/list.h"
#include "lib/memb.h"

#include <string.h>

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

#ifndef CSMA_MAX_BACKOFF_EXPONENT
#ifdef CSMA_CONF_MAX_BACKOFF_EXPONENT
#define CSMA_MAX_BACKOFF_EXPONENT CSMA_CONF_MAX_BACKOFF_EXPONENT
#else
#define CSMA_MAX_BACKOFF_EXPONENT 3
#endif /* CSMA_CONF_MAX_BACKOFF_EXPONENT */
#endif /* CSMA_MAX_BACKOFF_EXPONENT */

#ifndef CSMA_MAX_MAC_TRANSMISSIONS
#ifdef CSMA_CONF_MAX_MAC_TRANSMISSIONS
#define CSMA_MAX_MAC_TRANSMISSIONS CSMA_CONF_MAX_MAC_TRANSMISSIONS
#else
#define CSMA_MAX_MAC_TRANSMISSIONS 3
#endif /* CSMA_CONF_MAX_MAC_TRANSMISSIONS */
#endif /* CSMA_MAX_MAC_TRANSMISSIONS */

#if CSMA_MAX_MAC_TRANSMISSIONS < 1
#error CSMA_CONF_MAX_MAC_TRANSMISSIONS must be at least 1.
#error Change CSMA_CONF_MAX_MAC_TRANSMISSIONS in contiki-conf.h or in your Makefile.
#endif /* CSMA_CONF_MAX_MAC_TRANSMISSIONS < 1 */

/* Packet metadata */
struct qbuf_metadata {
  mac_callback_t sent;
  void *cptr;
  uint8_t max_transmissions;
};

/* Every neighbor has its own packet queue */
struct neighbor_queue {
  struct neighbor_queue *next;
  linkaddr_t addr;
  struct ctimer transmit_timer;
  uint8_t transmissions;
  uint8_t collisions, deferrals;
  LIST_STRUCT(queued_packet_list);
};

/* The maximum number of co-existing neighbor queues */
#ifdef CSMA_CONF_MAX_NEIGHBOR_QUEUES
#define CSMA_MAX_NEIGHBOR_QUEUES CSMA_CONF_MAX_NEIGHBOR_QUEUES
#else
#define CSMA_MAX_NEIGHBOR_QUEUES 2
#endif /* CSMA_CONF_MAX_NEIGHBOR_QUEUES */

/* The maximum number of pending packet per neighbor */
#ifdef CSMA_CONF_MAX_PACKET_PER_NEIGHBOR
#define CSMA_MAX_PACKET_PER_NEIGHBOR CSMA_CONF_MAX_PACKET_PER_NEIGHBOR
#else
#define CSMA_MAX_PACKET_PER_NEIGHBOR MAX_QUEUED_PACKETS
#endif /* CSMA_CONF_MAX_PACKET_PER_NEIGHBOR */

#define MAX_QUEUED_PACKETS QUEUEBUF_NUM
MEMB(neighbor_memb, struct neighbor_queue, CSMA_MAX_NEIGHBOR_QUEUES);
MEMB(packet_memb, struct rdc_buf_list, MAX_QUEUED_PACKETS);
MEMB(metadata_memb, struct qbuf_metadata, MAX_QUEUED_PACKETS);
LIST(neighbor_list);

static void packet_sent(void *ptr, int status, int num_transmissions);
static void transmit_packet_list(void *ptr);
/*---------------------------------------------------------------------------*/
static struct neighbor_queue *
neighbor_queue_from_addr(const linkaddr_t *addr)
{
  struct neighbor_queue *n = list_head(neighbor_list);
  while(n != NULL) {
    if(linkaddr_cmp(&n->addr, addr)) {
      return n;
    }
    n = list_item_next(n);
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static clock_time_t
default_timebase(void)
{
  clock_time_t time;
  /* The retransmission time must be proportional to the channel
     check interval of the underlying radio duty cycling layer. */
  time = NETSTACK_RDC.channel_check_interval();

  /* If the radio duty cycle has no channel check interval (i.e., it
     does not turn the radio off), we make the retransmission time
     proportional to the configured MAC channel check rate. */
  if(time == 0) {
    time = (CLOCK_SECOND / 3125) ? (CLOCK_SECOND / 3125) : 1;
  }
  return time;
}
/*---------------------------------------------------------------------------*/
static void
transmit_packet_list(void *ptr)
{
  struct neighbor_queue *n = ptr;
  if(n) {
    struct rdc_buf_list *q = list_head(n->queued_packet_list);
    if(q != NULL) {
      PRINTF("csma: preparing number %d %p, queue len %d\n", n->transmissions, q,
          list_length(n->queued_packet_list));
      /* Send packets in the neighbor's list */
      NETSTACK_RDC.send_list(packet_sent, n, q);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
free_packet(struct neighbor_queue *n, struct rdc_buf_list *p, int status)
{
  clock_time_t tx_delay;

  if(p != NULL) {
    /* Remove packet from list and deallocate */
    list_remove(n->queued_packet_list, p);

    queuebuf_free(p->buf);
    memb_free(&metadata_memb, p->ptr);
    memb_free(&packet_memb, p);
    PRINTF("csma: free_queued_packet, queue length %d, free packets %d\n",
           list_length(n->queued_packet_list), memb_numfree(&packet_memb));
    if(list_head(n->queued_packet_list) != NULL) {
      /* There is a next packet. We reset current tx information */
      n->transmissions = 0;
      n->collisions = 0;
      n->deferrals = 0;
      /* Set a timer for next transmissions */
      tx_delay = (status == MAC_TX_OK) ? 0 : default_timebase();
      ctimer_set(&n->transmit_timer, tx_delay, transmit_packet_list, n);
    } else {
      /* This was the last packet in the queue, we free the neighbor */
      ctimer_stop(&n->transmit_timer);
      list_remove(neighbor_list, n);
      memb_free(&neighbor_memb, n);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
tx_done(int status, struct rdc_buf_list *q, struct neighbor_queue *n)
{
  mac_callback_t sent;
  struct qbuf_metadata *metadata;
  void *cptr;

  metadata = (struct qbuf_metadata *)q->ptr;
  sent = metadata->sent;
  cptr = metadata->cptr;

  switch(status) {
  case MAC_TX_OK:
    PRINTF("csma: rexmit ok %d\n", n->transmissions);
    break;
  case MAC_TX_COLLISION:
  case MAC_TX_NOACK:
    PRINTF("csma: drop with status %d after %d transmissions, %d collisions\n",
                 status, n->transmissions, n->collisions);
    break;
  default:
    PRINTF("csma: rexmit failed %d: %d\n", n->transmissions, status);
    break;
  }

  free_packet(n, q, status);
  mac_call_sent_callback(sent, cptr, status, n->transmissions);
}
/*---------------------------------------------------------------------------*/
static void
rexmit(struct rdc_buf_list *q, struct neighbor_queue *n)
{
  clock_time_t time;
  int backoff_exponent;
  int backoff_transmissions;

  time = default_timebase();
  backoff_exponent = n->collisions;

  /* Proceed to exponentiation. */
  backoff_transmissions = 1 << backoff_exponent;

  /* Pick a time for next transmission, within the interval:
         * [time, time + 2^backoff_exponent * time[ */
  time = time + (random_rand() % (backoff_transmissions * time));

  PRINTF("csma: retransmitting with time %lu %p\n", time, q);
  ctimer_set(&n->transmit_timer, time, transmit_packet_list, n);
  /* This is needed to correctly attribute energy that we spent
     transmitting this packet. */
  queuebuf_update_attr_from_packetbuf(q->buf);
}
/*---------------------------------------------------------------------------*/
static void
collision(struct rdc_buf_list *q, struct neighbor_queue *n,
          int num_transmissions)
{
  struct qbuf_metadata *metadata;

  metadata = (struct qbuf_metadata *)q->ptr;

  n->collisions += num_transmissions;

  if(n->collisions > CSMA_MAX_BACKOFF_EXPONENT) {
    n->collisions = 0;
    n->transmissions += num_transmissions;
  }

  if(n->transmissions >= metadata->max_transmissions) {
    tx_done(MAC_TX_COLLISION, q, n);
  } else {
    PRINTF("csma: rexmit collision %d\n", n->transmissions);
    rexmit(q, n);
  }
}
/*---------------------------------------------------------------------------*/
static void
noack(struct rdc_buf_list *q, struct neighbor_queue *n, int num_transmissions)
{
  struct qbuf_metadata *metadata;

  metadata = (struct qbuf_metadata *)q->ptr;

  n->transmissions += num_transmissions;
  n->collisions = 0;

  if(n->transmissions >= metadata->max_transmissions) {
    tx_done(MAC_TX_NOACK, q, n);
  } else {
    PRINTF("csma: rexmit noack %d\n", n->transmissions);
    rexmit(q, n);
  }
}
/*---------------------------------------------------------------------------*/
static void
tx_ok(struct rdc_buf_list *q, struct neighbor_queue *n, int num_transmissions)
{
  n->transmissions += num_transmissions;
  tx_done(MAC_TX_OK, q, n);
}
/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_transmissions)
{
  struct neighbor_queue *n;
  struct rdc_buf_list *q;

  n = ptr;
  if(n == NULL) {
    return;
  }

  /* Find out what packet this callback refers to */
  for(q = list_head(n->queued_packet_list);
      q != NULL; q = list_item_next(q)) {
    if(queuebuf_attr(q->buf, PACKETBUF_ATTR_MAC_SEQNO) ==
       packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO)) {
      break;
    }
  }

  if(q == NULL) {
    PRINTF("csma: seqno %d not found\n",
           packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
    return;
  } else if(q->ptr == NULL) {
    PRINTF("csma: no metadata\n");
    return;
  }

  switch(status) {
  case MAC_TX_OK:
    tx_ok(q, n, num_transmissions);
    break;
  case MAC_TX_NOACK:
    noack(q, n, num_transmissions);
    break;
  case MAC_TX_COLLISION:
    collision(q, n, num_transmissions);
    break;
  case MAC_TX_DEFERRED:
    n->deferrals += num_transmissions;
    break;
  default:
    tx_done(status, q, n);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct rdc_buf_list *q;
  struct neighbor_queue *n;
  static uint8_t initialized = 0;
  static uint16_t seqno;
  const linkaddr_t *addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  if(!initialized) {
    initialized = 1;
    /* Initialize the sequence number to a random value as per 802.15.4. */
    seqno = random_rand();
  }

  if(seqno == 0) {
    /* PACKETBUF_ATTR_MAC_SEQNO cannot be zero, due to a pecuilarity
       in framer-802154.c. */
    seqno++;
  }
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, seqno++);

  /* Look for the neighbor entry */
  n = neighbor_queue_from_addr(addr);
  if(n == NULL) {
    /* Allocate a new neighbor entry */
    n = memb_alloc(&neighbor_memb);
    if(n != NULL) {
      /* Init neighbor entry */
      linkaddr_copy(&n->addr, addr);
      n->transmissions = 0;
      n->collisions = 0;
      n->deferrals = 0;
      /* Init packet list for this neighbor */
      LIST_STRUCT_INIT(n, queued_packet_list);
      /* Add neighbor to the list */
      list_add(neighbor_list, n);
    }
  }

  if(n != NULL) {
    /* Add packet to the neighbor's queue */
    if(list_length(n->queued_packet_list) < CSMA_MAX_PACKET_PER_NEIGHBOR) {
      q = memb_alloc(&packet_memb);
      if(q != NULL) {
        q->ptr = memb_alloc(&metadata_memb);
        if(q->ptr != NULL) {
          q->buf = queuebuf_new_from_packetbuf();
          if(q->buf != NULL) {
            struct qbuf_metadata *metadata = (struct qbuf_metadata *)q->ptr;
            /* Neighbor and packet successfully allocated */
            if(packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS) == 0) {
              /* Use default configuration for max transmissions */
              metadata->max_transmissions = CSMA_MAX_MAC_TRANSMISSIONS;
            } else {
              metadata->max_transmissions =
                packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS);
            }
            metadata->sent = sent;
            metadata->cptr = ptr;
#if PACKETBUF_WITH_PACKET_TYPE
            if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
               PACKETBUF_ATTR_PACKET_TYPE_ACK) {
              list_push(n->queued_packet_list, q);
            } else
#endif
            {
              list_add(n->queued_packet_list, q);
            }

            PRINTF("csma: send_packet, queue length %d, free packets %d\n",
                   list_length(n->queued_packet_list), memb_numfree(&packet_memb));
            /* If q is the first packet in the neighbor's queue, send asap */
            if(list_head(n->queued_packet_list) == q) {
              ctimer_set(&n->transmit_timer, 0, transmit_packet_list, n);
            }
            return;
          }
          memb_free(&metadata_memb, q->ptr);
          PRINTF("csma: could not allocate queuebuf, dropping packet\n");
        }
        memb_free(&packet_memb, q);
        PRINTF("csma: could not allocate queuebuf, dropping packet\n");
      }
      /* The packet allocation failed. Remove and free neighbor entry if empty. */
      if(list_length(n->queued_packet_list) == 0) {
        list_remove(neighbor_list, n);
        memb_free(&neighbor_memb, n);
      }
    } else {
      PRINTF("csma: Neighbor queue full\n");
    }
    PRINTF("csma: could not allocate packet, dropping packet\n");
  } else {
    PRINTF("csma: could not allocate neighbor, dropping packet\n");
  }
  mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 1);
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  NETSTACK_LLSEC.input();
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
  memb_init(&metadata_memb);
  memb_init(&neighbor_memb);
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
