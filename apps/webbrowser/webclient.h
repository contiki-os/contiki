/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
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
 * This file is part of the "contiki" web browser.
 *
 * $Id: webclient.h,v 1.2 2010/06/14 14:08:17 nifi Exp $
 *
 */
#ifndef __WEBCLIENT_H__
#define __WEBCLIENT_H__

#include "contiki-net.h"
#include "http-strings.h"
#include "http-user-agent-string.h"


/* Callback functions that have to be implemented by the application
   program. */
struct webclient_state;
void webclient_datahandler(char *data, uint16_t len);
void webclient_connected(void);
void webclient_timedout(void);
void webclient_aborted(void);
void webclient_closed(void);


/* Functions. */
void webclient_init(void);
unsigned char webclient_get(const char *host, uint16_t port, const char *file);
void webclient_close(void);

void webclient_appcall(void *state);
/*DISPATCHER_UIPCALL(webclient_appcall, state);*/

char *webclient_mimetype(void);
char *webclient_filename(void);
char *webclient_hostname(void);
unsigned short webclient_port(void);

#endif /* __WEBCLIENT_H__ */
