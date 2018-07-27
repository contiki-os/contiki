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

extern const unsigned char server_cert[788];
extern const unsigned long server_cert_len;

#if NETSTACK_WITH_IPV6
static void print_local_addresses(void)
{
  int i;
  uint8_t state;

  printf("Client IPv6 address:\n");
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
#endif

static struct uip_wolfssl_ctx *sk = NULL;

static struct etimer et;

PROCESS(dtls_client_process, "DTLS process");
AUTOSTART_PROCESSES(&dtls_client_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dtls_client_process, ev, data)
{
    int ret = 0;
    char buf[64];
    uip_ipaddr_t server;
    PROCESS_BEGIN();


    sk = dtls_socket_register(wolfDTLSv1_2_client_method());
    if (!sk) {
        while(1)
            ;
    }
    /* Load certificate file for the DTLS client */
    if (wolfSSL_CTX_use_certificate_buffer(sk->ctx, server_cert,
                server_cert_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
        while(1)
            ;


    sk->ssl = wolfSSL_new(sk->ctx);
    wolfSSL_SetIOReadCtx(sk->ssl, sk);
    wolfSSL_SetIOWriteCtx(sk->ssl, sk);
    if (sk->ssl == NULL) {

        while(1)
            ;

    }

#ifdef NETSTACK_CONF_WITH_IPV4
    uip_ipaddr(&server, 172, 18, 0, 1);
#else
    uip_ip6addr(&server, 0xfd00, 0, 0, 0, 0, 0, 0, 1);
#endif


    dtls_set_endpoint(sk, &server, SERVER_PORT);

    do {
        etimer_set(&et, CLOCK_SECOND * 5);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        printf("connecting to server...\n");
        ret = wolfSSL_connect(sk->ssl);
        if (ret != SSL_SUCCESS) {
            free(sk->ssl);
            sk->ssl = wolfSSL_new(sk->ctx);
            wolfSSL_SetIOReadCtx(sk->ssl, sk);
            wolfSSL_SetIOWriteCtx(sk->ssl, sk);
        }
        PROCESS_PAUSE();
    } while(ret != SSL_SUCCESS);

    PROCESS_WAIT_EVENT();
    do {
        ret = wolfSSL_read(sk->ssl, buf, 63);
        printf("wolfSSL_read returned %d\r\n", ret);
    } while (ret <= 0);
    buf[ret] = (char)0;
    printf("%s\r\n", buf);

    printf("Closing connection.\r\n");
    dtls_socket_close(sk);
    sk->ssl = NULL;
    sk->peer_port = 0;
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
