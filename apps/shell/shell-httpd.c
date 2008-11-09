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
 * $Id: shell-httpd.c,v 1.1 2008/11/09 12:37:26 adamdunkels Exp $
 */

/**
 * \file
 *         Httpd Contiki shell command
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "dev/leds.h"
#include "dev/watchdog.h"

#include "httpd.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_httpd_process, "httpd");
SHELL_COMMAND(httpd_command,
	      "httpd",
	      "httpd: serve files with HTTP",
	      &shell_httpd_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_httpd_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_httpd_init(void)
{
  shell_register_command(&httpd_command);
}
/*---------------------------------------------------------------------------*/
void
webserver_log_file(uip_ipaddr_t *requester, char *file)
{
  char buf[48];

  /* Print out IP address of requesting host. */
  sprintf(buf, "%d.%d.%d.%d: ", requester->u8[0], requester->u8[1],
				requester->u8[2], requester->u8[3]);
  shell_output_str(&httpd_command, buf, file);
}
/*---------------------------------------------------------------------------*/
void
webserver_log(char *msg)
{
  shell_output_str(&httpd_command, msg, "");
}
/*---------------------------------------------------------------------------*/
