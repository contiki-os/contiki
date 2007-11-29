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
 * $Id: test-collect.c,v 1.2 2007/11/29 10:29:03 nifi Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/collect.h"
#include "net/rime/neighbor.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include <stdio.h>

static struct collect_conn tc;

/*---------------------------------------------------------------------------*/
PROCESS(test_collect_process, "Test collect process");
PROCESS(depth_blink_process, "Depth indicator");
AUTOSTART_PROCESSES(&test_collect_process, &depth_blink_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(depth_blink_process, ev, data)
{
  static struct etimer et;
  static int count;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 1);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    count = collect_depth(&tc);
    if(count == COLLECT_MAX_DEPTH) {
      leds_on(LEDS_RED);
    } else {
      leds_off(LEDS_RED);
      count /= NEIGHBOR_ETX_SCALE;
      while(count > 0) {
	leds_on(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 16);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	leds_off(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 4);
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
  printf("Sink got message from %d.%d, seqno %d, hops %d: len %d '%s'\n",
	 originator->u8[0], originator->u8[1],
	 seqno, hops,
	 rimebuf_datalen(),
	 (char *)rimebuf_dataptr());

}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_collect_process, ev, data)
{
  PROCESS_BEGIN();

  collect_open(&tc, 128, &callbacks);
  
  while(1) {
    static struct etimer et;

    etimer_set(&et, CLOCK_SECOND * 10);
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&et)) {
      rimebuf_clear();
      rimebuf_set_datalen(sprintf(rimebuf_dataptr(),
				  "%s", "Hello") + 1);
      collect_send(&tc, 4);
    }

    if(ev == sensors_event) {
      if(data == &button_sensor) {
	printf("Button\n");
	collect_set_sink(&tc, 1);
      }
    }
    
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
