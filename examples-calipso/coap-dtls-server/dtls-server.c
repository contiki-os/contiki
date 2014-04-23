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

#include <string.h>

#include "config.h"
//#include "ecc.h"
#define DEBUG 1
#ifndef DEBUG
#define DEBUG DEBUG_PRINT
#endif
#include "net/uip-debug.h"

#include "debug.h"
#include "dtls.h"
#include "net-buffer.h"

#ifdef WITH_PKI
#include "dtls-server.h"
#endif

#include "er-coap-13-engine.h"

#define REST_RES_TOGGLE 1

#include "erbium.h"

#include "rpl.h"

#if defined (PLATFORM_HAS_BUTTON)
#include "dev/button-sensor.h"
#endif /* PLATFORM_HAS_BUTTON */

#if defined (PLATFORM_HAS_LEDS)
#include "dev/leds.h"
#endif /* PLATFORM_HAS_LEDS */

#define UIP_IP_BUF_   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF_  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;

static dtls_context_t *dtls_context;
static uint8_t dtls_connected = 0;

static const unsigned char ecdsa_priv_key[] = {
			0xD9, 0xE2, 0x70, 0x7A, 0x72, 0xDA, 0x6A, 0x05,
			0x04, 0x99, 0x5C, 0x86, 0xED, 0xDB, 0xE3, 0xEF,
			0xC7, 0xF1, 0xCD, 0x74, 0x83, 0x8F, 0x75, 0x70,
			0xC8, 0x07, 0x2D, 0x0A, 0x76, 0x26, 0x1B, 0xD4};

static const unsigned char ecdsa_pub_key_x[] = {
			0xD0, 0x55, 0xEE, 0x14, 0x08, 0x4D, 0x6E, 0x06,
			0x15, 0x59, 0x9D, 0xB5, 0x83, 0x91, 0x3E, 0x4A,
			0x3E, 0x45, 0x26, 0xA2, 0x70, 0x4D, 0x61, 0xF2,
			0x7A, 0x4C, 0xCF, 0xBA, 0x97, 0x58, 0xEF, 0x9A};

static const unsigned char ecdsa_pub_key_y[] = {
			0xB4, 0x18, 0xB6, 0x4A, 0xFE, 0x80, 0x30, 0xDA,
			0x1D, 0xDC, 0xF4, 0xF4, 0x2E, 0x2F, 0x26, 0x31,
			0xD0, 0x43, 0xB1, 0xFB, 0x03, 0xE2, 0x2F, 0x4D,
			0x17, 0xDE, 0x43, 0xF9, 0xF9, 0xAD, 0xEE, 0x70};

MEMB(network_buffer, buf_record_t, DEF_BUFFER_SIZE);
LIST(nt_buffer);

/* ------------------------------- COAPS ------------------------------------- */
static int payload_len_current = 0;
static struct uip_ds6_addr *root_if;
/* Pointer to the RPL DODAG that the root creates. */
static struct rpl_dag_t* server_dag;
//static uint8_t payload_exponent = 0; // exponent from 0 to 6, 2^exponent

//#define MAX_PAYLOAD_EXPONENT 6   // max payload length 2^6=64
/*
 * Resources are defined by the RESOURCE macro.
 * Signature: resource name, the RESTful methods it handles, and its URI path (omitting the leading slash).
 */
/******************************************************************************/
#if defined (PLATFORM_HAS_LEDS)
/* A simple actuator example. Toggles the red led */
RESOURCE(toggle, METHOD_GET | METHOD_PUT | METHOD_POST, "actuators/toggle", "title=\"Red LED\";rt=\"Control\"");
void
toggle_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	PRINTF("Toggle \n");
	static int led_state = 0;
	if (!led_state) {
		leds_on(LEDS_YELLOW);
		led_state = 1;
	} else {
		leds_off(LEDS_YELLOW);
		led_state = 0;
	}
}
#else
/* A simple actuator example. Toggles the red led */
RESOURCE(toggle, METHOD_GET | METHOD_PUT | METHOD_POST, "actuators/toggle", "title=\"Red LED\";rt=\"Control\"");
void
toggle_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	PRINTF("Toggle \n");
}
#endif /* PLATFORM_HAS_LEDS */
/******************************************************************************/

int
read_from_peer(struct dtls_context_t *ctx, 
	       session_t *session, uint8 *data, size_t len) {
  /* Set upip length*/
  uip_len = len;
  memmove(uip_appdata, data, len);
  PRINTF("CoAp receive\n");
  coap_receive(ctx, session);
  return 0;
}

int
send_to_peer(struct dtls_context_t *ctx, 
	     session_t *session, uint8 *data, size_t len) {

  uip_ipaddr_copy(&server_conn->ripaddr, &session->addr);
  server_conn->rport = session->port;

  uip_udp_packet_send(server_conn, data, len);

  /* Restore server connection to allow data from any node */
  memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  memset(&server_conn->rport, 0, sizeof(server_conn->rport));

  /*PRINTF("Server sends message to ");
  PRINT6ADDR(&session->addr);
  PRINTF(":%d uip_datalen %d\n", session->port, len);*/

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
  PRINTF("---------------------------------------------------------------------------------------------------------\n", key_size);
  PRINTF("KEY size %d\n", key_size);
  dtls_debug_hexdump("Key X:", other_pub_x, key_size);
  dtls_debug_hexdump("Key Y:", other_pub_y, key_size);
  PRINTF("fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n", key_size);  
  return 0;
}
#endif /* DTLS_ECC */

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
dtls_handle_read(dtls_context_t *ctx) {
/*  session_t session;

  if(uip_newdata()) {
    uip_ipaddr_copy(&session.addr, &UIP_IP_BUF_->srcipaddr);
    session.port = UIP_UDP_BUF_->srcport;
    session.size = sizeof(session.addr) + sizeof(session.port);
    
    PRINTF("Server received message from ");
    PRINT6ADDR(&session.addr);
    PRINTF(":%d uip_datalen %d\n", session.port, uip_datalen());

    dtls_handle_message(ctx, &session, uip_appdata, uip_datalen());
  } else {*/
	buf_record_t* head = list_head(nt_buffer);
	while (head) {
		PRINTF("Process message from buffer\n");
		dtls_handle_message(ctx, &head->session, head->data, head->length);
		list_remove(nt_buffer, head);
		memb_free(&network_buffer, head);
		head = list_head(nt_buffer);
	}
  //}
}
/*---------------------------------------------------------------------------*/
static void
push_to_buffer() {
  if(uip_newdata()) {
	//PRINTF("Got message from network push to buffer\n");
	buf_record_t* record = memb_alloc(&network_buffer);
	if(record) {
	    uip_ipaddr_copy(&record->session.addr, &UIP_IP_BUF_->srcipaddr);
	    record->session.port = UIP_UDP_BUF_->srcport;
	    record->session.size = sizeof(record->session.addr) + sizeof(record->session.port);

	    PRINTF("Server received message from ");
	    PRINT6ADDR(&record->session.addr);
	    PRINTF(":%d uip_datalen %d\n", record->session.port, uip_datalen());

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

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
	  state = uip_ds6_if.addr_list[i].state;
	  if(uip_ds6_if.addr_list[i].isused &&
	  (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
		  PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
		  PRINTF("\n");
	  }
  }
  PRINTF("RPL root: ");  
  PRINT6ADDR(root_if);
  PRINTF("\n");
}

//--------------------------------------------------------------------------------------------------
static void
set_global_address(void)
{
		/* We have the option to derive the address
	 * from the link-local AR9170 (MAC) address.
	 */
	uip_ipaddr_t ipaddr;
	PRINTF("SERVER: Setting global address and dag.\n");
	uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0x0200, 0, 0, 0x0001);
	//uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
	//uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
	
	/* Attempt to lookup for this address.
	 * Obviously, it was just stored.
	 */
	root_if = uip_ds6_addr_lookup(&ipaddr);
	if (root_if != NULL) {
		
		PRINTF("SERVER: ROOT_IF Found.\n");
		/* Proceed with DAG creation, if the node is root. */
		server_dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
		/* Set the RPL prefix. */
		rpl_set_prefix(server_dag, &ipaddr, 64);
		PRINTF("SERVER: Created a new RPL dag\n");
		
	} else {
		/* We should, somehow, stop operations here. TODO. */
		PRINTF("SERVER: ERROR; Cannot create a new RPL DAG.\n");
	}
}
/*---------------------------------------------------------------------------*/
int
on_event(struct dtls_context_t *ctx, session_t *session, dtls_alert_level_t level,
              unsigned short code) {
  if (code == DTLS_EVENT_CONNECTED) {
    dtls_connected = 1;
    PRINTF("DTLS-Server Connected 33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333\n");
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
init_dtls() {
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
#if UIP_CONF_ROUTER
  uip_ipaddr_t ipaddr;
#endif /* UIP_CONF_ROUTER */

  PRINTF("DTLS server started\n");

  server_conn = udp_new(NULL, 0, NULL);
  udp_bind(server_conn, UIP_HTONS(5683));

  dtls_set_log_level(DTLS_LOG_DEBUG);

  dtls_context = dtls_new_context(server_conn);
  if (dtls_context)
    dtls_set_handler(dtls_context, &cb);
}

/*---------------------------------------------------------------------------*/
/*----------------------- The Application Process -----------------------*/
PROCESS_NAME(coap_server_example);
PROCESS(coap_server_example, "CoAP Server Example");
PROCESS_THREAD(coap_server_example, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Starting Erbium Example Server\n");
  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate the application-specific resources. */

  rest_activate_resource(&resource_toggle);


  while(1) {
    PROCESS_YIELD();
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  //uint32_t priv[8] = {0x67518B3C, 0x7A18A7F, 0x1992A23B, 0x37FDCAAF, 0xB683B602, 0xEC816E0, 0xC35C04D6, 0x9CE15179};
  //uint32_t pub_x[8];
  //uint32_t pub_y[8];
  static struct etimer et;

  PROCESS_BEGIN();
  
  PRINTF("Welecome to CoAP-DTLS server. Wait for network.\n");
  
  PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_CONTINUE));  
  
  PRINTF("Network ready. Starting server\n");
  
  memb_init(&network_buffer);
  //ecc_gen_pub_key(priv, pub_x, pub_y);

  set_global_address();
  print_local_addresses();

  dtls_init();
  init_dtls();

#ifdef ENABLE_LOWPAN_DTLS_COMPRESSION
  PRINTF("With DTLS Header Compression\n");
#endif /* ENABLE_LOWPAN_DTLS_COMPRESSION */

  if (!dtls_context) {
    dsrv_log(LOG_EMERG, "cannot create context\n");
    PROCESS_EXIT();
  }
  rest_init_engine();
  rest_activate_resource(&resource_toggle);

  coap_register_as_transaction_handler();

  PRINTF("Starting Erbium Example Server\n");

  int in_progress = 0;
  etimer_set(&et, CLOCK_SECOND * 1);

  while(1) {
    PROCESS_WAIT_EVENT();
	if(ev == tcpip_event ) {
		push_to_buffer();
	} else if (ev == PROCESS_EVENT_TIMER) {
		//print_local_addresses();
		if(server_conn->lport != UIP_HTONS(5683)) {
		  set_global_address();
		  server_conn = udp_new(NULL, 0, NULL);
		  udp_bind(server_conn, UIP_HTONS(5683));
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
