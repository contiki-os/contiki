/*
 * Copyright (c) 2007, Takahide Matsutsuka.
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
 * $Id: contiki-client-main.c,v 1.2 2007/11/28 06:14:11 matsutsuka Exp $
 *
 */

/*
 * \file
 * 	This is a sample main file with slip network.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

//#define WITH_LOADER_ARCH		1

#include "contiki.h"

/* devices */
#include "net/uip.h"
#include "net/uip-fw-drv.h"
#include "dev/slip.h"
#include "dev/rs232.h"
#include "sys/process.h"

/* desktop programs */
#include "program-handler.h"
//#include "process-list-dsc.h"

/* network programs */
#include "netconf-dsc.h"
#include "www-dsc.h"
#include "telnet-dsc.h"
//#include "dhcp-dsc.h"
#include "email-dsc.h"
#include "ftp-dsc.h"
#include "irc-dsc.h"


/*---------------------------------------------------------------------------*/
/* inteface */
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(0, 0, 0, 0,  0, 0, 0, 0, slip_send)};

/* ip address of contiki */
const uip_ipaddr_t hostaddr = { { 10, 0, 1, 10 } };

PROCESS_NAME(netconf_process);
PROCESS_NAME(email_process);
PROCESS_NAME(www_process);
PROCESS_NAME(simpletelnet_process);
PROCESS_NAME(ftp_process);
PROCESS_NAME(irc_process);

/*---------------------------------------------------------------------------*/
int
main(void)
{
  /* initialize process manager. */
  process_init();

  uip_init();
  uip_sethostaddr(&hostaddr);
  uip_fw_default(&slipif);

  /* start services */
  process_start(&etimer_process, NULL);
  process_start(&ctk_process, NULL);
//  process_start(&program_handler_process, NULL);
  process_start(&tcpip_process, NULL);
  process_start(&slip_process, NULL);
  process_start(&uip_fw_process, NULL);
  process_start(&rs232_process, NULL);

//  process_start(&email_process, NULL);
//  process_start(&www_process, NULL);
//  process_start(&ftp_process, NULL);
//  process_start(&irc_process, NULL);

//  process_start(&netconf_process, NULL);
  process_start(&simpletelnet_process, NULL);

#if 0
  /* register programs to the program handler */
  program_handler_add(&processes_dsc, "Processes", 1);
  program_handler_add(&netconf_dsc, "Network conf", 1);
  program_handler_add(&email_dsc, "E-mail", 1);
  program_handler_add(&irc_dsc, "IRC", 1);
  program_handler_add(&vnc_dsc, "VNC client", 1);
  program_handler_add(&dhcp_dsc, "DHCP client", 1);
#endif
  while(1) {
    process_run();
    etimer_request_poll();
  }
}
