/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *         A simple web server forwarding page generation to a protothread
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include <stdio.h>
#include <string.h>

#include "contiki-net.h"

//#include "urlconv.h"

#include "httpd-simple.h"
#define webserver_log_file(...)
#define webserver_log(...)

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

#define STATE_WAITING 0
#define STATE_OUTPUT  1

MEMB(conns, struct httpd_state, CONNS);

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_period  0x2e
#define ISO_slash   0x2f

/*---------------------------------------------------------------------------*/
static const char *NOT_FOUND = "<html><body bgcolor=\"white\">"
"<center>"
"<h1>404 - file not found</h1>"
"</center>"
"</body>"
"</html>";
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_string(struct httpd_state *s, const char *str))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, str);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
const char http_content_type_html[] = "Content-type: text/html\r\n\r\n";
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
  /* char *ptr; */

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, statushdr);

  /* ptr = strrchr(s->filename, ISO_period); */
  /* if(ptr == NULL) { */
  /*   s->ptr = http_content_type_plain; */
  /* } else if(strcmp(http_html, ptr) == 0) { */
  /*   s->ptr = http_content_type_html; */
  /* } else if(strcmp(http_css, ptr) == 0) { */
  /*   s->ptr = http_content_type_css; */
  /* } else if(strcmp(http_png, ptr) == 0) { */
  /*   s->ptr = http_content_type_png; */
  /* } else if(strcmp(http_gif, ptr) == 0) { */
  /*   s->ptr = http_content_type_gif; */
  /* } else if(strcmp(http_jpg, ptr) == 0) { */
  /*   s->ptr = http_content_type_jpg; */
  /* } else { */
  /*   s->ptr = http_content_type_binary; */
  /* } */
  /* SEND_STRING(&s->sout, s->ptr); */
  SEND_STRING(&s->sout, http_content_type_html);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
const char http_header_200[] = "HTTP/1.0 200 OK\r\nServer: Contiki/2.4 http://www.sics.se/contiki/\r\nConnection: close\r\n";
const char http_header_404[] = "HTTP/1.0 404 Not found\r\nServer: Contiki/2.4 http://www.sics.se/contiki/\r\nConnection: close\r\n";
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  PT_BEGIN(&s->outputpt);

  s->script = NULL;
  s->script = httpd_simple_get_script(&s->filename[1]);
  if(s->script == NULL) {
    strncpy(s->filename, "/notfound.html", sizeof(s->filename));
    PT_WAIT_THREAD(&s->outputpt,
                   send_headers(s, http_header_404));
    PT_WAIT_THREAD(&s->outputpt,
                   send_string(s, NOT_FOUND));
    uip_close();
    webserver_log_file(&uip_conn->ripaddr, "404 - not found");
    PT_EXIT(&s->outputpt);
  } else {
    PT_WAIT_THREAD(&s->outputpt,
                   send_headers(s, http_header_200));
    PT_WAIT_THREAD(&s->outputpt, s->script(s));
  }
  s->script = NULL;
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
const char http_get[] = "GET ";
const char http_index_html[] = "/index.html";
//const char http_referer[] = "Referer:"
static
PT_THREAD(handle_input(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sin);

  PSOCK_READTO(&s->sin, ISO_space);

  if(strncmp(s->inputbuf, http_get, 4) != 0) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  PSOCK_READTO(&s->sin, ISO_space);

  if(s->inputbuf[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }

#if URLCONV
  s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
  urlconv_tofilename(s->filename, s->inputbuf, sizeof(s->filename));
#else /* URLCONV */
  if(s->inputbuf[1] == ISO_space) {
    strncpy(s->filename, http_index_html, sizeof(s->filename));
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, s->inputbuf, sizeof(s->filename));
  }
#endif /* URLCONV */

  webserver_log_file(&uip_conn->ripaddr, s->filename);

  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);
#if 0
    if(strncmp(s->inputbuf, http_referer, 8) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
      webserver_log(s->inputbuf);
    }
#endif
  }

  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
}

/*---------------------------------------------------------------------------*/
void
httpd_appcall(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  if(uip_closed() || uip_aborted() || uip_timedout()) {
    if(s != NULL) {
      s->script = NULL;
      memb_free(&conns, s);
    }
  } else if(uip_connected()) {
    s = (struct httpd_state *)memb_alloc(&conns);
    if(s == NULL) {
      uip_abort();
      webserver_log_file(&uip_conn->ripaddr, "reset (no memory block)");
      return;
    }
    tcp_markconn(uip_conn, s);
    PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->script = NULL;
    s->state = STATE_WAITING;
    timer_set(&s->timer, CLOCK_SECOND * 10);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
        uip_abort();
        s->script = NULL;
        memb_free(&conns, s);
        webserver_log_file(&uip_conn->ripaddr, "reset (timeout)");
      }
    } else {
      timer_restart(&s->timer);
    }
    handle_connection(s);
  } else {
    uip_abort();
  }
}

/*---------------------------------------------------------------------------*/
void
httpd_init(void)
{

  tcp_listen(UIP_HTONS(80));
  memb_init(&conns);
#if URLCONV
  urlconv_init();
#endif /* URLCONV */
}
/*---------------------------------------------------------------------------*/
