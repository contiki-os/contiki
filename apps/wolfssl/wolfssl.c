/*
 * **** This file incorporates work covered by the following copyright and ****
 * **** permission notice:                                                 ****
 *
 * Copyright (C) 2006-2017 wolfSSL Inc.
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/types.h>
#include "wolfssl.h"
#include <wolfssl/ssl.h>
#include "uip.h"

static int wolfssl_is_initialized = 0;

//#define SSL_DATABUF_LEN (UIP_CONF_BUFFER_SIZE)
#define SSL_DATABUF_LEN (1500)
#define MODE_TLS 0
#define MODE_DTLS 1

int tls_socket_connect(uip_wolfssl_ctx *sk, const uip_ipaddr_t *addr, uint16_t port)
{
    int ret;
    if (!sk->ssl) {
        sk->ssl = wolfSSL_new(sk->ctx);
        wolfSSL_SetIOReadCtx(sk->ssl, sk);
        wolfSSL_SetIOWriteCtx(sk->ssl, sk);
    }
    ret = tcp_socket_connect(&sk->conn.tcp, addr, port);
    if (ret < 0) {
        return ret;
    }
    ret = wolfSSL_connect(sk->ssl);
    if (ret == SSL_SUCCESS) {
        wolfSSL_set_using_nonblock(sk->ssl, 0);
        return 0;
    } else {
        return ret;
    }
}

int tls_socket_accept(uip_wolfssl_ctx *sk)
{
    int ret;
    if (sk->ssl_rb_len == 0)
        return -2;
    if (!sk->ssl) {
        sk->ssl = wolfSSL_new(sk->ctx);
        wolfSSL_SetIOReadCtx(sk->ssl, sk);
        wolfSSL_SetIOWriteCtx(sk->ssl, sk);
    }
    ret = wolfSSL_accept(sk->ssl);
    if (ret == SSL_SUCCESS) {
        wolfSSL_set_using_nonblock(sk->ssl, 0);
        return 0;
    } else {
        return ret;
    }
}

static void release_databuf(uip_wolfssl_ctx *ctx)
{
    if (ctx->input_databuf)
        free(ctx->input_databuf);
    if (ctx->output_databuf)
        free(ctx->output_databuf);
    if (ctx->ssl_rx_databuf)
        free(ctx->ssl_rx_databuf);
}

void tls_socket_close(uip_wolfssl_ctx *sk)
{
    tcp_socket_close(&sk->conn.tcp);
    release_databuf(sk);
    wolfSSL_free(sk->ssl);
}

void dtls_socket_close(uip_wolfssl_ctx *sk)
{
    udp_socket_close(&sk->conn.udp);
    release_databuf(sk);
    wolfSSL_free(sk->ssl);
}

/* Called by uIP when a new packet is received from the network.
 */
static int tls_socket_recv_callback(struct tcp_socket *s,
        void *ptr,
        const uint8_t *input_data_ptr,
        int input_data_len)
{
    struct uip_wolfssl_ctx *sk = (struct uip_wolfssl_ctx *)s;
    if (sk->ssl_rb_len < SSL_DATABUF_LEN) {
        if (input_data_len > (SSL_DATABUF_LEN - sk->ssl_rb_len))
            input_data_len = SSL_DATABUF_LEN - sk->ssl_rb_len;
        memcpy(sk->ssl_rx_databuf + sk->ssl_rb_len, input_data_ptr, input_data_len);
        sk->ssl_rb_len += input_data_len;
        process_post(sk->process, PROCESS_EVENT_POLL, sk);
        return 0; /* all data consumed */
    }else {
       printf("wolfSSL: Input buffer full!\n");
       return input_data_len; /* keep in input buffer */
    }
}

void dtls_set_endpoint(struct uip_wolfssl_ctx *sk, const uip_ipaddr_t *addr, uint16_t port)
{
    printf("wolfSSL: Setting peer address and port\n");
    sk->peer_port = port;
    memcpy(&sk->peer_addr, addr, sizeof (uip_ipaddr_t));
}

static void dtls_socket_recv_callback(struct udp_socket *s,
        void *ptr,
        const uip_ipaddr_t *source_addr,
        uint16_t source_port,
        const uip_ipaddr_t *dest_addr,
        uint16_t dest_port,
        const uint8_t *input_data_ptr,
        uint16_t input_data_len)
{
    struct uip_wolfssl_ctx *sk = (struct uip_wolfssl_ctx *)s;
    if (sk->ssl_rb_len < SSL_DATABUF_LEN) {
        if (input_data_len > (SSL_DATABUF_LEN - sk->ssl_rb_len))
            input_data_len = SSL_DATABUF_LEN - sk->ssl_rb_len;
        memcpy(sk->ssl_rx_databuf + sk->ssl_rb_len, input_data_ptr, input_data_len);
        sk->ssl_rb_len += input_data_len;
        process_post(sk->process, PROCESS_EVENT_POLL, sk);
        if (sk->peer_port == 0) {
            dtls_set_endpoint(sk, source_addr, source_port);
        }
    }else {
        printf("wolfSSL: Input buffer full!\n");
    }
}


static void
tls_socket_event_callback(struct tcp_socket *s, void *ptr,
      tcp_socket_event_t ev)
{
}

static struct uip_wolfssl_ctx *do_socket_register(WOLFSSL_METHOD* method, int mode)
{
    struct uip_wolfssl_ctx *ctx = NULL;
    if (!wolfssl_is_initialized) {
        wolfSSL_Init();
        wolfSSL_Debugging_ON();
        wolfssl_is_initialized++;
    }

    ctx = malloc(sizeof(uip_wolfssl_ctx));
    if (!ctx)
        return ctx;
    memset(ctx, 0, sizeof(uip_wolfssl_ctx));

    /* Allocate buffers for UIP */
    ctx->input_databuf = malloc(SSL_DATABUF_LEN);
    ctx->output_databuf = malloc(SSL_DATABUF_LEN);

    /* Allocate RX buffer for TLS socket */
    ctx->ssl_rx_databuf = malloc(SSL_DATABUF_LEN);

    if(!ctx->input_databuf || !ctx->output_databuf || !ctx->ssl_rx_databuf)
        goto error;

    ctx->ctx = wolfSSL_CTX_new(method);
    ctx->process = PROCESS_CURRENT();
    if (!ctx->ctx)
        goto error;
    if (MODE_TLS == mode) {
        if (tcp_socket_register(&ctx->conn.tcp, NULL,
                ctx->input_databuf, SSL_DATABUF_LEN,
                ctx->output_databuf, SSL_DATABUF_LEN,
                tls_socket_recv_callback,
                tls_socket_event_callback) < 0)

            goto error;
        wolfSSL_SetIORecv(ctx->ctx, uIPReceive);
        wolfSSL_SetIOSend(ctx->ctx, uIPSend);
    } else {
        if (udp_socket_register(&ctx->conn.udp, NULL,
                    dtls_socket_recv_callback ) < 0)
            goto error;
        wolfSSL_SetIORecv(ctx->ctx, uIPReceive);
        wolfSSL_SetIOSend(ctx->ctx, uIPSendTo);
    }
    return ctx;

error:
    if (ctx) {
        if (ctx->ctx)
            wolfSSL_CTX_free(ctx->ctx);
        if (ctx->input_databuf)
            free(ctx->input_databuf);
        if (ctx->output_databuf)
            free(ctx->output_databuf);
        if (ctx->ssl_rx_databuf)
            free(ctx->ssl_rx_databuf);
        free(ctx);
    }
    return NULL;
}

struct uip_wolfssl_ctx *tls_socket_register(WOLFSSL_METHOD* method)
{
    return do_socket_register(method, MODE_TLS);
}

struct uip_wolfssl_ctx *dtls_socket_register(WOLFSSL_METHOD* method)
{
    return do_socket_register(method, MODE_DTLS);
}

