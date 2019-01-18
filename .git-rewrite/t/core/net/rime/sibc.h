/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimesibc Stubborn identified broadcast
 * @{
 *
 * The sibc module provides stubborn identified best-effort local area
 * broadcast. A message sent with the sibc module is repeated until
 * either the mssage is canceled or a new message is sent. Messages
 * sent with the sibc module are identified with a sender ID.
 *
 * \section channels Channels
 *
 * The sibc module uses 1 channel.
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
 * $Id: sibc.h,v 1.6 2007/03/31 18:31:29 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Rime module Stubborn Identified BroadCast (sibc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SIBC_H__
#define __SIBC_H__

#include "net/rime/uc.h"
#include "net/rime/ctimer.h"
#include "net/rime/queuebuf.h"

struct sibc_conn;

struct sibc_callbacks {
  void (* recv)(struct sibc_conn *c, rimeaddr_t *from);
  void (* sent)(struct sibc_conn *c);
};

struct sibc_conn {
  struct ibc_conn c;
  struct ctimer t;
  struct queuebuf *buf;
  const struct sibc_callbacks *u;
};

void sibc_open(struct sibc_conn *c, u16_t channel,
		const struct sibc_callbacks *u);
void sibc_close(struct sibc_conn *c);

int sibc_send_stubborn(struct sibc_conn *c, clock_time_t t);
void sibc_cancel(struct sibc_conn *c);

void sibc_set_timer(struct sibc_conn *c, clock_time_t t);

#endif /* __SIBC_H__ */
/** @} */
/** @} */
