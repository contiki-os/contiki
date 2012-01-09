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
 *         A simple webserver
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __HTTPD_SIMPLE_H__
#define __HTTPD_SIMPLE_H__

#include "contiki-net.h"

/* The current internal border router webserver ignores the requested file name */
/* and needs no per-connection output buffer, so save some RAM */
#ifndef WEBSERVER_CONF_CFS_PATHLEN
#define HTTPD_PATHLEN 2
#else /* WEBSERVER_CONF_CFS_CONNS */
#define HTTPD_PATHLEN WEBSERVER_CONF_CFS_PATHLEN
#endif /* WEBSERVER_CONF_CFS_CONNS */

struct httpd_state;
typedef char (* httpd_simple_script_t)(struct httpd_state *s);

struct httpd_state {
  struct timer timer;
  struct psock sin, sout;
  struct pt outputpt;
  char inputbuf[HTTPD_PATHLEN + 24];
/*char outputbuf[UIP_TCP_MSS]; */
  char filename[HTTPD_PATHLEN];
  httpd_simple_script_t script;
  char state;
};

void httpd_init(void);
void httpd_appcall(void *state);

httpd_simple_script_t httpd_simple_get_script(const char *name);

#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, strlen(str))

#endif /* __HTTPD_SIMPLE_H__ */
