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
 * $Id: sky-treeroute.c,v 1.3 2007/05/22 21:05:09 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/tree.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "dev/light.h"
#include "dev/sht11.h"
#include <stdio.h>
#include <string.h>
#include "contiki-net.h"

static struct tree_conn tc;

struct sky_treeroute_msg {
  u16_t light1;
  u16_t light2;
  u16_t temperature;
  u16_t humidity;
};

/*---------------------------------------------------------------------------*/
PROCESS(test_tree_process, "Test tree process");
PROCESS(depth_blink_process, "Depth indicator");
PROCESS(tcp_server_process, "TCP server");
AUTOSTART_PROCESSES(&test_tree_process, &depth_blink_process,
		    &tcp_server_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(depth_blink_process, ev, data)
{
  static struct etimer et;
  static int count;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    count = tree_depth(&tc);
    if(count == TREE_MAX_DEPTH) {
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
struct logbuf {
  rimeaddr_t originator;
  u8_t seqno, hops;
  struct sky_treeroute_msg msg;
};
#define BUFSIZE 4
static struct logbuf buffer[BUFSIZE];
static int nextbuf, firstbuf;

static void
recv(rimeaddr_t *originator, u8_t seqno, u8_t hops)
{
  struct sky_treeroute_msg *msg;
  struct logbuf *log;
  
  msg = rimebuf_dataptr();
  /*  printf("Got message from %d.%d, seqno %d, hops %d: len %d '%s'\n",
	 originator->u8[0], originator->u8[1],
	 seqno, hops,
	 rimebuf_datalen(),
	 (char *)rimebuf_dataptr());*/
  printf("From %d.%d\n", originator->u8[0], originator->u8[1]);

  log = &buffer[nextbuf];
  rimeaddr_copy(&log->originator, originator);
  log->seqno = seqno;
  log->hops = hops;
  memcpy(&log->msg, msg, sizeof(struct sky_treeroute_msg));
  nextbuf = (nextbuf + 1) % BUFSIZE;
  
  /*  memset(&buffer[nextbuf * BUFLINELEN], 0, BUFLINELEN);
  snprintf(&buffer[nextbuf * BUFLINELEN], BUFLINELEN,
	   "%d.%d %d %d %s%c",
	   originator->u8[0], originator->u8[1],
	   seqno, hops,
	   (char *)rimebuf_dataptr(), 0);
	   nextbuf = (nextbuf + 1) % BUFSIZE;*/
}
/*---------------------------------------------------------------------------*/
static const struct tree_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_tree_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit;)
  PROCESS_BEGIN();

  button_sensor.activate();
  
  tree_open(&tc, 128, &callbacks);
  
  while(1) {
    static struct etimer et;

    etimer_set(&et, CLOCK_SECOND * 20);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == sensors_event) {
      if(data == &button_sensor) {
	printf("Button\n");
	tree_set_sink(&tc, 1);
      }
    }

    if(etimer_expired(&et)) {
      struct sky_treeroute_msg *msg;
      /*      leds_toggle(LEDS_BLUE);*/
      rimebuf_clear();
      msg = (struct sky_treeroute_msg *)rimebuf_dataptr();
      rimebuf_set_datalen(sizeof(struct sky_treeroute_msg));
      msg->light1 = sensors_light1();
      msg->light2 = sensors_light2();
      msg->temperature = sht11_temp();
      msg->humidity = sht11_humidity();
      /*      rimebuf_set_datalen(snprintf(rimebuf_dataptr(), RIMEBUF_SIZE,
				   "%d %d %d %d",
				  sensors_light1(), sensors_light2(),
				  sht11_temp() - 3960, sht11_humidity()));*/
      /*      printf("Sending '%s'\n", (char *)rimebuf_dataptr());*/
      tree_send(&tc);
    }
  }
 exit:
  tree_close(&tc);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static struct psock ps;
static
PT_THREAD(handle_connection(struct psock *p))
{
  PSOCK_BEGIN(p);
  PSOCK_SEND_STR(p, "Contiki data collection server\n");
  while(1) {
    while(firstbuf != nextbuf) {
      static char buf[50];
      struct logbuf *l;

      l = &buffer[firstbuf];
      snprintf(buf, sizeof(buf), "%d.%d %d %d %u %u %u %u\n",
	       l->originator.u8[0], l->originator.u8[1],
	       l->seqno, l->hops,
	       l->msg.light1, l->msg.light2,
	       l->msg.temperature, l->msg.humidity);
      PSOCK_SEND_STR(p, buf);
      firstbuf = (firstbuf + 1) % BUFSIZE;
    }
    PSOCK_WAIT_UNTIL(p, nextbuf != firstbuf);
  }
  PSOCK_CLOSE(p);
  PSOCK_END(p);
}
PROCESS_THREAD(tcp_server_process, ev, data)
{
  PROCESS_BEGIN();
  tcp_listen(HTONS(1010));
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    if(uip_connected()) {
      tree_set_sink(&tc, 1);
      PSOCK_INIT(&ps, NULL, 0);
      while(!(uip_aborted() || uip_closed() || uip_timedout())) {
	PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
	handle_connection(&ps);
      }
    }
  }
  PROCESS_END();
}
