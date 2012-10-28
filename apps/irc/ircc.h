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
 */
#ifndef __IRCC_H__
#define __IRCC_H__

#include "contiki-net.h"

struct ircc_state {
  
  struct pt pt;
  struct psock s;

  struct uip_conn *conn;
  
  unsigned char command;
  
  char *msg;
  char channel[32];
  char outputbuf[200];
  char inputbuf[400];
  char *nick;
  char *server;
};

void ircc_init(void);

void ircc_appcall(void *s);

struct ircc_state *ircc_connect(struct ircc_state *s,
				char *server, uip_ipaddr_t *ipaddr, char *nick);

void ircc_join(struct ircc_state *s, char *channel);
void ircc_part(struct ircc_state *s);
void ircc_list(struct ircc_state *s);
void ircc_msg(struct ircc_state *s, char *msg);
void ircc_actionmsg(struct ircc_state *s, char *msg);

void ircc_sent(struct ircc_state *s);

void ircc_text_output(struct ircc_state *s, char *text1, char *text2);

void ircc_connected(struct ircc_state *s);
void ircc_closed(struct ircc_state *s);

void ircc_quit(struct ircc_state *s);

#endif /* __IRCC_H__ */
