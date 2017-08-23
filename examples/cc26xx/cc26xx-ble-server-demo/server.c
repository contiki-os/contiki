/*
 * Copyright (c) 2017, Graz University of Technology
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * 		A simple IPv6-over-BLE UDP-client.
 *
 * \author
 * 		Michael Spoerk <michael.spoerk@tugraz.at>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define CLIENT_PORT 					61617
#define SERVER_PORT 					61616

#define UDP_LEN_MAX						255
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF   	((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
static struct uip_udp_conn *server_conn;

static char buf[UDP_LEN_MAX];
static uint16_t packet_counter;
/*---------------------------------------------------------------------------*/
PROCESS(ipv6_ble_server_process, "IPv6 over BLE - server process");
AUTOSTART_PROCESSES(&ipv6_ble_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  if(uip_newdata()) {
	  /* process received message */
      strncpy(buf, uip_appdata, uip_datalen());
      buf[uip_datalen()] = '\0';
	  PRINTF("rec. message: <%s>\n", buf);

	  /* send response message */
	  uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
	  sprintf(buf, "Hello client %04u!", packet_counter);
	  PRINTF("send message: <%s>\n", buf);
	  uip_udp_packet_send(server_conn, buf, strlen(buf));
	  packet_counter++;

	  memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ipv6_ble_server_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("CC26XX-IPv6-over-BLE server started\n");

  server_conn = udp_new(NULL, UIP_HTONS(CLIENT_PORT), NULL);
  udp_bind(server_conn, UIP_HTONS(SERVER_PORT));

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
    	tcpip_handler();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

