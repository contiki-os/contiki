/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Border router header file
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef BORDER_ROUTER_H_
#define BORDER_ROUTER_H_

#include "contiki.h"
#include "net/ip/uip.h"
#include <stdio.h>

int border_router_cmd_handler(const uint8_t *data, int len);
int slip_config_handle_arguments(int argc, char **argv);
void write_to_slip(const uint8_t *buf, int len);

void border_router_set_prefix_64(const uip_ipaddr_t *prefix_64);
void border_router_set_mac(const uint8_t *data);
void border_router_set_sensors(const char *data, int len);
void border_router_print_stat(void);

void tun_init(void);

int slip_init(void);
int slip_set_fd(int maxfd, fd_set *rset, fd_set *wset);
void slip_handle_fd(fd_set *rset, fd_set *wset);

#endif /* BORDER_ROUTER_H_ */
