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
 * $Id: example-runicast.c,v 1.2 2008/11/11 13:50:21 fros4943 Exp $
 */

/**
 * \file
 *         Reliable single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

#define MAX_RETRANSMISSIONS 4

/*---------------------------------------------------------------------------*/
PROCESS(test_runicast_process, "runicast test");
AUTOSTART_PROCESSES(&test_runicast_process);
/*---------------------------------------------------------------------------*/
static uint8_t in_tx;
static void
recv_runicast(struct runicast_conn *c, rimeaddr_t *from, uint8_t seqno)
{
  printf("runicast message received from %d.%d, seqno %d\n",
	 from->u8[0], from->u8[1], seqno);
}
static void
sent_runicast(struct runicast_conn *c, rimeaddr_t *to, uint8_t retransmissions)
{
  in_tx = 0;
  printf("runicast message sent to %d.%d, retransmissions %d\n",
	 to->u8[0], to->u8[1], retransmissions);
}
static void
timedout_runicast(struct runicast_conn *c, rimeaddr_t *to, uint8_t retransmissions)
{
  in_tx = 0;
  printf("runicast message timed out when sending to %d.%d, retransmissions %d\n",
	 to->u8[0], to->u8[1], retransmissions);
}
static const struct runicast_callbacks runicast_callbacks = {recv_runicast,
						   sent_runicast,
						   timedout_runicast};
static struct runicast_conn runicast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_runicast_process, ev, data)
{
  PROCESS_EXITHANDLER(runicast_close(&runicast);)

  PROCESS_BEGIN();

  runicast_open(&runicast, 128, &runicast_callbacks);
  in_tx = 0;

  while(1) {
    static struct etimer et;

    etimer_set(&et, 10*CLOCK_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(!in_tx &&
        !(rimeaddr_node_addr.u8[0] == 1 &&
        rimeaddr_node_addr.u8[1] == 0)) {
      rimeaddr_t recv;

      rimebuf_copyfrom("Hello", 5);
      recv.u8[0] = 1;
      recv.u8[1] = 0;

      /*printf("%u.%u: sending runicast to address %u.%u\n",
          rimeaddr_node_addr.u8[0],
          rimeaddr_node_addr.u8[1],
          recv.u8[0],
          recv.u8[1]);*/

      in_tx = 1;
      runicast_send(&runicast, &recv, MAX_RETRANSMISSIONS);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
