/*
 * Copyright (c) 2011, Swedish Institute of Computer Science
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
 */

/**
 * \file
 *	A small command-line interface for the querying remote database systems.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "dev/serial-line.h"
#include "net/rime/rime.h"
#include "net/rime/mesh.h"

#include "antelope.h"
/*---------------------------------------------------------------------------*/
#define MAX_QUERY_SIZE 100

#define NETDB_CHANNEL 70

#ifndef SERVER_ID
#define SERVER_ID 4
#endif
/*---------------------------------------------------------------------------*/
PROCESS(netdb_process, "NetDB");
AUTOSTART_PROCESSES(&netdb_process);

static unsigned server_id = SERVER_ID;
static struct mesh_conn mesh;
/*---------------------------------------------------------------------------*/
PROCESS(shell_process, "Shell Process");

PROCESS_THREAD(shell_process, ev, data)
{
  linkaddr_t addr;

  PROCESS_BEGIN();

  printf("NetDB client\n");

  for(;;) {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
    if(strncmp(data, "server ", 7) == 0) {
      server_id = atoi((char *)data + 7);
    } else {
      printf("%lu Transmitting query \"%s\" to node %u\n", clock_time(), (char *)data, server_id);
      packetbuf_copyfrom(data, strlen(data));
      addr.u8[0] = server_id;
      addr.u8[1] = 0;
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
                         PACKETBUF_ATTR_PACKET_TYPE_STREAM);
      mesh_send(&mesh, &addr);
    }
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
static void
sent(struct mesh_conn *c)
{
}

static void
timedout(struct mesh_conn *c)
{
  printf("Failed to send packet: time out\n");
}

static void
received(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
  char *data;
  unsigned len;
  static char reply[MAX_QUERY_SIZE + 1];

  data = (char *)packetbuf_dataptr();
  len = packetbuf_datalen();

  if(len > MAX_QUERY_SIZE) {
    printf("Too long query: %d bytes\n", len);
    return;
  }

  memcpy(reply, data, len);
  reply[len] = '\0';  

  printf("%lu Reply received from %d.%d (%d hops): %s",
         clock_time(), from->u8[0], from->u8[1], (int)hops, reply);
}

static const struct mesh_callbacks callbacks = {received, sent, timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(netdb_process, ev, data)
{
  PROCESS_EXITHANDLER(mesh_close(&mesh));
  PROCESS_BEGIN();

  mesh_open(&mesh, NETDB_CHANNEL, &callbacks);
  process_start(&shell_process, NULL);

  PROCESS_END();
}
