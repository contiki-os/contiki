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
 * $Id: sky-treeroute.c,v 1.2 2007/04/02 20:02:23 adamdunkels Exp $
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
#include <stdio.h>

static struct tree_conn tc;

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
    etimer_set(&et, CLOCK_SECOND * 1);
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
static void
recv(rimeaddr_t *originator, u8_t seqno, u8_t hops, u8_t retransmissions)
{
  printf("Got message from %d.%d, seqno %d, hops %d, retransmissions %d: len %d '%s'\n",
	 originator->u8[0], originator->u8[1],
	 seqno, hops, retransmissions,
	 rimebuf_datalen(),
	 (char *)rimebuf_dataptr());
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

    etimer_set(&et, CLOCK_SECOND * 8);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == sensors_event) {
      if(data == &button_sensor) {
	printf("Button\n");
	tree_set_sink(&tc, 1);
      }
    }

    if(etimer_expired(&et)) {
      rimebuf_clear();
      rimebuf_set_datalen(sprintf(rimebuf_dataptr(), "%d %d",
				  sensors_light1(), sensors_light2()));
      printf("Sending '%s'\n", rimebuf_dataptr());
      tree_send(&tc);
    }
    
  }
 exit:
  tree_close(&tc);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
