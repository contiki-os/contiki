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
 * $Id: chameleon.h,v 1.2 2009/03/12 21:58:20 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for Chameleon, Rime's header processing module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __CHAMELEON_H__
#define __CHAMELEON_H__


#include "net/rime/channel.h"
#include "net/rime/chameleon-bitopt.h"
#include "net/rime/chameleon-raw.h"

struct chameleon_module {
  struct channel *(* input)(void);
  int (* output)(struct channel *);
  int (* hdrsize)(const struct packetbuf_attrlist *);
  void (* init)(void);
};

void chameleon_init(const struct chameleon_module *header_processing_module);

int chameleon_hdrsize(const struct packetbuf_attrlist attrlist[]);
void chameleon_input(void);
int chameleon_output(struct channel *c);

#endif /* __CHAMELEON_H__ */
