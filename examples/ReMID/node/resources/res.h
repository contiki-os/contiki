/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 */


#ifndef __RES_H__
#define __RES_H__

#include <stdio.h>
#include "contiki.h"
#include "net/linkaddr.h"
#include "net/ip/uip.h"
#include "lib/list.h"
#include "jsonparse.h"


#define MAX_DATA_LEN  REST_MAX_CHUNK_SIZE

int res_string_to_linkaddr(char* address, unsigned int size, linkaddr_t* lladdr);
uint8_t res_reply_char_if_possible(char c, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset);
uint8_t res_reply_string_if_possible(char *s, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset);
uint8_t res_reply_02hex_if_possible(unsigned int hex, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset);
uint8_t res_reply_uint16_if_possible(uint16_t d, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset);
void res_reply_lladdr_if_possible(const linkaddr_t *lladdr, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset);
uint8_t res_reply_ip_if_possible(const uip_ipaddr_t *addr, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset);
int res_json_find_field(struct jsonparse_state *js, char *field_buf, int field_buf_len);

#endif
