/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Code for sending the JSON data as a UDP packet
 *         Specify proto = "udp", port = <server-udp-port>
 *         host = <ipv6-server-address>
 *
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "contiki.h"
#include "httpd-ws.h"
#include "jsontree.h"
#include "jsonparse.h"
#include "json-ws.h"
#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_FULL
#include "net/uip-debug.h"

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;
static uint16_t server_port;

#define SENDER_PORT 8181

/*---------------------------------------------------------------------------*/
int
json_ws_udp_setup(const char *host, uint16_t port)
{

  server_port = port;

  if(client_conn != NULL) {
    /* this should be a macro uip_udp_conn_free() or something */
    uip_udp_remove(client_conn);
    client_conn = NULL;
  }

  uip_ipaddr_t *ipaddr;

  /* First check if the host is an IP address. */
  ipaddr = &server_ipaddr;
  if(uiplib_ipaddrconv(host, &server_ipaddr) == 0) {
#if 0 && UIP_UDP
    if(resolv_lookup(host, &ipaddr) != RESOLV_STATUS_CACHED) {
      return 0;
    }
#else /* UIP_UDP */
    return 0;
#endif /* UIP_UDP */
  }

  /* new connection with remote host */
  client_conn = udp_new(&server_ipaddr, UIP_HTONS(server_port), NULL);
  udp_bind(client_conn, UIP_HTONS(SENDER_PORT));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
         UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
  return 1;
}

/*---------------------------------------------------------------------------*/

static char *udp_buf;
static int pos;
static int size;

static int
putchar_udp(int c)
{
  if(udp_buf != NULL && pos <= size) {
    udp_buf[pos++] = c;
    return c;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
void
json_ws_udp_send(struct jsontree_value *tree, const char *path)
{
  struct jsontree_context json;
  /* maxsize = 70 bytes */
  char buf[70];

  udp_buf = buf;

  /* reset state and set max-size */
  /* NOTE: packet will be truncated at 70 bytes */
  pos = 0;
  size = sizeof(buf);

  json.values[0] = (struct json_value *)tree;
  jsontree_reset(&json);
  find_json_path(&json, path);
  json.path = json.depth;
  json.putchar = putchar_udp;
  while(jsontree_print_next(&json) && json.path <= json.depth);

  printf("Real UDP size: %d\n", pos);
  buf[pos] = 0;

  uip_udp_packet_sendto(client_conn, &buf, pos,
                        &server_ipaddr, UIP_HTONS(server_port));
}
/*---------------------------------------------------------------------------*/
void
json_ws_udp_debug(char *string)
{
  int len;

  len = strlen(string);
  uip_udp_packet_sendto(client_conn, string, len,
                        &server_ipaddr, UIP_HTONS(server_port));
}
/*---------------------------------------------------------------------------*/
