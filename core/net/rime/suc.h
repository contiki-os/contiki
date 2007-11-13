/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimesuc Stubborn unicast
 * @{
 *
 * The suc module takes one packet and sends it repetedly.
 *
 * \section channels Channels
 *
 * The suc module uses 1 channel.
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
 * $Id: suc.h,v 1.9 2007/11/13 21:00:10 adamdunkels Exp $
 */

/**
 * \file
 *         Stubborn unicast header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SUC_H__
#define __SUC_H__

#include "net/rime/uc.h"
#include "net/rime/ctimer.h"
#include "net/rime/queuebuf.h"

struct suc_conn;

struct suc_callbacks {
  void (* recv)(struct suc_conn *c, rimeaddr_t *from);
  void (* sent)(struct suc_conn *c);
};

struct suc_conn {
  struct uc_conn c;
  struct ctimer t;
  struct queuebuf *buf;
  const struct suc_callbacks *u;
  rimeaddr_t receiver;
};

void suc_open(struct suc_conn *c, u16_t channel,
	       const struct suc_callbacks *u);
void suc_close(struct suc_conn *c);

int suc_send_stubborn(struct suc_conn *c, rimeaddr_t *receiver,
		      clock_time_t rxmittime);
void suc_cancel(struct suc_conn *c);

int suc_send(struct suc_conn *c, rimeaddr_t *receiver);

void suc_set_timer(struct suc_conn *c, clock_time_t t);

rimeaddr_t *suc_receiver(struct suc_conn *c);

#endif /* __SUC_H__ */
/** @} */
/** @} */
