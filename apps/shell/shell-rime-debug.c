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
 * $Id: shell-rime-debug.c,v 1.5 2010/03/19 13:24:26 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "shell-rime-debug.h"

#include "dev/leds.h"

#include "lib/random.h"

#include "net/rime.h"
#include "net/rime/route.h"
#include "net/rime/trickle.h"

#include "net/rime/timesynch.h"

#include <stdio.h>
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */
#include <string.h>


#define COLLECT_MSG_HDRSIZE 2
struct collect_msg {
  uint16_t timestamp;
  uint8_t data[1];
};

static struct broadcast_conn broadcast;
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS(shell_broadcast_process, "broadcast");
SHELL_COMMAND(broadcast_command,
	      "broadcast",
	      "broadcast: broadcast data to all neighbors",
	      &shell_broadcast_process);
PROCESS(shell_unicast_process, "unicast");
SHELL_COMMAND(unicast_command,
	      "unicast",
	      "unicast <node addr>: unicast data to specific neighbor",
	      &shell_unicast_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_broadcast_process, ev, data)
{
  struct shell_input *input;
  int len;
  struct collect_msg *msg;
  
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    len = input->len1 + input->len2;

    if(len == 0) {
      PROCESS_EXIT();
    }

    if(len < PACKETBUF_SIZE) {
      packetbuf_clear();
      packetbuf_set_datalen(len + COLLECT_MSG_HDRSIZE);
      msg = packetbuf_dataptr();
      memcpy(msg->data, input->data1, input->len1);
      memcpy(msg->data + input->len1, input->data2, input->len2);
#if TIMESYNCH_CONF_ENABLED
      msg->timestamp = timesynch_time();
#else
      msg->timestamp = 0;
#endif
      /*      printf("Sending %d bytes\n", len);*/
      broadcast_send(&broadcast);
    }
  }
  PROCESS_END();
}
static void
recv_broadcast(struct broadcast_conn *c, const rimeaddr_t *from)
{
  struct collect_msg *msg;
  rtimer_clock_t latency;
  
  msg = packetbuf_dataptr();

#if TIMESYNCH_CONF_ENABLED
  latency = timesynch_time() - msg->timestamp;
#else
  latency = 0;
#endif

  printf("broadcast message received from %d.%d, latency %lu ms, data '%.*s'\n",
	 from->u8[0], from->u8[1],
	 (1000L * latency) / RTIMER_ARCH_SECOND,
	 packetbuf_datalen() - COLLECT_MSG_HDRSIZE,
	 msg->data);
}
static const struct broadcast_callbacks broadcast_callbacks = {recv_broadcast};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_unicast_process, ev, data)
{
  struct shell_input *input;
  static rimeaddr_t receiver;
  int len;
  const char *nextptr;
  struct collect_msg *msg;
  char buf[30];
  
  PROCESS_BEGIN();
  
  receiver.u8[0] = shell_strtolong(data, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    shell_output_str(&unicast_command,
		     "unicast <receiver>: recevier must be specified", "");
    PROCESS_EXIT();
  }
  ++nextptr;
  receiver.u8[1] = shell_strtolong(nextptr, &nextptr);

  snprintf(buf, sizeof(buf), "%d.%d", receiver.u8[0], receiver.u8[1]);
  shell_output_str(&unicast_command, "Sending unicast packets to ", buf);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    len = input->len1 + input->len2;

    if(len == 0) {
      PROCESS_EXIT();
    }
    
    if(len < PACKETBUF_SIZE) {
      packetbuf_clear();
      packetbuf_set_datalen(len + COLLECT_MSG_HDRSIZE);
      msg = packetbuf_dataptr();
      memcpy(msg->data, input->data1, input->len1);
      memcpy(msg->data + input->len1, input->data2, input->len2);
#if TIMESYNCH_CONF_ENABLED
      msg->timestamp = timesynch_time();
#else
      msg->timestamp = 0;
#endif
      /*      printf("Sending %d bytes\n", len);*/
      unicast_send(&uc, &receiver);
    }
  }
  PROCESS_END();
}
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  struct collect_msg *msg;
  rtimer_clock_t latency;
  
  msg = packetbuf_dataptr();

#if TIMESYNCH_CONF_ENABLED
  latency = timesynch_time() - msg->timestamp;
#else
  latency = 0;
#endif

  printf("unicast message received from %d.%d, latency %lu ms, data '%.*s'\n",
	 from->u8[0], from->u8[1],
	 (1000L * latency) / RTIMER_ARCH_SECOND,
	 packetbuf_datalen() - COLLECT_MSG_HDRSIZE,
	 msg->data);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
/*---------------------------------------------------------------------------*/
void
shell_rime_debug_init(void)
{
  unicast_open(&uc, SHELL_RIME_CHANNEL_UNICAST,
	       &unicast_callbacks);
  broadcast_open(&broadcast, SHELL_RIME_CHANNEL_BROADCAST,
		 &broadcast_callbacks);
  shell_register_command(&broadcast_command);
  shell_register_command(&unicast_command);
}
/*---------------------------------------------------------------------------*/
