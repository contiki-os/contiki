/*
* Copyright (c) 2015 NXP B.V.
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
* 3. Neither the name of NXP B.V. nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
* Author: Theo van Daele <theo.van.daele@nxp.com>
*
*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "rich.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-rpl.h"
#include "net/mac/tsch/tsch.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static struct etimer et;
static uip_ip6addr_t ip6addr_coordinator;
static linkaddr_t coord_address;
static struct uip_udp_conn *server_conn;
static int associated = 0;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);


/* Callback function called when device is associated via TSCH_CALLBACK_JOINING_NETWORK
   definition in configuration.Use to trigger set-up of schedule */
extern void
sink_server_associated(void)
{
  associated = 1;
  tsch_rpl_callback_joining_network();    /* In case it is used */
  process_post(&udp_server_process, PROCESS_EVENT_CONTINUE, NULL);
} 

static int 
sink_connected(void)
{
  const uip_lladdr_t *client_lladdr = uip_ds6_nbr_lladdr_from_ipaddr(&ip6addr_coordinator);
  if (client_lladdr != NULL) {
    PRINTF("Sink Connected\n");
    etimer_stop(&et);
  } else {
    etimer_restart(&et);
  }
  return (client_lladdr != NULL);
}

static void
tcpip_handler(void)
{
  if(uip_newdata()) {
    /* Option to print something when new data received */
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  struct tsch_slotframe *sf;
  int k;

  PROCESS_BEGIN();
  PRINTF("Sink server started\n");

  /* Get IPv6 address and link address of coordinator */
  uiplib_ipaddrconv(QUOTEME(UDP_ADDR_COORDINATOR), &ip6addr_coordinator);
  coord_address.u8[0] = 0;
  memcpy(&coord_address.u8[1],&ip6addr_coordinator.u8[9],7);

  /* Set power level */
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, TX_SINK_POWER);
  
  /* Start rich stack */
  rich_init(NULL);
  /* Wait till sink_server_associated() indicates association */
  PRINTF("Wait for association\n");
  while (!associated) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
  }
  PRINTF("Associated\n");
  
  /* Set schedule */
  sf = tsch_schedule_get_slotframe_by_handle(0);
  /* TS0: EB slot. Already configured with tsch_schedule_create_minimal() 
     after association in tsch_associate()*/  
  sf = tsch_schedule_get_slotframe_by_handle(0);
  /* TS2 .. TS16: C->Sink */
  for (k=2; k<=16; k++) {
  	tsch_schedule_add_link(sf, LINK_OPTION_RX, LINK_TYPE_NORMAL, &coord_address, k, 0);
  }
  /* TS17: Sink->C */
	tsch_schedule_add_link(sf, LINK_OPTION_TX , LINK_TYPE_NORMAL, &coord_address, 17, 0);
  tsch_schedule_print();

  server_conn = udp_new(&ip6addr_coordinator, UIP_HTONS(3001), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

  PRINTF("Wait for coordinator client to connect .....\n");
  etimer_set(&et, CHECK_CONNECT_INTERVAL);
  while(!sink_connected()) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); 
  }

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    tcpip_handler();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
