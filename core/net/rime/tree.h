/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimetree Tree-based hop-by-hop reliable data collection
 * @{
 *
 * The tree module implements a hop-by-hop reliable data collection
 * mechanism.
 *
 * \section channels Channels
 *
 * The tree module uses 2 channels; one for neighbor discovery and one
 * for data packets.
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
 * $Id: tree.h,v 1.9 2007/11/13 20:39:29 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for hop-by-hop reliable data collection
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __TREE_H__
#define __TREE_H__

#include "net/rime/ipolite.h"
#include "net/rime/ruc.h"
#include "net/rime/nbh.h"

struct tree_callbacks {
  void (* recv)(rimeaddr_t *originator, u8_t seqno,
		u8_t hops);
};

struct tree_conn {
  struct nbh_conn nbh_conn;
  struct ruc_conn ruc_conn;
  const struct tree_callbacks *cb;
  struct ctimer t;
  u8_t forwarding;
  u8_t local_rtmetric;
  u8_t seqno;
};

void tree_open(struct tree_conn *c, u16_t channels,
	       const struct tree_callbacks *callbacks);
void tree_close(struct tree_conn *c);

void tree_send(struct tree_conn *c, int rexmits);

void tree_set_sink(struct tree_conn *c, int should_be_sink);

int tree_depth(struct tree_conn *c);

#define TREE_MAX_DEPTH 255

#endif /* __TREE_H__ */
/** @} */
/** @} */
