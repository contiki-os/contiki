/**
 * \addtogroup rimeabc
 * @{
 */


/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: abc.c,v 1.16 2008/01/23 16:10:10 adamdunkels Exp $
 */

/**
 * \file
 *         Anonymous best-effort local area Broad Cast (abc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki-net.h"
#include "net/rime.h"

struct abc_hdr {
  u16_t channel;
};

LIST(channels);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
abc_open(struct abc_conn *c, u16_t channel,
	  const struct abc_callbacks *callbacks)
{
  c->channel = channel;
  c->u = callbacks;

  list_add(channels, c);
}
/*---------------------------------------------------------------------------*/
void
abc_close(struct abc_conn *c)
{
  list_remove(channels, c);
}
/*---------------------------------------------------------------------------*/
int
abc_send(struct abc_conn *c)
{
  if(rimebuf_hdralloc(sizeof(struct abc_hdr))) {
    struct abc_hdr *hdr = rimebuf_hdrptr();

    PRINTF("%d.%d: abc: abc_send on channel %d\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	   c->channel);
    
    hdr->channel = c->channel;
    rime_output();
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
abc_input_packet(void)
{
  struct abc_hdr *hdr;
  struct abc_conn *c;

  hdr = rimebuf_dataptr();

  if(rimebuf_hdrreduce(sizeof(struct abc_hdr))) {
    PRINTF("%d.%d: abc: abc_input_packet on channel %d\n",
	   rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	   hdr->channel);

    for(c = list_head(channels); c != NULL; c = c->next) {
      if(c->channel == hdr->channel) {
	c->u->recv(c);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/

/** @} */
