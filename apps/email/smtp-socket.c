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
 * $Id: smtp-socket.c,v 1.5 2010/10/19 18:29:03 adamdunkels Exp $
 */
#include "smtp.h"

#include "smtp-strings.h"
#include "contiki-net.h"

#include <string.h>

struct smtp_state {

  char connected;
  
  struct psock psock;

  char inputbuffer[4];
  
  char *to;
  char *cc;
  char *from;
  char *subject;
  char *msg;
  uint8_t msgwidth;
  uint8_t msgheight;
  uint8_t line;
};

static struct smtp_state s;

static char *localhostname;
static uip_ipaddr_t smtpserver;

#define ISO_nl 0x0a
#define ISO_cr 0x0d

#define ISO_period 0x2e

#define ISO_2  0x32
#define ISO_3  0x33
#define ISO_4  0x34
#define ISO_5  0x35


#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, (unsigned char)strlen(str))
/*---------------------------------------------------------------------------*/
static
PT_THREAD(smtp_thread(void))
{
  PSOCK_BEGIN(&s.psock);

  PSOCK_READTO(&s.psock, ISO_nl);
   
  if(strncmp(s.inputbuffer, smtp_220, 3) != 0) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(2);
    PSOCK_EXIT(&s.psock);
  }
  
  SEND_STRING(&s.psock, (char *)smtp_helo);
  SEND_STRING(&s.psock, localhostname);
  SEND_STRING(&s.psock, (char *)smtp_crnl);

  PSOCK_READTO(&s.psock, ISO_nl);
  
  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(3);
    PSOCK_EXIT(&s.psock);
  }  

  SEND_STRING(&s.psock, (char *)smtp_mail_from);
  SEND_STRING(&s.psock, s.from);
  SEND_STRING(&s.psock, (char *)smtp_crnl);

  PSOCK_READTO(&s.psock, ISO_nl);
  
  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(4);
    PSOCK_EXIT(&s.psock);
  }

  SEND_STRING(&s.psock, (char *)smtp_rcpt_to);
  SEND_STRING(&s.psock, s.to);
  SEND_STRING(&s.psock, (char *)smtp_crnl);

  PSOCK_READTO(&s.psock, ISO_nl);
  
  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(5);
    PSOCK_EXIT(&s.psock);
  }
  
  if(*s.cc != 0) {
    SEND_STRING(&s.psock, (char *)smtp_rcpt_to);
    SEND_STRING(&s.psock, s.cc);
    SEND_STRING(&s.psock, (char *)smtp_crnl);

    PSOCK_READTO(&s.psock, ISO_nl);
  
    if(s.inputbuffer[0] != ISO_2) {
      PSOCK_CLOSE(&s.psock);
      smtp_done(6);
      PSOCK_EXIT(&s.psock);
    }
  }
  
  SEND_STRING(&s.psock, (char *)smtp_data);
  
  PSOCK_READTO(&s.psock, ISO_nl);
  
  if(s.inputbuffer[0] != ISO_3) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(7);
    PSOCK_EXIT(&s.psock);
  }

  SEND_STRING(&s.psock, (char *)smtp_to);
  SEND_STRING(&s.psock, s.to);
  SEND_STRING(&s.psock, (char *)smtp_crnl);
  
  if(*s.cc != 0) {
    SEND_STRING(&s.psock, (char *)smtp_cc);
    SEND_STRING(&s.psock, s.cc);
    SEND_STRING(&s.psock, (char *)smtp_crnl);
  }
  
  SEND_STRING(&s.psock, (char *)smtp_from);
  SEND_STRING(&s.psock, s.from);
  SEND_STRING(&s.psock, (char *)smtp_crnl);
  
  SEND_STRING(&s.psock, (char *)smtp_subject);
  SEND_STRING(&s.psock, s.subject);
  SEND_STRING(&s.psock, (char *)smtp_crnl);

  for(s.line = 0; s.line < s.msgheight; ++s.line) {
    SEND_STRING(&s.psock, (char *)smtp_crnl);
    SEND_STRING(&s.psock, &s.msg[s.line * s.msgwidth]);
  }
  
  SEND_STRING(&s.psock, (char *)smtp_crnlperiodcrnl);

  PSOCK_READTO(&s.psock, ISO_nl);
  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(8);
    PSOCK_EXIT(&s.psock);
  }

  SEND_STRING(&s.psock, (char *)smtp_quit);
  smtp_done(SMTP_ERR_OK);
  PSOCK_END(&s.psock);
}
/*---------------------------------------------------------------------------*/
void
smtp_appcall(void *state)
{
  if(uip_closed()) {
    s.connected = 0;
    return;
  }
  if(uip_aborted() || uip_timedout()) {
    s.connected = 0;
    smtp_done(1);
    return;
  }
  smtp_thread();
}
/*---------------------------------------------------------------------------*/
void
smtp_configure(char *lhostname, uip_ipaddr_t *server)
{
  localhostname = lhostname;
  uip_ipaddr_copy(&smtpserver, server);
}
/*---------------------------------------------------------------------------*/
unsigned char
smtp_send(char *to, char *cc, char *from, char *subject,
	  char *msg, uint8_t msgwidth, uint8_t msgheight)
{
  struct uip_conn *conn;

  conn = tcp_connect(&smtpserver, UIP_HTONS(25), NULL);
  if(conn == NULL) {
    return 0;
  }
  s.connected = 1;
  s.to = to;
  s.cc = cc;
  s.from = from;
  s.subject = subject;
  s.msg = msg;
  s.msgwidth = msgwidth;
  s.msgheight = msgheight;

  PSOCK_INIT(&s.psock, (uint8_t *)s.inputbuffer, sizeof(s.inputbuffer));
  
  return 1;
}
/*---------------------------------------------------------------------------*/
void
smtp_init(void)
{
  s.connected = 0;
}
/*---------------------------------------------------------------------------*/

