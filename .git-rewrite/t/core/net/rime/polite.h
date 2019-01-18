/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimepolite Polite anonymous best effort local broadcast
 * @{
 *
 * The polite module sends one anonymous packet that is unique within a
 * time interval.
 *
 * \section channels Channels
 *
 * The polite module uses 1 channel.
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
 * $Id: polite.h,v 1.2 2007/12/17 09:14:08 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for Polite Anonymous best effort local Broadcast (polite)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __POLITE_H__
#define __POLITE_H__

#include "net/rime/abc.h"
#include "net/rime/queuebuf.h"
#include "net/rime/ctimer.h"

struct polite_conn;

struct polite_callbacks {
  void (* recv)(struct polite_conn *c);
  void (* sent)(struct polite_conn *c);
  void (* dropped)(struct polite_conn *c);
};

struct polite_conn {
  struct abc_conn c;
  const struct polite_callbacks *cb;
  struct ctimer t;
  struct queuebuf *q;
  u8_t hdrsize;
};

void polite_open(struct polite_conn *c, u16_t channel,
	       const struct polite_callbacks *cb);
void polite_close(struct polite_conn *c);
int  polite_send(struct polite_conn *c, clock_time_t interval, u8_t hdrsize);
void polite_cancel(struct polite_conn *c);

#endif /* __POLITE_H__ */

/** @} */
/** @} */
