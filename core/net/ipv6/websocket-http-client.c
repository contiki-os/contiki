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

#include "websocket-http-client.h"
#include "net/ip/uiplib.h"
#include "net/ip/resolv.h"

#include "ip64-addr.h"

#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

enum {
  STATE_WAITING_FOR_HEADER,
  STATE_WAITING_FOR_CONNECTED,
  STATE_STEADY_STATE,
};
/*---------------------------------------------------------------------------*/
static void
send_get(struct websocket_http_client_state *s)
{
  struct tcp_socket *tcps;

  tcps = &s->s;
  tcp_socket_send_str(tcps, "GET ");
  tcp_socket_send_str(tcps, s->file);
  tcp_socket_send_str(tcps, " HTTP/1.1\r\n");
  tcp_socket_send_str(tcps, "Host: ");
  tcp_socket_send_str(tcps, s->host);
  tcp_socket_send_str(tcps, "\r\n");
  if(strlen(s->header) > 0) {
    tcp_socket_send_str(tcps, s->header);
  }
  /* The Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw== header is
     supposed to be a random value, encoded as base64, that is SHA1
     hashed by the server and returned in a HTTP header. This is used
     to make sure that we are not seeing some cached version of this
     conversation. But we have no SHA1 code by default in Contiki, so
     we can't check the return value. Therefore we just use a
     hardcoded value here. */
  tcp_socket_send_str(tcps,
                      "Connection: Upgrade\r\n"
                      "Upgrade: websocket\r\n"
                      "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
                      "Sec-WebSocket-Version: 13\r\n"
                      "Sec-WebSocket-Protocol:");
  tcp_socket_send_str(tcps, s->subprotocol);
  tcp_socket_send_str(tcps, "\r\n");
  tcp_socket_send_str(tcps, "\r\n");
  PRINTF("websocket-http-client: send_get(): output buffer left %d\n", tcp_socket_max_sendlen(tcps));
}
/*---------------------------------------------------------------------------*/
static void
send_connect(struct websocket_http_client_state *s)
{
  struct tcp_socket *tcps;
  char buf[20];

  tcps = &s->s;
  tcp_socket_send_str(tcps, "CONNECT ");
  tcp_socket_send_str(tcps, s->host);
  tcp_socket_send_str(tcps, ":");
  sprintf(buf, "%d", s->port);
  tcp_socket_send_str(tcps, buf);
  tcp_socket_send_str(tcps, " HTTP/1.1\r\n");
  tcp_socket_send_str(tcps, "Host: ");
  tcp_socket_send_str(tcps, s->host);
  tcp_socket_send_str(tcps, "\r\n");
  tcp_socket_send_str(tcps, "Proxy-Connection: Keep-Alive\r\n\r\n");
}
/*---------------------------------------------------------------------------*/
static void
event(struct tcp_socket *tcps, void *ptr,
      tcp_socket_event_t e)
{
  struct websocket_http_client_state *s = ptr;

  if(e == TCP_SOCKET_CONNECTED) {
    if(s->proxy_port != 0) {
      send_connect(s);
    } else {
      send_get(s);
    }
  } else if(e == TCP_SOCKET_CLOSED) {
    websocket_http_client_closed(s);
  } else if(e == TCP_SOCKET_TIMEDOUT) {
    websocket_http_client_timedout(s);
  } else if(e == TCP_SOCKET_ABORTED) {
    websocket_http_client_aborted(s);
  } else if(e == TCP_SOCKET_DATA_SENT) {
    /* We could feed this information up to the websocket.c layer, but
       we currently do not do that. */
  }
}
/*---------------------------------------------------------------------------*/
static int
parse_header_byte(struct websocket_http_client_state *s,
                  uint8_t b)
{
  static const char *endmarker = "\r\n\r\n";

  PT_BEGIN(&s->parse_header_pt);

  /* Skip the first part of the HTTP response */
  while(b != ' ') {
    PT_YIELD(&s->parse_header_pt);
  }

  /* Skip the space that follow the first part */
  PT_YIELD(&s->parse_header_pt);

  /* Read the first three bytes that constistute the HTTP status
     code. We store the HTTP status code as an integer in the
     s->http_status field. */
  s->http_status = (b - '0');
  PT_YIELD(&s->parse_header_pt);
  s->http_status = s->http_status * 10 + (b - '0');
  PT_YIELD(&s->parse_header_pt);
  s->http_status = s->http_status * 10 + (b - '0');

  if((s->proxy_port != 0 && !(s->http_status == 200 || s->http_status == 101)) ||
     (s->proxy_port == 0 && s->http_status != 101)) {
    /* This is a websocket request, so the server should have answered
       with a 101 Switching protocols response. */
    PRINTF("Websocket HTTP client didn't get the 101 status code (got %d), closing connection\n",
           s->http_status);
    websocket_http_client_close(s);
    while(1) {
      PT_YIELD(&s->parse_header_pt);
    }
  }

  /* Keep eating header bytes until we reach the end of it. The end is
     indicated by the string "\r\n\r\n". We don't actually look at any
     of the headers.

     The s->i variable contains the number of consecutive bytes
     matched. If we match the total length of the string, we stop.
  */

  s->i = 0;
  do {
    PT_YIELD(&s->parse_header_pt);
    if(b == (uint8_t)endmarker[s->i]) {
      s->i++;
    } else {
      s->i = 0;
    }
  } while(s->i < strlen(endmarker));

  if(s->proxy_port != 0 && s->state == STATE_WAITING_FOR_HEADER) {
    send_get(s);
    s->state = STATE_WAITING_FOR_CONNECTED;
  } else {
    s->state = STATE_STEADY_STATE;
    websocket_http_client_connected(s);
  }
  PT_END(&s->parse_header_pt);
}
/*---------------------------------------------------------------------------*/
static int
input(struct tcp_socket *tcps, void *ptr,
      const uint8_t *inputptr, int inputdatalen)
{
  struct websocket_http_client_state *s = ptr;

  if(s->state == STATE_WAITING_FOR_HEADER ||
     s->state == STATE_WAITING_FOR_CONNECTED) {
    int i;
    for(i = 0; i < inputdatalen; i++) {
      parse_header_byte(s, inputptr[i]);
      if(s->state == STATE_STEADY_STATE) {
        i++;
        break;
      }
    }

    if(i < inputdatalen && s->state == STATE_STEADY_STATE) {
      websocket_http_client_datahandler(s, &inputptr[i], inputdatalen - i);
    }
  } else {
    websocket_http_client_datahandler(s, inputptr, inputdatalen);
  }

  return 0; /* all data consumed */
}
/*---------------------------------------------------------------------------*/
int
websocket_http_client_register(struct websocket_http_client_state *s,
                               const char *host,
                               uint16_t port,
                               const char *file,
                               const char *subprotocol,
                               const char *header)
{
  if(host == NULL) {
    return -1;
  }
  strncpy(s->host, host, sizeof(s->host));

  if(file == NULL) {
    return -1;
  }
  strncpy(s->file, file, sizeof(s->file));

  if(subprotocol == NULL) {
    return -1;
  }
  strncpy(s->subprotocol, subprotocol, sizeof(s->subprotocol));

  if(header == NULL) {
    strncpy(s->header, "", sizeof(s->header));
  } else {
    strncpy(s->header, header, sizeof(s->header));
  }

  if(port == 0) {
    s->port = 80;
  } else {
    s->port = port;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
int
websocket_http_client_get(struct websocket_http_client_state *s)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;
  uip_ip6addr_t *addr;
  uint16_t port;

  PRINTF("websocket_http_client_get: connecting to %s with file %s subprotocol %s header %s\n",
         s->host, s->file, s->subprotocol, s->header);


  s->state = STATE_WAITING_FOR_HEADER;

  if(tcp_socket_register(&s->s, s,
                         s->inputbuf, sizeof(s->inputbuf),
                         s->outputbuf, sizeof(s->outputbuf),
                         input, event) < 0) {
    return -1;
  }

  port = s->port;
  if(s->proxy_port != 0) {
    /* The proxy address should be an IPv6 address. */
    uip_ipaddr_copy(&ip6addr, &s->proxy_addr);
    port = s->proxy_port;
  } else if(uiplib_ip6addrconv(s->host, &ip6addr) == 0) {
    /* First check if the host is an IP address. */
    if(uiplib_ip4addrconv(s->host, &ip4addr) != 0) {
      ip64_addr_4to6(&ip4addr, &ip6addr);
    } else {
      /* Try to lookup the hostname. If it fails, we initiate a hostname
         lookup. */
      if(resolv_lookup(s->host, &addr) != RESOLV_STATUS_CACHED) {
        return -1;
      }
      return tcp_socket_connect(&s->s, addr, s->port);
    }
  }
  return tcp_socket_connect(&s->s, &ip6addr, port);
}
/*---------------------------------------------------------------------------*/
int
websocket_http_client_send(struct websocket_http_client_state *s,
                           const uint8_t *data,
                           uint16_t datalen)
{
  if(s->state == STATE_STEADY_STATE) {
    return tcp_socket_send(&s->s, data, datalen);
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
websocket_http_client_sendbuflen(struct websocket_http_client_state *s)
{
  return tcp_socket_max_sendlen(&s->s);
}
/*---------------------------------------------------------------------------*/
void
websocket_http_client_close(struct websocket_http_client_state *s)
{
  tcp_socket_close(&s->s);
}
/*---------------------------------------------------------------------------*/
const char *
websocket_http_client_hostname(struct websocket_http_client_state *s)
{
  return s->host;
}
/*---------------------------------------------------------------------------*/
void
websocket_http_client_init(struct websocket_http_client_state *s)
{
  uip_create_unspecified(&s->proxy_addr);
  s->proxy_port = 0;
}
/*---------------------------------------------------------------------------*/
void
websocket_http_client_set_proxy(struct websocket_http_client_state *s,
                                const uip_ipaddr_t *addr, uint16_t port)
{
  uip_ipaddr_copy(&s->proxy_addr, addr);
  s->proxy_port = port;
}
/*---------------------------------------------------------------------------*/
int
websocket_http_client_queuelen(struct websocket_http_client_state *s)
{
  return tcp_socket_queuelen(&s->s);
}
/*---------------------------------------------------------------------------*/
