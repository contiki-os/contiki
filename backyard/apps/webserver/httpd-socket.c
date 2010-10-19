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
 * $Id: httpd-socket.c,v 1.2 2010/10/19 18:29:03 adamdunkels Exp $
 */

#include "contiki.h"
#include "cfs.h"
#include "socket.h"

struct httpd_state {
  struct socket sin, sout;
  char inputbuf[100];
  char filename[40];
  char state;
  int fd;
  int ret;
  char outputbuf[UIP_TCP_MSS];
};

#define STATE_WAITING 0
#define STATE_OUTPUT  1

#define SEND_STRING(s, str) SOCKET_SEND(s, str, strlen(str))
MEMB(conns, struct httpd_state, 8);


/*---------------------------------------------------------------------------*/
static unsigned short
generate(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  memcpy(uip_appdata, s->outputbuf, s->ret);

  return s->ret;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  char *ptr;
  
  SOCKET_BEGIN(&s->sout);

  
  s->fd = cfs_open(s->filename, CFS_READ);
  if(s->fd < 0) {
    SEND_STRING(&s->sout, "HTTP/1.0 404 Not found\r\n");
    SOCKET_CLOSE_EXIT(&s->sout);
  } else {
    SEND_STRING(&s->sout, "HTTP/1.0 200 Found\r\n");
  }
  ptr = strrchr(s->filename, '.');
  if(strncmp(".html", ptr, 5) == 0) {
    SEND_STRING(&s->sout, "Content-type: text/html\r\n");
  } else if(strncmp(".css", ptr, 4) == 0) {
    SEND_STRING(&s->sout, "Content-type: text/css\r\n");
  } else if(strncmp(".png", ptr, 4) == 0) {
    SEND_STRING(&s->sout, "Content-type: image/png\r\n");
  } else if(strncmp(".jpg", ptr, 4) == 0) {
    SEND_STRING(&s->sout, "Content-type: image/jpeg\r\n");
  } else {
    SEND_STRING(&s->sout, "Content-type: text/plain\r\n");
  }
  SEND_STRING(&s->sout, "Server: Contiki/1.2-devel0\r\n\r\n");

  do {
    s->ret = cfs_read(s->fd, s->outputbuf, sizeof(s->outputbuf));
    if(s->ret > 0) {
      SOCKET_GENERATE_SEND(&s->sout, generate, s);
      /*      SOCKET_SEND(&s->sout, s->outputbuf, s->ret);*/
    }
  } while(s->ret > 0);


  cfs_close(s->filename);
  SOCKET_CLOSE(&s->sout);
  
  SOCKET_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_state *s))
{
  SOCKET_BEGIN(&s->sin);

  SOCKET_READTO(&s->sin, ' ');

  
  if(strncmp(s->inputbuf, "GET ", 4) != 0) {
    SOCKET_CLOSE_EXIT(&s->sin);
  }
  SOCKET_READTO(&s->sin, ' ');

  if(s->inputbuf[0] != '/') {
    SOCKET_CLOSE_EXIT(&s->sin);
  }

  if(s->inputbuf[1] == ' ') {
    strncpy(s->filename, "index.html", sizeof(s->filename));
  } else {
    s->inputbuf[SOCKET_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, &s->inputbuf[1], sizeof(s->filename));
  }

  s->state = STATE_OUTPUT;

  while(1) {
    SOCKET_READTO(&s->sin, '\n');

    if(strncmp(s->inputbuf, "Referer:", 8) == 0) {
      s->inputbuf[SOCKET_DATALEN(&s->sin) - 2] = 0;
      webserver_log(&s->inputbuf[9]);
    }
  }
  
  SOCKET_END(&s->sin);
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
    SOCKET_INIT(&s->sin, s->inputbuf, sizeof(s->inputbuf) - 1);
    SOCKET_INIT(&s->sout, s->inputbuf, sizeof(s->inputbuf) - 1);
    s->state = STATE_WAITING;
    handle_connection(s);
  } else if(s != NULL) {
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
