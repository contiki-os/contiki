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
#include "contiki-conf.h"
#include "shell-rime.h"

#include "dev/leds.h"

#include "lib/crc16.h"
#include "lib/random.h"

#include "net/rime.h"
#include "net/rime/route.h"
#include "net/rime/trickle.h"

#include "net/rime/timesynch.h"

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif /* CONTIKI_TARGET_NETSIM */

#include <stdio.h>
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */
#include <string.h>


#define COLLECT_REXMITS 4

#define TRICKLEMSG_HDR_SIZE 2

struct trickle_msg {
  uint16_t crc;
  char netcmd[1];
};

static struct trickle_conn trickle;
/*---------------------------------------------------------------------------*/
PROCESS(shell_netcmd_process, "netcmd");
PROCESS(shell_netcmd_server_process, "netcmd server");
SHELL_COMMAND(netcmd_command,
	      "netcmd",
	      "netcmd <command>: run a command on all nodes in the network",
	      &shell_netcmd_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_netcmd_server_process, ev, data)
{
  static struct process *child_command;
  int err;
  PROCESS_BEGIN();

  /* XXX: direct output to null. */
  printf("netcmd server got command string '%s'\n", (char *)data);
  err = shell_start_command(data, strlen((char * )data), NULL, &child_command);
  if(err == SHELL_FOREGROUND && process_is_running(child_command)) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			     (ev == PROCESS_EVENT_EXITED &&
			      data == child_command));
    if(ev == PROCESS_EVENT_EXIT) {
      process_exit(child_command);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_netcmd_process, ev, data)
{
  struct trickle_msg *msg;
  int len;
  
  PROCESS_BEGIN();

  /* Get the length of the command line, excluding a terminating NUL character. */
  len = strlen((char *)data);

  /* Check the length of the command line to see that it is small
     enough to fit in a packet. We count with 32 bytes of header,
     which may be a little too much, but at least we are on the safe
     side. */
  if(len > PACKETBUF_SIZE - 32) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", len);
    shell_output_str(&netcmd_command, "command line too large: ", buf);
  } else {
    
    packetbuf_clear();
    msg = packetbuf_dataptr();
    packetbuf_set_datalen(len + 1 + TRICKLEMSG_HDR_SIZE);
    strcpy(msg->netcmd, data);

    /* Terminate the string with a NUL character. */
    msg->netcmd[len] = 0;
    msg->crc = crc16_data(msg->netcmd, len, 0);
    printf("netcmd sending '%s'\n", msg->netcmd);
    trickle_send(&trickle);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
recv_trickle(struct trickle_conn *c)
{
  struct trickle_msg *msg;
  uint16_t crc;
  int len;
  
  msg = packetbuf_dataptr();

  if(packetbuf_datalen() > 1 + TRICKLEMSG_HDR_SIZE) {
    
    /* First ensure that the old process is killed. */
    process_exit(&shell_netcmd_server_process);
    
    len = packetbuf_datalen() - 1 - TRICKLEMSG_HDR_SIZE;
    
    /* Make sure that the incoming command is null-terminated. */
    msg->netcmd[len] = 0;
    memcpy(&crc, &msg->crc, sizeof(crc));
    
    if(crc == crc16_data(msg->netcmd, len, 0)) {
      /* Start the server process with the incoming command. */
      process_start(&shell_netcmd_server_process, msg->netcmd);
    }
  }
}
const static struct trickle_callbacks trickle_callbacks = { recv_trickle };
/*---------------------------------------------------------------------------*/
void
shell_rime_netcmd_init(void)
{
  trickle_open(&trickle, CLOCK_SECOND * 4, SHELL_RIME_CHANNEL_NETCMD,
	       &trickle_callbacks);
  shell_register_command(&netcmd_command);
}
/*---------------------------------------------------------------------------*/
