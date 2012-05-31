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
 * $Id: shell-netperf.c,v 1.6 2010/02/18 20:56:12 adamdunkels Exp $
 */

/**
 * \file
 *         A small pogram to measure the communication performance between two nodes
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell-netperf.h"
#include "net/rime.h"
#include "contiki-conf.h"

#include <stdio.h>
#include <string.h>

#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */

/*---------------------------------------------------------------------------*/
#define DATALEN 90
#define MAX_RETRIES 8

#define CONTINUE_EVENT 128

struct power {
  unsigned long lpm, cpu, rx, tx;
};

struct stats {
  uint16_t sent, received, timedout;
  unsigned long total_tx_latency;
  unsigned long total_rx_latency;
  clock_time_t start, end;
  struct power power0, power;
};


enum {
  DATAPATH_COMMAND_NONE,
  DATAPATH_COMMAND_ECHO_REQUEST,
  DATAPATH_COMMAND_STREAM_ECHO_REQUEST,
  DATAPATH_COMMAND_ECHO_REPLY,
} datapath_commands;

struct datapath_msg {
  rimeaddr_t receiver;
  rtimer_clock_t tx, rx;
  uint8_t datapath_command;
  uint8_t received;
};

enum {
  CTRL_COMMAND_CLEAR,
  CTRL_COMMAND_STATS,
  CTRL_COMMAND_STATS_REPLY
} ctrlpath_commands;

struct ctrl_msg {
  uint8_t command;
  struct stats stats;
};

static struct runicast_conn ctrl;
static struct unicast_conn unicast;
static struct broadcast_conn broadcast;
static struct mesh_conn mesh;
static struct rucb_conn rucb;

static rimeaddr_t receiver;
static uint8_t is_sender;
static int left_to_send;

static struct stats stats;

enum {
  TYPE_NONE             = 0,
  TYPE_BROADCAST        = 1,
  TYPE_UNICAST          = 2,
  TYPE_UNICAST_PINGPONG = 3,
  TYPE_UNICAST_STREAM   = 4,
};

static uint8_t current_type;

/*---------------------------------------------------------------------------*/
PROCESS(shell_netperf_process, "netperf");
SHELL_COMMAND(netperf_command,
	      "netperf",
	      "netperf: measure single-hop network performance",
	      &shell_netperf_process);
/*---------------------------------------------------------------------------*/
static void
memcpy_misaligned(void *dest, const void *source, int len)
{
  int i;
  uint8_t *destptr;
  const uint8_t *sourceptr;
  if(((int)dest & 1) == 1 ||
     ((int)source & 1) == 1) {
    destptr = dest;
    sourceptr = source;
    for(i = 0; i < len; ++i) {
      *destptr++ = *sourceptr++;
    }
  } else {
    memcpy(dest, source, len);
  }
}
/*---------------------------------------------------------------------------*/
static void
print_remote_stats(struct stats *s)
{
  unsigned long total_time;
  
  printf("%d 1 %d %d %d %lu %lu %lu %lu %lu %lu %lu # for automatic processing\n",
	 current_type,
	 s->sent, s->received, s->timedout,
	 s->end - s->start,
	 s->total_tx_latency, s->total_rx_latency,
	 s->power.cpu - s->power0.cpu,
	 s->power.lpm - s->power0.lpm,
	 s->power.rx - s->power0.rx,
	 s->power.tx - s->power0.tx);

  printf("Remote node statistics:\n");
  total_time = s->power.cpu + s->power.lpm - s->power0.cpu - s->power0.lpm;
  printf("  Remote radio duty cycle:   rx %lu.%02lu%% tx %lu.%02lu%%\n",
	 (100 * (s->power.rx - s->power0.rx))/total_time,
	 ((10000 * (s->power.rx - s->power0.rx))/total_time) % 100,
	 (100 * (s->power.tx - s->power0.tx))/total_time,
	 ((10000 * (s->power.tx - s->power0.tx))/total_time) % 100);

  printf("  Packets:                   sent %d received %d\n",
	 s->sent, s->received);
}
/*---------------------------------------------------------------------------*/
static void
print_local_stats(struct stats *s)
{
  unsigned long total_time;
  
  printf("%d 0 %d %d %d %lu %lu %lu %lu %lu %lu %lu # for automatic processing\n",
	 current_type, 
	 s->sent, s->received, s->timedout,
	 s->end - s->start,
	 s->total_tx_latency, s->total_rx_latency,
	 s->power.cpu - s->power0.cpu,
	 s->power.lpm - s->power0.lpm,
	 s->power.rx - s->power0.rx,
	 s->power.tx - s->power0.tx);

  printf("Local node statistics:\n");
 
  printf("  Total transfer time:       %lu.%02lu seconds, %lu.%02lu packets/second\n",
	 (s->end - s->start) / CLOCK_SECOND,
	 ((10 * (s->end - s->start)) / CLOCK_SECOND) % 10,
	 ((1UL * CLOCK_SECOND * s->sent) / (s->end - s->start)),
	 (((100UL * CLOCK_SECOND * s->sent) / (s->end - s->start)) % 100));
 
  printf("  Average round-trip-time:   %lu ms (%lu + %lu)\n",
	 (1000 * (s->total_rx_latency + s->total_tx_latency) / s->received) /
	 RTIMER_ARCH_SECOND,
	 (1000 * (s->total_tx_latency) / s->received) /
	 RTIMER_ARCH_SECOND,
	 (1000 * (s->total_rx_latency) / s->received) /
	 RTIMER_ARCH_SECOND);
  
  total_time = s->power.cpu + s->power.lpm - s->power0.cpu - s->power0.lpm;
  printf("  Radio duty cycle:          rx %lu.%02lu%% tx %lu.%02lu%%\n",
	 (100 * (s->power.rx - s->power0.rx))/total_time,
	 ((10000 * (s->power.rx - s->power0.rx))/total_time) % 100,
	 (100 * (s->power.tx - s->power0.tx))/total_time,
	 ((10000 * (s->power.tx - s->power0.tx))/total_time) % 100);

  printf("  Packets received:          %d.%lu%%, %d of %d\n",
	 100 * s->received / s->sent,
	 (10000L * s->received / s->sent) % 10,
	 s->received, s->sent);
}
/*---------------------------------------------------------------------------*/
static void
sample_power_profile(struct power *p)
{
  energest_flush();
  p->lpm = energest_type_time(ENERGEST_TYPE_LPM);
  p->cpu = energest_type_time(ENERGEST_TYPE_CPU);
  p->rx = energest_type_time(ENERGEST_TYPE_LISTEN);
  p->tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
}
/*---------------------------------------------------------------------------*/
static void
clear_stats(void)
{
  memset(&stats, 0, sizeof(stats));
  stats.start = clock_time();
  sample_power_profile(&stats.power0);
}
/*---------------------------------------------------------------------------*/
static void
setup_sending(rimeaddr_t *to, int num)
{
  is_sender = 1;
  left_to_send = num;
  rimeaddr_copy(&receiver, to);
  clear_stats();
}
/*---------------------------------------------------------------------------*/
static void
finalize_stats(struct stats *s)
{
  stats.end = clock_time();
  sample_power_profile(&stats.power);
}
/*---------------------------------------------------------------------------*/
static unsigned long filesize, bytecount, packetsize;
static int download_complete;
static void
write_chunk(struct rucb_conn *c, int offset, int flag,
	    char *data, int datalen)
{
  printf("+");
}
static int
read_chunk(struct rucb_conn *c, int offset, char *to, int maxsize)
{
  int size;
  /*  printf("-");*/
  size = maxsize;
  if(bytecount + maxsize >= filesize) {
    size = filesize - bytecount;
  }
  if(size > packetsize) {
    size = packetsize;
  }
  bytecount += size;
  if(bytecount == filesize) {
    /*    end_time_rucb = clock_time();*/
    download_complete = 1;
    process_post(&shell_netperf_process, PROCESS_EVENT_CONTINUE, NULL);
  }
  return size;
}
const static struct rucb_callbacks rucb_callbacks =
  { write_chunk, read_chunk, NULL };
/*---------------------------------------------------------------------------*/
static int
construct_next_packet(void)
{
  struct datapath_msg *msg;
  packetbuf_clear();
  if(left_to_send > 0) {
    packetbuf_set_datalen(DATALEN);
    msg = packetbuf_dataptr();
    msg->datapath_command = DATAPATH_COMMAND_NONE;
    msg->received = 0;
#if TIMESYNCH_CONF_ENABLED
    msg->tx = msg->rx = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
    msg->tx = msg->rx = 0;
#endif /* TIMESYNCH_CONF_ENABLED */
    rimeaddr_copy(&msg->receiver, &receiver);
    left_to_send--;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
construct_next_echo(void)
{
  struct datapath_msg *msg;
  if(construct_next_packet()) {
    msg = packetbuf_dataptr();
    msg->datapath_command = DATAPATH_COMMAND_ECHO_REQUEST;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
construct_next_stream_echo(void)
{
  struct datapath_msg *msg;
  if(construct_next_packet()) {
    msg = packetbuf_dataptr();
    msg->datapath_command = DATAPATH_COMMAND_STREAM_ECHO_REQUEST;
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
		       PACKETBUF_ATTR_PACKET_TYPE_STREAM);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
process_incoming_packet(void)
{
  struct datapath_msg *msg = packetbuf_dataptr();
  rtimer_clock_t now;
  struct datapath_msg msg_copy;

#if TIMESYNCH_CONF_ENABLED
  now = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
  now = 0;
#endif /* TIMESYNCH_CONF_ENABLED */
  
  memcpy_misaligned(&msg_copy, (uint8_t *)msg, sizeof(msg_copy));
  stats.received++;
  stats.total_tx_latency += msg_copy.rx - msg_copy.tx;
  stats.total_rx_latency += now - msg_copy.rx;
}
/*---------------------------------------------------------------------------*/
static int
construct_reply(const rimeaddr_t *from)
{
  struct datapath_msg *msg = packetbuf_dataptr();
#if TIMESYNCH_CONF_ENABLED
  rtimer_clock_t now = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
  rtimer_clock_t now = 0;
#endif /* TIMESYNCH_CONF_ENABLED */

  memcpy_misaligned(&msg->rx, &now, sizeof(rtimer_clock_t));

  switch(msg->datapath_command) {
  case DATAPATH_COMMAND_ECHO_REQUEST:
    msg->datapath_command = DATAPATH_COMMAND_ECHO_REPLY;
    break;
  case DATAPATH_COMMAND_STREAM_ECHO_REQUEST:
    msg->datapath_command = DATAPATH_COMMAND_ECHO_REPLY;
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
		       PACKETBUF_ATTR_PACKET_TYPE_STREAM);
    break;
  }
  if(msg->datapath_command == DATAPATH_COMMAND_ECHO_REPLY) {
    packetbuf_set_datalen(sizeof(struct datapath_msg));
    msg->received = stats.received;
    stats.sent++;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
sent_mesh(struct mesh_conn *c)
{
  stats.sent++;
}
static void
timedout_mesh(struct mesh_conn *c)
{
  if(is_sender) {
    if(construct_next_echo()) {
      mesh_send(&mesh, &receiver);
    }
  }
  stats.timedout++;
}
static void
recv_mesh(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
  process_incoming_packet();
  if(is_sender) {
    if(construct_next_echo()) {
      mesh_send(&mesh, &receiver);
    }
  } else {
    if(construct_reply(from)) {
      mesh_send(&mesh, from);
    }
  }
}
const static struct mesh_callbacks mesh_callbacks =
  { recv_mesh, sent_mesh, timedout_mesh };
/*---------------------------------------------------------------------------*/
static void
sent_ctrl(struct runicast_conn *c, const rimeaddr_t *to, uint8_t rexmits)
{
  process_post(&shell_netperf_process, CONTINUE_EVENT, NULL);
}
static void
timedout_ctrl(struct runicast_conn *c, const rimeaddr_t *to, uint8_t rexmits)
{
  shell_output_str(&netperf_command, "netperf control connection failed", "");
  process_exit(&shell_netperf_process);
}
static void
recv_ctrl(struct runicast_conn *c, const rimeaddr_t *from, uint8_t seqno)
{
  static uint8_t last_seqno = -1;
  struct stats s;
  struct ctrl_msg *msg = packetbuf_dataptr();

  if(last_seqno == seqno) {
    return;
  }
  last_seqno = seqno;
  switch(msg->command) {
  case CTRL_COMMAND_STATS:
    msg->command = CTRL_COMMAND_STATS_REPLY;
    finalize_stats(&stats);
    memcpy_misaligned(&msg->stats, &stats, sizeof(stats));
    packetbuf_set_datalen(sizeof(struct ctrl_msg));
    runicast_send(c, from, MAX_RETRIES);
    break;
  case CTRL_COMMAND_STATS_REPLY:
    memcpy_misaligned(&s, &msg->stats, sizeof(stats));
    print_remote_stats(&s);
    process_post(&shell_netperf_process, CONTINUE_EVENT, NULL);
    break;
  case CTRL_COMMAND_CLEAR:
    clear_stats();
    break;
  }
}
const static struct runicast_callbacks runicast_callbacks =
  { recv_ctrl, sent_ctrl, timedout_ctrl };
/*---------------------------------------------------------------------------*/
static int
send_ctrl_command(const rimeaddr_t *to, uint8_t command)
{
  struct ctrl_msg *msg;
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct ctrl_msg));
  msg = packetbuf_dataptr();
  msg->command = command;
  return runicast_send(&ctrl, to, MAX_RETRIES);
}
/*---------------------------------------------------------------------------*/
static void
recv_broadcast(struct broadcast_conn *c, const rimeaddr_t *from)
{
  process_incoming_packet();
  if(is_sender) {
  } else if(construct_reply(from)) {
    broadcast_send(&broadcast);
  }
}
const static struct broadcast_callbacks broadcast_callbacks =
  { recv_broadcast };
/*---------------------------------------------------------------------------*/
static void
recv_unicast(struct unicast_conn *c, const rimeaddr_t *from)
{
  process_incoming_packet();
  if(is_sender) {
    struct datapath_msg *msg = packetbuf_dataptr();
    if(msg->datapath_command == DATAPATH_COMMAND_ECHO_REPLY) {
      process_post(&shell_netperf_process, CONTINUE_EVENT, NULL);
    }
  } else if(construct_reply(from)) {
    unicast_send(&unicast, from);
  }
}
const static struct unicast_callbacks unicast_callbacks =
  { recv_unicast };
/*---------------------------------------------------------------------------*/
static void
print_usage(void)
{
  shell_output_str(&netperf_command,
		   "netperf [-b|u|p|s] <receiver> <num packets>: perform network measurements to receiver", "");
  shell_output_str(&netperf_command,
		   "        -b measure broadcast performance", "");
  shell_output_str(&netperf_command,
		   "        -u measure one-way unicast performance", "");
  shell_output_str(&netperf_command,
		   "        -p measure ping-pong unicast performance", "");
  shell_output_str(&netperf_command,
		   "        -s measure ping-pong stream unicast performance", "");
}
/*---------------------------------------------------------------------------*/
void
shell_netperf_init(void)
{
  runicast_open(&ctrl, SHELL_RIME_CHANNEL_NETPERF, &runicast_callbacks);
  broadcast_open(&broadcast, SHELL_RIME_CHANNEL_NETPERF + 1, &broadcast_callbacks);
  unicast_open(&unicast, SHELL_RIME_CHANNEL_NETPERF + 2, &unicast_callbacks);
  mesh_open(&mesh, SHELL_RIME_CHANNEL_NETPERF + 3, &mesh_callbacks);
  rucb_open(&rucb, SHELL_RIME_CHANNEL_NETPERF + 5, &rucb_callbacks);
  shell_register_command(&netperf_command);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_netperf_process, ev, data)
{
  static struct etimer e;
  static rimeaddr_t receiver;
  const char *nextptr;
  const char *args;
  static char recvstr[40];
  static int i, num_packets;
  static uint8_t do_broadcast, do_unicast, do_pingpong, do_stream_pingpong;

  PROCESS_BEGIN();

  current_type = TYPE_NONE;
  
  do_broadcast = do_unicast = do_pingpong =
    do_stream_pingpong = 0;
  
  args = data;

  /* Parse the -bups options */
  while(*args == '-') {
    ++args;
    while(*args != ' ' &&
	  *args != 0) {
      if(*args == 'b') {
	do_broadcast = 1;
      }
      if(*args == 'u') {
	do_unicast = 1;
      }
      if(*args == 'p') {
	do_pingpong = 1;
      }
      if(*args == 's') {
	do_stream_pingpong = 1;
      }
      ++args;
    }
    while(*args == ' ') {
      args++;
    }
  }

  /* Parse the receiver address */
  receiver.u8[0] = shell_strtolong(args, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    print_usage();
    PROCESS_EXIT();
  }
  args = nextptr + 1;
  receiver.u8[1] = shell_strtolong(args, &nextptr);

  /* Store the receiver address as a string since we need to print it
     out later. */
  snprintf(recvstr, sizeof(recvstr), "%d.%d",
	   receiver.u8[0], receiver.u8[1]);

  /* Parse the number of packets to send */
  args = nextptr;
  while(*args == ' ') {
    ++args;
  }
  num_packets = shell_strtolong(args, &nextptr);  
  if(nextptr == data || num_packets == 0) {
    print_usage();
    PROCESS_EXIT();
  }

  /* Send broadcast packets, if requested */
  if(do_broadcast) {
    current_type = TYPE_BROADCAST;
    shell_output_str(&netperf_command, "-------- Broadcast --------", "");
    
    shell_output_str(&netperf_command, "Contacting ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_CLEAR)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    shell_output_str(&netperf_command, "Measuring broadcast performance to ", recvstr);
    
    setup_sending(&receiver, num_packets);
    for(i = 0; i < num_packets; ++i) {
      if(construct_next_packet()) {
	broadcast_send(&broadcast);
	stats.sent++;
      }
      PROCESS_PAUSE();
    }
    
    shell_output_str(&netperf_command, "Requesting statistics from ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_STATS)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    /* Wait for reply */
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    finalize_stats(&stats);
    print_local_stats(&stats);
  }

  if(do_unicast) {
    current_type = TYPE_UNICAST;
    shell_output_str(&netperf_command, "-------- Unicast one-way --------", "");
    
    shell_output_str(&netperf_command, "Contacting ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_CLEAR)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    shell_output_str(&netperf_command, "Measuring unicast performance to ", recvstr);
    
    setup_sending(&receiver, num_packets);
    
    for(i = 0; i < num_packets; ++i) {
      if(construct_next_packet()) {
	unicast_send(&unicast, &receiver);
	stats.sent++;
      }
      PROCESS_PAUSE();
    }
    
    shell_output_str(&netperf_command, "Requesting statistics from ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_STATS)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    /* Wait for reply */
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    finalize_stats(&stats);
    print_local_stats(&stats);
  }
  if(do_pingpong) {
    current_type = TYPE_UNICAST_PINGPONG;
    shell_output_str(&netperf_command, "-------- Unicast ping-pong--------", "");
    
    shell_output_str(&netperf_command, "Contacting ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_CLEAR)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    shell_output_str(&netperf_command, "Measuring two-way unicast performance to ", recvstr);
    
    setup_sending(&receiver, num_packets);
    
    for(i = 0; i < num_packets; ++i) {
      if(construct_next_echo()) {
        
	unicast_send(&unicast, &receiver);
	stats.sent++;
      }
      etimer_set(&e, CLOCK_SECOND);
      PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT || etimer_expired(&e));
    }
    
    shell_output_str(&netperf_command, "Requesting statistics from ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_STATS)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    /* Wait for reply */
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    finalize_stats(&stats);
    print_local_stats(&stats);
  }
  if(do_stream_pingpong) {
    current_type = TYPE_UNICAST_STREAM;
    shell_output_str(&netperf_command, "-------- Unicast stream ping-pong--------", "");
    
    shell_output_str(&netperf_command, "Contacting ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_CLEAR)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    shell_output_str(&netperf_command, "Measuring two-way unicast stream performance to ", recvstr);
    
    setup_sending(&receiver, num_packets);
    
    for(i = 0; i < num_packets; ++i) {
      if(construct_next_stream_echo()) {
	unicast_send(&unicast, &receiver);
	stats.sent++;
      }
      etimer_set(&e, CLOCK_SECOND);
      PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT || etimer_expired(&e));
    }
    
    shell_output_str(&netperf_command, "Requesting statistics from ", recvstr);
    while(!send_ctrl_command(&receiver, CTRL_COMMAND_STATS)) {
      PROCESS_PAUSE();
    }
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    /* Wait for reply */
    PROCESS_YIELD_UNTIL(ev == CONTINUE_EVENT);
    
    finalize_stats(&stats);
    print_local_stats(&stats);
    
  }

  shell_output_str(&netperf_command, "Done", "");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
