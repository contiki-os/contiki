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
 * $Id: shell-tcpsend.c,v 1.5 2010/05/31 15:22:08 nifi Exp $
 */

#include <string.h>
#include <stddef.h>

#include "contiki.h"
#include "shell.h"
#include "telnet.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

/*---------------------------------------------------------------------------*/
PROCESS(shell_tcpsend_process, "tcpsend");
SHELL_COMMAND(tcpsend_command,
	      "tcpsend",
	      "tcpsend <host> <port>: open a TCP connection",
	      &shell_tcpsend_process);
/*---------------------------------------------------------------------------*/

#define MAX_SERVERLEN 16

static uip_ipaddr_t serveraddr;
static char server[MAX_SERVERLEN + 1];

static struct telnet_state s;

static unsigned char running;

#define MAX_LINELEN 80

static char outputline[MAX_LINELEN];
static uint8_t sending;
/*---------------------------------------------------------------------------*/
void
telnet_text_output(struct telnet_state *s, char *text1, char *text2)
{
  char buf1[MAX_SERVERLEN];
  int len;
  
  strncpy(buf1, text1, sizeof(buf1));
  len = strlen(buf1);
  if(len < sizeof(buf1) - 1) {
    buf1[len] = ' ';
    buf1[len + 1] = 0;
  }
  shell_output_str(&tcpsend_command, buf1, text2);
}
/*---------------------------------------------------------------------------*/
void
telnet_newdata(struct telnet_state *s, char *data, uint16_t len)
{
  shell_output(&tcpsend_command, data, len, "", 0);
}
/*---------------------------------------------------------------------------*/
static void
send_line(struct telnet_state *s, char *data, int len)
{
  if(!sending) {
    strncpy(outputline, data, MIN(sizeof(outputline), len));
    telnet_send(s, data, len);
    sending = 1;
  } else {
    shell_output_str(&tcpsend_command, "Cannot send data, still sending previous data", "");
  }
}
/*---------------------------------------------------------------------------*/
void
telnet_sent(struct telnet_state *s)
{
  sending = 0;
}
/*---------------------------------------------------------------------------*/
void
telnet_closed(struct telnet_state *s)
{
  telnet_text_output(s, server, "connection closed");
  running = 0;
}
/*---------------------------------------------------------------------------*/
void
telnet_aborted(struct telnet_state *s)
{
  telnet_text_output(s, server, "connection aborted");
  running = 0;
}
/*---------------------------------------------------------------------------*/
void
telnet_timedout(struct telnet_state *s)
{
  telnet_text_output(s, server, "connection timed out");
  running = 0;
}
/*---------------------------------------------------------------------------*/
void
telnet_connected(struct telnet_state *s)
{
  telnet_text_output(s, server, "connected");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_tcpsend_process, ev, data)
{
  char *next;
  const char *dummy; 
  struct shell_input *input;
  uint16_t port;
  
  PROCESS_BEGIN();

  next = strchr(data, ' ');
  if(next == NULL) {
    shell_output_str(&tcpsend_command,
		     "tcpsend <server> <port>: server as address", "");
    PROCESS_EXIT();
  }
  *next = 0;
  ++next;
  strncpy(server, data, sizeof(server));
  port = shell_strtolong(next, &dummy);
  
  running = 1;

  uiplib_ipaddrconv(server, &serveraddr);
  telnet_connect(&s, &serveraddr, port);
  while(running) {
    PROCESS_WAIT_EVENT();

    if(ev == shell_event_input) {
      input = data;
      if(input->len1 + input->len2 == 0) {
	PROCESS_EXIT();
      }

      if(input->len1 > 0) {
	send_line(&s, input->data1, input->len1);
      }
    } else if(ev == tcpip_event) {
      telnet_app(data);
#if 0            
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 resolv_lookup((char *)data) != NULL) {
	uip_ipaddr_copy(serveraddr, ipaddr);
	telnet_connect(&s, server, serveraddr, nick);
      } else {
	shell_output_str(&tcpsend_command, "Host not found.", "");
      }
#endif /* 0 */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_tcpsend_init(void)
{
  shell_register_command(&tcpsend_command);
}
/*---------------------------------------------------------------------------*/
