/*
 * **** This file incorporates work covered by the following copyright and ****
 * **** permission notice:                                                 ****
 *
 * Copyright (C) 2006-2018 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 *
 */
#include "contiki-net.h"
#include "sys/cc.h"
#include "wolfssl.h"
#include "uip-debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 11111
#define DEBUG 1
extern const unsigned char server_cert[788];
extern const unsigned char server_key[121];
extern unsigned int server_cert_len;
extern unsigned int server_key_len;

static struct uip_wolfssl_ctx *sk = NULL;
static struct etimer et;

static void print_local_addresses(void)
{
  int i;
  uint8_t state;

  printf("Server IPv6 address:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state
                                          == ADDR_PREFERRED)) {
      printf("  ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}

static const char Contiki_dtls_string[] = "DTLS OK\r\n";
PROCESS(dtls_server_process, "DTLS process");
AUTOSTART_PROCESSES(&dtls_server_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dtls_server_process, ev, data)
{
    uip_ipaddr_t ipaddr;
    PROCESS_BEGIN();
    uip_ds6_init();
    /* Wait one second */
	uip_ip6addr(&ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    print_local_addresses();
    sk = dtls_socket_register(wolfDTLSv1_2_server_method());
    if (!sk) {
        while(1)
            ;
    }
    /* Load certificate file for the DTLS server */
    if (wolfSSL_CTX_use_certificate_buffer(sk->ctx, server_cert,
                server_cert_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
        while(1)
            ;

    /* Load the private key */
    if (wolfSSL_CTX_use_PrivateKey_buffer(sk->ctx, server_key,
                server_key_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
        while(1) {}

    sk->ssl = wolfSSL_new(sk->ctx);
    wolfSSL_CTX_set_verify(sk->ctx, SSL_VERIFY_NONE, 0);
    wolfSSL_SetIOReadCtx(sk->ssl, sk);
    wolfSSL_SetIOWriteCtx(sk->ssl, sk);
    if (sk->ssl == NULL) {

        while(1)
            ;

    }

    if (udp_socket_bind(&sk->conn.udp, SERVER_PORT) < 0) {
        while(1)
            ;
    }
    printf("Listening on %d\n", SERVER_PORT);
    while(1) {
        int ret;

        PROCESS_WAIT_EVENT();
        ret = wolfSSL_accept(sk->ssl);
        if (ret != SSL_SUCCESS) {
            printf("err: %d\r\n", ret);
            continue;
        }
        printf("Connection accepted\r\n");
        /* Send header */
        printf("Sending 'DTLS OK'...\r\n");
        wolfSSL_write(sk->ssl, Contiki_dtls_string, sizeof(Contiki_dtls_string));
        printf("Closing connection.\r\n");
        dtls_socket_close(sk);
        sk->ssl = NULL;
        sk->peer_port = 0;
        printf("Listening on %d\n", SERVER_PORT);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
