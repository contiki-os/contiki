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
 * $Id: shell-rime-ping.c,v 1.9 2009/03/31 17:39:36 adamdunkels Exp $
 */

/**
 * \file
 *         The Contiki shell Rime ping application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


#include "shell.h"
#include "net/rime.h"

#include <string.h>
#include <stdio.h>
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */

struct rime_ping_msg {
  rtimer_clock_t pingtime;
  rtimer_clock_t pongtime;
};

static struct mesh_conn mesh;
static int waiting_for_pong = 0;

/*---------------------------------------------------------------------------*/
PROCESS(shell_rime_ping_process, "rime-ping");
SHELL_COMMAND(rime_ping_command,
	      "rime-ping",
	      "rime-ping <node addr>: send a message to a specific node and get a reply",
	      &shell_rime_ping_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rime_ping_process, ev, data)
{
  static int i;
  static struct etimer timeout, periodic;
  static rimeaddr_t receiver;
  struct rime_ping_msg *ping;
  const char *nextptr;
  char buf[32];

  PROCESS_BEGIN();

  receiver.u8[0] = shell_strtolong(data, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    shell_output_str(&rime_ping_command,
		     "ping <receiver>: recevier must be specified", "");
    PROCESS_EXIT();
  }
  ++nextptr;
  receiver.u8[1] = shell_strtolong(nextptr, &nextptr);

  snprintf(buf, sizeof(buf), "%d.%d", receiver.u8[0], receiver.u8[1]);
  shell_output_str(&rime_ping_command, "Sending 4 pings to ", buf);

  for(i = 0; i < 4; ++i) {
    packetbuf_clear();
    ping = packetbuf_dataptr();
    packetbuf_set_datalen(sizeof(struct rime_ping_msg));
#if TIMESYNCH_CONF_ENABLED
    ping->pingtime = timesynch_time();
#else
    ping->pingtime = rtimer_arch_now();
#endif
    mesh_send(&mesh, &receiver);

    etimer_set(&timeout, CLOCK_SECOND * 8);
    etimer_set(&periodic, CLOCK_SECOND * 1);
    waiting_for_pong = 1;
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timeout) ||
			     waiting_for_pong == 0);
    if(waiting_for_pong == 0) {
      PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    } else {
      shell_output_str(&rime_ping_command,
		       "Timed out", "");
    }
    waiting_for_pong = 0;
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
timedout_mesh(struct mesh_conn *c)
{
  /*  printf("packet timedout\n");*/
}
static void
sent_mesh(struct mesh_conn *c)
{
}
static void
recv_mesh(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
  struct rime_ping_msg ping;
  char buf[64];
  rtimer_clock_t pingrecvtime;
  
  memcpy(&ping, packetbuf_dataptr(), sizeof(struct rime_ping_msg));

  if(waiting_for_pong == 0) {
#if TIMESYNCH_CONF_ENABLED
    ping.pongtime = timesynch_time();
#else
    ping.pongtime = ping.pingtime;
#endif
    memcpy(packetbuf_dataptr(), &ping, sizeof(struct rime_ping_msg));
    mesh_send(&mesh, from);
  } else {
#if TIMESYNCH_CONF_ENABLED
    pingrecvtime = timesynch_time();
#else
    pingrecvtime = rtimer_arch_now();
#endif
    snprintf(buf, sizeof(buf), "%lu ms (%lu + %lu), %d hops.",
	    (1000L * (pingrecvtime - ping.pingtime)) / RTIMER_ARCH_SECOND,
	    (1000L * (ping.pongtime - ping.pingtime)) / RTIMER_ARCH_SECOND,
	    (1000L * (pingrecvtime - ping.pongtime)) / RTIMER_ARCH_SECOND,
	    hops);

    shell_output_str(&rime_ping_command,
		     "Pong recived; rtt ", buf);
    waiting_for_pong = 0;
    process_post(&shell_rime_ping_process, PROCESS_EVENT_CONTINUE, NULL);
  }
}
CC_CONST_FUNCTION static struct mesh_callbacks mesh_callbacks = { recv_mesh,
						      sent_mesh,
						      timedout_mesh };
/*---------------------------------------------------------------------------*/
void
shell_rime_ping_init(void)
{
  mesh_open(&mesh, SHELL_RIME_CHANNEL_PING, &mesh_callbacks);

  shell_register_command(&rime_ping_command);
}
/*---------------------------------------------------------------------------*/
