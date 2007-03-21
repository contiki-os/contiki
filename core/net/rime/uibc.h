/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup uibc Unique Identified best effort local area BroadCast
 * @{
 *
 * Send one packet that is unique within a time interval.
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
 * $Id: uibc.h,v 1.1 2007/03/21 23:21:01 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for Unique Identified best effort local area BroadCast (uibc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __UIBC_H__
#define __UIBC_H__

#include "net/rime.h"

struct uibc_conn;

struct uibc_callbacks {
  void (* recv)(struct uibc_conn *c, rimeaddr_t *from);
  void (* sent)(struct uibc_conn *c);
  void (* dropped)(struct uibc_conn *c);
};

struct uibc_conn {
  struct ibc_conn c;
  const struct uibc_callbacks *cb;
  struct ctimer t;
  struct queuebuf *q;
};

void uibc_open(struct uibc_conn *c, u16_t channel,
	       const struct uibc_callbacks *cb);
void uibc_close(struct uibc_conn *c);
int  uibc_send(struct uibc_conn *c, clock_time_t interval);

#endif /* __UIBC_H__ */

/** @} */
/** @} */
