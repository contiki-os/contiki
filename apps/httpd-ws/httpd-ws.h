/*
 * Copyright (c) 2010-2012, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         A simple webserver for web services
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __HTTPD_WS_H__
#define __HTTPD_WS_H__

#include "contiki-net.h"

#ifndef WEBSERVER_CONF_CFS_PATHLEN
#define HTTPD_PATHLEN 80
#else /* WEBSERVER_CONF_CFS_CONNS */
#define HTTPD_PATHLEN WEBSERVER_CONF_CFS_PATHLEN
#endif /* WEBSERVER_CONF_CFS_CONNS */

#ifndef WEBSERVER_CONF_INBUF_SIZE
#define HTTPD_INBUF_SIZE (HTTPD_PATHLEN + 90)
#else /* WEBSERVER_CONF_INBUF_SIZE */
#define  HTTPD_INBUF_SIZE WEBSERVER_CONF_INBUF_SIZE
#endif /* WEBSERVER_CONF_INBUF_SIZE */

#if HTTPD_INBUF_SIZE < UIP_TCP_MSS || HTTPD_INBUF_SIZE < UIP_RECEIVE_WINDOW
#error HTTPD_INBUF_SIZE is too small. Must be at least a TCP window in size.
#endif

#ifndef WEBSERVER_CONF_OUTBUF_SIZE
#define HTTPD_OUTBUF_SIZE (UIP_TCP_MSS + 20)
#else /* WEBSERVER_CONF_OUTBUF_SIZE */
#define  HTTPD_OUTBUF_SIZE WEBSERVER_CONF_OUTBUF_SIZE
#endif /* WEBSERVER_CONF_OUTBUF_SIZE */

struct httpd_ws_state;
typedef char (* httpd_ws_script_t)(struct httpd_ws_state *s);
typedef int (* httpd_ws_output_headers_t)(struct httpd_ws_state *s,
                                          char *buffer, int buf_size,
                                          int index);

#define HTTPD_WS_GET      1
#define HTTPD_WS_POST     2
#define HTTPD_WS_PUT      3
#define HTTPD_WS_RESPONSE 4

#define HTTPD_WS_STATE_UNUSED         0
#define HTTPD_WS_STATE_INPUT          1
#define HTTPD_WS_STATE_OUTPUT         2
#define HTTPD_WS_STATE_REQUEST_OUTPUT 3
#define HTTPD_WS_STATE_REQUEST_INPUT  4

struct httpd_ws_state {
  struct timer timer;
  struct psock sin, sout;
  struct pt outputpt;
  char inputbuf[HTTPD_INBUF_SIZE];
  char filename[HTTPD_PATHLEN];
  const char *content_type;
  uint16_t content_len;
  char outbuf[HTTPD_OUTBUF_SIZE];
  uint16_t outbuf_pos;
  char state;
  char request_type;
  int response_index;

  httpd_ws_output_headers_t output_extra_headers;
  httpd_ws_script_t script;

#ifdef HTTPD_WS_CONF_USER_STATE
  HTTPD_WS_CONF_USER_STATE;
#endif
};

void httpd_ws_init(void);
void httpd_ws_appcall(void *state);

struct httpd_ws_state *httpd_ws_request(char request_type,
                                        const char *host_ip,
                                        const char *host_hdr,
                                        uint16_t port,
                                        const char *file,
                                        const char *content_type,
                                        uint16_t content_len,
                                        httpd_ws_script_t generator);

#define SEND_STRING(s, str, len) PSOCK_SEND((s), (uint8_t *)(str), (len))

httpd_ws_script_t httpd_ws_get_script(struct httpd_ws_state *s);

PROCESS_NAME(httpd_ws_process);

#endif /* __HTTPD_WS_H__ */
