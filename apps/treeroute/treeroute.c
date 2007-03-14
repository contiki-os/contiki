/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: treeroute.c,v 1.1 2007/03/14 01:05:23 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime.h"

#include "dev/radio-sensor.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>

PROCESS(routing_process, "Routing process");
PROCESS(hopcount_blink_process, "Hopcount indicator");
AUTOSTART_PROCESSES(&routing_process, &hopcount_blink_process);

static struct ruc_conn ruc_conn;
static struct sibc_conn sibc_conn;

struct hdr {
  node_id_t originator_id;
  u8_t originator_seqno;
  u8_t hopcount;
  u8_t hoplim;
  u8_t retransmissions;
  u8_t datalen;
  u8_t data[1];
};

#define SINK 0
#define HOPCOUNT_MAX 63

#define MAX_HOPLIM 10

static int forwarding;

static struct hdr hello;

static int seqno;

static void (* receiver)(u8_t *data, int len,
			 u8_t originator, u8_t seqno, u8_t last_hop_id,
			 u8_t hops, u8_t retransmissions) = NULL;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hopcount_blink_process, ev, data)
{
  static struct etimer et;
  static int count;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 1);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    count = hello.hopcount;
    if(count == HOPCOUNT_MAX) {
      leds_on(LEDS_RED);
    } else {
      leds_off(LEDS_RED);
      while(count > 0) {
	leds_on(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 10);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	leds_off(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 10);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	--count;
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
update_hopcount(void)
{
  struct neighbor *n;
  
  if(hello.hopcount != SINK) {
    n = neighbor_best();
    if(n == NULL) {
      /*      if(hopcount != HOPCOUNT_MAX) {
	printf("%d: didn't find a best neighbor, setting hopcount to max\n", node_id);
	}*/
      hello.hopcount = HOPCOUNT_MAX;
    } else {
      if(n->hopcount + 1 != hello.hopcount) {
	hello.hopcount = n->hopcount + 1;
      }
    }
  }

  /*  DEBUG_PRINTF("%d: new hopcount %d\n", node_id, hopcount);*/
#if NETSIM
  {
    char buf[8];
    if(hello.hopcount == HOPCOUNT_MAX) {
      strcpy(buf, " ");
    } else {
      snprintf(buf, sizeof(buf), "%d", hello.hopcount);
    }
    ether_set_text(buf);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
neighbor_packet_received(struct sibc_conn *c, node_id_t from)
{
  struct hdr *hdr = rimebuf_dataptr();
  struct neighbor *n;

  /*  printf("%d: neighbor_packet_received from %d with hopcount %d\n",
	 node_id, from, hdr->hopcount
	 );*/
  
  n = neighbor_find(from);

  if(n == NULL) {
    neighbor_add(from, hdr->hopcount, radio_sensor.value(1));
  } else {
    neighbor_update(n, hdr->hopcount, radio_sensor.value(1));
  }

  update_hopcount();

}
/*---------------------------------------------------------------------------*/
static int
node_packet_received(struct ruc_conn *c, node_id_t from, u8_t seqno)
{
  struct hdr *hdr = rimebuf_dataptr();
  struct neighbor *n;

  if(hello.hopcount == SINK) {
    printf("Sink packet received\n");
    if(receiver != NULL) {
      receiver(hdr->data, hdr->datalen,
	       hdr->originator_id, hdr->originator_seqno,
	       from, MAX_HOPLIM - hdr->hoplim,
	       hdr->retransmissions);
    }
    return 1;
  } else if(hdr->hoplim > 1 && hello.hopcount != HOPCOUNT_MAX) {
    printf("%d: packet received from %d, forwarding %d, best neighbor %p\n", node_id, from, forwarding, neighbor_best());
    if(!forwarding) {
      forwarding = 1;
      n = neighbor_best();
      if(n != NULL) {
	ruc_send(c, NEIGHBOR_NODEID(n));
      }
      return 1;
    } else {

      printf("%d: still forwarding another packet, not sending ACK\n", node_id);
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
node_packet_sent(struct ruc_conn *c)
{
  forwarding = 0;
}
/*---------------------------------------------------------------------------*/
static const struct sibc_ulayer sibc_ulayer = {neighbor_packet_received, NULL};
static const struct ruc_ulayer ruc_ulayer = {node_packet_received,
					     node_packet_sent};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(routing_process, ev, data)
{
  struct neighbor *n;
  struct hdr *hdr;
  
  PROCESS_BEGIN();
  
  rime_init();
  
  sibc_setup(&sibc_conn, CHANNEL_TREEROUTE_META, &sibc_ulayer);
  ruc_setup(&ruc_conn, CHANNEL_TREEROUTE_DATA, &ruc_ulayer);
  
  button_sensor.activate();
  radio_sensor.activate();

  PROCESS_PAUSE();

  hello.hopcount = HOPCOUNT_MAX;
  
  rimebuf_clear();
  rimebuf_reference(&hello, sizeof(hello));
  sibc_send_stubborn(&sibc_conn, CLOCK_SECOND * 8);
  
  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == sensors_event) {

      if(data == &pir_sensor) {
	rimebuf_clear();
	hdr = rimebuf_dataptr();
	hdr->originator_seqno = seqno++;
	hdr->originator_id = node_id;
	hdr->hopcount = hello.hopcount;
	hdr->hoplim = MAX_HOPLIM;
	hdr->retransmissions = 0;
	hdr->datalen = 0;
	rimebuf_set_datalen(sizeof(struct hdr));
	printf("Sending sensor event\n");
	n = neighbor_best();
	if(n != NULL) {
	  printf("Sending to best neighbor\n");
	  ruc_send(&ruc_conn, NEIGHBOR_NODEID(n));
	} else {
	  printf("Didn't find any neighbor\n");
	}
      }

      if(data == &button_sensor) {
	printf("Button\n");
	hello.hopcount = SINK;
      }
    }
    
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
