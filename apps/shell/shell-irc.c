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
#include "ircc.h"


/*---------------------------------------------------------------------------*/
PROCESS(shell_irc_process, "irc");
SHELL_COMMAND(irc_command,
	      "irc",
	      "irc <server> <nick>: start an IRC chat",
	      &shell_irc_process);
/*---------------------------------------------------------------------------*/

#define MAX_SERVERLEN 16
#define MAX_NICKLEN 16

static uip_ipaddr_t serveraddr;
static char server[MAX_SERVERLEN + 1];
static char nick[MAX_NICKLEN + 1];

static struct ircc_state s;

static unsigned char running;

/*---------------------------------------------------------------------------*/
void
ircc_text_output(struct ircc_state *s, char *text1, char *text2)
{
  char buf1[MAX_NICKLEN + MAX_SERVERLEN];
  int len;
  
  strncpy(buf1, text1, sizeof(buf1));
  len = strlen(buf1);
  if(len < sizeof(buf1) - 1) {
    buf1[len] = ' ';
    buf1[len + 1] = 0;
  }
  shell_output_str(&irc_command, buf1, text2);
}
/*---------------------------------------------------------------------------*/
static void
parse_line(char *line)
{
  if(line[0] == '/') {
    if(strncmp(&line[1], "join", 4) == 0) {
      ircc_join(&s, &line[6]);
      ircc_text_output(&s, "Join", &line[6]);
    } else if(strncmp(&line[1], "list", 4) == 0) {
      ircc_list(&s);
      ircc_text_output(&s, "Channel list", "");
    } else if(strncmp(&line[1], "part", 4) == 0) {
      ircc_part(&s);
      ircc_text_output(&s, "Leaving channel", "");
    } else if(strncmp(&line[1], "quit", 4) == 0) {
      ircc_quit(&s);
    } else if(strncmp(&line[1], "me", 2) == 0) {
      ircc_actionmsg(&s, &line[4]);
      ircc_text_output(&s, "*", &line[4]);
    } else {
      ircc_text_output(&s, &line[1], "Not implemented");
    }
  } else {
    ircc_msg(&s, &line[0]);
    ircc_text_output(&s, nick, line);
  }
}
/*---------------------------------------------------------------------------*/
void
ircc_sent(struct ircc_state *s)
{

}
/*---------------------------------------------------------------------------*/
void
ircc_closed(struct ircc_state *s)
{
  ircc_text_output(s, server, "connection closed");
  running = 0;
}
/*---------------------------------------------------------------------------*/
void
ircc_connected(struct ircc_state *s)
{
  ircc_text_output(s, server, "connected");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_irc_process, ev, data)
{
  char *next;
  struct shell_input *input;
  
  PROCESS_BEGIN();

  next = strchr(data, ' ');
  if(next == NULL) {
    shell_output_str(&irc_command,
		     "irc <server> <nick>: server as address", "");
    PROCESS_EXIT();
  }
  *next = 0;
  ++next;
  strncpy(server, data, sizeof(server));
  strncpy(nick, next, sizeof(nick));
  
  running = 1;

  uiplib_ipaddrconv(server, &serveraddr);
  ircc_connect(&s, server, &serveraddr, nick);
  while(running) {
    PROCESS_WAIT_EVENT();

    if(ev == shell_event_input) {
      input = data;
      if(input->len1 > 0) {
	parse_line(input->data1);
      }
    } else if(ev == tcpip_event) {
      ircc_appcall(data);
#if 0            
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 resolv_lookup((char *)data, &ipaddr) == RESOLV_STATUS_CACHED) {
	uip_ipaddr_copy(serveraddr, ipaddr);
	ircc_connect(&s, server, serveraddr, nick);
      } else {
	shell_output_str(&irc_command, "Host not found.", "");
      }
#endif /* 0 */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_irc_init(void)
{
  shell_register_command(&irc_command);
}
/*---------------------------------------------------------------------------*/
