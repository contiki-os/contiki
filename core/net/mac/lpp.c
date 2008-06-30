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
 * $Id: lpp.c,v 1.5 2008/06/30 08:08:59 adamdunkels Exp $
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
#include "lib/random.h"

#include "net/rime.h"
#include "net/mac/mac.h"
#include "net/mac/lpp.h"
#include "net/rime/rimebuf.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#define TYPE_PROBE        1
#define TYPE_DATA         2
struct lpp_hdr {
  uint16_t type;
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

static const struct radio_driver *radio;
static void (* receiver_callback)(const struct mac_driver *);
static struct pt pt;
static struct ctimer timer;

static struct queuebuf *queued_packet;

#define LISTEN_TIME CLOCK_SECOND / 128
#define OFF_TIME CLOCK_SECOND / 4

#define DUMP_QUEUED_PACKET 1

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
/**
 * Send a probe packet.
 */
static void
send_probe(void)
{
  struct lpp_hdr *hdr;

  rimebuf_clear();
  hdr = rimebuf_dataptr();
  rimebuf_set_datalen(sizeof(struct lpp_hdr));

  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr->receiver, rimebuf_addr(RIMEBUF_ADDR_RECEIVER));
  hdr->type = TYPE_PROBE;
  PRINTF("Sending probe\n");
  radio->send(rimebuf_hdrptr(), rimebuf_totlen());
}
/*---------------------------------------------------------------------------*/
/**
 * Duty cycle the radio. The protothread is driven by a ctimer that is
 * initiated in the lpp_init() function.
 */
static int
dutycycle(void *ptr)
{
  struct ctimer *t = ptr;
  
  PT_BEGIN(&pt);

  while(1) {
    if(queued_packet != NULL) {
      
      /* We are currently sending a packet so we should keep the radio
	 turned on and not send any probes at this point. */
      ctimer_set(t, OFF_TIME * 2, (void (*)(void *))dutycycle, t);
      PT_YIELD(&pt);
      queuebuf_free(queued_packet);
      queued_packet = NULL;
      PRINTF("Removing old packet\n");
    }
    turn_radio_on();
    send_probe();
    ctimer_set(t, LISTEN_TIME, (void (*)(void *))dutycycle, t);
    PT_YIELD(&pt);
    turn_radio_off();

    /* There is a bit of randomness here right now to avoid collisions
       due to synchronization effects. Not sure how needed it is
       though. XXX */
    ctimer_set(t, OFF_TIME / 2 + (random_rand() % OFF_TIME / 2),
	       (void (*)(void *))dutycycle, t);
    PT_YIELD(&pt);
  }

  PT_END(&pt);
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
  struct lpp_hdr *hdr;

  rimebuf_hdralloc(sizeof(struct lpp_hdr));
  hdr = rimebuf_hdrptr();

  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr->receiver, rimebuf_addr(RIMEBUF_ADDR_RECEIVER));
  hdr->type = TYPE_DATA;

  rimebuf_compact();
  PRINTF("queueing packet type %d\n", hdr->type);

  if(rimebuf_attr(RIMEBUF_ATTR_PACKET_TYPE) == RIMEBUF_ATTR_PACKET_TYPE_ACK) {
    /* Immediately send ACKs - we're assuming that the other node is
       listening. */
    /*    printf("Immediately sending ACK\n");*/
    return radio->send(rimebuf_hdrptr(), rimebuf_totlen());
  } else {

    /* If a packet is already queued, the DUMP_QUEUED_PACKET option
       determines if the queued packet should be replaced with the new
       packet, or if the new packet should be dropped. XXX haven't
       measured the effect of this option */
#if DUMP_QUEUED_PACKET
    if(queued_packet != NULL) {
      queuebuf_free(queued_packet);
    }
    queued_packet = queuebuf_new_from_rimebuf();
#else /* DUMP_QUEUED_PACKET */
    if(queued_packet == NULL) {
      queued_packet = queuebuf_new_from_rimebuf();
    }
#endif /* DUMP_QUEUED_PACKET */

    /* Wait for a probe packet from a neighbor */
    turn_radio_on();
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
  struct lpp_hdr *hdr, *qhdr;
  
  rimebuf_clear();
  len = radio->read(rimebuf_dataptr(), RIMEBUF_SIZE);
  if(len > 0) {
    rimebuf_set_datalen(len);
    hdr = rimebuf_dataptr();
    rimebuf_hdrreduce(sizeof(struct lpp_hdr));
    PRINTF("got packet type %d\n", hdr->type);
    if(hdr->type == TYPE_PROBE) {
      if(queued_packet != NULL) {
	qhdr = queuebuf_dataptr(queued_packet);
	if(rimeaddr_cmp(&qhdr->receiver, &hdr->sender) ||
	   rimeaddr_cmp(&qhdr->receiver, &rimeaddr_null)) {
	  PRINTF("%d.%d: got a probe from %d.%d\n",
		 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		 hdr->sender.u8[0], hdr->sender.u8[1]);

	  radio->send(queuebuf_dataptr(queued_packet),
		      queuebuf_datalen(queued_packet));
	  queuebuf_free(queued_packet);
	  queued_packet = NULL;

	  turn_radio_on(); /* XXX Awaiting an ACK: we should check the
			      packet type of the queued packet to see
			      if it is a data packet. If not, we
			      should not turn the radio on. */
	}
      }
    } else if(hdr->type == TYPE_DATA) {
      PRINTF("%d.%d: got data from %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     hdr->sender.u8[0], hdr->sender.u8[1]);
    }
    len = rimebuf_datalen();
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
  ctimer_set(&timer, LISTEN_TIME, (void (*)(void *))dutycycle, &timer);
  return &lpp_driver;
}
/*---------------------------------------------------------------------------*/
