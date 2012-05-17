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
 *         Wget Contiki shell command
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"

#include "webclient.h"

#include <stdio.h>
#include <string.h>

#define MAX_URLLEN 128
static char url[MAX_URLLEN];

static uint8_t running;

/*---------------------------------------------------------------------------*/
PROCESS(shell_wget_process, "wget");
SHELL_COMMAND(wget_command,
	      "wget",
	      "wget [URL]: download a file with HTTP",
	      &shell_wget_process);
/*---------------------------------------------------------------------------*/
static void
open_url(char *url)
{
  unsigned char i, c;
  static char host[32];
  char *file;
  register char *urlptr;
  static uip_ipaddr_t addr;

  /* Trim off any spaces in the end of the url. */
  urlptr = url + strlen(url) - 1;
  while(*urlptr == ' ' && urlptr > url) {
    *urlptr = 0;
    --urlptr;
  }

  /* Don't even try to go further if the URL is empty. */
  if(urlptr == url) {
    return;
  }

  /* See if the URL starts with http://, otherwise prepend it. */
  if(strncmp(url, http_http, 7) != 0) {
    while(urlptr >= url) {
      *(urlptr + 7) = *urlptr;
      --urlptr;
    }
    strncpy(url, http_http, 7);
  }

  /* Find host part of the URL. */
  urlptr = &url[7];
  if(*urlptr == '[') {
    /* IPv6 address */
    c = ']';
    ++urlptr;
  } else {
    c = ':';
  }
  for(i = 0; i < sizeof(host); ++i) {
    if(*urlptr == 0 ||
       *urlptr == '/' ||
       *urlptr == ' ' ||
       *urlptr == c) {
      host[i] = 0;
      break;
    }
    host[i] = *urlptr;
    ++urlptr;
  }
  if(*urlptr == ']') {
    /* Move past end of IPv6 host */
    urlptr++;
  }

  /* XXX: Here we should find the port part of the URL, but this isn't
     currently done because of laziness from the programmer's side
     :-) */
  
  /* Find file part of the URL. */
  while(*urlptr != '/' && *urlptr != 0) {
    ++urlptr;
  }
  if(*urlptr == '/') {
    file = urlptr;
  } else {
    file = "/";
  }
  
#if UIP_UDP
  /* Try to lookup the hostname. If it fails, we initiate a hostname
     lookup and print out an informative message on the statusbar. */
  if(uiplib_ipaddrconv(host, &addr) == 0) {
    uip_ipaddr_t *addrptr;
    shell_output_str(&wget_command, "Not an IP address", "");
    if(resolv_lookup(host, &addrptr) == RESOLV_STATUS_UNCACHED) {
      shell_output_str(&wget_command, "Not resolved", "");
      resolv_query(host);
      return;
    }
    uip_ipaddr_copy(&addr, addrptr);
  }
#else /* UIP_UDP */
  uiplib_ipaddrconv(host, &addr);
#endif /* UIP_UDP */

  /* The hostname we present in the hostname table, so we send out the
     initial GET request. */
  shell_output_str(&wget_command, "Host ", host);
  shell_output_str(&wget_command, "File ", file);

  if(webclient_get(host, 80, file) == 0) {
    shell_output_str(&wget_command, "Out of memory", "");
  } else {
    shell_output_str(&wget_command, "Connecting...", "");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_wget_process, ev, data)
{
  PROCESS_BEGIN();

  strncpy(url, data, sizeof(url));
  open_url(url);

  running = 1;
  
  while(running) {
    PROCESS_WAIT_EVENT();
    
    if(ev == tcpip_event) {
      webclient_appcall(data);
#if UIP_UDP
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 resolv_lookup((char *)data, NULL) == RESOLV_STATUS_CACHED) {
	open_url(url);
      } else {
	shell_output_str(&wget_command, "Host not found.", "");
      }
#endif /* UIP_UDP */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_wget_init(void)
{
  shell_register_command(&wget_command);
}
/*---------------------------------------------------------------------------*/
void
webclient_datahandler(char *data, uint16_t len)
{
  if(len == 0) {
    running = 0;
  } else {
    shell_output(&wget_command, data, len, NULL, 0);
  }
}
/*---------------------------------------------------------------------------*/
void
webclient_connected(void)
{
  shell_output_str(&wget_command, "connected", "");
}
/*---------------------------------------------------------------------------*/
void
webclient_timedout(void)
{
  shell_output_str(&wget_command, "timedout", "");
  running = 0;
}
/*---------------------------------------------------------------------------*/
void
webclient_aborted(void)
{
  shell_output_str(&wget_command, "aborted", "");
  running = 0;
}
/*---------------------------------------------------------------------------*/
void
webclient_closed(void)
{
  shell_output_str(&wget_command, "closed", "");
  running = 0;
}
/*---------------------------------------------------------------------------*/
