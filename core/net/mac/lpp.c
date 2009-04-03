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
 * $Id: lpp.c,v 1.18 2009/04/03 20:08:05 adamdunkels Exp $
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
#include "net/mac/mac.h"
#include "net/mac/lpp.h"
#include "net/rime/packetbuf.h"
#include "net/rime/announcement.h"
#include "sys/compower.h"

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

static struct compower_activity current_packet;

static const struct radio_driver *radio;
static void (* receiver_callback)(const struct mac_driver *);
static struct pt dutycycle_pt;
static struct ctimer timer;

static uint8_t is_listening = 0;
static clock_time_t off_time_adjustment = 0;

#define LISTEN_TIME (CLOCK_SECOND / 128)
#define OFF_TIME (CLOCK_SECOND / 4)

/* If CLOCK_SECOND is less than 4, we may end up with an OFF_TIME that
   is 0 which will make compilation fail due to a modulo operation in
   the code. To ensure that OFF_TIME is greater than zero, we use the
   construct below. */
#if OFF_TIME == 0
#undef OFF_TIME
#define OFF_TIME 1
#endif

#define PACKET_LIFETIME (LISTEN_TIME + OFF_TIME)
#define UNICAST_TIMEOUT	(2 * PACKET_LIFETIME)
#define PROBE_AFTER_TRANSMISSION_TIME (LISTEN_TIME * 2)

#define ENCOUNTER_LIFETIME (16 * OFF_TIME)

struct queue_list_item {
  struct queue_list_item *next;
  struct queuebuf *packet;
  struct ctimer timer;
  struct compower_activity compower;
};

#ifdef QUEUEBUF_CONF_NUM
#define MAX_QUEUED_PACKETS QUEUEBUF_CONF_NUM / 2
#else /* QUEUEBUF_CONF_NUM */
#define MAX_QUEUED_PACKETS 4
#endif /* QUEUEBUF_CONF_NUM */

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
/*---------------------------------------------------------------------------*/
static void
turn_radio_on(void)
{
  radio->on();
  leds_on(LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
static void
turn_radio_off(void)
{
  radio->off();
  leds_off(LEDS_YELLOW);
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
  for(e = list_head(encounter_list); e != NULL; e = e->next) {
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
  struct encounter *e;

  if(rimeaddr_cmp(neighbor, &rimeaddr_null)) {
    /* We have been asked to turn on the radio for a broadcast, so we
       just turn on the radio. */
    turn_radio_on();
    list_add(queued_packets_list, i);
    return;
  }
  
  /* We go through the list of encounters to find if we have recorded
     an encounter with this particular neighbor. If so, we can compute
     the time for the next expected encounter and setup a ctimer to
     switch on the radio just before the encounter. */
  for(e = list_head(encounter_list); e != NULL; e = e->next) {
    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      clock_time_t wait, now;

      /* We expect encounters to happen roughly every OFF_TIME time
	 units. The next expected encounter is at time e->time +
	 OFF_TIME. To compute a relative offset, we subtract with
	 clock_time(). Because we are only interested in turning on
	 the radio within the OFF_TIME period, we compute the waiting
	 time with modulo OFF_TIME. */

      now = clock_time();
      wait = ((clock_time_t)(e->time - now)) % (OFF_TIME);

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
remove_queued_packet(void *item)
{
  struct queue_list_item *i = item;

  ctimer_stop(&i->timer);  
  queuebuf_free(i->packet);
  list_remove(pending_packets_list, i);
  list_remove(queued_packets_list, i);

  /* XXX potential optimization */
  if(list_length(queued_packets_list) == 0 && is_listening == 0) {
    turn_radio_off();
    compower_accumulate(&i->compower);
  }

  memb_free(&queued_packets_memb, i);
}
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
  rimeaddr_copy(&hdr->receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));


  /* Construct the announcements */
  adata = (struct announcement_msg *)((char *)hdr + sizeof(struct lpp_hdr));
  
  adata->num = 0;
  for(a = announcement_list(); a != NULL; a = a->next) {
    adata->data[adata->num].id = a->id;
    adata->data[adata->num].value = a->value;
    adata->num++;
  }

  packetbuf_set_datalen(sizeof(struct lpp_hdr) +
		      ANNOUNCEMENT_MSG_HEADERLEN +
		      sizeof(struct announcement_data) * adata->num);

  /*  PRINTF("Sending probe\n");*/

  /*  printf("probe\n");*/
  
  /* XXX should first check access to the medium (CCA - Clear Channel
     Assessment) and add LISTEN_TIME to off_time_adjustment if there
     is a packet in the air. */
  radio->send(packetbuf_hdrptr(), packetbuf_totlen());

  compower_accumulate(&compower_idle_activity);
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

    /* Send a probe packet. */
    send_probe();
    
    /* Turn on the radio for a while in anticipation of a data packet
       from a neighbor. */
    turn_radio_on();

    /* Set a timer so that we keep the radio on for LISTEN_TIME. */
    ctimer_set(t, LISTEN_TIME, (void (*)(void *))dutycycle, t);
    PT_YIELD(&dutycycle_pt);

    /* If we have no packets to send (indicated by the list length of
       queued_packets_list being zero), we should turn the radio
       off. Othersize, we keep the radio on. */
    
    if(list_length(queued_packets_list) == 0) {
      
      /* If we are not listening for announcements, we turn the radio
	 off and wait until we send the next probe. */
      if(is_listening == 0) {
	turn_radio_off();
	compower_accumulate(&compower_idle_activity);
	ctimer_set(t, OFF_TIME + off_time_adjustment, (void (*)(void *))dutycycle, t);
	off_time_adjustment = 0;
	PT_YIELD(&dutycycle_pt);

      } else {
	is_listening--;
	ctimer_set(t, OFF_TIME, (void (*)(void *))dutycycle, t);
	PT_YIELD(&dutycycle_pt);
      }
    } else {
      ctimer_set(t, OFF_TIME, (void (*)(void *))dutycycle, t);
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
static int
send_packet(void)
{
  struct lpp_hdr hdr;
  clock_time_t timeout;

  rimeaddr_copy(&hdr.sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr.receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  hdr.type = TYPE_DATA;

  packetbuf_hdralloc(sizeof(struct lpp_hdr));
  memcpy(packetbuf_hdrptr(), &hdr, sizeof(struct lpp_hdr));

  packetbuf_compact();
  PRINTF("%d.%d: queueing packet to %d.%d, channel %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 hdr.receiver.u8[0], hdr.receiver.u8[1],
	 packetbuf_attr(PACKETBUF_ATTR_CHANNEL));
  {
    struct queue_list_item *i;
    i = memb_alloc(&queued_packets_memb);
    if(i != NULL) {
      i->packet = queuebuf_new_from_packetbuf();
      if(i->packet == NULL) {
	memb_free(&queued_packets_memb, i);
	return 0;
      } else {

        timeout = UNICAST_TIMEOUT;
        if(rimeaddr_cmp(&hdr.receiver, &rimeaddr_null)) {
          timeout = PACKET_LIFETIME;
        }
	ctimer_set(&i->timer, timeout, remove_queued_packet, i);

	/* Wait for a probe packet from a neighbor. The actual packet
	   transmission is handled by the read_packet() function,
	   which receives the probe from the neighbor. */
        turn_radio_on_for_neighbor(&hdr.receiver, i);
      }
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Read a packet from the underlying radio driver. If the incoming
 * packet is a probe packet and the sender of the probe matches the
 * destination address of the queued packet (if any), the queued packet
 * is sent.
 */
static int
read_packet(void)
{
  int len;
  struct lpp_hdr *hdr;
  clock_time_t reception_time;

  reception_time = clock_time();
  
  packetbuf_clear();
  len = radio->read(packetbuf_dataptr(), PACKETBUF_SIZE);
  if(len > sizeof(struct lpp_hdr)) {
    packetbuf_set_datalen(len);
    hdr = packetbuf_dataptr();
    packetbuf_hdrreduce(sizeof(struct lpp_hdr));
    /*    PRINTF("got packet type %d\n", hdr->type);*/
    if(hdr->type == TYPE_PROBE) {
      /* Parse incoming announcements */
      struct announcement_msg *adata = packetbuf_dataptr();
      int i;
	
      /*	PRINTF("%d.%d: probe from %d.%d with %d announcements\n",
		rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		hdr->sender.u8[0], hdr->sender.u8[1], adata->num);*/
	
      for(i = 0; i < adata->num; ++i) {
	/*	  PRINTF("%d.%d: announcement %d: %d\n",
		  rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		  adata->data[i].id,
		  adata->data[i].value);*/

	announcement_heard(&hdr->sender,
			   adata->data[i].id,
			   adata->data[i].value);
      }

      register_encounter(&hdr->sender, reception_time);
      
      if(list_length(queued_packets_list) > 0) {
	struct queue_list_item *i;
	for(i = list_head(queued_packets_list); i != NULL; i = i->next) {
	  struct lpp_hdr *qhdr;
	  
	  qhdr = queuebuf_dataptr(i->packet);
	  if(rimeaddr_cmp(&qhdr->receiver, &hdr->sender) ||
	     rimeaddr_cmp(&qhdr->receiver, &rimeaddr_null)) {
	    PRINTF("%d.%d: got a probe from %d.%d, sending packet to %d.%d\n",
		   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		   hdr->sender.u8[0], hdr->sender.u8[1],
		   qhdr->receiver.u8[0], qhdr->receiver.u8[1]);
	    queuebuf_to_packetbuf(i->packet);
	    
	    radio->send(queuebuf_dataptr(i->packet),
			queuebuf_datalen(i->packet));

	    /* Attribute the energy spent on listening for the probe
	       to this packet transmission. */
	    compower_accumulate(&i->compower);
	    
	    /* If the packet was not a broadcast packet, we dequeue it
	       now. Broadcast packets should be transmitted to all
	       neighbors, and are dequeued by the dutycycling function
	       instead, after the appropriate time. */
	    if(!rimeaddr_cmp(&qhdr->receiver, &rimeaddr_null)) {
	      remove_queued_packet(i);

#if WITH_PROBE_AFTER_TRANSMISSION
	      /* Send a probe packet to catch any reply from the other node. */
	      restart_dutycycle(PROBE_AFTER_TRANSMISSION_TIME);
#endif /* WITH_PROBE_AFTER_TRANSMISSION */
	    }

#if WITH_ACK_OPTIMIZATION
	    if(packetbuf_attr(PACKETBUF_ATTR_RELIABLE)) {
	      /* We're sending a packet that needs an ACK, so we keep
		 the radio on in anticipation of the ACK. */
	      turn_radio_on();
	    }
#endif /* WITH_ACK_OPTIMIZATION */

	  }
	}
      }

    } else if(hdr->type == TYPE_DATA) {
      PRINTF("%d.%d: got data from %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     hdr->sender.u8[0], hdr->sender.u8[1]);
      
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

#if WITH_PROBE_AFTER_RECEPTION
      /* XXX send probe after receiving a packet to facilitate data
        streaming. We must first copy the contents of the packetbuf into
        a queuebuf to avoid overwriting the data with the probe packet. */
      if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
        struct queuebuf *q;
        q = queuebuf_new_from_packetbuf();
        if(q != NULL) {
	  send_probe();
	  queuebuf_to_packetbuf(q);
          queuebuf_free(q);
        }
      }
#endif /* WITH_PROBE_AFTER_RECEPTION */
    }

    len = packetbuf_datalen();
  }
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
  turn_radio_on();
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  if(keep_radio_on) {
    turn_radio_on();
  } else {
    turn_radio_off();
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static const struct mac_driver lpp_driver = {
  "LPP",
  send_packet,
  read_packet,
  set_receive_function,
  on,
  off,
};
/*---------------------------------------------------------------------------*/
static void
input_packet(const struct radio_driver *d)
{
  if(receiver_callback) {
    receiver_callback(&lpp_driver);
  }
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
lpp_init(const struct radio_driver *d)
{
  radio = d;
  radio->set_receive_function(input_packet);
  restart_dutycycle(random_rand() % OFF_TIME);

  announcement_register_listen_callback(listen_callback);

  memb_init(&queued_packets_memb);
  list_init(queued_packets_list);
  list_init(pending_packets_list);
  return &lpp_driver;
}
/*---------------------------------------------------------------------------*/
