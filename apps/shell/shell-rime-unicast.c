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
 * $Id: shell-rime-unicast.c,v 1.4 2010/03/19 13:24:26 adamdunkels Exp $
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

#define MAX_DATALEN 32
#define UNICAST_MSG_HDRSIZE 2
struct unicast_msg {
  uint16_t timestamp;
  uint8_t data[MAX_DATALEN];
};

static uint8_t is_receiving;
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS(shell_unicast_send_process, "unicast-send");
SHELL_COMMAND(unicast_send_command,
	      "unicast-send",
	      "unicast-send <node addr>: send data to specific neighbor",
	      &shell_unicast_send_process);
PROCESS(shell_unicast_recv_process, "unicast-recv");
SHELL_COMMAND(unicast_recv_command,
	      "unicast-recv",
	      "unicast-recv: receive data that neighbors send with unicast-send",
	      &shell_unicast_recv_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_unicast_send_process, ev, data)
{
  struct shell_input *input;
  static rimeaddr_t receiver;
  int len;
  const char *nextptr;
  struct unicast_msg *msg;
  
  PROCESS_BEGIN();
  
  receiver.u8[0] = shell_strtolong(data, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    shell_output_str(&unicast_send_command,
		     "unicast <receiver>: recevier must be specified", "");
    PROCESS_EXIT();
  }
  ++nextptr;
  receiver.u8[1] = shell_strtolong(nextptr, &nextptr);

  /*  snprintf(buf, sizeof(buf), "%d.%d", receiver.u8[0], receiver.u8[1]);
      shell_output_str(&unicast_send_command, "Sending unicast packets to ", buf);*/

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    len = input->len1 + input->len2;

    if(len == 0) {
      PROCESS_EXIT();
    }
    
    if(len < MAX_DATALEN) {
      packetbuf_clear();
      packetbuf_set_datalen(len + UNICAST_MSG_HDRSIZE);
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
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  struct unicast_msg *msg;
#define OUTPUT_BLOB_HDRSIZE 6
  struct {
    uint16_t len;
    uint16_t from;
    uint16_t latency;
    uint16_t data[MAX_DATALEN];
  } output_blob;

  if(is_receiving == 0) {
    return;
  }
  
  msg = packetbuf_dataptr();

#if TIMESYNCH_CONF_ENABLED
  output_blob.latency = timesynch_time() - msg->timestamp;
#else
  output_blob.latency = 0;
#endif
  rimeaddr_copy((rimeaddr_t *)&output_blob.from, from);
  memcpy(output_blob.data, msg->data, packetbuf_datalen() - UNICAST_MSG_HDRSIZE);
  output_blob.len = 2 + (packetbuf_datalen() - UNICAST_MSG_HDRSIZE) / 2;
  shell_output(&unicast_recv_command, &output_blob,
	       OUTPUT_BLOB_HDRSIZE + (packetbuf_datalen() - UNICAST_MSG_HDRSIZE),
	       NULL, 0);
  /*
  printf("unicast message received from %d.%d, latency %lu ms, data '%.*s'\n",
	 from->u8[0], from->u8[1],
	 (1000L * latency) / RTIMER_ARCH_SECOND,
	 packetbuf_datalen() - UNICAST_MSG_HDRSIZE,
	 msg->data);*/
  
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};

PROCESS_THREAD(shell_unicast_recv_process, ev, data)
{
  PROCESS_EXITHANDLER(is_receiving = 0;);
  
  PROCESS_BEGIN();

  is_receiving = 1;
  
  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_rime_unicast_init(void)
{
  unicast_open(&uc, SHELL_RIME_CHANNEL_UNICAST,
	       &unicast_callbacks);
  shell_register_command(&unicast_send_command);
  shell_register_command(&unicast_recv_command);
}
/*---------------------------------------------------------------------------*/
