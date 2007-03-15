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
 * $Id: ibc.c,v 1.3 2007/03/15 19:43:07 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/ibc.h"
#include "net/rime.h"
#include <string.h>

struct ibc_hdr {
  rimeaddr_t sender;
};

/*---------------------------------------------------------------------------*/
static void
recv_from_abc(struct abc_conn *bc)
{
  rimeaddr_t sender;
  struct ibc_conn *c = (struct ibc_conn *)bc;
  struct ibc_hdr *hdr = rimebuf_dataptr();

  rimeaddr_copy(&sender, &hdr->sender);
  
  rimebuf_hdrreduce(sizeof(struct ibc_hdr));
  DEBUGF(1, "%d: ibc: recv_from_bc\n", node_id);
  c->u->recv(c, &sender);
}
/*---------------------------------------------------------------------------*/
static const struct abc_callbacks ibc = {recv_from_abc};
/*---------------------------------------------------------------------------*/
void
ibc_setup(struct ibc_conn *c, u16_t channel,
	  const struct ibc_callbacks *u)
{
  abc_setup(&c->c, channel, &ibc);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
int
ibc_send(struct ibc_conn *c)
{
  DEBUGF(1, "%d: ibc_send\n", node_id);
  if(rimebuf_hdrextend(sizeof(struct ibc_hdr))) {
    struct ibc_hdr *hdr = rimebuf_hdrptr();
    rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
    return abc_send(&c->c);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
