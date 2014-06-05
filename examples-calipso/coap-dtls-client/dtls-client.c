/*
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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"

#include "dev/serial-line.h"

#include <string.h>

#include "config.h"
#define DEBUG 1
#ifndef DEBUG
#define DEBUG DEBUG_PRINT
#endif
#include "net/uip-debug.h"

#ifdef WITH_PKI
#include "dtls-client.h"
#endif
#include "debug.h"
#include "dtls.h"
#include "net-buffer.h"

#define UIP_IP_BUF_   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF_  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])

#define MAX_PAYLOAD_LEN 120


#include "er-coap-13-engine.h"

#if defined (PLATFORM_HAS_BUTTON)
#include "dev/button-sensor.h"
#endif /* PLATFORM_HAS_BUTTON */
#if defined (PLATFORM_HAS_LEDS)
#include "dev/leds.h"
#endif /* PLATFORM_HAS_LEDS */

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT+1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 1
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
// char* service_urls[NUMBER_OF_URLS] = {".well-known/core", "hello", "/actuators/toggle", "battery/", "error/in//path"};
   char* service_urls[NUMBER_OF_URLS] = {"/actuators/toggle"};
//#if PLATFORM_HAS_BUTTON
static int uri_switch = 0;
//#endif /* PLATFORM_HAS_BUTTON */

static struct uip_udp_conn *client_conn;
static dtls_context_t *dtls_context;
static session_t dst;
static char buf[200];
static size_t buflen = 0;
static uint8_t dtls_connected = 0;


static const unsigned char ecdsa_priv_key[] = {
			0x41, 0xC1, 0xCB, 0x6B, 0x51, 0x24, 0x7A, 0x14,
			0x43, 0x21, 0x43, 0x5B, 0x7A, 0x80, 0xE7, 0x14,
			0x89, 0x6A, 0x33, 0xBB, 0xAD, 0x72, 0x94, 0xCA,
			0x40, 0x14, 0x55, 0xA1, 0x94, 0xA9, 0x49, 0xFA};

static const unsigned char ecdsa_pub_key_x[] = {
			0x36, 0xDF, 0xE2, 0xC6, 0xF9, 0xF2, 0xED, 0x29,
			0xDA, 0x0A, 0x9A, 0x8F, 0x62, 0x68, 0x4E, 0x91,
			0x63, 0x75, 0xBA, 0x10, 0x30, 0x0C, 0x28, 0xC5,
			0xE4, 0x7C, 0xFB, 0xF2, 0x5F, 0xA5, 0x8F, 0x52};

static const unsigned char ecdsa_pub_key_y[] = {
			0x71, 0xA0, 0xD4, 0xFC, 0xDE, 0x1A, 0xB8, 0x78,
			0x5A, 0x3C, 0x78, 0x69, 0x35, 0xA7, 0xCF, 0xAB,
			0xE9, 0x3F, 0x98, 0x72, 0x09, 0xDA, 0xED, 0x0B,
			0x4F, 0xAB, 0xC3, 0x6F, 0xC7, 0x72, 0xF8, 0x29};

MEMB(network_buffer, buf_record_t, DEF_BUFFER_SIZE);
LIST(nt_buffer);
/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_handler(void *response)
{
  uint8_t *chunk;
  int len = coap_get_payload(response, &chunk);
  if (len > 0)
    PRINTF("|(%u Byte payload)%.*s", len, len, (char *)chunk);
  else
    PRINTF("| no payload\n");

}
/*----------------------- The Application Process -----------------------*/
PROCESS_NAME(coap_client_example);
PROCESS(coap_client_example, "CoAP Client Example");
PROCESS_THREAD(coap_client_example, ev, data)
{
  PROCESS_BEGIN();
  //PRINTF("CoAP Client example %d\n", UIP_CONF_RPL);
  static struct etimer app_et;

  static coap_packet_t request[1]; /* This way the packet can be treated as pointer as usual. */

  etimer_set(&app_et, 2 * CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();
    if (etimer_expired(&app_et) && dtls_connected) {
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[uri_switch]);

      PRINTF("--Requesting %s--\n", service_urls[uri_switch]);

      COAP_BLOCKING_REQUEST(&(dst.addr), dst.port, request, client_chunk_handler, dtls_context, &dst);

      PRINTF("\n--Done--\n");

      uri_switch = (uri_switch+1) % NUMBER_OF_URLS;
      etimer_set(&app_et, 2 * CLOCK_SECOND);
    } else {
      //PRINTF("Not connected\n");
      etimer_set(&app_et, 20 * CLOCK_SECOND);
    }
  }

  PROCESS_END();
}

int
read_from_peer(struct dtls_context_t *ctx, 
	       session_t *session, uint8 *data, size_t len) {
  /* Set upip length*/
  uip_len = len;
  memmove(uip_appdata, data, len);
  coap_receive(ctx, session);
  return 0;
}

int
send_to_peer(struct dtls_context_t *ctx, 
	     session_t *session, uint8 *data, size_t len) {

  //struct uip_udp_conn *conn = (struct uip_udp_conn *)dtls_get_app_data(ctx);

  uip_ipaddr_copy(&client_conn->ripaddr, &session->addr);
  client_conn->rport = session->port;
  
  //PRINTF("before send.");
  
  uip_udp_packet_send(client_conn, data, len);

  /* Restore server connection to allow data from any node */
  /* FIXME: do we want this at all? */
  memset(&client_conn->ripaddr, 0, sizeof(client_conn->ripaddr));
  memset(&client_conn->rport, 0, sizeof(client_conn->rport));

  PRINTF("send to ");
  PRINT6ADDR(&session->addr);
  PRINTF(": %d len %d\n", session->port, len);

  return len;
}

#ifdef DTLS_PSK
static int
get_psk_key(struct dtls_context_t *ctx,
	    const session_t *session,
	    const unsigned char *id, size_t id_len,
	    const dtls_psk_key_t **result) {

  static const dtls_psk_key_t psk = {
    .id = (unsigned char *)"Client_identity",
    .id_length = 15,
    .key = (unsigned char *)"secretPSK",
    .key_length = 9
  };
   
  *result = &psk;
  return 0;
}
#endif /* DTLS_PSK */

#ifdef DTLS_ECC
static int
get_ecdsa_key(struct dtls_context_t *ctx,
	      const session_t *session,
	      const dtls_ecdsa_key_t **result) {
  static const dtls_ecdsa_key_t ecdsa_key = {
    .curve = DTLS_ECDH_CURVE_SECP256R1,
    .priv_key = ecdsa_priv_key,
    .pub_key_x = ecdsa_pub_key_x,
    .pub_key_y = ecdsa_pub_key_y
  };

  *result = &ecdsa_key;
  return 0;
}

static int
verify_ecdsa_key(struct dtls_context_t *ctx,
		 const session_t *session,
		 const unsigned char *other_pub_x,
		 const unsigned char *other_pub_y,
		 size_t key_size) {
  PRINTF("KEY size %d\n", key_size);
  dtls_debug_hexdump("Key X:", other_pub_x, key_size);
  dtls_debug_hexdump("Key Y:", other_pub_y, key_size);
  return 0;
}
#endif /* DTLS_ECC */

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
dtls_handle_read(dtls_context_t *ctx) {
  if(uip_newdata()) {
	/*PRINTF("Got message from network\n");
	PRINTF("Client received %u Byte message from ", uip_datalen());
	PRINT6ADDR(&dst.addr);
	PRINTF(":%d\n", dst.port);*/

    dtls_handle_message(ctx, &dst, uip_appdata, uip_datalen());
  } else {
	buf_record_t* head = list_head(nt_buffer);
	while (head) {
		//PRINTF("Process message from buffer\n");
	    dtls_handle_message(ctx, &dst, head->data, head->length);
	    list_remove(nt_buffer, head);
	    memb_free(&network_buffer, head);
		head = list_head(nt_buffer);
	}
  }
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void
push_to_buffer() {
  if(uip_newdata()) {
	PRINTF("Got message from network push to buffer\n");
	PRINTF("Client received %u Byte message from ", uip_datalen());
	PRINT6ADDR(&dst.addr);
	PRINTF(":%d\n", dst.port);
	buf_record_t* record = memb_alloc(&network_buffer);
	if(record) {
		record->length = uip_datalen();
		memcpy(record->data, uip_appdata, record->length);
		list_add(nt_buffer, record);
	}
  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }
}

//--------------------------------------------------------------------------------------------------
static void
set_global_address(void)
{
	uip_ipaddr_t ipaddr;
	PRINTF("SERVER: Setting global address and dag.\n");
	uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  /*uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0x0200, 0, 0, 0x0002);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);*/
}
//--------------------------------------------------------------------------------------------------

static void
set_connection_address(uip_ipaddr_t *ipaddr)
{//60f:7b2:12:4b00
	//0720:652e:0a00:612e:0a00:612e:0a00:612e
	//aaaa::2665:11ff:fec2:3369
	//uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x2665, 0x11ff, 0xfec2, 0x3369);
	//uip_ip6addr(ipaddr, 0x0720, 0x652e, 0x0a00, 0x612e, 0x0a00, 0x612e, 0x0a00, 0x612e);
  //uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x60f, 0x7b2, 0x12, 0x4b00);
  uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0200, 0, 0, 0x0001);
}
/*---------------------------------------------------------------------------*/
int
on_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level,
              unsigned short code) {
  if (code == DTLS_EVENT_CONNECTED) {
    dtls_connected = 1;
    PRINTF("DTLS-Client Connected2222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222\n");
    process_start(&coap_client_example, NULL);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
init_dtls(session_t *dst) {
  static dtls_handler_t cb = {
    .write = send_to_peer,
    .read  = read_from_peer,
    .event = on_event,
#ifdef DTLS_PSK
    .get_psk_key = get_psk_key,
#endif /* DTLS_PSK */
#ifdef DTLS_ECC
    .get_ecdsa_key = get_ecdsa_key,
    .verify_ecdsa_key = verify_ecdsa_key
#endif /* DTLS_ECC */
  };
  PRINTF("DTLS client started\n");

  print_local_addresses();

  set_connection_address(&dst->addr);
  dst->port = REMOTE_PORT;
  dst->size = sizeof(dst->addr) + sizeof(dst->port);

  PRINTF("set connection 1 address to ");
  PRINT6ADDR(&dst->addr);
  PRINTF(":%d\n", (dst->port));

  client_conn = udp_new(NULL, 0, NULL);
  udp_bind(client_conn, UIP_HTONS(LOCAL_PORT));

  dtls_set_log_level(DTLS_LOG_DEBUG);

  dtls_context = dtls_new_context(client_conn);
  if (dtls_context)
    dtls_set_handler(dtls_context, &cb);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  //static int connected = 0;
  static struct etimer et;

  PROCESS_BEGIN();
  
  PRINTF("Welecome to CoAP-DTLS client. Wait for network.\n");
    
  PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_CONTINUE));
    
  PRINTF("Network ready. Starting client\n");

  memb_init(&network_buffer);

  set_global_address();
  dtls_init();

  init_dtls(&dst);
  //serial_line_init();

  if (!dtls_context) {
    dsrv_log(LOG_EMERG, "cannot create context\n");
    PROCESS_EXIT();
  }

#ifdef ENABLE_LOWPAN_DTLS_COMPRESSION
  PRINTF("With DTLS Header Compression\n");
#endif /* ENABLE_LOWPAN_DTLS_COMPRESSION */


  /* receives all CoAP messages */
  coap_receiver_init();
  coap_register_as_transaction_handler();
  PRINTF("Initializing CAOP Receiver!\n");

  dtls_connect(dtls_context, &dst); 
  /* In case we evaluate the Handshake no CoAP required */
  //process_start(&coap_client_example, NULL);

  etimer_set(&et, CLOCK_SECOND * 1);
  int in_progress = 0;
  while(1) {
	PROCESS_YIELD();
	if(ev == tcpip_event ) {
		  push_to_buffer();
	} else if (ev == PROCESS_EVENT_TIMER) {
		//PRINTF("UPDATE %d\n", dtls_connected);
		if(client_conn->lport != UIP_HTONS(LOCAL_PORT)) {
			client_conn = udp_new(NULL, 0, NULL);
			udp_bind(client_conn, UIP_HTONS(LOCAL_PORT));
		}
		if(!in_progress) {
		  in_progress = 1;
		  dtls_handle_read(dtls_context);
	      coap_check_transactions();
	      in_progress = 0;
		}
		etimer_set(&et, CLOCK_SECOND * 1);
	}
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
