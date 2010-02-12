/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * $Id: httpd-cgi.h,v 1.3 2010/02/12 16:42:59 dak664 Exp $
 *
 */

#ifndef __HTTPD_CGI_H__
#define __HTTPD_CGI_H__

#include "contiki.h"
#include "httpd.h"

typedef PT_THREAD((* httpd_cgifunction)(struct httpd_state *, char *));

httpd_cgifunction httpd_cgi(char *name);

struct httpd_cgi_call {
  struct httpd_cgi_call *next;
  const char *name;
  httpd_cgifunction function;
};

void httpd_cgi_add(struct httpd_cgi_call *c);

#define HTTPD_CGI_CALL(name, str, function) \
static struct httpd_cgi_call name = {NULL, str, function}

void httpd_cgi_init(void);
void web_set_temp(char *s);
void web_set_voltage(char *s);
uint8_t httpd_cgi_sprint_ip6(uip_ip6addr_t addr, char * result);

#endif /* __HTTPD_CGI_H__ */
