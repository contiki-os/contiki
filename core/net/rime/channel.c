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
 */

/**
 * \file
 *         Rime's channel abstraction
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/chameleon.h"
#include "net/rime.h"
#include "lib/list.h"

LIST(channel_list);

/*---------------------------------------------------------------------------*/
void
channel_init(void)
{
  list_init(channel_list);
}
/*---------------------------------------------------------------------------*/
void
channel_set_attributes(uint16_t channelno,
		       const struct packetbuf_attrlist attrlist[])
{
  struct channel *c;
  c = channel_lookup(channelno);
  if(c != NULL) {
    c->attrlist = attrlist;
    c->hdrsize = chameleon_hdrsize(attrlist);
  }
}
/*---------------------------------------------------------------------------*/
void
channel_open(struct channel *c, uint16_t channelno)
{
  c->channelno = channelno;
  list_add(channel_list, c);
}
/*---------------------------------------------------------------------------*/
void
channel_close(struct channel *c)
{
  list_remove(channel_list, c);
}
/*---------------------------------------------------------------------------*/
struct channel *
channel_lookup(uint16_t channelno)
{
  struct channel *c;
  for(c = list_head(channel_list); c != NULL; c = list_item_next(c)) {
    if(c->channelno == channelno) {
      return c;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
