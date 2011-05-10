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
 * $Id: shell-rime-sniff.c,v 1.3 2009/03/12 21:58:20 adamdunkels Exp $
 */

/**
 * \file
 *         Contiki shell Rime sniffer application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "shell.h"
#include "net/rime.h"

static uint8_t sniff_for_attributes;

/*---------------------------------------------------------------------------*/
PROCESS(shell_sniff_process, "sniff");
SHELL_COMMAND(sniff_command,
	      "sniff",
	      "sniff: dump incoming packets",
	      &shell_sniff_process);
/*---------------------------------------------------------------------------*/
enum {
  SNIFFER_PACKET_INPUT,
  SNIFFER_PACKET_OUTPUT,
  SNIFFER_ATTRIBUTES_INPUT,
  SNIFFER_ATTRIBUTES_OUTPUT,
};
/*---------------------------------------------------------------------------*/
struct sniff_attributes_blob {
  uint16_t len;
  uint16_t type;
  uint16_t rssi;
  uint16_t lqi;
  uint16_t timestamp;
  uint16_t listen_time;
  uint16_t transmit_time;
  uint16_t channel;
  rimeaddr_t src, dest;
};
struct sniff_packet_blob {
  uint16_t len;
  uint16_t type;
};
/*---------------------------------------------------------------------------*/
static void
sniff_attributes_output(int type)
{
  struct sniff_attributes_blob msg;
  msg.len = 10;
  msg.type = type;
  msg.rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  msg.lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  msg.timestamp = packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP);
  msg.listen_time = packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME);
  msg.transmit_time = packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME);
  msg.channel = packetbuf_attr(PACKETBUF_ATTR_CHANNEL);
  rimeaddr_copy(&msg.src, packetbuf_addr(PACKETBUF_ADDR_SENDER));
  rimeaddr_copy(&msg.dest, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  
  shell_output(&sniff_command, &msg, sizeof(msg), NULL, 0);
}
/*---------------------------------------------------------------------------*/
static void
sniff_packet_output(int type)
{
  struct sniff_packet_blob msg;

  msg.len = packetbuf_totlen() / 2 + 1;
  msg.type = type;
  shell_output(&sniff_command, &msg, sizeof(msg),
	       packetbuf_dataptr(), (packetbuf_datalen() & 0xfffe) +
	       2 * (packetbuf_totlen() & 1));
}
/*---------------------------------------------------------------------------*/
static void
input_sniffer(void)
{
  if(sniff_for_attributes) {
    sniff_attributes_output(SNIFFER_ATTRIBUTES_INPUT);
  } else {
    sniff_packet_output(SNIFFER_PACKET_INPUT);
  }
}
/*---------------------------------------------------------------------------*/
static void
output_sniffer(int mac_status)
{
  if(sniff_for_attributes) {
    sniff_attributes_output(SNIFFER_ATTRIBUTES_OUTPUT);
  } else {
    sniff_packet_output(SNIFFER_PACKET_OUTPUT);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sniff_process, ev, data)
{
  RIME_SNIFFER(s, input_sniffer, output_sniffer);
  PROCESS_EXITHANDLER(goto exit;);
  PROCESS_BEGIN();

  sniff_for_attributes = 0;
  
  if(data != NULL) {
    char *arg = data;
    if(arg[0] == '-' &&
       arg[1] == 'a') {
      sniff_for_attributes = 1;
    }
  }

  rime_sniffer_add(&s);
  
  PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);

 exit:
  rime_sniffer_remove(&s);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_rime_sniff_init(void)
{
  shell_register_command(&sniff_command);
}
/*---------------------------------------------------------------------------*/
