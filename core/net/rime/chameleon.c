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
 * $Id: chameleon.c,v 1.7 2009/11/13 09:14:52 fros4943 Exp $
 */

/**
 * \file
 *         Chameleon, Rime's header processing module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/chameleon.h"
#include "net/rime/channel.h"
#include "net/rime.h"
#include "lib/list.h"

#include <stdio.h>

static const struct chameleon_module *header_module;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
chameleon_init(const struct chameleon_module *m)
{
  header_module = m;
  channel_init();
}
/*---------------------------------------------------------------------------*/
#if DEBUG
static void
printbin(int n, int digits)
{
  int i;
  char output[128];

  for(i = 0; i < digits; ++i) {
    output[digits - i - 1] = (n & 1) + '0';
    n >>= 1;
  }
  output[i] = 0;

  printf(output);
}

static void
printhdr(uint8_t *hdr, int len)
{
  int i, j;

  j = 0;
  for(i = 0; i < len; ++i) {
    printbin(hdr[i], 8);
    printf(" (0x%0x), ", hdr[i]);
    ++j;
    if(j == 10) {
      printf("\n");
      j = 0;
    }
  }

  if(j != 0) {
    printf("\n");
  }
}
#endif /* DEBUG */
/*---------------------------------------------------------------------------*/
void
chameleon_input(void)
{
  struct channel *c;
  PRINTF("%d.%d: chameleon_input\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
#if DEBUG
  printhdr(packetbuf_dataptr(), packetbuf_datalen());
#endif /* DEBUG */
  if(header_module) {
    c = header_module->input();
    if(c != NULL) {
      PRINTF("%d.%d: chameleon_input channel %d\n",
	     rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	     c->channelno);
      packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, c->channelno);
      abc_input(c);
    } else {
      PRINTF("%d.%d: chameleon_input channel not found for incoming packet\n",
	     rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    }
  }
}
/*---------------------------------------------------------------------------*/
int
chameleon_output(struct channel *c)
{
  int ret;

  PRINTF("%d.%d: chameleon_output channel %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 c->channelno);

  if(header_module) {
    ret = header_module->output(c);
    packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, c->channelno);
#if DEBUG
    printhdr(packetbuf_hdrptr(), packetbuf_hdrlen());
#endif /* DEBUG */
    if(ret) {
      if (rime_output() == RIME_OK) {
        return 1;
      }
      return 0;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
chameleon_hdrsize(const struct packetbuf_attrlist attrlist[])
{
  if(header_module != NULL &&
     header_module->hdrsize != NULL) {
    return header_module->hdrsize(attrlist);
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
