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
 * $Id: example-ruc.c,v 1.1 2008/01/25 18:00:51 adamdunkels Exp $
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
PROCESS(test_ruc_process, "ruc test");
AUTOSTART_PROCESSES(&test_ruc_process);
/*---------------------------------------------------------------------------*/
static void
recv_ruc(struct ruc_conn *c, rimeaddr_t *from, uint8_t seqno)
{
  printf("ruc message received from %d.%d, seqno %d\n",
	 from->u8[0], from->u8[1], seqno);
}
static void
sent_ruc(struct ruc_conn *c, rimeaddr_t *to, uint8_t retransmissions)
{
  printf("ruc message sent to %d.%d, retransmissions %d\n",
	 to->u8[0], to->u8[1], retransmissions);
}
static void
timedout_ruc(struct ruc_conn *c, rimeaddr_t *to, uint8_t retransmissions)
{
  printf("ruc message timed out when sending to %d.%d, retransmissions %d\n",
	 to->u8[0], to->u8[1], retransmissions);
}
static const struct ruc_callbacks ruc_callbacks = {recv_ruc,
						   sent_ruc,
						   timedout_ruc};
static struct ruc_conn ruc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_ruc_process, ev, data)
{
  PROCESS_EXITHANDLER(ruc_close(&ruc);)
    
  PROCESS_BEGIN();

  ruc_open(&ruc, 128, &ruc_callbacks);

  while(1) {
    static struct etimer et;
    rimeaddr_t addr;
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    rimebuf_copyfrom("Hello", 5);
    addr.u8[0] = 41;
    addr.u8[1] = 41;
    ruc_send(&ruc, &addr, MAX_RETRANSMISSIONS);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
