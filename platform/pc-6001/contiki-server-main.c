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
 *
 */

/*
 * \file
 * 	This is a sample main file with slip network.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#include "contiki.h"

/* devices */
#include "uip.h"
#include "dev/slip.h"
#include "dev/rs232.h"

/* network server programs */
#include "webserver-nogui.h"
#include "cmdd.h"
#include "telnetd.h"
#include "ctk/libconio_arch-small.h"

/*---------------------------------------------------------------------------*/
/* inteface */
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(0, 0, 0, 0,  0, 0, 0, 0, slip_send)};

/* ip address of contiki */
const uip_ipaddr_t hostaddr = { { 10, 0, 1, 10 } };

/*---------------------------------------------------------------------------*/
int
main(void)
{
  /* initialize process manager. */
  process_init();

  clrscr_arch();
  uip_init();
  uip_sethostaddr(&hostaddr);
  uip_fw_default(&slipif);

  /* start services */
  process_start(&etimer_process, NULL);
  process_start(&tcpip_process, NULL);
  process_start(&slip_process, NULL);
  process_start(&uip_fw_process, NULL);
  process_start(&rs232_process, NULL);

  process_start(&webserver_nogui_process, NULL);
//  process_start(&cmdd_process, NULL);
//  process_start(&telnetd_process, NULL);

  while(1) {
    process_run();
    etimer_request_poll();
  }
}
