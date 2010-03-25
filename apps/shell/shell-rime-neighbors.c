/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: shell-rime-neighbors.c,v 1.4 2010/03/25 08:55:20 adamdunkels Exp $
 */

/**
 * \file
 *         The Contiki shell Rime ping application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <string.h>
#include "shell.h"
#include "net/rime.h"

PROCESS(shell_neighbors_process, "neighbors");
SHELL_COMMAND(neighbors_command,
	      "neighbors",
	      "neighbors: dump neighbor list in binary format",
	      &shell_neighbors_process);

static uint8_t listening_for_neighbors = 0;

/*---------------------------------------------------------------------------*/
static void
received_announcement(struct announcement *a, const rimeaddr_t *from,
		      uint16_t id, uint16_t value)
{
  struct {
    uint16_t len;
    uint16_t addr;
    uint16_t rssi;
    uint16_t lqi;
  } msg;

  if(listening_for_neighbors) {
    memset(&msg, 0, sizeof(msg));
    msg.len = 3;
    rimeaddr_copy((rimeaddr_t *)&msg.addr, from);
    msg.rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
    msg.lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    shell_output(&neighbors_command, &msg, sizeof(msg), "", 0);
  }
}
static struct announcement neighbor_announcement;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_neighbors_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(announcement_remove(&neighbor_announcement);)
  PROCESS_BEGIN();
  
  listening_for_neighbors = 1;
  announcement_listen(1);

  etimer_set(&et, CLOCK_SECOND * 10);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  listening_for_neighbors = 0;

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_rime_neighbors_init(void)
{
  announcement_register(&neighbor_announcement,
			SHELL_RIME_ANNOUNCEMENT_IDENTIFIER_NEIGHBORS,
			received_announcement);
  announcement_set_value(&neighbor_announcement, 0);

  shell_register_command(&neighbors_command);
}
/*---------------------------------------------------------------------------*/
