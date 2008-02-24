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
 * $Id: example-mh.c,v 1.2 2008/02/24 22:15:46 adamdunkels Exp $
 */

/**
 * \file
 *         Testing the multihop forwarding layer (mh) in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_mh_process, "mh example");
AUTOSTART_PROCESSES(&example_mh_process);
/*---------------------------------------------------------------------------*/
static void
recv(struct mh_conn *c, rimeaddr_t *sender)
{
  printf("mh message received '%s'\n", (char *)rimebuf_dataptr());
}
static rimeaddr_t *
forward(struct mh_conn *c, rimeaddr_t *originator, rimeaddr_t *dest,
	rimeaddr_t *prevhop, uint8_t hops)
{
  printf("Forwarding message '%s'\n", (char *)rimebuf_dataptr());
  return NULL;
}
static const struct mh_callbacks mh_call = {recv, forward};
static struct mh_conn mh;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_mh_process, ev, data)
{
  PROCESS_EXITHANDLER(mh_close(&mh);)
    
  PROCESS_BEGIN();

  mh_open(&mh, 128, &mh_call);

  while(1) {
    static struct etimer et;
    rimeaddr_t to;
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    rimebuf_copyfrom("Hej", 4);
    to.u8[0] = 161;
    to.u8[1] = 161;
    mh_send(&mh, &to);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
