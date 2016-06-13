/*
 * Copyright (c) 2015, CETIC.
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
 */

/**
 * \file
 *         Simple CoAP Library
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */

#include "contiki.h"

#include "er-coap-engine.h"

#include "coap-common.h"
#include "core-interface.h"
#include "block-transfer.h"
#include "rd-client.h"

#if WITH_CETIC_6LN_NVM
#include "nvm-config.h"
#endif

#include <string.h>
#include <stdio.h>

#define DEBUG 1
#include "net/ip/uip-debug.h"

PROCESS(rd_client_process, "RD Client");

static char const * resources_list = ",";
static int resources_list_size = 0;

static uip_ipaddr_t rd_server_ipaddr;
static uint16_t rd_server_port;

#define REGISTRATION_NAME_MAX_SIZE 64
static char registration_name[REGISTRATION_NAME_MAX_SIZE+1];

#ifdef RD_CLIENT_CONF_SERVER_ADDR
#define RD_CLIENT_SERVER_ADDR(ipaddr) RD_CLIENT_CONF_SERVER_ADDR(ipaddr)
#else
#define RD_CLIENT_SERVER_ADDR(ipaddr)   uip_create_unspecified(ipaddr)
//uip_ip6addr(ipaddr, 0xbbbb, 0, 0, 0, 0xa, 0xbff, 0xfe0c, 0xd0e)
#endif

#ifdef RD_CLIENT_CONF_SERVER_PORT
#define RD_CLIENT_SERVER_PORT RD_CLIENT_CONF_SERVER_PORT
#else
#define RD_CLIENT_SERVER_PORT COAP_DEFAULT_PORT
#endif

static enum rd_client_status_t status = RD_CLIENT_UNCONFIGURED;

static uint8_t registered = 0;
static uint8_t new_address = 0;
/*---------------------------------------------------------------------------*/
void
client_registration_request_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* Check the offset for boundaries of the resource data. */
  PRINTF("Requesting %u bytes at %d\n", preferred_size, (int)*offset);
  if(*offset >= resources_list_size) {
    PRINTF("Invalid offset %d\n", (int)*offset);
    *offset = -1;
    return;
  }

  if(*offset + preferred_size > resources_list_size) {
    preferred_size = resources_list_size - *offset;
  }
  coap_set_payload(request, resources_list + *offset, preferred_size);
  *offset += preferred_size;
  if(*offset >= resources_list_size) {
    *offset = -1;
  }

}
/*---------------------------------------------------------------------------*/
void
client_registration_response_handler(void *response)
{
  if(response != NULL) {
    if (((coap_packet_t *)response)->code == CREATED_2_01) {
      const char *str=NULL;
      int len = coap_get_header_location_path(response, &str);
      if(len > 0) {
        memcpy(registration_name, str, len);
        registration_name[len] = '\0';
        PRINTF("Registration name: %s\n", registration_name);
        registered = 1;
      } else {
        PRINTF("Location-path missing\n");
      }
    } else if (((coap_packet_t *)response)->code == CONTINUE_2_31) {
      /* Resource list not fully sent */
    } else {
      PRINTF("Unknown response code : %d\n", ((coap_packet_t *)response)->code);
    }
  } else {
    PRINTF("Timeout or error status\n");
  }
}
/*---------------------------------------------------------------------------*/
void
client_update_response_handler(void *response)
{
  if(response != NULL) {
    if (((coap_packet_t *)response)->code == CHANGED_2_04) {
      PRINTF("Updated\n");
      registered = 1;
    } else {
      PRINTF("Unknown response code : %d\n", ((coap_packet_t *)response)->code);
    }
  } else {
    PRINTF("Timeout or error status\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rd_client_process, ev, data)
{
  static struct etimer et;
  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */
  static char query_buffer[200];
  static char rd_client_name[64];

  PROCESS_BEGIN();
  PROCESS_PAUSE();
  PRINTF("RD client started\n");
  sprintf(rd_client_name, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
      uip_lladdr.addr[0], uip_lladdr.addr[1], uip_lladdr.addr[2], uip_lladdr.addr[3],
      uip_lladdr.addr[4], uip_lladdr.addr[5], uip_lladdr.addr[6], uip_lladdr.addr[7]);
  while(1) {
    new_address = 0;
    while(!registered) {
      while(uip_is_addr_unspecified(&rd_server_ipaddr)) {
        status = RD_CLIENT_UNCONFIGURED;
        PROCESS_YIELD();
      }
      status = RD_CLIENT_REGISTERING;
      etimer_set(&et, CLOCK_SECOND);
      PROCESS_YIELD_UNTIL(etimer_expired(&et));
      PRINTF("Registering to ");
      PRINT6ADDR(&rd_server_ipaddr);
      PRINTF(" %d with %s\n", rd_server_port, resources_list);
      coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
      coap_set_header_uri_path(request, "rd");
      sprintf(query_buffer, "ep=%s&b=U&lt=%d", rd_client_name, RD_CLIENT_LIFETIME);
      coap_set_header_uri_query(request, query_buffer);
      coap_set_payload(request, (uint8_t *) resources_list, resources_list_size);

      COAP_BLOCKING_REQUEST_BLOCK_RESPONSE(&rd_server_ipaddr, UIP_HTONS(rd_server_port), request, client_registration_request_handler, client_registration_response_handler);
    }
    status = RD_CLIENT_REGISTERED;
    etimer_set(&et, (RD_CLIENT_LIFETIME * CLOCK_SECOND) / 10 * 9);
    PROCESS_YIELD_UNTIL(etimer_expired(&et) || new_address);
    registered = 0;

    if(!new_address) {
      PRINTF("Update endpoint %s\n", registration_name);
      coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
      coap_set_header_uri_path(request, registration_name);
      sprintf(query_buffer, "b=U&lt=%d", RD_CLIENT_LIFETIME);
      coap_set_header_uri_query(request, query_buffer);

      COAP_BLOCKING_REQUEST(&rd_server_ipaddr, UIP_HTONS(rd_server_port), request, client_update_response_handler);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
rd_client_init(void)
{
  RD_CLIENT_SERVER_ADDR(&rd_server_ipaddr);
  rd_server_port = RD_CLIENT_SERVER_PORT;
  process_start(&rd_client_process, NULL);
}
/*---------------------------------------------------------------------------*/
int
rd_client_status(void)
{
  return status;
}
/*---------------------------------------------------------------------------*/
void
rd_client_set_rd_address(uip_ipaddr_t const *ipaddr, uint16_t port)
{
  if(!uip_ipaddr_cmp(ipaddr, &rd_server_ipaddr)) {
    new_address = 1;
  }
  uip_ipaddr_copy(&rd_server_ipaddr, ipaddr);
  rd_server_port = port;
  process_poll(&rd_client_process);
}
/*---------------------------------------------------------------------------*/
void
rd_client_set_resources_list(char const * new_list)
{
  resources_list = new_list;
  resources_list_size = strlen(new_list);
  if(resources_list_size > 1) {
    resources_list_size -= 1;
  } else {
    resources_list_size = 0;
  }
}
/*---------------------------------------------------------------------------*/
