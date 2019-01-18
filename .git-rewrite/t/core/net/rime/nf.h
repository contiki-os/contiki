/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimenf Best-effort network flooding
 * @{
 *
 * The nf module does best-effort flooding.
 *
 * \section channels Channels
 *
 * The nf module uses 1 channel.
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
 * $Id: nf.h,v 1.10 2007/05/15 08:09:21 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the best-effort network flooding (nf)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __NF_H__
#define __NF_H__


#include "net/rime/ctimer.h"
#include "net/rime/queuebuf.h"
#include "net/rime/ipolite.h"

struct nf_conn;

struct nf_callbacks {
  int (* recv)(struct nf_conn *c, rimeaddr_t *from,
	       rimeaddr_t *originator, u8_t seqno, u8_t hops);
  void (* sent)(struct nf_conn *c);
  void (* dropped)(struct nf_conn *c);
};

struct nf_conn {
  struct ipolite_conn c;
  const struct nf_callbacks *u;
  clock_time_t queue_time;
  rimeaddr_t last_originator;
  u8_t last_originator_seqno;
};

void nf_open(struct nf_conn *c, clock_time_t queue_time,
	     u16_t channel, const struct nf_callbacks *u);
void nf_close(struct nf_conn *c);

int nf_send(struct nf_conn *c, u8_t seqno);

#endif /* __SIBC_H__ */
/** @} */
/** @} */
