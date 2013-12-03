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
#ifndef TELNET_H_
#define TELNET_H_

#include "contiki-net.h"

struct telnet_state {
  unsigned char flags;
  char *text;
  uint16_t textlen;
  uint16_t sentlen;
};

/*DISPATCHER_UIPCALL(telnet_app, s);*/
void telnet_app(void *s);
unsigned char telnet_send(struct telnet_state *s, char *text, uint16_t len);
unsigned char telnet_close(struct telnet_state *s);
unsigned char telnet_abort(struct telnet_state *s);
struct telnet_state *telnet_connect(struct telnet_state *s, uip_ipaddr_t *addr, uint16_t port);


/* Callbacks, implemented by the caller. */
void telnet_connected(struct telnet_state *s);
void telnet_closed(struct telnet_state *s);
void telnet_sent(struct telnet_state *s);
void telnet_aborted(struct telnet_state *s);
void telnet_timedout(struct telnet_state *s);
void telnet_newdata(struct telnet_state *s, char *data, uint16_t len);
#endif /* TELNET_H_ */
