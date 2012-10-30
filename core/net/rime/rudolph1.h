/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rudolph1 Multi-hop reliable bulk data transfer
 * @{
 *
 * The rudolph1 module implements a multi-hop reliable bulk data
 * transfer mechanism.
 *
 * \section channels Channels
 *
 * The rudolph1 module uses 2 channels; one for data transmissions and
 * one for NACKs and repair packets.
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
 *         Header file for the multi-hop reliable bulk data transfer mechanism
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RUDOLPH1_H__
#define __RUDOLPH1_H__

#include "net/rime/trickle.h"
#include "net/rime/ipolite.h"
#include "sys/ctimer.h"

struct rudolph1_conn;

enum {
  RUDOLPH1_FLAG_NONE,
  RUDOLPH1_FLAG_NEWFILE,
  RUDOLPH1_FLAG_LASTCHUNK,
};

struct rudolph1_callbacks {
  void (* write_chunk)(struct rudolph1_conn *c, int offset, int flag,
		       uint8_t *data, int len);
  int (* read_chunk)(struct rudolph1_conn *c, int offset, uint8_t *to,
		     int maxsize);
};

struct rudolph1_conn {
  struct trickle_conn trickle;
  struct ipolite_conn ipolite;
  const struct rudolph1_callbacks *cb;
  struct ctimer t;
  clock_time_t send_interval;
  uint16_t chunk, highest_chunk_heard;
  uint8_t version;
  /*  uint8_t trickle_interval;*/
  uint8_t nacks;
};

void rudolph1_open(struct rudolph1_conn *c, uint16_t channel,
		   const struct rudolph1_callbacks *cb);
void rudolph1_close(struct rudolph1_conn *c);
void rudolph1_send(struct rudolph1_conn *c, clock_time_t send_interval);
void rudolph1_stop(struct rudolph1_conn *c);

#endif /* __RUDOLPH1_H__ */
/** @} */
/** @} */
