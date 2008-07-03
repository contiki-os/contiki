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
 * $Id: example-multihop.c,v 1.1 2008/07/03 22:36:02 adamdunkels Exp $
 */

/**
 * \file
 *         Testing the multihop forwarding layer (multihop) in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_multihop_process, "multihop example");
AUTOSTART_PROCESSES(&example_multihop_process);
/*---------------------------------------------------------------------------*/
static void
recv(struct multihop_conn *c, rimeaddr_t *sender)
{
  printf("multihop message received '%s'\n", (char *)rimebuf_dataptr());
}
static rimeaddr_t *
forward(struct multihop_conn *c, rimeaddr_t *originator, rimeaddr_t *dest,
	rimeaddr_t *prevhop, uint8_t hops)
{
  printf("Forwarding message '%s'\n", (char *)rimebuf_dataptr());
  return NULL;
}
static const struct multihop_callbacks multihop_call = {recv, forward};
static struct multihop_conn multihop;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_multihop_process, ev, data)
{
  PROCESS_EXITHANDLER(multihop_close(&multihop);)
    
  PROCESS_BEGIN();

  multihop_open(&multihop, 128, &multihop_call);

  while(1) {
    static struct etimer et;
    rimeaddr_t to;
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    rimebuf_copyfrom("Hej", 4);
    to.u8[0] = 161;
    to.u8[1] = 161;
    multihop_send(&multihop, &to);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
