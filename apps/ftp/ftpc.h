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
#ifndef FTPC_H_
#define FTPC_H_

#include "contiki-net.h"

void ftpc_init(void);

void *ftpc_connect(uip_ipaddr_t *ipaddr, uint16_t port);
char ftpc_list(void *connection);
void ftpc_cwd(void *connection, char *dir);
void ftpc_cdup(void *connection);
char ftpc_get(void *connection, char *filename);
void ftpc_close(void *connection);


void ftpc_appcall(void *state);

#define FTPC_OK        200
#define FTPC_COMPLETED 250
#define FTPC_NODIR     431
#define FTPC_NOTDIR    550

/* Functions to be implemented by the calling module: */
void ftpc_connected(void *connection);
void ftpc_cwd_done(unsigned short status);
char *ftpc_username(void);
char *ftpc_password(void);
void ftpc_closed(void);
void ftpc_aborted(void);
void ftpc_timedout(void);
void ftpc_list_file(char *filename);
void ftpc_data(uint8_t *data, uint16_t len);

#endif /* FTPC_H_ */
