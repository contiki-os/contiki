/*
 * Copyright (c) 2015, Copyright  Robert Olsson / Radio Sensors AB  
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
 *
 * Author  : Robert Olsson robert@radio-sensors.com
 * Created : 2015-12-28
 */

/**
 * \file
 *         A simple TCP server for client app.
 */

#include <string.h>
#include "contiki-net.h"
#include "leds.h"

#define DEBUG DEBUG_PRINT
#include "uip-debug.h"

#define PORT 1236

static struct psock ps;
static uint8_t buf[120];
static int i;
static uint8_t state;

static void
print_local_addresses(void)
{
  PRINTF("Server IPv6 addresses:\n\r");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n\r");
    }
  }
}

static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}

static
PT_THREAD(handle_connection(struct psock *p))
{
  PSOCK_BEGIN(p);
  //PSOCK_SEND_STR(p, "Type something!\n");
  leds_on(LEDS_RED);
  PSOCK_READTO(p, '\n');
  printf("RX=%s", buf);
  //PSOCK_SEND_STR(p, "Got: ");
  //PSOCK_SEND(p, buf, PSOCK_DATALEN(p));
  //PSOCK_SEND_STR(p, "EOL\r\n");
  //PSOCK_CLOSE(p);
  PSOCK_END(p);
}

PROCESS(server_process, "Server");
AUTOSTART_PROCESSES(&server_process);

PROCESS_THREAD(server_process, ev, data)
{
  PROCESS_BEGIN();

  set_global_address();
  leds_init();
  print_local_addresses();
  printf("Starting TCP server on port=%d\n", PORT);
  tcp_listen(UIP_HTONS(PORT));

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    if(uip_aborted() )
	printf("TCP aborted\n");
    if(uip_timedout() )
	printf("TCP timeoutn\n");
    if(uip_closed() )
	printf("TCP closed\n");

    if(uip_connected()) {
      printf("TCP Connected\n\r");
      PSOCK_INIT(&ps, buf, sizeof(buf));

      while(!(uip_aborted() || uip_closed() || uip_timedout())) {
	PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
	handle_connection(&ps);
      }
    }
  }
  PROCESS_END();
}

