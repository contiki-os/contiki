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
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/netstack.h"
#include "dev/slip.h"
#include "rest-engine.h"
#include "rpl-tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

static void get_rssi_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_last_rssi_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }

PROCESS(border_router_process, "Border router process");
AUTOSTART_PROCESSES(&border_router_process);

RESOURCE(resource_get_rssi, 
         "title=\"Get RSSI\"",
         get_rssi_handler,
         NULL,
         NULL,
         NULL);
static void
get_rssi_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int rssi_level;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &rssi_level);
    CONTENT_PRINTF("%d", rssi_level);
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

RESOURCE(resource_get_last_rssi, 
         "title=\"Get last RSSI\"",
         get_last_rssi_handler,
         NULL,
         NULL,
         NULL);
static void
get_last_rssi_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int last_rssi_level;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &last_rssi_level);
    CONTENT_PRINTF("%d", last_rssi_level);
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}

/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_len = 0;
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  memcpy(&prefix, prefix_64, 16);
  prefix_set = 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;

  PROCESS_PAUSE();

  PRINTF("RPL-Border router started\n");

  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    PRINTF("Waiting for prefix\n");
  }

  PRINTF("Obtained prefix: ");
  uip_debug_ipaddr_print(&prefix);
  PRINTF("\n");

  rpl_tools_init(&prefix);

  rest_init_engine();
  rest_activate_resource(&resource_get_rssi, "Get-RSSI");
  rest_activate_resource(&resource_get_last_rssi, "Get-Last-RSSI");


  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
