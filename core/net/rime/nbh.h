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
 * $Id: nbh.h,v 1.2 2007/11/17 10:32:54 adamdunkels Exp $
 */

/**
 * \file
 *         Neighborhood discovery header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __NBH_H__
#define __NBH_H__

#include "net/rime/ibc.h"

struct nbh_conn;

struct nbh_callbacks {
  void (* recv)(struct nbh_conn *c, rimeaddr_t *from, uint16_t val);
  void (* sent)(struct nbh_conn *c);
};

struct nbh_conn {
  struct ibc_conn c;
  const struct nbh_callbacks *u;
  struct ctimer t;
  uint16_t val;
};

void nbh_open(struct nbh_conn *c, uint16_t channel,
	       const struct nbh_callbacks *u);
void nbh_close(struct nbh_conn *c);

void nbh_start(struct nbh_conn *c, uint16_t val);

#endif /* __NBH_H__ */
