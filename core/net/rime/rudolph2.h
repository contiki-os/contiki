/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rudolph2 Single-hop reliable bulk data transfer
 * @{
 *
 * The rudolph2 module implements a single-hop reliable bulk data
 * transfer mechanism.
 *
 * \section channels Channels
 *
 * The rudolph2 module uses 2 channels; one for data packets and one
 * for NACK and repair packets.
 *
 */

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
 *         Header file for the single-hop reliable bulk data transfer module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef RUDOLPH2_H_
#define RUDOLPH2_H_

#include "net/rime/polite.h"
#include "sys/ctimer.h"

struct rudolph2_conn;

enum {
  RUDOLPH2_FLAG_NONE,
  RUDOLPH2_FLAG_NEWFILE,
  RUDOLPH2_FLAG_LASTCHUNK,
};

struct rudolph2_callbacks {
  void (* write_chunk)(struct rudolph2_conn *c, int offset, int flag,
		       uint8_t *data, int len);
  int (* read_chunk)(struct rudolph2_conn *c, int offset, uint8_t *to,
		     int maxsize);
};

#define RUDOLPH2_DATASIZE 64

struct rudolph2_conn {
  struct polite_conn c;
  const struct rudolph2_callbacks *cb;
  struct ctimer t;
  uint16_t snd_nxt, rcv_nxt;
  uint16_t version;
  uint8_t hops_from_base;
  uint8_t nacks;
  uint8_t flags;
};

void rudolph2_open(struct rudolph2_conn *c, uint16_t channel,
		   const struct rudolph2_callbacks *cb);
void rudolph2_close(struct rudolph2_conn *c);
void rudolph2_send(struct rudolph2_conn *c, clock_time_t interval);
void rudolph2_stop(struct rudolph2_conn *c);

void rudolph2_set_version(struct rudolph2_conn *c, int version);
int rudolph2_version(struct rudolph2_conn *c);

#endif /* RUDOLPH2_H_ */
/** @} */
/** @} */
    
