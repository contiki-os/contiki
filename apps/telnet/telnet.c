/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 *
 */

#include "contiki-net.h"

#include "telnet.h"

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#define FLAG_CLOSE 1
#define FLAG_ABORT 2
/*-----------------------------------------------------------------------------------*/
unsigned char
telnet_send(struct telnet_state *s, char *text, uint16_t len)
{
  if(s->text != NULL) {
    return 1;
  }
  s->text = text;
  s->textlen = len;
  s->sentlen = 0;
  return 0;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
telnet_close(struct telnet_state *s)
{
  s->flags = FLAG_CLOSE;
  if(s->text != NULL) {
    return 1;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
telnet_abort(struct telnet_state *s)
{
  s->flags = FLAG_ABORT;
  if(s->text != NULL) {
    return 1;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
acked(struct telnet_state *s)
{
  s->textlen -= s->sentlen;
  if(s->textlen == 0) {
    s->text = NULL;
    telnet_sent(s);
  } else {
    s->text += s->sentlen;
  }
  s->sentlen = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
senddata(struct telnet_state *s)
{
  if(s->text == NULL) {
    uip_send(s->text, 0);
    return;
  }
  if(s->textlen > uip_mss()) {
    s->sentlen = uip_mss();
  } else {
    s->sentlen = s->textlen;
  }
  uip_send(s->text, s->sentlen);
}
/*-----------------------------------------------------------------------------------*/
struct telnet_state *
telnet_connect(struct telnet_state *s, uip_ipaddr_t *addr, uint16_t port)
{
  struct uip_conn *conn;
  
  conn = tcp_connect(addr, uip_htons(port), s);
  if(conn == NULL) {
    return NULL;
  }
  return s;
}
/*-----------------------------------------------------------------------------------*/
void
telnet_app(void *ts)
{
  struct telnet_state *s = (struct telnet_state *)ts;
    
  if(uip_connected()) {
    s->flags = 0;
    telnet_connected(s);
    senddata(s);
    return;
  }
  
  if(uip_closed()) {
    telnet_closed(s);
  }
  
  if(uip_aborted()) {
    telnet_aborted(s);
  }
  if(uip_timedout()) {
    telnet_timedout(s);
  }

  if(s->flags & FLAG_CLOSE) {
    uip_close();
    return;
  }
  if(s->flags & FLAG_ABORT) {
    uip_abort();
    return;
  }
  if(uip_acked()) {
    acked(s);
  }
  if(uip_newdata()) {
    telnet_newdata(s, (char *)uip_appdata, uip_datalen());
  }
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata(s);
  } else if(uip_poll()) {
    senddata(s);
  }
}
/*-----------------------------------------------------------------------------------*/
