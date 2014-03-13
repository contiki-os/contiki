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
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"

#include "net/rime/rime.h"
#include "net/rime/meshconn.h"

#include <stdio.h>
#include <string.h>

static struct meshconn_conn meshconn;
static struct process *front_process;

static int initiator = 0;

/*---------------------------------------------------------------------------*/
PROCESS(shell_rsh_process, "rsh");
SHELL_COMMAND(rsh_command,
	      "rsh",
	      "rsh <node>: remote login to another node",
	      &shell_rsh_process);
PROCESS(shell_rsh_server_process, "rsh server");
/*SHELL_COMMAND(rsh_server_command,
	      "rsh server",
	      "rsh server: never invoked directly",
	      &shell_rsh_server_process);*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rsh_process, ev, data)
{
  static linkaddr_t receiver;
  struct shell_input *input;
  const char *nextptr;
  char buf[40];
  
  PROCESS_BEGIN();
  
  receiver.u8[0] = shell_strtolong(data, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    shell_output_str(&rsh_command,
		     "rsh <node>: node address must be specified", "");
    PROCESS_EXIT();
  }
  ++nextptr;
  receiver.u8[1] = shell_strtolong(nextptr, &nextptr);

  sprintf(buf, "%d.%d", receiver.u8[0], receiver.u8[1]);
  shell_output_str(&rsh_command, "Connecting to ", buf);

  initiator = 1;
  meshconn_connect(&meshconn, &receiver);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == shell_event_input) {
      input = data;
      if(input->len1 + input->len2 == 0) {
	meshconn_close_connection(&meshconn);
	PROCESS_EXIT();
      }
      if(input->len1 + input->len2 >= PACKETBUF_SIZE) {
	shell_output_str(&rsh_command, "rsh: input too long", "");
      } else {
	packetbuf_clear();
	memcpy(packetbuf_dataptr(), input->data1, input->len1);
	memcpy((char *)packetbuf_dataptr() + input->len1,
	       input->data2, input->len2);
	packetbuf_set_datalen(input->len1 + input->len2);
	meshconn_send(&meshconn);
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rsh_server_process, ev, data)
{
  struct shell_input *input;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input ||
			     (ev == PROCESS_EVENT_EXITED &&
			      data == front_process));
    if(ev == PROCESS_EVENT_EXITED) {
      front_process = NULL;
    } else if(ev == shell_event_input) {
      input = data;
      packetbuf_clear();
      memcpy(packetbuf_dataptr(), input->data1, input->len1);
      memcpy((char *)packetbuf_dataptr() + input->len1,
	     input->data2, input->len2);
      packetbuf_set_datalen(input->len1 + input->len2);
      /*      printf("Sending meshconn with %d + %d bytes\n",
	      input->len1, input->len2);*/
      meshconn_send(&meshconn);
    }
    /*    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);*/
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
connected_meshconn(struct meshconn_conn *c)
{
  /*  printf("connected\n");*/
}
static void
recv_meshconn(struct meshconn_conn *c)
{
  /*  int ret;
      struct process *started_process;*/

  /*  printf("got '%.*s'\n", packetbuf_datalen(), (char *)packetbuf_dataptr());*/
  /* Echo reply */
  if(!initiator) {
    packetbuf_copyfrom("abcdefghijklmnopq", 18);
    meshconn_send(c);
  }
  
  return;
#if 0
  if(front_process != NULL &&
     process_is_running(front_process)) {
    struct shell_input input;
    input.data1 = packetbuf_dataptr();
    input.len1 = packetbuf_datalen();
    input.data2 = "";
    input.len2 = 0;
    process_post_synch(front_process, shell_event_input, &input);
  } else {
    
    ret = shell_start_command(packetbuf_dataptr(), packetbuf_datalen(),
			      &rsh_server_command, &started_process);
    
    if(started_process != NULL &&
       ret == SHELL_FOREGROUND &&
       process_is_running(started_process)) {
      front_process = started_process;
    }
  }
  /*  shell_input(packetbuf_dataptr(), packetbuf_datalen());*/
  /*  shell_output(&rsh_command, packetbuf_dataptr(), packetbuf_datalen(), "", 0);*/
#endif
}
static void
closed_meshconn(struct meshconn_conn *c)
{
  /*  printf("closed\n");*/
}
static void
timedout_meshconn(struct meshconn_conn *c)
{
  /*  printf("timedout\n");*/
}
static void
reset_meshconn(struct meshconn_conn *c)
{
  /*  printf("reset\n");*/
}
/*---------------------------------------------------------------------------*/
static struct meshconn_callbacks meshconn_callbacks = { connected_meshconn,
							recv_meshconn,
							closed_meshconn,
							timedout_meshconn,
							reset_meshconn };
/*---------------------------------------------------------------------------*/
void
shell_rsh_init(void)
{
  meshconn_open(&meshconn, 23, &meshconn_callbacks);
  shell_register_command(&rsh_command);

  process_start(&shell_rsh_server_process, NULL);
}
/*---------------------------------------------------------------------------*/
