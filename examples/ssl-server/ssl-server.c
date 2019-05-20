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

#define SERVER_PORT 443
extern const unsigned char server_cert[788];
extern const unsigned char server_key[121];
extern unsigned int server_cert_len;
extern unsigned int server_key_len;
static char http_request[200];

static struct uip_wolfssl_ctx *sk = NULL;

static const char Contiki_http_string[] = "HTTP/1.0 200 OK\r\n"
"Content-Type: text/html;\r\n\r\n"
"<h2>Server: Contiki ssl-socket example</h2>\r\n\r\n";
PROCESS(ssl_server_process, "TCP echo process");
AUTOSTART_PROCESSES(&ssl_server_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ssl_server_process, ev, data)
{
    PROCESS_BEGIN();
    printf("\n\nStarting example HTTPS server on port %d\n", SERVER_PORT);
    sk = tls_socket_register(wolfSSLv23_server_method());
    if (!sk) {
        while(1)
            ;
    }
    /* Load certificate file for the HTTPS server */
    if (wolfSSL_CTX_use_certificate_buffer(sk->ctx, server_cert,
                server_cert_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
        while(1)
            ;

    /* Load the private key */
    if (wolfSSL_CTX_use_PrivateKey_buffer(sk->ctx, server_key,
                server_key_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
        while(1) {}
    /* Create SSL socket */
    tcp_socket_listen(&sk->conn.tcp, SERVER_PORT);

    printf("Listening on %d\n", SERVER_PORT);
    while(1) {
        int len;
        int ret;

        PROCESS_WAIT_EVENT();
        ret = tls_socket_accept(sk);
        if (ret < 0) {
            printf("Accept returned %d\r\n", ret);
            continue;
        }
        printf("Connection accepted\r\n");
        do {
            PROCESS_WAIT_EVENT();
            memset(http_request, 0, sizeof(http_request));
            len = wolfSSL_read(sk->ssl, http_request, sizeof(http_request));
            printf("Recv HTTP request (%d bytes)\r\n", len);
        } while (len < 4);

        /* Send header */
        printf("Sending response...\r\n");
        wolfSSL_write(sk->ssl, Contiki_http_string, sizeof(Contiki_http_string));
        printf("Closing connection.\r\n");
        tls_socket_close(sk);
        sk->ssl = NULL;
        printf("Listening on %d\n", SERVER_PORT);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
