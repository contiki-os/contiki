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
 * $Id: example-meshconn.c,v 1.3 2010/01/14 14:32:22 joxe Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/meshconn.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

static struct meshconn_conn meshconn;
/*---------------------------------------------------------------------------*/
PROCESS(test_meshconn_process, "Meshconnconn test");
AUTOSTART_PROCESSES(&test_meshconn_process);
/*---------------------------------------------------------------------------*/
static void
connected(struct meshconn_conn *c)
{
  printf("connected\n");
}
static void
closed(struct meshconn_conn *c)
{
  printf("closed\n");
}
static void
reset(struct meshconn_conn *c)
{
  printf("reset\n");
}
static void
timedout(struct meshconn_conn *c)
{
  printf("timedout\n");
}
static void
recv(struct meshconn_conn *c)
{
  printf("Data received from %.*s (%d)\n",
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

  /*  meshconn_send(&meshconn, from);*/
}

const static struct meshconn_callbacks callbacks = { connected,
						     recv,
						     closed,
						     timedout,
						     reset };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_meshconn_process, ev, data)
{
  PROCESS_EXITHANDLER(meshconn_close(&meshconn);)
  PROCESS_BEGIN();

  meshconn_open(&meshconn, 128, &callbacks);

  while(1) {
    rimeaddr_t addr;
    static struct etimer et;

    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et) ||
			     (ev == sensors_event && data == &button_sensor));

    printf("Button\n");

    /*
     * Send a message containing "Hej" (3 characters) to node number
     * 6.
     */

    addr.u8[0] = 53;
    addr.u8[1] = 0;
    meshconn_connect(&meshconn, addr);
    
    packetbuf_copyfrom("Hej", 3);
    meshconn_send(&meshconn, &addr);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
