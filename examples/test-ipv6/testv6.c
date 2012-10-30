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

#include "contiki-net.h"
#include "webserver-nogui.h"

static const struct uip_eth_addr ethaddr = {{0x00,0x06,0x98,0x01,0x02,0x29}};

/*---------------------------------------------------------------------------*/
PROCESS(test_process, "Test");
PROCESS(test_tcpip_process, "TCP/IP test");
AUTOSTART_PROCESSES(&test_process, &test_tcpip_process, &webserver_nogui_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_process, ev, data)
{
  uip_ip6addr_t ip6addr;
  static struct etimer etimer;
  
  PROCESS_BEGIN();

  uip_ip6addr(&ip6addr, 0xfc00,0,0,0,0,0,0,0x232);  
  uip_sethostaddr(&ip6addr);
  uip_setethaddr(ethaddr);

  uip_ip6addr(&ip6addr, 0xfc00,0,0,0,0,0,0,0x231);  

  tcp_connect(&ip6addr, UIP_HTONS(7), NULL);
  
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_tcpip_process, ev, data)
{
  PROCESS_BEGIN();

  tcp_listen(UIP_HTONS(800));

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    if(uip_newdata()) {
      ((char *)uip_appdata)[uip_datalen()] = 0;
      printf("New uIP data: '%s'\n", uip_appdata);
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
