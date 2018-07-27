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
#ifndef WOLFSSL_SK_H
#define WOLFSSL_SK_H
#include "wolfssl/ssl.h"
#include "wolfssl/wolfio.h"
#include "user_settings.h"
#include "contiki-net.h"
#include "lib/random.h"
#include "sys/cc.h"
#include <stdlib.h>

struct uip_wolfssl_ctx *tls_socket_register(WOLFSSL_METHOD* method);
int tls_socket_accept(uip_wolfssl_ctx *sk);
int tls_socket_connect(uip_wolfssl_ctx *sk, const uip_ipaddr_t *addr, uint16_t port);
void tls_socket_close(uip_wolfssl_ctx *sk);

struct uip_wolfssl_ctx *dtls_socket_register(WOLFSSL_METHOD* method);
void dtls_set_endpoint(uip_wolfssl_ctx *sk, const uip_ipaddr_t *addr, uint16_t port);
void dtls_socket_close(uip_wolfssl_ctx *sk);

int wolfssl_tcp_send_cb(WOLFSSL* ssl, char *buf, int sz, void *ctx);
int wolfssl_udp_send_cb(WOLFSSL* ssl, char *buf, int sz, void *ctx);
int wolfssl_recv_cb(WOLFSSL *ssl, char *buf, int sz, void *ctx);
int contiki_rand_generator();

#endif
