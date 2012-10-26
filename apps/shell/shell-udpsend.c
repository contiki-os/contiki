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

#include "contiki.h"
#include "shell.h"
#include "contiki-net.h"

/*---------------------------------------------------------------------------*/
PROCESS(shell_udpsend_process, "udpsend");
SHELL_COMMAND(udpsend_command,
	      "udpsend",
	      "udpsend <host> <remote port> [local port]: send UDP data",
	      &shell_udpsend_process);
/*---------------------------------------------------------------------------*/

#define MAX_SERVERLEN 16

static struct uip_udp_conn *udpconn;

static uip_ipaddr_t serveraddr;
static char server[MAX_SERVERLEN + 1];

static unsigned char running;

/*---------------------------------------------------------------------------*/
static void
send_line(char *line, int len)
{
  uip_udp_packet_send(udpconn, line, len);
}
/*---------------------------------------------------------------------------*/
static void
newdata(char *data, uint16_t len)
{
  shell_output(&udpsend_command, data, len, "", 0);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_udpsend_process, ev, data)
{
  const char *next, *nextptr;
  struct shell_input *input;
  uint16_t port, local_port;
  
  PROCESS_BEGIN();

  next = strchr(data, ' ');
  if(next == NULL) {
    shell_output_str(&udpsend_command,
		     "udpsend <server> <port> [localport]: server as address", "");
    PROCESS_EXIT();
  }
  if(next - (char *)data > sizeof(server)) {
    shell_output_str(&udpsend_command, "Too long input", "");
    PROCESS_EXIT();
  }
  strncpy(server, data, sizeof(server));
  /* NULL-terminate the server string. */
  server[next - (char *)data] = 0;
  ++next;
  port = shell_strtolong(next, &nextptr);

  uiplib_ipaddrconv(server, &serveraddr);
  udpconn = udp_new(&serveraddr, uip_htons(port), NULL);
  
  if(next != nextptr) {
    local_port = shell_strtolong(nextptr, &nextptr);
    udp_bind(udpconn, uip_htons(local_port));
  }
  running = 1;


  while(running) {
    PROCESS_WAIT_EVENT();

    if(ev == shell_event_input) {
      input = data;
      if(input->len1 + input->len2 == 0) {
	PROCESS_EXIT();
      }

      if(input->len1 > 0) {
	send_line(input->data1, input->len1);
      }
    } else if(ev == tcpip_event) {
      if(uip_newdata()) {
	newdata(uip_appdata, uip_datalen());
      }
#if 0
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 resolv_lookup((char *)data) != NULL) {
	uip_ipaddr_copy(serveraddr, ipaddr);
	telnet_connect(&s, server, serveraddr, nick);
      } else {
	shell_output_str(&udpsend_command, "Host not found.", "");
      }
#endif /* 0 */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_udpsend_init(void)
{
  shell_register_command(&udpsend_command);
}
/*---------------------------------------------------------------------------*/
