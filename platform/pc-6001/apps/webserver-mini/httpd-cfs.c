/*
 * Copyright (c) 2004, Adam Dunkels.
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
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: httpd-cfs.c,v 1.2 2010/10/19 18:29:05 adamdunkels Exp $
 */

#include <string.h>

#include "contiki-net.h"
#include "cfs/cfs.h"

#include "webserver.h"
#include "libconio_arch-small.h"

#include "httpd-cfs.h"

#define STATE_WAITING 0
#define STATE_OUTPUT  1

#define SEND_STRING(s, str) PSOCK_SEND(s, str, strlen(str))
MEMB(conns, struct httpd_state, 2);

/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  
  do {
    /* Read data from file system into buffer */
    s->len = cfs_read(s->fd, s->outputbuf, sizeof(s->outputbuf));

    /* If there is data in the buffer, send it */
    if(s->len > 0) {
      PSOCK_SEND(&s->sout, s->outputbuf, s->len);
    } else {
      break;
    }
  } while(s->len > 0);
      
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_state *s, char *statushdr))
{
  char *ptr;

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, statushdr);
  SEND_STRING(&s->sout, "Server: " CONTIKI_VERSION_STRING "\r\n");

  ptr = strrchr(s->filename, '.');
  if(ptr == NULL) {
    SEND_STRING(&s->sout, "Content-type: text/plain\r\n\r\n");
  } else if(strncmp(".html", ptr, 5) == 0) {
    SEND_STRING(&s->sout, "Content-type: text/html\r\n\r\n");
  } else if(strncmp(".css", ptr, 4) == 0) {
    SEND_STRING(&s->sout, "Content-type: text/css\r\n\r\n");
  } else if(strncmp(".png", ptr, 4) == 0) {
    SEND_STRING(&s->sout, "Content-type: image/png\r\n\r\n");
  } else if(strncmp(".jpg", ptr, 4) == 0) {
    SEND_STRING(&s->sout, "Content-type: image/jpeg\r\n\r\n");
  } else {
    SEND_STRING(&s->sout, "Content-type: application/octet-stream\r\n\r\n");
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  PT_BEGIN(&s->outputpt);

  s->fd = cfs_open(s->filename, CFS_READ);
  if(s->fd < 0) {
    s->fd = cfs_open("404.html", CFS_READ);
    if(s->fd < 0) {
      uip_abort();
      PT_EXIT(&s->outputpt);
    }
    PT_WAIT_THREAD(&s->outputpt,
		   send_headers(s, "HTTP/1.0 404 Not found\r\n"));
    PT_WAIT_THREAD(&s->outputpt,
		   send_file(s));
  } else {
    PT_WAIT_THREAD(&s->outputpt,
		   send_headers(s, "HTTP/1.0 200 OK\r\n"));
    PT_WAIT_THREAD(&s->outputpt,
		   send_file(s));
    cfs_close(s->fd);
  }
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sin);

  PSOCK_READTO(&s->sin, ' ');
  
  if(strncmp(s->inputbuf, "GET ", 4) != 0) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  PSOCK_READTO(&s->sin, ' ');

  if(s->inputbuf[0] != '/') {
    PSOCK_CLOSE_EXIT(&s->sin);
  }

  if(s->inputbuf[1] == ' ') {
    strncpy(s->filename, "index.html", sizeof(s->filename));
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, &s->inputbuf[1], sizeof(s->filename));
  }

  //  webserver_log_file(&uip_conn->ripaddr, s->filename);
  libputs_arch(s->filename);
  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, '\n');

    if(strncmp(s->inputbuf, "Referer:", 8) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
      libputs_arch(&s->inputbuf[9]);
      //      webserver_log(&s->inputbuf[9]);
    }
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
      memb_free(&conns, s);
    }
  } else if(uip_connected()) {
    s = (struct httpd_state *)memb_alloc(&conns);
    if(s == NULL) {
      uip_abort();
      return;
    }
    tcp_markconn(uip_conn, s);
    PSOCK_INIT(&s->sin, s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->state = STATE_WAITING;
    timer_set(&s->timer, CLOCK_SECOND * 10);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
	uip_abort();
      }
    } else {
      timer_reset(&s->timer);
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
}
/*---------------------------------------------------------------------------*/
