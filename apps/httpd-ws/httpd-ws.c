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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "contiki-net.h"
#include "httpd-ws.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef WEBSERVER_CONF_CFS_CONNS
#define CONNS UIP_CONNS
#else /* WEBSERVER_CONF_CFS_CONNS */
#define CONNS WEBSERVER_CONF_CFS_CONNS
#endif /* WEBSERVER_CONF_CFS_CONNS */

#ifndef WEBSERVER_CONF_CFS_URLCONV
#define URLCONV 0
#else /* WEBSERVER_CONF_CFS_URLCONV */
#define URLCONV WEBSERVER_CONF_CFS_URLCONV
#endif /* WEBSERVER_CONF_CFS_URLCONV */

#if URLCONV
#include "urlconv.h"
#endif /* URLCONV */

static struct httpd_ws_state conns[CONNS];

PROCESS(httpd_ws_process, "Web server (WS)");

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_period  0x2e
#define ISO_slash   0x2f

uint16_t http_connections = 0;

static const char http_10[] = " HTTP/1.0\r\n";
static const char http_content_type[] = "Content-Type:";
static const char http_content_type_html[] = "text/html";
static const char http_content_len[] = "Content-Length:";
static const char http_header_404[] =
  "HTTP/1.0 404 Not found\r\nServer: Contiki\r\nConnection: close\r\n";
static const char http_header_200[] =
  "HTTP/1.0 200 OK\r\nServer: Contiki\r\nConnection: close\r\n";
static const char html_not_found[] =
  "<html><body><h1>Page not found</h1></body></html>";
/*---------------------------------------------------------------------------*/
/* just set all states to unused */
static void
httpd_state_init(void)
{
  int i;

  for(i = 0; i < CONNS; i++) {
    conns[i].state = HTTPD_WS_STATE_UNUSED;
  }
}
/*---------------------------------------------------------------------------*/
static struct httpd_ws_state *
httpd_state_alloc(void)
{
  int i;

  for(i = 0; i < CONNS; i++) {
    if(conns[i].state == HTTPD_WS_STATE_UNUSED) {
      conns[i].state = HTTPD_WS_STATE_INPUT;
      return &conns[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
#define httpd_state_free(s) (s->state = HTTPD_WS_STATE_UNUSED)
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_string(struct httpd_ws_state *s, const char *str, uint16_t len))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, str, len);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_ws_state *s, const char *statushdr))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, statushdr, strlen(statushdr));
  s->outbuf_pos = snprintf(s->outbuf, sizeof(s->outbuf),
                           "%s %s\r\n\r\n", http_content_type,
                           s->content_type == NULL
                           ? http_content_type_html : s->content_type);
  SEND_STRING(&s->sout, s->outbuf, s->outbuf_pos);
  s->outbuf_pos = 0;

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_ws_state *s))
{
  PT_BEGIN(&s->outputpt);

  s->content_type = http_content_type_html;
  s->script = httpd_ws_get_script(s);
  if(s->script == NULL) {
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_404));
    PT_WAIT_THREAD(&s->outputpt,
                   send_string(s, html_not_found, strlen(html_not_found)));
    uip_close();
/*     webserver_log_file(&uip_conn->ripaddr, "404 - not found"); */
    PT_EXIT(&s->outputpt);
  } else {
    if(s->request_type == HTTPD_WS_POST) {
      /* A post has a body that needs to be read */
      s->state = HTTPD_WS_STATE_INPUT;
      PT_WAIT_UNTIL(&s->outputpt, s->state == HTTPD_WS_STATE_OUTPUT);
    }
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_200));
    PT_WAIT_THREAD(&s->outputpt, s->script(s));
  }
  s->script = NULL;
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_request(struct httpd_ws_state *s))
{
  PT_BEGIN(&s->outputpt);

  /* send the request line */
  PT_WAIT_THREAD(&s->outputpt,
                 send_string(s, s->filename, strlen(s->filename)));
  /* send host  */
  if(s->outbuf_pos > 0) {
    PT_WAIT_THREAD(&s->outputpt, send_string(s, s->outbuf, s->outbuf_pos));
  }

  if(s->content_type != NULL) {
    s->outbuf_pos = snprintf(s->outbuf, sizeof(s->outbuf), "%s %s\r\n",
                             http_content_type, s->content_type);
    PT_WAIT_THREAD(&s->outputpt, send_string(s, s->outbuf, s->outbuf_pos));
  }
  /* send the extra header(s) */
  if(s->output_extra_headers != NULL) {
    s->response_index = 0;
    while((s->outbuf_pos =
           s->output_extra_headers(s,
                                   s->outbuf, sizeof(s->outbuf),
                                   s->response_index)) > 0) {
      PT_WAIT_THREAD(&s->outputpt, send_string(s, s->outbuf, s->outbuf_pos));
      s->response_index++;
    }
  }

  /* send content length */
  if(s->content_len > 0) {
    s->outbuf_pos = snprintf(s->outbuf, sizeof(s->outbuf), "%s %u\r\n",
                             http_content_len, s->content_len);
  }
  /* send header separator */
  if(s->outbuf_pos + 2 < sizeof(s->outbuf)) {
    s->outbuf[s->outbuf_pos++] = '\r';
    s->outbuf[s->outbuf_pos++] = '\n';
  }
  PT_WAIT_THREAD(&s->outputpt, send_string(s, s->outbuf, s->outbuf_pos));
  s->outbuf_pos = 0;

  if(s->script != NULL) {
    PT_WAIT_THREAD(&s->outputpt, s->script(s));
  }
  s->state = HTTPD_WS_STATE_REQUEST_INPUT;

  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_ws_state *s))
{
  PSOCK_BEGIN(&s->sin);
  PSOCK_READTO(&s->sin, ISO_space);

  if(strncmp(s->inputbuf, "GET ", 4) == 0) {
    s->request_type = HTTPD_WS_GET;
  } else if(strncmp(s->inputbuf, "POST ", 5) == 0) {
    s->request_type = HTTPD_WS_POST;
    s->content_len = 0;
  } else if(strncmp(s->inputbuf, "HTTP ", 5) == 0) {
    s->request_type = HTTPD_WS_RESPONSE;
  } else {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  PSOCK_READTO(&s->sin, ISO_space);

  /* TODO handle HTTP response */

  if(s->inputbuf[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }

#if URLCONV
  s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
  urlconv_tofilename(s->filename, s->inputbuf, sizeof(s->filename));
#else /* URLCONV */
  s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
  snprintf(s->filename, sizeof(s->filename), "%s", s->inputbuf);
#endif /* URLCONV */

/*   webserver_log_file(&uip_conn->ripaddr, s->filename); */
  s->state = HTTPD_WS_STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);

    if(s->request_type == HTTPD_WS_POST &&
       strncmp(s->inputbuf, http_content_len, 15) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
      s->content_len = atoi(&s->inputbuf[16]);
    }

    /* should have a header callback here check_header(s) */

    if(PSOCK_DATALEN(&s->sin) > 2) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
    } else if(s->request_type == HTTPD_WS_POST) {
      PSOCK_READBUF_LEN(&s->sin, s->content_len);
      s->inputbuf[PSOCK_DATALEN(&s->sin)] = 0;
      /* printf("Content: '%s'\nSize:%d\n", s->inputbuf, PSOCK_DATALEN(&s->sin)); */
      s->state = HTTPD_WS_STATE_OUTPUT;
    }
  }
  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_ws_state *s)
{
  if(s->state == HTTPD_WS_STATE_REQUEST_OUTPUT) {
    handle_request(s);
  }
  handle_input(s);
  if(s->state == HTTPD_WS_STATE_OUTPUT) {
    handle_output(s);
  }
}
/*---------------------------------------------------------------------------*/
void
httpd_ws_appcall(void *state)
{
  struct httpd_ws_state *s = (struct httpd_ws_state *)state;

  if(uip_closed() || uip_aborted() || uip_timedout()) {
    if(s != NULL) {
      PRINTF("HTTPD-WS: closed/aborted (%d)\n", http_connections);
      http_connections--;
      httpd_state_free(s);
    } else {
      PRINTF("HTTPD-WS: closed/aborted ** NO HTTPD_WS_STATE!!! ** (%d)\n",
             http_connections);
    }
  } else if(uip_connected()) {
    if(s == NULL) {
      s = httpd_state_alloc();
      if(s == NULL) {
        uip_abort();
        PRINTF("HTTPD-WS: aborting - no resource (%d)\n", http_connections);
        /*       webserver_log_file(&uip_conn->ripaddr, "reset (no memory block)"); */
        return;
      }
      http_connections++;

      tcp_markconn(uip_conn, s);
      s->state = HTTPD_WS_STATE_INPUT;
    } else {
      /* this is a request that is to be sent! */
      s->state = HTTPD_WS_STATE_REQUEST_OUTPUT;
    }
    PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    timer_set(&s->timer, CLOCK_SECOND * 30);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
        uip_abort();
        PRINTF("HTTPD-WS: aborting - http timeout (%d)\n", http_connections);
        http_connections--;
        httpd_state_free(s);
/*         webserver_log_file(&uip_conn->ripaddr, "reset (timeout)"); */
      } else {
        PRINTF("HTTPD-WS: uip-poll (%d)\n", http_connections);
      }
    } else {
/*       PRINTF("HTTPD-WS: restart timer %s (%d)\n", s->filename, */
/* 	     http_connections); */
      timer_restart(&s->timer);
    }
    handle_connection(s);
  } else {
    PRINTF("HTTPD-WS: aborting - no state (%d)\n", http_connections);
    uip_abort();
  }
}
/*---------------------------------------------------------------------------*/
void
httpd_ws_init(void)
{
  tcp_listen(UIP_HTONS(80));
  httpd_state_init();
#if URLCONV
  urlconv_init();
#endif /* URLCONV */
}
/*---------------------------------------------------------------------------*/
struct httpd_ws_state *
httpd_ws_request(char request_type, const char *host_ip, const char *host_hdr,
                 uint16_t port, const char *file,
                 const char *content_type, uint16_t content_len,
                 httpd_ws_script_t generator)
{
  struct httpd_ws_state *s;
  struct uip_conn *conn;
  uip_ipaddr_t *ipaddr;
  uip_ipaddr_t addr;
  char *request_str;

  /* First check if the host is an IP address. */
  ipaddr = &addr;
  if(uiplib_ipaddrconv(host_ip, &addr) == 0) {
#if 0 && UIP_UDP
    ipaddr = resolv_lookup(host_ip);

    if(ipaddr == NULL) {
      return NULL;
    }
#else /* UIP_UDP */
    return NULL;
#endif /* UIP_UDP */
  }

  s = httpd_state_alloc();
  if(s == NULL) {
    /* no memory left... do no request... */
    return NULL;
  }
  http_connections++;

  switch(request_type) {
  case HTTPD_WS_POST:
    request_str = "POST ";
    break;
  case HTTPD_WS_PUT:
    request_str = "PUT ";
    break;
  default:
    request_str = "GET ";
    break;
  }

  s->request_type = request_type;
  s->content_len = content_len;
  s->content_type = content_type;
  s->script = generator;
  s->state = HTTPD_WS_STATE_REQUEST_OUTPUT;

  /* create a request line for a POST - should check size of it!!! */
  /* Assume post for now */
  snprintf(s->filename, sizeof(s->filename), "%s%s%s",
           request_str, file, http_10);
  s->outbuf_pos = snprintf(s->outbuf, sizeof(s->outbuf), "Host:%s\r\n",
                           host_hdr != NULL ? host_hdr : host_ip);

  PROCESS_CONTEXT_BEGIN(&httpd_ws_process);
  conn = tcp_connect(ipaddr, uip_htons(port), s);
  PROCESS_CONTEXT_END(&httpd_ws_process);
  if(conn == NULL) {
    PRINTF("HTTPD-WS: aborting... could not allocate tcp connection (%d)\n",
           http_connections);
    httpd_state_free(s);
    http_connections--;
    return NULL;
  }
  PRINTF("HTTPD-WS: created http connection (%d)\n", http_connections);

  return s;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(httpd_ws_process, ev, data)
{
  static struct etimer et;
  int i;

  PROCESS_BEGIN();

  httpd_ws_init();

  PRINTF("Buffer size, input %d, output\n",
         HTTPD_INBUF_SIZE, HTTPD_OUTBUF_SIZE);

  /* Delay 2-4 seconds */
  etimer_set(&et, CLOCK_SECOND * 10);

  /* GC any http session that is too long lived - either because other
     end never closed or if any other state cause too long lived http
     sessions */
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event || etimer_expired(&et));
    if(ev == tcpip_event) {
      httpd_ws_appcall(data);
    } else if(etimer_expired(&et)) {
      PRINTF("HTTPD States: ");
      for(i = 0; i < CONNS; i++) {
        PRINTF("%d ", conns[i].state);
        if(conns[i].state != HTTPD_WS_STATE_UNUSED &&
           timer_expired(&conns[i].timer)) {
          conns[i].state = HTTPD_WS_STATE_UNUSED;
          PRINTF("\n*** RELEASED HTTPD Session\n");
          http_connections--;
        }
      }
      PRINTF("\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
