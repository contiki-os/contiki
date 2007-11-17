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
 * $Id: sky-treeroute.c,v 1.5 2007/11/17 10:28:49 adamdunkels Exp $
 */

/**
 * \file
 *         A program that collects statistics from a network of Tmote Sky nodes
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/neighbor.h"
#include "net/rime.h"
#include "net/rime/tree.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/battery-sensor.h"

#include "dev/light.h"
#include "dev/sht11.h"
#include <stdio.h>
#include <string.h>
#include "contiki-net.h"

static struct tree_conn tc;

struct sky_treeroute_msg {
  uint16_t light1;
  uint16_t light2;
  uint16_t temperature;
  uint16_t humidity;
  uint16_t battery;
  uint16_t best_neighbor;
  uint16_t best_neighbor_etx;
  uint16_t best_neighbor_rtmetric;
  uint32_t energy_lpm;
  uint32_t energy_cpu;
  uint32_t energy_rx;
  uint32_t energy_tx;
  uint32_t energy_rled;

  uint16_t tx, rx;
  uint16_t reliabletx, reliablerx,
    rexmit, acktx, noacktx, ackrx, timedout, badackrx;
  /* Reasons for dropping incoming packets: */
  uint16_t toolong, tooshort, badsynch, badcrc;
  uint16_t contentiondrop, /* Packet dropped due to contention */
    sendingdrop; /* Packet dropped when we were sending a packet */
  uint16_t lltx, llrx;

};

#define REXMITS 4

/*---------------------------------------------------------------------------*/
PROCESS(test_tree_process, "Test tree process");
PROCESS(depth_blink_process, "Depth indicator");
AUTOSTART_PROCESSES(&test_tree_process, &depth_blink_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(depth_blink_process, ev, data)
{
  static struct etimer et;
  static int count;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 10);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    count = tree_depth(&tc);
    if(count == TREE_MAX_DEPTH) {
      leds_on(LEDS_BLUE);
    } else {
      leds_off(LEDS_BLUE);
      count /= NEIGHBOR_ETX_SCALE;
      while(count > 0) {
	leds_on(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 32);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	leds_off(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 8);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	--count;
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
recv(rimeaddr_t *originator, u8_t seqno, u8_t hops)
{
  struct sky_treeroute_msg *msg;
  
  msg = rimebuf_dataptr();
  printf("%u %u %u %u %u %u %u %u %u %u %u %lu %lu %lu %lu %lu ",
	 originator->u16[0], seqno, hops,
	 msg->light1, msg->light2, msg->temperature, msg->humidity,
	 msg->battery,

	 msg->best_neighbor, msg->best_neighbor_etx, msg->best_neighbor_rtmetric,
	 msg->energy_lpm, msg->energy_cpu, msg->energy_rx, msg->energy_tx, msg->energy_rled
	 );
  printf("%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
	 msg->tx, msg->rx, msg->reliabletx, msg->reliablerx, msg->rexmit,
	 msg->acktx, msg->noacktx, msg->ackrx, msg->timedout, msg->badackrx,
	 msg->toolong, msg->tooshort, msg->badsynch, msg->badcrc,
	 msg->contentiondrop, msg->sendingdrop, msg->lltx, msg->llrx);
  
}
/*---------------------------------------------------------------------------*/
static const struct tree_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_tree_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit;)
  PROCESS_BEGIN();

  battery_sensor.activate();
  button_sensor.activate();
  
  tree_open(&tc, 128, &callbacks);
  
  while(1) {
    static struct etimer et;

    etimer_set(&et, CLOCK_SECOND * 20);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == sensors_event) {
      if(data == &button_sensor) {
	tree_set_sink(&tc, 1);
      }
    }

    if(etimer_expired(&et)) {
      struct sky_treeroute_msg *msg;
      struct neighbor *n;
      /*      leds_toggle(LEDS_BLUE);*/
      rimebuf_clear();
      msg = (struct sky_treeroute_msg *)rimebuf_dataptr();
      rimebuf_set_datalen(sizeof(struct sky_treeroute_msg));
      msg->light1 = sensors_light1();
      msg->light2 = sensors_light2();
      msg->temperature = sht11_temp();
      msg->humidity = sht11_humidity();
      msg->battery = battery_sensor.value(0);
      msg->energy_lpm = energest_type_time(ENERGEST_TYPE_LPM);
      msg->energy_cpu = energest_type_time(ENERGEST_TYPE_CPU);
      msg->energy_rx = energest_type_time(ENERGEST_TYPE_LISTEN);
      msg->energy_tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
      msg->energy_rled = energest_type_time(ENERGEST_TYPE_LED_RED);
      msg->best_neighbor = msg->best_neighbor_etx =
	msg->best_neighbor_rtmetric = 0;
      n = neighbor_best();
      if(n != NULL) {
	msg->best_neighbor = n->addr.u16[0];
	msg->best_neighbor_etx = neighbor_etx(n);
	msg->best_neighbor_rtmetric = n->rtmetric;
      }

      msg->tx = rimestats.tx;
      msg->rx = rimestats.rx;
      msg->reliabletx = rimestats.reliabletx;
      msg->reliablerx = rimestats.reliablerx;
      msg->rexmit = rimestats.rexmit;
      msg->acktx = rimestats.acktx;
      msg->noacktx = rimestats.noacktx;
      msg->ackrx = rimestats.ackrx;
      msg->timedout = rimestats.timedout;
      msg->badackrx = rimestats.badackrx;
      msg->toolong = rimestats.toolong;
      msg->tooshort = rimestats.tooshort;
      msg->badsynch = rimestats.badsynch;
      msg->badcrc = rimestats.badcrc;
      msg->contentiondrop = rimestats.contentiondrop;
      msg->sendingdrop = rimestats.sendingdrop;
      msg->lltx = rimestats.lltx;
      msg->llrx = rimestats.llrx;

      tree_send(&tc, REXMITS);
    }
  }
 exit:
  tree_close(&tc);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
