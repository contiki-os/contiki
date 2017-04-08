/*
 * Copyright (c) 2014, Thingsquare, http://www.thingsquare.com/.
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
#ifndef WEBSOCKET_HTTP_CLIENT_H_
#define WEBSOCKET_HTTP_CLIENT_H_

#include "contiki.h"
#include "tcp-socket.h"

#ifdef WEBSOCKET_HTTP_CLIENT_CONF_INPUTBUFSIZE
#define WEBSOCKET_HTTP_CLIENT_INPUTBUFSIZE WEBSOCKET_HTTP_CLIENT_CONF_INPUTBUFSIZE
#else /* WEBSOCKET_HTTP_CLIENT_CONF_INPUTBUFSIZE */
#define WEBSOCKET_HTTP_CLIENT_INPUTBUFSIZE 100
#endif /* WEBSOCKET_HTTP_CLIENT_CONF_INPUTBUFSIZE */

#ifdef WEBSOCKET_HTTP_CLIENT_CONF_OUTPUTBUFSIZE
#define WEBSOCKET_HTTP_CLIENT_OUTPUTBUFSIZE WEBSOCKET_HTTP_CLIENT_CONF_OUTPUTBUFSIZE
#else /* WEBSOCKET_HTTP_CLIENT_CONF_OUTPUTBUFSIZE */
#define WEBSOCKET_HTTP_CLIENT_OUTPUTBUFSIZE 300
#endif /* WEBSOCKET_HTTP_CLIENT_CONF_OUTPUTBUFSIZE */

#ifdef WEBSOCKET_HTTP_CLIENT_CONF_MAX_HOSTLEN
#define WEBSOCKET_HTTP_CLIENT_MAX_HOSTLEN WEBSOCKET_HTTP_CLIENT_CONF_MAX_HOSTLEN
#else /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_HOSTLEN */
#define WEBSOCKET_HTTP_CLIENT_MAX_HOSTLEN 32
#endif /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_HOSTLEN */

#ifdef WEBSOCKET_HTTP_CLIENT_CONF_MAX_FILELEN
#define WEBSOCKET_HTTP_CLIENT_MAX_FILELEN WEBSOCKET_HTTP_CLIENT_CONF_MAX_FILELEN
#else /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_FILELEN */
#define WEBSOCKET_HTTP_CLIENT_MAX_FILELEN 32
#endif /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_FILELEN */

#ifdef WEBSOCKET_HTTP_CLIENT_CONF_MAX_SUBPROTOCOLLEN
#define WEBSOCKET_HTTP_CLIENT_MAX_SUBPROTOCOLLEN WEBSOCKET_HTTP_CLIENT_CONF_MAX_SUBPROTOCOLLEN
#else /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_SUBPROTOCOLLEN */
#define WEBSOCKET_HTTP_CLIENT_MAX_SUBPROTOCOLLEN 24
#endif /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_SUBPROTOCOLLEN */

#ifdef WEBSOCKET_HTTP_CLIENT_CONF_MAX_HEADERLEN
#define WEBSOCKET_HTTP_CLIENT_MAX_HEADERLEN WEBSOCKET_HTTP_CLIENT_CONF_MAX_HEADERLEN
#else /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_HEADERLEN */
#define WEBSOCKET_HTTP_CLIENT_MAX_HEADERLEN 128
#endif /* WEBSOCKET_HTTP_CLIENT_CONF_MAX_HEADERLEN */

struct websocket_http_client_state {
  struct tcp_socket s;
  uint8_t inputbuf[WEBSOCKET_HTTP_CLIENT_INPUTBUFSIZE];
  uint8_t outputbuf[WEBSOCKET_HTTP_CLIENT_OUTPUTBUFSIZE];
  char host[WEBSOCKET_HTTP_CLIENT_MAX_HOSTLEN];
  char file[WEBSOCKET_HTTP_CLIENT_MAX_FILELEN];
  char subprotocol[WEBSOCKET_HTTP_CLIENT_MAX_SUBPROTOCOLLEN];
  char header[WEBSOCKET_HTTP_CLIENT_MAX_HEADERLEN];
  uint16_t port;

  int state;
  struct pt parse_header_pt;
  int http_status;
  int i;

  uip_ipaddr_t proxy_addr;
  uint16_t proxy_port;
};

void websocket_http_client_init(struct websocket_http_client_state *s);
void websocket_http_client_set_proxy(struct websocket_http_client_state *s,
                                     const uip_ipaddr_t *addr, uint16_t port);

int websocket_http_client_register(struct websocket_http_client_state *s,
                                   const char *host,
                                   uint16_t port,
                                   const char *file,
                                   const char *subprotocol,
                                   const char *hdr);
int websocket_http_client_get(struct websocket_http_client_state *s);
int websocket_http_client_send(struct websocket_http_client_state *s,
                               const uint8_t *data,
                               uint16_t datalen);
int websocket_http_client_sendbuflen(struct websocket_http_client_state *s);

void websocket_http_client_close(struct websocket_http_client_state *s);

const char *websocket_http_client_hostname(struct websocket_http_client_state *s);

int websocket_http_client_queuelen(struct websocket_http_client_state *s);

/* Callback functions that have to be implemented by the application
   program. */
void websocket_http_client_datahandler(struct websocket_http_client_state *s,
				       const uint8_t *data, uint16_t len);
void websocket_http_client_connected(struct websocket_http_client_state *s);
void websocket_http_client_timedout(struct websocket_http_client_state *s);
void websocket_http_client_aborted(struct websocket_http_client_state *s);
void websocket_http_client_closed(struct websocket_http_client_state *s);

#endif /* WEBSOCKET_HTTP_CLIENT_H_ */
