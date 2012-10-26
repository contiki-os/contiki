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
 */

/**
 * \file
 *         Shell commands for Rime communication primitives
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "shell-rime.h"

#include "dev/leds.h"

#include "lib/crc16.h"
#include "lib/random.h"

#include "net/rime.h"
#include "net/netstack.h"
#include "net/rime/route.h"

#include "net/rime/timesynch.h"

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif /* CONTIKI_TARGET_NETSIM */

#include <stdio.h>
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */
#include <string.h>


#define DEFAULT_COLLECT_REXMITS 15


#define COLLECT_MSG_HDRSIZE 4
struct collect_msg {
  uint16_t timestamp;
  uint16_t crc;
  uint8_t data[1];
};

struct collect_conn shell_collect_conn;
static int waiting_for_collect = 0;

static int is_sink = 0;

/*---------------------------------------------------------------------------*/
PROCESS(shell_mac_process, "mac");
SHELL_COMMAND(mac_command,
	      "mac",
	      "mac <onoroff>: turn MAC protocol on (1) or off (0)",
	      &shell_mac_process);
PROCESS(shell_send_process, "send");
SHELL_COMMAND(send_command,
	      "send",
	      "send <rexmits>: send data to the collector node, with rexmits hop-by-hop retransmissions",
	      &shell_send_process);
PROCESS(shell_collect_process, "collect");
SHELL_COMMAND(collect_command,
	      "collect",
	      "collect: collect data from the network",
	      &shell_collect_process);
#if WITH_TREEDEPTH
PROCESS(shell_treedepth_process, "treedepth");
SHELL_COMMAND(treedepth_command,
	      "treedepth",
	      "treedepth: print the collection tree depth",
	      &shell_treedepth_process);
#endif /* WITH_TREEDEPTH */
PROCESS(shell_routes_process, "routes");
SHELL_COMMAND(routes_command,
	      "routes",
	      "routes: dump route list in binary format",
	      &shell_routes_process);
PROCESS(shell_packetize_process, "packetize");
SHELL_COMMAND(packetize_command,
	      "packetize",
	      "packetize: put data into one packet",
	      &shell_packetize_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_mac_process, ev, data)
{
  int onoroff;
  const char *next;
  
  PROCESS_BEGIN();
  onoroff = shell_strtolong((char *)data, &next);
  if(next == data) {
    shell_output_str(&mac_command, "mac: current MAC layer: ", NETSTACK_RDC.name);
    shell_output_str(&mac_command, "mac usage: ", mac_command.description);
  } else {
    if(onoroff) {
      NETSTACK_RDC.on();
      shell_output_str(&mac_command, "mac: turned MAC on: ", NETSTACK_RDC.name);
    } else {
      NETSTACK_RDC.off(1);
      shell_output_str(&mac_command, "mac: turned MAC off (keeping radio on): ",
		       NETSTACK_RDC.name);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_packetize_process, ev, data)
{
  static struct queuebuf *q = NULL;
  static char *ptr;
  static int size;
  int len;
  PROCESS_BEGIN();

  while(1) {
    struct shell_input *input;
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);

    if(q == NULL) {
      packetbuf_clear();
      q = queuebuf_new_from_packetbuf();
      if(q == NULL) {
	shell_output_str(&packetize_command, "packetize: could not allocate packet buffer", "");
	PROCESS_EXIT();
      }
      ptr = queuebuf_dataptr(q);
      size = 0;
    }
    
    input = data;

    len = input->len1 + input->len2;

    if(len + size >= PACKETBUF_SIZE ||
       len  == 0) {
      shell_output(&packetize_command,
		   ptr, size,
		   "", 0);
      queuebuf_free(q);
      q = NULL;
      PROCESS_EXIT();
    }

    memcpy(ptr + size, input->data1, input->len1);
    size += input->len1;
    memcpy(ptr + size, input->data2, input->len2);
    size += input->len2;
    
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_routes_process, ev, data)
{
  struct {
    uint16_t len;
    uint16_t dest;
    uint16_t nexthop;
    uint16_t hop_count;
    uint16_t seqno;
  } msg;
  int i;
  struct route_entry *r;
  
  PROCESS_BEGIN();

  memset(&msg, 0, sizeof(msg));
  msg.len = 4;
  for(i = 0; i < route_num(); ++i) {
    r = route_get(i);
    rimeaddr_copy((rimeaddr_t *)&msg.dest, &r->dest);
    rimeaddr_copy((rimeaddr_t *)&msg.nexthop, &r->nexthop);
    msg.hop_count = r->cost;
    msg.seqno = r->seqno;
    shell_output(&routes_command, &msg, sizeof(msg), "", 0);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#if WITH_TREEDEPTH
PROCESS_THREAD(shell_treedepth_process, ev, data)
{
  char buf[20];

  PROCESS_BEGIN();

  snprintf(buf, sizeof(buf), "%d", collect_depth(&collect));

  shell_output_str(&treedepth_command, buf, "");

  PROCESS_END();
}
#endif /* WITH_TREEDEPTH */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_collect_process, ev, data)
{
  
  PROCESS_BEGIN();

#if TIMESYNCH_CONF_ENABLED
  timesynch_set_authority_level(0);
#endif
  collect_set_sink(&shell_collect_conn, 1);
  
  is_sink = 1;
  waiting_for_collect = 1;

  PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
  
  waiting_for_collect = 0;
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_send_process, ev, data)
{
  struct shell_input *input;
  int len;
  struct collect_msg *msg;
  static int num_rexmits;
  const char *next;

  PROCESS_BEGIN();

  num_rexmits = shell_strtolong((char *)data, &next);

  if(next == data) {
    /* If no argument was given, we send packets with a default number
       of retransmissions. */
    num_rexmits = DEFAULT_COLLECT_REXMITS;
  }

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
      msg->crc = crc16_data(msg->data, len, 0);
      collect_send(&shell_collect_conn, num_rexmits);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
recv_collect(const rimeaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  struct collect_msg collect_msg;
  char *dataptr;
  rtimer_clock_t latency;
  int len;

  /* Copy the collect message header. */
  memcpy(&collect_msg, packetbuf_dataptr(), sizeof(collect_msg));
  dataptr = ((struct collect_msg *)packetbuf_dataptr())->data;
  
#if TIMESYNCH_CONF_ENABLED
  latency = timesynch_time() - collect_msg.timestamp;
#else
  latency = 0;
#endif
  
  if(waiting_for_collect) {
    struct {
      uint16_t len;
      uint16_t originator;
      uint16_t seqno;
      uint16_t hops;
      uint16_t latency;
    } msg;

    if(packetbuf_datalen() >= COLLECT_MSG_HDRSIZE) {
      len = packetbuf_datalen() - COLLECT_MSG_HDRSIZE;

      if(collect_msg.crc == crc16_data(dataptr, len, 0)) {
	msg.len = 5 + (packetbuf_datalen() - COLLECT_MSG_HDRSIZE) / 2;
	rimeaddr_copy((rimeaddr_t *)&msg.originator, originator);
	msg.seqno = seqno;
	msg.hops = hops;
	msg.latency = latency;
	
	shell_output(&collect_command,
		     &msg, sizeof(msg),
		     dataptr, packetbuf_datalen() - COLLECT_MSG_HDRSIZE);
      }
    }
  }

}
static const struct collect_callbacks collect_callbacks = { recv_collect };
/*---------------------------------------------------------------------------*/
void
shell_rime_init(void)
{
  collect_open(&shell_collect_conn, SHELL_RIME_CHANNEL_COLLECT,
               COLLECT_ROUTER, &collect_callbacks);
  collect_set_keepalive(&shell_collect_conn, 10 * 60 * CLOCK_SECOND);

  shell_register_command(&collect_command);
  shell_register_command(&mac_command);
  shell_register_command(&packetize_command);
  shell_register_command(&routes_command);
  shell_register_command(&send_command);

#if WITH_TREEDEPTH
  shell_register_command(&treedepth_command);
#endif /* WITH_TREEDEPTH */

}
/*---------------------------------------------------------------------------*/
