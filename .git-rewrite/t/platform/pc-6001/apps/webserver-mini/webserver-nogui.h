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
 * This file is part of the Contiki OS
 *
 * $Id: webserver-nogui.h,v 1.1 2007/11/28 09:40:13 matsutsuka Exp $
 *
 */
#ifndef __WEBSERVER_NOGUI_H__
#define __WEBSERVER_NOGUI_H__

#include "contiki-net.h"

PROCESS_NAME(webserver_nogui_process);

#ifdef WEBSERVER_CONF_LOG_ENABLED
#define webserver_log(msg)	webserver_log(msg) 
#define webserver_log_file(requester, file)	\
	webserver_log_file(requester, file)
void weblog_message(char *msg);
#else /* WEBSERVER_CONF_LOG_ENABLED */
void weblog_message(char *msg);
#define webserver_log(msg)
#define webserver_log_file(requester, file)
#endif /* WEBSERVER_CONF_LOG_ENABLED */

#endif /* __WEBSERVER_H__ */
