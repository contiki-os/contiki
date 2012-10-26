/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 *         Multi-hop telnetd for the Tmote Sky
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "webserver-nogui.h"
#include "telnetd.h"
#include "shell.h"

#include "contiki.h"
#include "contiki-net.h"
#include "net/rime.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(sky_telnetd_process, "Sky telnet process");
AUTOSTART_PROCESSES(&telnetd_process, &sky_telnetd_process, &webserver_nogui_process);
/*---------------------------------------------------------------------------*/
PROCESS(shell_id_process, "id");
SHELL_COMMAND(id_command,
	      "id",
	      "id: print the identity of the node",
	      &shell_id_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_id_process, ev, data)
{
  char buf[40];
  PROCESS_BEGIN();
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", uip_ipaddr_to_quad(&uip_hostaddr));
  shell_output_str(&id_command, "IP address: ", buf);
  snprintf(buf, sizeof(buf), "%d.%d",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  shell_output_str(&id_command, "Rime address: ", buf);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sky_telnetd_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  shell_blink_init();
  shell_ps_init();
  shell_rime_init();
  shell_rime_ping_init();
  shell_rime_netcmd_init();

  shell_register_command(&id_command);
  
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  
  PROCESS_END();
    
}
/*---------------------------------------------------------------------------*/
