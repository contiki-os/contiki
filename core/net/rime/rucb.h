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
 */

/**
 * \file
 *         Header file for the reliable unicast bulk transfer module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef RUCB_H_
#define RUCB_H_

#include "net/rime/runicast.h"

struct rucb_conn;

enum {
  RUCB_FLAG_NONE,
  RUCB_FLAG_NEWFILE,
  RUCB_FLAG_LASTCHUNK,
};

struct rucb_callbacks {
  void (* write_chunk)(struct rucb_conn *c, int offset, int flag,
		       char *data, int len);
  int (* read_chunk)(struct rucb_conn *c, int offset, char *to,
		     int maxsize);
  void (* timedout)(struct rucb_conn *c);
};

#define RUCB_DATASIZE 64

struct rucb_conn {
  struct runicast_conn c;
  const struct rucb_callbacks *u;
  linkaddr_t receiver, sender;
  uint16_t chunk;
  uint8_t last_seqno;
  int last_size;
};

void rucb_open(struct rucb_conn *c, uint16_t channel,
	      const struct rucb_callbacks *u);
void rucb_close(struct rucb_conn *c);

int rucb_send(struct rucb_conn *c, const linkaddr_t *receiver);


#endif /* RUCB_H_ */
