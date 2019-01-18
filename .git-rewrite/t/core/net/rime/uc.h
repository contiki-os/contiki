/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeuc Single-hop unicast
 * @{
 *
 * The uc module sends a packet to a single receiver.
 *
 * \section channels Channels
 *
 * The uc module uses 1 channel.
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
 * $Id: uc.h,v 1.6 2007/03/31 18:31:29 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for Rime's single-hop unicast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __UC_H__
#define __UC_H__

#include "net/rime/ibc.h"

struct uc_conn;

struct uc_callbacks {
  void (* recv)(struct uc_conn *c, rimeaddr_t *from);
};

struct uc_conn {
  struct ibc_conn c;
  const struct uc_callbacks *u;
};

void uc_open(struct uc_conn *c, u16_t channel,
	      const struct uc_callbacks *u);
void uc_close(struct uc_conn *c);

int uc_send(struct uc_conn *c, rimeaddr_t *receiver);

#endif /* __UC_H__ */
/** @} */
/** @} */
