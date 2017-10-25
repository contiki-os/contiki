/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "websocket-http-client.h"

typedef enum {
  WEBSOCKET_ERR = 0,
  WEBSOCKET_OK = 1,
  WEBSOCKET_IN_PROGRESS = 2,
  WEBSOCKET_HOSTNAME_NOT_FOUND = 3,
  WEBSOCKET_CONNECTED = 4,
  WEBSOCKET_DATA = 5,
  WEBSOCKET_RESET = 6,
  WEBSOCKET_TIMEDOUT = 7,
  WEBSOCKET_CLOSED = 8,
  WEBSOCKET_PINGED = 9,
  WEBSOCKET_DATA_RECEIVED = 10,
  WEBSOCKET_PONG_RECEIVED = 11,
} websocket_result_t;

struct websocket;

typedef void (* websocket_callback)(struct websocket *s,
				    websocket_result_t result,
				    const uint8_t *data,
				    uint16_t datalen);
#ifdef WEBSOCKET_CONF_MAX_MSGLEN
#define WEBSOCKET_MAX_MSGLEN WEBSOCKET_CONF_MAX_MSGLEN
#else /* WEBSOCKET_CONF_MAX_MSGLEN */
#define WEBSOCKET_MAX_MSGLEN 200
#endif /* WEBSOCKET_CONF_MAX_MSGLEN */

struct websocket {
  struct websocket *next;     /* Must be first. */
  struct websocket_http_client_state s;
  websocket_callback callback;

  uint8_t mask[4];
  uint32_t left, len;
  uint8_t opcode;

  uint8_t state;

  uint8_t headercacheptr;
  uint8_t headercache[10]; /* The maximum websocket header + mask is 6
                              + 4 bytes long */
};

enum {
  WEBSOCKET_STATE_CLOSED = 0,
  WEBSOCKET_STATE_DNS_REQUEST_SENT = 1,
  WEBSOCKET_STATE_HTTP_REQUEST_SENT = 2,
  WEBSOCKET_STATE_WAITING_FOR_HEADER = 3,
  WEBSOCKET_STATE_RECEIVING_HEADER = 4,
  WEBSOCKET_STATE_HEADER_RECEIVED = 5,
  WEBSOCKET_STATE_RECEIVING_DATA = 6,
};


void websocket_init(struct websocket *s);

void websocket_set_proxy(struct websocket *s,
                         const uip_ipaddr_t *addr, uint16_t port);

websocket_result_t websocket_open(struct websocket *s,
                                  const char *url,
                                  const char *subprotocol,
                                  const char *hdr,
                                  websocket_callback c);

int websocket_send(struct websocket *s,
		   const uint8_t *data, uint16_t datalen);

int websocket_send_str(struct websocket *s,
                       const char *strptr);

void websocket_close(struct websocket *s);

int websocket_ping(struct websocket *s);

int websocket_queuelen(struct websocket *s);

#endif /* WEBSOCKET_H */
