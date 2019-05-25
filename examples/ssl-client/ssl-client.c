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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef NETSTACK_CONF_WITH_IPV4
#define SET_SERVER_ADDRESS(x) uip_ipaddr((x), 172, 18, 0, 1)
#else
#define SET_SERVER_ADDRESS(x) uip_ip6addr((x), 0xfd00, 0, 0, 0, 0, 0, 0, 1)
#endif

#define SERVER_PORT 443
extern const unsigned char server_cert[788];
extern const unsigned char server_key[121];
extern unsigned int server_cert_len;
extern unsigned int server_key_len;
static char http_request[200];

static struct uip_wolfssl_ctx *sk = NULL;

struct etimer et;

static const char Contiki_http_string[] = "GET / HTTP/1.0\r\n";
static uip_ipaddr_t server_address;
PROCESS(ssl_client_process, "TCP echo process");
AUTOSTART_PROCESSES(&ssl_client_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ssl_client_process, ev, data)
{
    PROCESS_BEGIN();

    SET_SERVER_ADDRESS(&server_address);
    sk = tls_socket_register(wolfSSLv23_client_method());
    if (!sk) {
        while(1)
            ;
    }

    while(1) {
        int len;
        int ret;
        etimer_set(&et, CLOCK_SECOND * 5);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        printf("Connecting to server:%d\n", SERVER_PORT);
        ret = tls_socket_connect(sk, &server_address, SERVER_PORT);
        if (ret < 0) {
            continue;
        }
        PROCESS_WAIT_EVENT();
        printf("Connection established\r\n");

        /* Send request*/
        printf("Sending request...\r\n");
        wolfSSL_write(sk->ssl, Contiki_http_string, sizeof(Contiki_http_string));
        do {
            PROCESS_WAIT_EVENT();
            memset(http_request, 0, sizeof(http_request));
            len = wolfSSL_read(sk->ssl, http_request, sizeof(http_request));
            printf("Recv HTTP response (%d bytes)\r\n", len);
        } while (len < 4);
        printf("Closing connection.\r\n");
        tls_socket_close(sk);
        sk->ssl = NULL;
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
