/*
 * Copyright (c) 2013, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef HTTP_SOCKET_H
#define HTTP_SOCKET_H

#include "tcp-socket.h"

struct http_socket;

typedef enum {
  HTTP_SOCKET_ERR,
  HTTP_SOCKET_OK,
  HTTP_SOCKET_HEADER,
  HTTP_SOCKET_DATA,
  HTTP_SOCKET_CLOSED,
  HTTP_SOCKET_TIMEDOUT,
  HTTP_SOCKET_ABORTED,
  HTTP_SOCKET_HOSTNAME_NOT_FOUND,
} http_socket_event_t;

struct http_socket_header {
  uint16_t status_code;
  int64_t content_length;
  struct {
    int64_t first_byte_pos;
    int64_t last_byte_pos;
    int64_t instance_length;
  } content_range;
};

typedef void (* http_socket_callback_t)(struct http_socket *s,
                                        void *ptr,
                                        http_socket_event_t ev,
                                        const uint8_t *data,
                                        uint16_t datalen);

#define MAX(n, m)   (((n) < (m)) ? (m) : (n))

#define HTTP_SOCKET_INPUTBUFSIZE  UIP_TCP_MSS
#define HTTP_SOCKET_OUTPUTBUFSIZE MAX(UIP_TCP_MSS, 128)

#define HTTP_SOCKET_URLLEN        128

#define HTTP_SOCKET_TIMEOUT       ((2 * 60 + 30) * CLOCK_SECOND)

struct http_socket {
  struct http_socket *next;
  struct tcp_socket s;
  uip_ipaddr_t proxy_addr;
  uint16_t proxy_port;
  int64_t pos;
  uint64_t length;
  const uint8_t *postdata;
  uint16_t postdatalen;
  http_socket_callback_t callback;
  void *callbackptr;
  int did_tcp_connect;
  char url[HTTP_SOCKET_URLLEN];
  uint8_t inputbuf[HTTP_SOCKET_INPUTBUFSIZE];
  uint8_t outputbuf[HTTP_SOCKET_OUTPUTBUFSIZE];

  struct etimer timeout_timer;
  uint8_t timeout_timer_started;
  struct pt pt, headerpt;
  int header_chars;
  char header_field[15];
  struct http_socket_header header;
  uint8_t header_received;
  uint64_t bodylen;
  const char *content_type;
};

void http_socket_init(struct http_socket *s);

int http_socket_get(struct http_socket *s, const char *url,
                    int64_t pos, uint64_t length,
                    http_socket_callback_t callback,
                    void *callbackptr);

int http_socket_post(struct http_socket *s, const char *url,
                     const void *postdata,
                     uint16_t postdatalen,
                     const char *content_type,
                     http_socket_callback_t callback,
                     void *callbackptr);

int http_socket_close(struct http_socket *socket);

void http_socket_set_proxy(struct http_socket *s,
                           const uip_ipaddr_t *addr, uint16_t port);


#endif /* HTTP_SOCKET_H */
