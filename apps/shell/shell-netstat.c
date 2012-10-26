/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "contiki.h"
#include "shell.h"
#include "contiki-net.h"

static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56,
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e,
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49,
 0x53, 0x48, 0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49,
 0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41,
 0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43,
 0x4b, 0};
static const char none[] = /*  "NONE"*/
{0x4e, 0x4f, 0x4e, 0x45, 0};
static const char running[] = /*  "RUNNING"*/
{0x52, 0x55, 0x4e, 0x4e, 0x49, 0x4e, 0x47,
 0};
static const char called[] = /*  "CALLED"*/
{0x43, 0x41, 0x4c, 0x4c, 0x45, 0x44, 0};
static const char file_name[] = /*  "file-stats"*/
{0x66, 0x69, 0x6c, 0x65, 0x2d, 0x73, 0x74,
 0x61, 0x74, 0x73, 0};
static const char tcp_name[] = /*  "tcp-connections"*/
{0x74, 0x63, 0x70, 0x2d, 0x63, 0x6f, 0x6e,
 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e,
 0x73, 0};
static const char proc_name[] = /*  "processes"*/
{0x70, 0x72, 0x6f, 0x63, 0x65, 0x73, 0x73,
 0x65, 0x73, 0};

static const char *states[] = {
  closed,
  syn_rcvd,
  syn_sent,
  established,
  fin_wait_1,
  fin_wait_2,
  closing,
  time_wait,
  last_ack,
  none,
  running,
  called};

#define BUFLEN 100

/*---------------------------------------------------------------------------*/
PROCESS(shell_netstat_process, "netstat");
SHELL_COMMAND(netstat_command,
	      "netstat",
	      "netstat: show UDP and TCP connections",
	      &shell_netstat_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_netstat_process, ev, data)
{
  char buf[BUFLEN];
  int i;
  struct uip_conn *conn;
  PROCESS_BEGIN();

  for(i = 0; i < UIP_CONNS; ++i) {
    conn = &uip_conns[i];
    snprintf(buf, BUFLEN,
	     "%d, %u.%u.%u.%u:%u, %s, %u, %u, %c %c",
	     uip_htons(conn->lport),
	     conn->ripaddr.u8[0],
	     conn->ripaddr.u8[1],
	     conn->ripaddr.u8[2],
	     conn->ripaddr.u8[3],
	     uip_htons(conn->rport),
	     states[conn->tcpstateflags & UIP_TS_MASK],
	     conn->nrtx,
	     conn->timer,
	     (uip_outstanding(conn))? '*':' ',
	     (uip_stopped(conn))? '!':' ');
    shell_output_str(&netstat_command, "TCP ", buf);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_netstat_init(void)
{
  shell_register_command(&netstat_command);
}
/*---------------------------------------------------------------------------*/
