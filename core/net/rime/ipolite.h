/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeipolite Ipolite anonymous best effort local broadcast
 * @{
 *
 * The ipolite module sends one anonymous packet that is unique within a
 * time interval.
 *
 * \section channels Channels
 *
 * The ipolite module uses 1 channel.
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
 * $Id: ipolite.h,v 1.2 2007/12/17 09:14:08 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for Ipolite Anonymous best effort local Broadcast (ipolite)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __IPOLITE_H__
#define __IPOLITE_H__

#include "net/rime/ibc.h"
#include "net/rime/ctimer.h"
#include "net/rime/queuebuf.h"

struct ipolite_conn;

struct ipolite_callbacks {
  void (* recv)(struct ipolite_conn *c, rimeaddr_t *from);
  void (* sent)(struct ipolite_conn *c);
  void (* dropped)(struct ipolite_conn *c);
};

struct ipolite_conn {
  struct ibc_conn c;
  const struct ipolite_callbacks *cb;
  struct ctimer t;
  struct queuebuf *q;
  u8_t hdrsize;
};

void ipolite_open(struct ipolite_conn *c, u16_t channel,
	       const struct ipolite_callbacks *cb);
void ipolite_close(struct ipolite_conn *c);
int  ipolite_send(struct ipolite_conn *c, clock_time_t interval, u8_t hdrsize);
void ipolite_cancel(struct ipolite_conn *c);

#endif /* __IPOLITE_H__ */

/** @} */
/** @} */
