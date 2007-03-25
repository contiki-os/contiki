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
 * $Id: test-meshroute.c,v 1.3 2007/03/25 12:10:29 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/mesh.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

static struct mesh_conn mesh;
/*---------------------------------------------------------------------------*/
PROCESS(test_mesh_process, "Mesh test");
AUTOSTART_PROCESSES(&test_mesh_process);
/*---------------------------------------------------------------------------*/
static void
sent(struct mesh_conn *c)
{
  printf("packet sent\n");
}
static void
timedout(struct mesh_conn *c)
{
  printf("packet timedout\n");
}
static void
recv(struct mesh_conn *c, rimeaddr_t *from)
{
  printf("Data received from %d: %.*s (%d)\n", from->u16[0],
	 rimebuf_datalen(), (char *)rimebuf_dataptr(), rimebuf_datalen());

  rimebuf_copyfrom("Hopp", 4);
  mesh_send(&mesh, from);
}

const static struct mesh_callbacks callbacks = {recv, sent, timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_mesh_process, ev, data)
{
  PROCESS_EXITHANDLER(mesh_close(&mesh);)
  PROCESS_BEGIN();

  mesh_open(&mesh, 128, &callbacks);

  button_sensor.activate();

  while(1) {
    rimeaddr_t addr;
    static struct etimer et;

    /*    etimer_set(&et, CLOCK_SECOND * 4);*/
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et) ||
			     (ev == sensors_event && data == &button_sensor));

    printf("Button\n");

    /*
     * Send a message containing "Hej" (3 characters) to node number
     * 6.
     */
    
    rimebuf_copyfrom("Hej", 3);
    addr.u8[0] = 161;
    addr.u8[1] = 161;
    mesh_send(&mesh, &addr);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
