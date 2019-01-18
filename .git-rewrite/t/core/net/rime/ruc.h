/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeruc Single-hop reliable unicast
 * @{
 *
 * The ruc module implements a single-hop reliable unicast mechanism.
 *
 * \section channels Channels
 *
 * The ruc module uses 1 channel.
 *
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: ruc.h,v 1.11 2007/12/16 14:33:32 adamdunkels Exp $
 */

/**
 * \file
 *         Reliable unicast header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RUC_H__
#define __RUC_H__

#include "net/rime/suc.h"

struct ruc_conn;

struct ruc_callbacks {
  void (* recv)(struct ruc_conn *c, rimeaddr_t *from, u8_t seqno);
  void (* sent)(struct ruc_conn *c, rimeaddr_t *to, u8_t retransmissions);
  void (* timedout)(struct ruc_conn *c, rimeaddr_t *to, u8_t retransmissions);
};

struct ruc_conn {
  struct suc_conn c;
  const struct ruc_callbacks *u;
  u8_t sndnxt;
  u8_t rxmit;
  u8_t max_rxmit;
};

void ruc_open(struct ruc_conn *c, u16_t channel,
	       const struct ruc_callbacks *u);
void ruc_close(struct ruc_conn *c);

int ruc_send(struct ruc_conn *c, rimeaddr_t *receiver, u8_t max_retransmissions);

#endif /* __RUC_H__ */
/** @} */
/** @} */
