/*
 * Copyright (c) 2015, Yanzi Networks AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *      IPSO Objects and OMA LWM2M example.
 * \author
 *      Joakim Eriksson, joakime@sics.se
 *      Niclas Finne, nfi@sics.se
 */

#include "contiki.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "er-coap-constants.h"
#include "er-coap-engine.h"
#include "lwm2m-engine.h"
#include "oma-tlv.h"
#include "dev/serial-line.h"
#include "serial-protocol.h"

#if CONTIKI_TARGET_WISMOTE
#include "dev/uart1.h"
#endif

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define URL_WELL_KNOWN               ".well-known/core"
#define URL_DEVICE_MODEL             "/3/0/1"
#define URL_DEVICE_FIRMWARE_VERSION  "/3/0/3"
#define URL_LIGHT_CONTROL            "/3311/0/5850"
#define URL_POWER_CONTROL            "/3312/0/5850"

#define MAX_NODES 10

#define NODE_HAS_TYPE  (1 << 0)

struct node {
  uip_ipaddr_t ipaddr;
  char type[32];
  uint8_t flags;
  uint8_t retries;
};

static struct node nodes[MAX_NODES];
static uint8_t node_count;

static struct node *current_target;
static char current_uri[32] = URL_LIGHT_CONTROL;
static char current_value[32] = "1";
static int current_request = COAP_PUT;
static uint8_t fetching_type = 0;

PROCESS(router_process, "router process");
AUTOSTART_PROCESSES(&router_process);
/*---------------------------------------------------------------------------*/
static struct node *
add_node(const uip_ipaddr_t *addr)
{
  int i;
  for(i = 0; i < node_count; i++) {
    if(uip_ipaddr_cmp(&nodes[i].ipaddr, addr)) {
      /* Node already added */
      return &nodes[i];
    }
  }
  if(node_count < MAX_NODES) {
    uip_ipaddr_copy(&nodes[node_count].ipaddr, addr);
    return &nodes[node_count++];
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
set_value(const uip_ipaddr_t *addr, char *uri, char *value)
{
  int i;
  printf("#set value ");
  uip_debug_ipaddr_print(addr);
  printf(" URI: %s Value: %s\n", uri, value);

  for(i = 0; i < node_count; i++) {
    if(uip_ipaddr_cmp(&nodes[i].ipaddr, addr)) {
      /* setup command */
      current_target = &nodes[i];
      current_request = COAP_PUT;
      strncpy(current_uri, uri, sizeof(current_uri) - 1);
      strncpy(current_value, value, sizeof(current_value) - 1);
      process_poll(&router_process);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
get_value(const uip_ipaddr_t *addr, char *uri)
{
  int i;
  printf("#get value ");
  uip_debug_ipaddr_print(addr);
  printf(" URI: %s\n", uri);

  for(i = 0; i < node_count; i++) {
    if(uip_ipaddr_cmp(&nodes[i].ipaddr, addr)) {
      /* setup command */
      current_target = &nodes[i];
      current_request = COAP_GET;
      strncpy(current_uri, uri, sizeof(current_uri) - 1);
      current_value[0] = 0;
      process_poll(&router_process);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
print_node_list(void)
{
  int i;
  int out = 0;
  for(i = 0; i < node_count; i++) {
    if(nodes[i].flags & NODE_HAS_TYPE) {
      if(out++) {
        printf(";");
      }
      printf("%s,", nodes[i].type);
      uip_debug_ipaddr_print(&nodes[i].ipaddr);
    }
  }
  printf("\n");
}
/*---------------------------------------------------------------------------*/
/**
 * This function is will be passed to COAP_BLOCKING_REQUEST() to
 * handle responses.
 */
static void
client_chunk_handler(void *response)
{
  const uint8_t *chunk;
  unsigned int format;
  int len = coap_get_payload(response, &chunk);
  coap_get_header_content_format(response, &format);

  /* if(len > 0) { */
  /*   printf("|%.*s (%d,%d)", len, (char *)chunk, len, format); */
  /* } */
  if(current_target != NULL && fetching_type) {
    if(len > sizeof(current_target->type) - 1) {
      len = sizeof(current_target->type) - 1;
    }
    memcpy(current_target->type, chunk, len);
    current_target->type[len] = 0;
    current_target->flags |= NODE_HAS_TYPE;

    PRINTF("\nNODE ");
    PRINT6ADDR(&current_target->ipaddr);
    PRINTF(" HAS TYPE %s\n", current_target->type);
  } else {
    /* otherwise update the current value */
    if(format == LWM2M_TLV) {
      oma_tlv_t tlv;
      /* we can only read int32 for now ? */
      if(oma_tlv_read(&tlv, chunk, len) > 0) {
        /* printf("TLV.type=%d len=%d id=%d value[0]=%d\n", */
        /*        tlv.type, tlv.length, tlv.id, tlv.value[0]); */

        int value = oma_tlv_get_int32(&tlv);
        snprintf(current_value, sizeof(current_value), "%d", value);
      }
    } else {
      if(len > sizeof(current_value) - 1) {
        len = sizeof(current_value) - 1;
      }
      memcpy(current_value, chunk, len);
      current_value[len] = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
setup_network(void)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;
  rpl_dag_t *dag;
  int i;
  uint8_t state;

#if CONTIKI_TARGET_WISMOTE
  uart1_set_input(serial_line_input_byte);
  serial_line_init();
#endif

#if UIP_CONF_ROUTER
/**
 * The choice of server address determines its 6LoWPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit compressed address of fd00::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from link local (MAC) address */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
#endif

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */

  PRINTF("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(router_process, ev, data)
{
  /* This way the packet can be treated as pointer as usual. */
  static coap_packet_t request[1];
  static struct etimer timer;
  uip_ds6_route_t *r;
  uip_ipaddr_t *nexthop;
  int n;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  /* receives all CoAP messages */
  coap_init_engine();

  setup_network();

  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_MAC.off(1);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_YIELD();

    /* Handle serial line input */
    if(ev == serial_line_event_message) {
      serial_protocol_input((char *) data);
    }

    if(etimer_expired(&timer)) {
      current_target = NULL;
      n = 0;
      for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
        current_target = add_node(&r->ipaddr);
        if(current_target == NULL ||
           (current_target->flags & NODE_HAS_TYPE) != 0 ||
           current_target->retries > 5) {
          continue;
        }
        PRINTF("  ");
        PRINT6ADDR(&r->ipaddr);
        PRINTF("  ->  ");
        nexthop = uip_ds6_route_nexthop(r);
        if(nexthop != NULL) {
          PRINT6ADDR(nexthop);
          PRINTF("\n");
        } else {
          PRINTF("-");
        }
        PRINTF("\n");
        n++;
        break;
      }
    }

    /* This is a node type discovery */
    if(current_target != NULL &&
       (current_target->flags & NODE_HAS_TYPE) == 0 &&
       current_target->retries < 6) {

      /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, URL_DEVICE_MODEL);

      current_target->retries++;

      PRINTF("CoAP request to [");
      PRINT6ADDR(&current_target->ipaddr);
      PRINTF("]:%u (%u tx)\n", UIP_HTONS(REMOTE_PORT),
             current_target->retries);

      fetching_type = 1;
      COAP_BLOCKING_REQUEST(&current_target->ipaddr, REMOTE_PORT, request,
                            client_chunk_handler);
      fetching_type = 0;
      strncpy(current_uri, URL_LIGHT_CONTROL, sizeof(current_uri));
      printf("\n--Done--\n");
    }

    /* If having a type this is another type of request */
    if(current_target != NULL &&
       (current_target->flags & NODE_HAS_TYPE) && strlen(current_uri) > 0) {
      /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
      coap_init_message(request, COAP_TYPE_CON, current_request, 0);
      coap_set_header_uri_path(request, current_uri);

      if(strlen(current_value) > 0) {
        coap_set_payload(request, (uint8_t *)current_value,
                         strlen(current_value));
      }

      PRINTF("CoAP request to [");
      PRINT6ADDR(&current_target->ipaddr);
      PRINTF("]:%u %s\n", UIP_HTONS(REMOTE_PORT), current_uri);

      COAP_BLOCKING_REQUEST(&current_target->ipaddr, REMOTE_PORT, request,
                            client_chunk_handler);

      /* print out result of command */
      if(current_request == COAP_PUT) {
        printf("s ");
      } else {
        printf("g ");
      }
      uip_debug_ipaddr_print(&current_target->ipaddr);
      printf(" %s %s\n", current_uri, current_value);

      current_target = NULL;
      current_uri[0] = 0;
      current_value[0] = 0;

    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
