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
 * $Id: httpd-cfs.c,v 1.26 2011/01/25 20:13:41 oliverschmidt Exp $
 */

#include <stdio.h>
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */
#include <string.h>

#include "contiki-net.h"

#include "webserver.h"
#include "cfs/cfs.h"
#include "lib/petsciiconv.h"
#include "http-strings.h"
#include "urlconv.h"

#include "httpd-cfs.h"

#ifndef WEBSERVER_CONF_CFS_CONNS
#define CONNS UIP_CONNS
#else /* WEBSERVER_CONF_CFS_CONNS */
#define CONNS WEBSERVER_CONF_CFS_CONNS
#endif /* WEBSERVER_CONF_CFS_CONNS */

#ifndef WEBSERVER_CONF_CFS_URLCONV
#define URLCONV 1
#else /* WEBSERVER_CONF_CFS_URLCONV */
#define URLCONV WEBSERVER_CONF_CFS_URLCONV
#endif /* WEBSERVER_CONF_CFS_URLCONV */

#define STATE_WAITING 0
#define STATE_OUTPUT  1

#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, strlen(str))
MEMB(conns, struct httpd_state, CONNS);

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_period  0x2e
#define ISO_slash   0x2f

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
      PSOCK_SEND(&s->sout, (uint8_t *)s->outputbuf, s->len);
    } else {
      break;
    }
  } while(s->len > 0);
      
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_string(struct httpd_state *s, const char *str))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, str);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
  const char *ptr;

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, statushdr);

  ptr = strrchr(s->filename, ISO_period);
  if(ptr == NULL) {
    ptr = http_content_type_plain;
  } else if(strcmp(http_htm, ptr) == 0) {
    ptr = http_content_type_html;
  } else if(strcmp(http_css, ptr) == 0) {
    ptr = http_content_type_css;
  } else if(strcmp(http_png, ptr) == 0) {
    ptr = http_content_type_png;
  } else if(strcmp(http_gif, ptr) == 0) {
    ptr = http_content_type_gif;
  } else if(strcmp(http_jpg, ptr) == 0) {
    ptr = http_content_type_jpg;
  } else {
    ptr = http_content_type_binary;
  }
  SEND_STRING(&s->sout, ptr);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  PT_BEGIN(&s->outputpt);

  petsciiconv_topetscii(s->filename, sizeof(s->filename));
  s->fd = cfs_open(&s->filename[1], CFS_READ);
  petsciiconv_toascii(s->filename, sizeof(s->filename));
  if(s->fd < 0) {
    strcpy(s->filename, "/notfound.htm");
    s->fd = cfs_open(&s->filename[1], CFS_READ);
    petsciiconv_toascii(s->filename, sizeof(s->filename));
    PT_WAIT_THREAD(&s->outputpt,
                   send_headers(s, http_header_404));
    if(s->fd < 0) {
      PT_WAIT_THREAD(&s->outputpt,
                     send_string(s, "not found"));
      uip_close();
      webserver_log_file(&uip_conn->ripaddr, "404 (no notfound.htm)");
      PT_EXIT(&s->outputpt);
    }
    webserver_log_file(&uip_conn->ripaddr, "404 - notfound.htm");
  } else {
    PT_WAIT_THREAD(&s->outputpt,
		   send_headers(s, http_header_200));
  }
  PT_WAIT_THREAD(&s->outputpt, send_file(s));
  cfs_close(s->fd);
  s->fd = -1;
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
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
    strncpy(s->filename, http_index_htm, sizeof(s->filename));
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, s->inputbuf, sizeof(s->filename));
  }
#endif /* URLCONV */

  petsciiconv_topetscii(s->filename, sizeof(s->filename));
  webserver_log_file(&uip_conn->ripaddr, s->filename);
  petsciiconv_toascii(s->filename, sizeof(s->filename));
  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);

    if(strncmp(s->inputbuf, http_referer, 8) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
      petsciiconv_topetscii(s->inputbuf, PSOCK_DATALEN(&s->sin) - 2);
      webserver_log(s->inputbuf);
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
      if(s->fd >= 0) {
        cfs_close(s->fd);
	s->fd = -1;
      }
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
    s->fd = -1;
    s->state = STATE_WAITING;
    timer_set(&s->timer, CLOCK_SECOND * 10);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
	uip_abort();
	if(s->fd >= 0) {
	  cfs_close(s->fd);
	  s->fd = -1;
	}
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
