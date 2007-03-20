/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: rudolph0.h,v 1.1 2007/03/20 12:25:27 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RUDOLPH0_H__
#define __RUDOLPH0_H__

#include "net/rime.h"
#include "net/rime/sabc.h"
#include "net/rime/uabc.h"
#include "contiki-net.h"

struct rudolph0_conn;

struct rudolph0_callbacks {
  int (* new_file)(struct rudolph0_conn *c);
  void (* received_file)(struct rudolph0_conn *c, int cfs_fd);
};

#define RUDOLPH0_DATASIZE 32

struct rudolph0_hdr {
  u8_t type;
  u8_t version;
  u16_t chunk;
};

struct rudolph0_datapacket {
  struct rudolph0_hdr h;
  u8_t datalen;
  u8_t data[RUDOLPH0_DATASIZE];
};

struct rudolph0_conn {
  struct sabc_conn c;
  struct uabc_conn nackc;
  const struct rudolph0_callbacks *cb;
  int cfs_fd;
  u8_t state;
  struct rudolph0_datapacket current;
};

void rudolph0_open(struct rudolph0_conn *c, u16_t channel,
		   const struct rudolph0_callbacks *cb);
void rudolph0_close(struct rudolph0_conn *c);
void rudolph0_send(struct rudolph0_conn *c, int cfs_fd);

#endif /* __RUDOLPH0_H__ */
