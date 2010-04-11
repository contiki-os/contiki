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
 * $Id: httpd-cfs.h,v 1.3 2010/04/11 20:10:12 oliverschmidt Exp $
 *
 */

#ifndef __HTTPD_CFS_H__
#define __HTTPD_CFS_H__

#include "contiki-net.h"

#ifndef WEBSERVER_CONF_CFS_PATHLEN
#define HTTPD_PATHLEN 80
#else /* WEBSERVER_CONF_CFS_CONNS */
#define HTTPD_PATHLEN WEBSERVER_CONF_CFS_PATHLEN
#endif /* WEBSERVER_CONF_CFS_CONNS */

struct httpd_state {
  struct timer timer;
  struct psock sin, sout;
  struct pt outputpt;
  char inputbuf[HTTPD_PATHLEN + 30];
  char outputbuf[UIP_TCP_MSS];
  char filename[HTTPD_PATHLEN];
  char state;
  int fd;
  int len;
};


void httpd_init(void);
void httpd_appcall(void *state);

#endif /* __HTTPD_CFS_H__ */
