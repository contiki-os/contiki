/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki desktop environment
 *
 * $Id: contiki-main.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 *
 */

#include <stdio.h>
#include <gdk/gdktypes.h>
#include <gtk/gtk.h>

#include "contiki.h"
#include "contiki-net.h"

#include "ctk/ctk.h"

#include "ctk/ctk-vncserver.h"

#include "net/tapdev-drv.h"
#include "program-handler.h"
#include "webserver.h"
#include "ctk/ctk-gtksim-service.h"

#include "about-dsc.h"
#include "calc-dsc.h"
#include "dhcp-dsc.h"
#include "email-dsc.h"
#include "ftp-dsc.h"
#include "irc-dsc.h"
#include "netconf-dsc.h"
#include "process-list-dsc.h"
#include "shell-dsc.h"
#include "www-dsc.h"

#include "cmdd.h"

static struct uip_fw_netif tapif =
  {UIP_FW_NETIF(0,0,0,0, 0,0,0,0, tapdev_output)};

PROCESS(init_process, "Init");

PROCESS_THREAD(init_process, ev, data)
{
  uip_ipaddr_t addr;

  PROCESS_BEGIN();
  
  uip_ipaddr(&addr, 192,168,2,2);
  uip_sethostaddr(&addr);

  uip_ipaddr(&addr, 192,168,2,1);
  uip_setdraddr(&addr);

  uip_ipaddr(&addr, 255,255,255,0);
  uip_setnetmask(&addr);

  printf("init\n");
  
  program_handler_add(&netconf_dsc, "Network setup", 1);
  program_handler_add(&ftp_dsc, "FTP client", 1);
  program_handler_add(&www_dsc, "Web browser", 1);
  program_handler_add(&processes_dsc, "Processes", 1);
  program_handler_add(&shell_dsc, "Command shell", 1);
  program_handler_add(&calc_dsc, "Calculator", 1);
  /*  program_handler_add(&email_dsc, "E-mail", 1);*/

  program_handler_add(&irc_dsc, "IRC", 1);
  /*  program_handler_add(&vnc_dsc, "VNC client", 1);*/

  program_handler_add(&dhcp_dsc, "DHCP client", 1);

  uip_fw_default(&tapif);

  while(1) {
    PROCESS_WAIT_EVENT();
  }
  
  PROCESS_END();
}

PROCINIT(&tcpip_process, &ctk_process, &tapdev_process,
	 &ctk_gtksim_service_process, &resolv_process,
	 &program_handler_process,
	 &uip_fw_process,
	 &init_process);

static gint
idle_callback(gpointer data)
{
  process_run();
  return TRUE;
}
/*-----------------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  uip_ipaddr_t addr;

  gtk_init(&argc, &argv);
  
  process_init();

  procinit_init();
  
  uip_ipaddr(&addr, 192,168,2,2);
  uip_sethostaddr(&addr);

  uip_ipaddr(&addr, 192,168,2,1);
  uip_setdraddr(&addr);

  uip_ipaddr(&addr, 255,255,255,0);
  uip_setnetmask(&addr);

  /*  program_handler_add(&netconf_dsc, "Network setup", 1);
  program_handler_add(&ftp_dsc, "FTP client", 1);
  program_handler_add(&editor_dsc, "Editor", 1);
  program_handler_add(&www_dsc, "Web browser", 1);
  program_handler_add(&processes_dsc, "Processes", 1);
  program_handler_add(&shell_dsc, "Command shell", 1);
  program_handler_add(&calc_dsc, "Calculator", 1);*/
  /*  program_handler_add(&email_dsc, "E-mail", 1);*/

  /*  program_handler_add(&irc_dsc, "IRC", 1);*/
  /*  program_handler_add(&vnc_dsc, "VNC client", 1);*/

  /*  program_handler_add(&dhcp_dsc, "DHCP client", 1);*/

  uip_fw_default(&tapif);
  
  gtk_timeout_add(20, idle_callback, NULL);
  gtk_main();

  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#undef BYTE_ORDER
#undef UIP_HTONS

/*-----------------------------------------------------------------------------------*/

void nntpc_done(int i) {}

void log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}

void
uip_log(char *m)
{
  printf("%s\n", m);
}
