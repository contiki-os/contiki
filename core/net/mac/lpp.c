/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: lpp.c,v 1.40 2010/12/02 15:55:17 dak664 Exp $
 */

/**
 * \file
 *         Low power probing (R. Musaloiu-Elefteri, C. Liang,
 *         A. Terzis. Koala: Ultra-Low Power Data Retrieval in
 *         Wireless Sensor Networks, IPSN 2008)
 *
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 *
 * This is an implementation of the LPP (Low-Power Probing) MAC
 * protocol. LPP is a power-saving MAC protocol that works by sending
 * a probe packet each time the radio is turned on. If another node
 * wants to transmit a packet, it can do so after hearing the
 * probe. To send a packet, the sending node turns on its radio to
 * listen for probe packets.
 *
 */

#include "dev/leds.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime.h"
#include "net/netstack.h"
#include "net/mac/mac.h"
#include "net/mac/lpp.h"
#include "net/packetbuf.h"
#include "net/rime/announcement.h"
#include "sys/compower.h"
#include "net/mac/framer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define WITH_ACK_OPTIMIZATION         0
#define WITH_PROBE_AFTER_RECEPTION    0
#define WITH_PROBE_AFTER_TRANSMISSION 0
#define WITH_ENCOUNTER_OPTIMIZATION   0
#define WITH_ADAPTIVE_OFF_TIME        0
#define WITH_PENDING_BROADCAST        0
#define WITH_STREAMING                1

#define LISTEN_TIME (CLOCK_SECOND / 128)
#define OFF_TIME (CLOCK_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE - LISTEN_TIME)

#define PACKET_LIFETIME (LISTEN_TIME + OFF_TIME)
#define UNICAST_TIMEOUT	(1 * PACKET_LIFETIME + PACKET_LIFETIME / 2)
#define PROBE_AFTER_TRANSMISSION_TIME (LISTEN_TIME * 2)

#define LOWEST_OFF_TIME (CLOCK_SECOND / 8)

#define ENCOUNTER_LIFETIME (16 * OFF_TIME)

#ifdef QUEUEBUF_CONF_NUM
#define MAX_QUEUED_PACKETS QUEUEBUF_CONF_NUM / 2
#else /* QUEUEBUF_CONF_NUM */
#define MAX_QUEUED_PACKETS 4
#endif /* QUEUEBUF_CONF_NUM */


/* If CLOCK_SECOND is less than 4, we may end up with an OFF_TIME that
   is 0 which will make compilation fail due to a modulo operation in
   the code. To ensure that OFF_TIME is greater than zero, we use the
   construct below. */
#if OFF_TIME < 2
#undef OFF_TIME
#define OFF_TIME 2
#endif

struct announcement_data {
  uint16_t id;
  uint16_t value;
};

#define ANNOUNCEMENT_MSG_HEADERLEN 2
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[];
};

#define LPP_PROBE_HEADERLEN 2

#define TYPE_PROBE        1
#define TYPE_DATA         2
struct lpp_hdr {
  uint16_t type;
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

static uint8_t lpp_is_on;

static struct compower_activity current_packet;

static struct pt dutycycle_pt;
static struct ctimer timer;

static uint8_t is_listening = 0;
static clock_time_t off_time_adjustment = 0;
static clock_time_t off_time = OFF_TIME;

struct queue_list_item {
  struct queue_list_item *next;
  struct queuebuf *packet;
  struct ctimer removal_timer;
  struct compower_activity compower;
  mac_callback_t sent_callback;
  void *sent_callback_ptr;
  uint8_t num_transmissions;
#if WITH_PENDING_BROADCAST
  uint8_t broadcast_flag;
#endif /* WITH_PENDING_BROADCAST */
};

#define BROADCAST_FLAG_NONE    0
#define BROADCAST_FLAG_WAITING 1
#define BROADCAST_FLAG_PENDING 2
#define BROADCAST_FLAG_SEND    3

LIST(pending_packets_list);
LIST(queued_packets_list);
MEMB(queued_packets_memb, struct queue_list_item, MAX_QUEUED_PACKETS);

struct encounter {
  struct encounter *next;
  rimeaddr_t neighbor;
  clock_time_t time;
  struct ctimer remove_timer;
  struct ctimer turn_on_radio_timer;
};

#define MAX_ENCOUNTERS 4
LIST(encounter_list);
MEMB(encounter_memb, struct encounter, MAX_ENCOUNTERS);

static uint8_t is_streaming = 0;
#if WITH_STREAMING
static struct ctimer stream_probe_timer, stream_off_timer;
#define STREAM_PROBE_TIME CLOCK_SECOND / 128
#define STREAM_OFF_TIME CLOCK_SECOND / 2
#endif /* WITH_STREAMING */

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

/*---------------------------------------------------------------------------*/
static void
turn_radio_on(void)
{
  NETSTACK_RADIO.on();
  /*  leds_on(LEDS_YELLOW);*/
}
/*---------------------------------------------------------------------------*/
static void
turn_radio_off(void)
{
  if(lpp_is_on && is_streaming == 0) {
    NETSTACK_RADIO.off();
  }
  /*  leds_off(LEDS_YELLOW);*/
}
/*---------------------------------------------------------------------------*/
static void
remove_encounter(void *encounter)
{
  struct encounter *e = encounter;

  ctimer_stop(&e->remove_timer);
  ctimer_stop(&e->turn_on_radio_timer);
  list_remove(encounter_list, e);
  memb_free(&encounter_memb, e);
}
/*---------------------------------------------------------------------------*/
static void
register_encounter(rimeaddr_t *neighbor, clock_time_t time)
{
  struct encounter *e;

  /* If we have an entry for this neighbor already, we renew it. */
  for(e = list_head(encounter_list); e != NULL; e = list_item_next(e)) {
    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      e->time = time;
      ctimer_set(&e->remove_timer, ENCOUNTER_LIFETIME, remove_encounter, e);
      break;
    }
  }
  /* No matchin encounter was found, so we allocate a new one. */
  if(e == NULL) {
    e = memb_alloc(&encounter_memb);
    if(e == NULL) {
      /* We could not allocate memory for this encounter, so we just drop it. */
      return;
    }
    rimeaddr_copy(&e->neighbor, neighbor);
    e->time = time;
    ctimer_set(&e->remove_timer, ENCOUNTER_LIFETIME, remove_encounter, e);
    list_add(encounter_list, e);
  }
}

#if WITH_ENCOUNTER_OPTIMIZATION
/*---------------------------------------------------------------------------*/
static void
turn_radio_on_callback(void *packet)
{
  struct queue_list_item *p = packet;

  list_remove(pending_packets_list, p);
  list_add(queued_packets_list, p);
  turn_radio_on();

  /*  printf("enc\n");*/
}
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

/*---------------------------------------------------------------------------*/
static void
stream_off(void *dummy)
{
  is_streaming = 0;
}
/*---------------------------------------------------------------------------*/
/* This function goes through all encounters to see if it finds a
   matching neighbor. If so, we set a ctimer that will turn on the
   radio just before we expect the neighbor to send a probe packet. If
   we cannot find a matching encounter, we just turn on the radio.

   The outbound packet is put on either the pending_packets_list or
   the queued_packets_list, depending on if the packet should be sent
   immediately.
*/
static void
turn_radio_on_for_neighbor(rimeaddr_t *neighbor, struct queue_list_item *i)
{

#if WITH_STREAMING
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
     PACKETBUF_ATTR_PACKET_TYPE_STREAM) {
    is_streaming = 1;
    turn_radio_on();
    list_add(queued_packets_list, i);
    ctimer_set(&stream_off_timer, STREAM_OFF_TIME,
	       stream_off, NULL);
    return;
  }
#endif /* WITH_STREAMING */
  
  if(rimeaddr_cmp(neighbor, &rimeaddr_null)) {
#if ! WITH_PENDING_BROADCAST
    /* We have been asked to turn on the radio for a broadcast, so we
       just turn on the radio. */
    turn_radio_on();
#endif /* ! WITH_PENDING_BROADCAST */
    list_add(queued_packets_list, i);
    return;
  }

#if WITH_ENCOUNTER_OPTIMIZATION
  struct encounter *e;
  
  /* We go through the list of encounters to find if we have recorded
     an encounter with this particular neighbor. If so, we can compute
     the time for the next expected encounter and setup a ctimer to
     switch on the radio just before the encounter. */
  for(e = list_head(encounter_list); e != NULL; e = list_item_next(e)) {
    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      clock_time_t wait, now;

      /* We expect encounters to happen roughly every OFF_TIME time
	 units. The next expected encounter is at time e->time +
	 OFF_TIME. To compute a relative offset, we subtract with
	 clock_time(). Because we are only interested in turning on
	 the radio within the OFF_TIME period, we compute the waiting
	 time with modulo OFF_TIME. */

      now = clock_time();
      wait = (((clock_time_t)(e->time - now)) % (OFF_TIME + LISTEN_TIME)) -
        2 * LISTEN_TIME;

      /*      printf("now %d e %d e-n %d w %d %d\n", now, e->time, e->time - now, (e->time - now) % (OFF_TIME), wait);
      
      printf("Time now %lu last encounter %lu next expected encouter %lu wait %lu/%d (%lu)\n",
	     (1000ul * (unsigned long)now) / CLOCK_SECOND,
	     (1000ul * (unsigned long)e->time) / CLOCK_SECOND,
	     (1000ul * (unsigned long)(e->time + OFF_TIME)) / CLOCK_SECOND,
	     (1000ul * (unsigned long)wait) / CLOCK_SECOND, wait,
	     (1000ul * (unsigned long)(wait + now)) / CLOCK_SECOND);*/
      
      /*      printf("Neighbor %d.%d found encounter, waiting %d ticks\n",
	      neighbor->u8[0], neighbor->u8[1], wait);*/
      
      ctimer_set(&e->turn_on_radio_timer, wait, turn_radio_on_callback, i);
      list_add(pending_packets_list, i);
      return;
    }
  }
#endif /* WITH_ENCOUNTER_OPTIMIZATION */
  
  /* We did not find the neighbor in the list of recent encounters, so
     we just turn on the radio. */
  /*  printf("Neighbor %d.%d not found in recent encounters\n",
      neighbor->u8[0], neighbor->u8[1]);*/
  turn_radio_on();
  list_add(queued_packets_list, i);
  return;
}
/*---------------------------------------------------------------------------*/
static void
remove_queued_packet(struct queue_list_item *i, uint8_t tx_ok)
{
  mac_callback_t sent;
  void *ptr;
  int num_transmissions = 0;
  int status;
  
  PRINTF("%d.%d: removing queued packet\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);


  queuebuf_to_packetbuf(i->packet);
  
  ctimer_stop(&i->removal_timer);
  queuebuf_free(i->packet);
  list_remove(pending_packets_list, i);
  list_remove(queued_packets_list, i);

  /* XXX potential optimization */
  if(list_length(queued_packets_list) == 0 && is_listening == 0) {
    turn_radio_off();
    compower_accumulate(&i->compower);
  }

  sent = i->sent_callback;
  ptr = i->sent_callback_ptr;
  num_transmissions = i->num_transmissions;
  memb_free(&queued_packets_memb, i);
  if(num_transmissions == 0 || tx_ok == 0) {
    status = MAC_TX_NOACK;
  } else {
    status = MAC_TX_OK;
  }
  mac_call_sent_callback(sent, ptr, status, num_transmissions);
}
/*---------------------------------------------------------------------------*/
static void
remove_queued_old_packet_callback(void *item)
{
  remove_queued_packet(item, 0);
}

#if WITH_PENDING_BROADCAST
/*---------------------------------------------------------------------------*/
static void
remove_queued_broadcast_packet_callback(void *item)
{
  remove_queued_packet(item, 1);
}
/*---------------------------------------------------------------------------*/
static void
set_broadcast_flag(struct queue_list_item *i, uint8_t flag)
{
  i->broadcast_flag = flag;
  ctimer_set(&i->removal_timer, PACKET_LIFETIME,
             remove_queued_broadcast_packet_callback, i);
}
#endif /* WITH_PENDING_BROADCAST */
/*---------------------------------------------------------------------------*/
static void
listen_callback(int periods)
{
  is_listening = periods;
  turn_radio_on();
}
/*---------------------------------------------------------------------------*/
/**
 * Send a probe packet.
 */
static void
send_probe(void)
{
  struct lpp_hdr *hdr;
  struct announcement_msg *adata;
  struct announcement *a;

  /* Set up the probe header. */
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct lpp_hdr));
  hdr = packetbuf_dataptr();
  hdr->type = TYPE_PROBE;
  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
  /*  rimeaddr_copy(&hdr->receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));*/
  rimeaddr_copy(&hdr->receiver, &rimeaddr_null);

  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &rimeaddr_null);
  {
    int hdrlen = NETSTACK_FRAMER.create();
    if(hdrlen < 0) {
      /* Failed to send */
      return;
    }
  }
  
  /* Construct the announcements */
  adata = (struct announcement_msg *)((char *)hdr + sizeof(struct lpp_hdr));
  
  adata->num = 0;
  for(a = announcement_list(); a != NULL; a = list_item_next(a)) {
    adata->data[adata->num].id = a->id;
    adata->data[adata->num].value = a->value;
    adata->num++;
  }

  packetbuf_set_datalen(sizeof(struct lpp_hdr) +
		      ANNOUNCEMENT_MSG_HEADERLEN +
		      sizeof(struct announcement_data) * adata->num);

  /*  PRINTF("Sending probe\n");*/

  /*  printf("probe\n");*/

  if(NETSTACK_RADIO.channel_clear()) {
    NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
  } else {
    off_time_adjustment = random_rand() % (OFF_TIME / 2);
  }

  compower_accumulate(&compower_idle_activity);
}
/*---------------------------------------------------------------------------*/
static void
send_stream_probe(void *dummy)
{
  /* Turn on the radio for sending a probe packet and 
     anticipating a data packet from a neighbor. */
  turn_radio_on();
  
  /* Send a probe packet. */
  send_probe();

#if WITH_STREAMING
  is_streaming = 1;
#endif /* WITH_STREAMING */
}
/*---------------------------------------------------------------------------*/
static int
num_packets_to_send(void)
{
#if WITH_PENDING_BROADCAST
  struct queue_list_item *i;
  int num = 0;
  
  for(i = list_head(queued_packets_list); i != NULL; i = list_item_next(i)) {
    if(i->broadcast_flag == BROADCAST_FLAG_SEND ||
       i->broadcast_flag == BROADCAST_FLAG_NONE) {
      ++num;
    }
  }
  return num;
#else /* WITH_PENDING_BROADCAST */
  return list_length(queued_packets_list);
#endif /* WITH_PENDING_BROADCAST */
}
/*---------------------------------------------------------------------------*/
/**
 * Duty cycle the radio and send probes. This function is called
 * repeatedly by a ctimer. The function restart_dutycycle() is used to
 * (re)start the duty cycling.
 */
static int
dutycycle(void *ptr)
{
  struct ctimer *t = ptr;
	
  PT_BEGIN(&dutycycle_pt);

  while(1) {

#if WITH_PENDING_BROADCAST
    {
	/* Before sending the probe, we mark all broadcast packets in
	   our output queue to be pending. This means that they are
	   ready to be sent, once we know that no neighbor is
	   currently broadcasting. */
      for(p = list_head(queued_packets_list); p != NULL; p = list_item_next(p)) {
	  if(p->broadcast_flag == BROADCAST_FLAG_WAITING) {
	    PRINTF("wait -> pending\n");
	    set_broadcast_flag(p, BROADCAST_FLAG_PENDING);
	  }
	}
      }
#endif /* WITH_PENDING_BROADCAST */
    
    /* Turn on the radio for sending a probe packet and 
       anticipating a data packet from a neighbor. */
    turn_radio_on();

    /* Send a probe packet. */
    send_probe();

    /* Set a timer so that we keep the radio on for LISTEN_TIME. */
    ctimer_set(t, LISTEN_TIME, (void (*)(void *))dutycycle, t);
    PT_YIELD(&dutycycle_pt);

#if WITH_PENDING_BROADCAST
    {
      struct queue_list_item *p;
      /* Go through the list of packets we are waiting to send, and
	 check if there are any pending broadcasts in the list. If
	 there are pending broadcasts, and we did not receive any
	 broadcast packets from a neighbor in response to our probe,
	 we mark the broadcasts as being ready to send. */
      for(p = list_head(queued_packets_list); p != NULL; p = list_item_next(p)) {
	if(p->broadcast_flag == BROADCAST_FLAG_PENDING) {
	  PRINTF("pending -> send\n");
	  set_broadcast_flag(p, BROADCAST_FLAG_SEND);
	  turn_radio_on();
	}
      }
    }
#endif /* WITH_PENDING_BROADCAST */

    /* If we have no packets to send (indicated by the list length of
       queued_packets_list being zero), we should turn the radio
       off. Othersize, we keep the radio on. */
    if(num_packets_to_send() == 0) {
      
      /* If we are not listening for announcements, we turn the radio
	 off and wait until we send the next probe. */
      if(is_listening == 0) {
        int current_off_time;
        if(!NETSTACK_RADIO.receiving_packet()) {
          turn_radio_off();
          compower_accumulate(&compower_idle_activity);
        }
        current_off_time = off_time - off_time_adjustment;
        if(current_off_time < LISTEN_TIME * 2) {
          current_off_time = LISTEN_TIME * 2;
        }
        off_time_adjustment = 0;
	ctimer_set(t, current_off_time, (void (*)(void *))dutycycle, t);
	PT_YIELD(&dutycycle_pt);

#if WITH_ADAPTIVE_OFF_TIME
	off_time += LOWEST_OFF_TIME;
	if(off_time > OFF_TIME) {
	  off_time = OFF_TIME;
	}
#endif /* WITH_ADAPTIVE_OFF_TIME */

      } else {
	/* We are listening for annonucements, so we count down the
	   listen time, and keep the radio on. */
	is_listening--;
	ctimer_set(t, OFF_TIME, (void (*)(void *))dutycycle, t);
	PT_YIELD(&dutycycle_pt);
      }
    } else {
      /* We had pending packets to send, so we do not turn the radio off. */

      ctimer_set(t, off_time, (void (*)(void *))dutycycle, t);
      PT_YIELD(&dutycycle_pt);
    }
  }

  PT_END(&dutycycle_pt);
}
/*---------------------------------------------------------------------------*/
static void
restart_dutycycle(clock_time_t initial_wait)
{
  PT_INIT(&dutycycle_pt);
  ctimer_set(&timer, initial_wait, (void (*)(void *))dutycycle, &timer);  
}
/*---------------------------------------------------------------------------*/
/**
 *
 * Send a packet. This function builds a complete packet with an LPP
 * header and queues the packet. When a probe is heard (in the
 * read_packet() function), and the sender of the probe matches the
 * receiver of the queued packet, the queued packet is sent.
 *
 * ACK packets are treated differently from other packets: if a node
 * sends a packet that it expects to be ACKed, the sending node keeps
 * its radio on for some time after sending its packet. So we do not
 * need to wait for a probe packet: we just transmit the ACK packet
 * immediately.
 *
 */
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct lpp_hdr hdr;
  clock_time_t timeout;
  uint8_t is_broadcast = 0;

  rimeaddr_copy(&hdr.sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr.receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  if(rimeaddr_cmp(&hdr.receiver, &rimeaddr_null)) {
    is_broadcast = 1;
  }
  hdr.type = TYPE_DATA;

  packetbuf_hdralloc(sizeof(struct lpp_hdr));
  memcpy(packetbuf_hdrptr(), &hdr, sizeof(struct lpp_hdr));
  packetbuf_compact();

  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);

  {
    int hdrlen = NETSTACK_FRAMER.create();
    if(hdrlen < 0) {
      /* Failed to send */
      mac_call_sent_callback(sent, ptr, MAC_TX_ERR_FATAL, 0);
      return;
    }
  }

  PRINTF("%d.%d: queueing packet to %d.%d, channel %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 hdr.receiver.u8[0], hdr.receiver.u8[1],
	 packetbuf_attr(PACKETBUF_ATTR_CHANNEL));
#if WITH_ACK_OPTIMIZATION
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) == PACKETBUF_ATTR_PACKET_TYPE_ACK) {
    /* Send ACKs immediately. */
    NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
    mac_call_sent_callback(sent, ptr, MAC_TX_OK, 1);
    return;
  }
#endif /* WITH_ACK_OPTIMIZATION */

#if WITH_ADAPTIVE_OFF_TIME
  off_time = LOWEST_OFF_TIME;
  restart_dutycycle(off_time);
#endif /* WITH_ADAPTIVE_OFF_TIME */

  {
    struct queue_list_item *i;
    i = memb_alloc(&queued_packets_memb);
    if(i != NULL) {
      i->sent_callback = sent;
      i->sent_callback_ptr = ptr;
      i->num_transmissions = 0;
      i->packet = queuebuf_new_from_packetbuf();
      if(i->packet == NULL) {
	memb_free(&queued_packets_memb, i);
        printf("null packet\n");
        mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 0);
	return;
      } else {
        if(is_broadcast) {
          timeout = PACKET_LIFETIME;
#if WITH_PENDING_BROADCAST
	  /* We set the broadcast state of the packet to be
	     waiting. This means that the packet is waiting for our
	     next probe to be sent. Our next probe is used to check if
	     there are any neighbors currently broadcasting a
	     packet. If so, we will get a broadcast packet in response
	     to our probe. If no broadcast packet is received in
	     response to our probe, we mark the packet as ready to be
	     sent. */
	  set_broadcast_flag(i, BROADCAST_FLAG_WAITING);
	  PRINTF("-> waiting\n");
#endif /* WITH_PENDING_BROADCAST */
        } else {
	  timeout = UNICAST_TIMEOUT;
#if WITH_PENDING_BROADCAST
	  i->broadcast_flag = BROADCAST_FLAG_NONE;
#endif /* WITH_PENDING_BROADCAST */
	}
	ctimer_set(&i->removal_timer, timeout,
                   remove_queued_old_packet_callback, i);

	/* Wait for a probe packet from a neighbor. The actual packet
	   transmission is handled by the read_packet() function,
	   which receives the probe from the neighbor. */
        turn_radio_on_for_neighbor(&hdr.receiver, i);

      }
    } else {
      printf("i == NULL\n");
      mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 0);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  if(buf_list != NULL) {
    queuebuf_to_packetbuf(buf_list->buf);
    send_packet(sent, ptr);
  }
}
/*---------------------------------------------------------------------------*/
static int
detect_ack(void)
{
#define INTER_PACKET_INTERVAL              RTIMER_ARCH_SECOND / 5000
#define ACK_LEN 3
#define AFTER_ACK_DETECTECT_WAIT_TIME      RTIMER_ARCH_SECOND / 1000
  rtimer_clock_t wt;
  uint8_t ack_received = 0;
  
  wt = RTIMER_NOW();
  leds_on(LEDS_GREEN);
  while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + INTER_PACKET_INTERVAL)) { }
  leds_off(LEDS_GREEN);
  /* Check for incoming ACK. */
  if((NETSTACK_RADIO.receiving_packet() ||
      NETSTACK_RADIO.pending_packet() ||
      NETSTACK_RADIO.channel_clear() == 0)) {
    int len;
    uint8_t ackbuf[ACK_LEN + 2];
    
    wt = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + AFTER_ACK_DETECTECT_WAIT_TIME)) { }
    
    len = NETSTACK_RADIO.read(ackbuf, ACK_LEN);
    if(len == ACK_LEN) {
      ack_received = 1;
    }
  }
  if(ack_received) {
    leds_toggle(LEDS_RED);
  }
  return ack_received;
}
/*---------------------------------------------------------------------------*/
/**
 * Read a packet from the underlying radio driver. If the incoming
 * packet is a probe packet and the sender of the probe matches the
 * destination address of the queued packet (if any), the queued packet
 * is sent.
 */
static void
input_packet(void)
{
  struct lpp_hdr hdr;
  clock_time_t reception_time;
  int ret;

  reception_time = clock_time();

  if(NETSTACK_FRAMER.parse() < 0) {
    printf("lpp input_packet framer error\n");
  }

  memcpy(&hdr, packetbuf_dataptr(), sizeof(struct lpp_hdr));;
  packetbuf_hdrreduce(sizeof(struct lpp_hdr));
  /*    PRINTF("got packet type %d\n", hdr->type);*/

  if(hdr.type == TYPE_PROBE) {
    struct announcement_msg adata;
    
    /* Register the encounter with the sending node. We now know the
       neighbor's phase. */
    register_encounter(&hdr.sender, reception_time);

    /* Parse incoming announcements */
    memcpy(&adata, packetbuf_dataptr(),
           MIN(packetbuf_datalen(), sizeof(adata)));
#if 0
    PRINTF("%d.%d: probe from %d.%d with %d announcements\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           hdr.sender.u8[0], hdr.sender.u8[1], adata->num);
    
    if(adata.num / sizeof(struct announcement_data) > sizeof(struct announcement_msg)) {
      /* Sanity check. The number of announcements is too large -
         corrupt packet has been received. */
      return 0;
    }

    for(i = 0; i < adata.num; ++i) {
      /*	  PRINTF("%d.%d: announcement %d: %d\n",
		  rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		  adata->data[i].id,
		  adata->data[i].value);*/

      announcement_heard(&hdr.sender,
                         adata.data[i].id,
                         adata.data[i].value);
    }
#endif  /* 0 */

    /* Go through the list of packets to be sent to see if any of
       them match the sender of the probe, or if they are a
       broadcast packet that should be sent. */
    if(list_length(queued_packets_list) > 0) {
      struct queue_list_item *i;
      for(i = list_head(queued_packets_list); i != NULL; i = list_item_next(i)) {
        const rimeaddr_t *receiver;
        uint8_t sent;

        sent = 0;
 
        receiver = queuebuf_addr(i->packet, PACKETBUF_ADDR_RECEIVER);
        if(rimeaddr_cmp(receiver, &hdr.sender) ||
           rimeaddr_cmp(receiver, &rimeaddr_null)) {
          queuebuf_to_packetbuf(i->packet);

#if WITH_PENDING_BROADCAST
          if(i->broadcast_flag == BROADCAST_FLAG_NONE ||
             i->broadcast_flag == BROADCAST_FLAG_SEND) {
            i->num_transmissions = 1;
            ret = NETSTACK_RADIO.send(queuebuf_dataptr(i->packet),
                                queuebuf_datalen(i->packet));
            sent = 1;
            PRINTF("%d.%d: got a probe from %d.%d, sent packet to %d.%d\n",
		   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
                   hdr.sender.u8[0], hdr.sender.u8[1],
                   receiver->u8[0], receiver->u8[1]);
	      
          } else {
            PRINTF("%d.%d: got a probe from %d.%d, did not send packet\n",
                   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
                   hdr.sender.u8[0], hdr.sender.u8[1]);
          }
#else /* WITH_PENDING_BROADCAST */
          i->num_transmissions = 1;
          ret = NETSTACK_RADIO.send(queuebuf_dataptr(i->packet),
                               queuebuf_datalen(i->packet));
          PRINTF("%d.%d: got a probe from %d.%d, sent packet to %d.%d\n",
                 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
                 hdr.sender.u8[0], hdr.sender.u8[1],
                 receiver->u8[0], receiver->u8[1]);
#endif /* WITH_PENDING_BROADCAST */

          /*          off();*/

          /* Attribute the energy spent on listening for the probe
             to this packet transmission. */
          compower_accumulate(&i->compower);
	    
          /* If the packet was not a broadcast packet, we dequeue it
             now. Broadcast packets should be transmitted to all
             neighbors, and are dequeued by the dutycycling function
             instead, after the appropriate time. */
          if(!rimeaddr_cmp(receiver, &rimeaddr_null)) {
#if RDC_CONF_HARDWARE_ACK   
       
            if(ret == RADIO_TX_OK) {
              remove_queued_packet(i, 1);
            } else {
              remove_queued_packet(i, 0);
            }
#else
            if(detect_ack()) {
              remove_queued_packet(i, 1);
            } else {
              remove_queued_packet(i, 0);
            }

#endif /* RDC_CONF_HARDWARE_ACK */


#if WITH_PROBE_AFTER_TRANSMISSION
            /* Send a probe packet to catch any reply from the other node. */
            restart_dutycycle(PROBE_AFTER_TRANSMISSION_TIME);
#endif /* WITH_PROBE_AFTER_TRANSMISSION */

#if WITH_STREAMING
            if(is_streaming) {
              ctimer_set(&stream_probe_timer, STREAM_PROBE_TIME,
                         send_stream_probe, NULL);
            }
#endif /* WITH_STREAMING */
          }

          if(sent) {
            turn_radio_off();
          }

#if WITH_ACK_OPTIMIZATION
          if(packetbuf_attr(PACKETBUF_ATTR_RELIABLE) ||
             packetbuf_attr(PACKETBUF_ATTR_ERELIABLE)) {
            /* We're sending a packet that needs an ACK, so we keep
               the radio on in anticipation of the ACK. */
            turn_radio_on();
          }
#endif /* WITH_ACK_OPTIMIZATION */

        }
      }
    }

  } else if(hdr.type == TYPE_DATA) {
    turn_radio_off();
    if(!rimeaddr_cmp(&hdr.receiver, &rimeaddr_null)) {
      if(!rimeaddr_cmp(&hdr.receiver, &rimeaddr_node_addr)) {
        /* Not broadcast or for us */
        PRINTF("%d.%d: data not for us from %d.%d\n",
               rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
               hdr.sender.u8[0], hdr.sender.u8[1]);
        return;
      }
      packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &hdr.receiver);
    }
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &hdr.sender);

    PRINTF("%d.%d: got data from %d.%d\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           hdr.sender.u8[0], hdr.sender.u8[1]);

    /* Accumulate the power consumption for the packet reception. */
    compower_accumulate(&current_packet);
    /* Convert the accumulated power consumption for the received
       packet to packet attributes so that the higher levels can
       keep track of the amount of energy spent on receiving the
       packet. */
    compower_attrconv(&current_packet);
      
    /* Clear the accumulated power consumption so that it is ready
       for the next packet. */
    compower_clear(&current_packet);

#if WITH_PENDING_BROADCAST
    if(rimeaddr_cmp(&hdr.receiver, &rimeaddr_null)) {
      /* This is a broadcast packet. Check the list of pending
         packets to see if we are currently sending a broadcast. If
         so, we refrain from sending our broadcast until one sleep
         cycle period, so that the other broadcaster will have
         finished sending. */
	
      struct queue_list_item *i;
      for(i = list_head(queued_packets_list); i != NULL; i = list_item_next(i)) {
        /* If the packet is a broadcast packet that is not yet
           ready to be sent, we do not send it. */
        if(i->broadcast_flag == BROADCAST_FLAG_PENDING) {
          PRINTF("Someone else is sending, pending -> waiting\n");
          set_broadcast_flag(i, BROADCAST_FLAG_WAITING);
        }
      }
    }
#endif /* WITH_PENDING_BROADCAST */


#if WITH_PROBE_AFTER_RECEPTION
    /* XXX send probe after receiving a packet to facilitate data
       streaming. We must first copy the contents of the packetbuf into
       a queuebuf to avoid overwriting the data with the probe packet. */
    if(rimeaddr_cmp(&hdr.receiver, &rimeaddr_node_addr)) {
      struct queuebuf *q;
      q = queuebuf_new_from_packetbuf();
      if(q != NULL) {
        send_probe();
        queuebuf_to_packetbuf(q);
        queuebuf_free(q);
      }
    }
#endif /* WITH_PROBE_AFTER_RECEPTION */

#if WITH_ADAPTIVE_OFF_TIME
    off_time = LOWEST_OFF_TIME;
    restart_dutycycle(off_time);
#endif /* WITH_ADAPTIVE_OFF_TIME */

    NETSTACK_MAC.input();
  }
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  lpp_is_on = 1;
  turn_radio_on();
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  lpp_is_on = 0;
  if(keep_radio_on) {
    turn_radio_on();
  } else {
    turn_radio_off();
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return OFF_TIME + LISTEN_TIME;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  restart_dutycycle(random_rand() % OFF_TIME);

  lpp_is_on = 1;
  
  announcement_register_listen_callback(listen_callback);

  memb_init(&queued_packets_memb);
  list_init(queued_packets_list);
  list_init(pending_packets_list);
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver lpp_driver = {
  "LPP",
  init,
  send_packet,
  send_list,
  input_packet,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
