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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * $Id: contiki-main.c,v 1.4 2007/11/20 21:19:12 oliverschmidt Exp $
 */

#include <stdio.h>

#include "contiki-net.h"
#include "net/ethernet-drv.h"

PROCINIT(&etimer_process,
	 &tcpip_process);

void clock_update(void);

/*-----------------------------------------------------------------------------------*/
void
uip_log(char *message)
{
  fprintf(stderr, "%s\n", message);
}
/*-----------------------------------------------------------------------------------*/
void
log_message(const char *part1, const char *part2)
{
  fprintf(stderr, "%s%s\n", part1, part2);
}
/*-----------------------------------------------------------------------------------*/
void
main(void)
{
  process_init();

  procinit_init();

  autostart_start((struct process **)autostart_processes);

#if 1
  {
    static struct ethernet_config config = {0xC0B0, "CS8900A.ETH"};
    uip_ipaddr_t addr;

    process_start((struct process *)&ethernet_process, (char *)&config);

    uip_ipaddr(&addr, 192,168,0,128);
    uip_sethostaddr(&addr);

    uip_ipaddr(&addr, 255,255,255,0);
    uip_setnetmask(&addr);

    uip_ipaddr(&addr, 192,168,0,1);
    uip_setdraddr(&addr);

    uip_ipaddr(&addr, 192,168,0,1);
    resolv_conf(&addr);
  }
#endif

  clock_init();

  printf("Contiki initiated, now starting process scheduling\n");
  
  while(1) {

    clock_update();

    if(process_run() < 2) {

      etimer_request_poll();
    }
  }
}
/*-----------------------------------------------------------------------------------*/
