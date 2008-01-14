/**
 * \addtogroup rime
 * @{
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
 * $Id: rime.c,v 1.15 2008/01/14 09:42:00 adamdunkels Exp $
 */

/**
 * \file
 *         Rime initialization and common code
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "net/rime/neighbor.h"
#include "net/rime/route.h"
#include "net/mac/mac.h"

#include "lib/list.h"

const struct mac_driver *rime_mac;

LIST(sniffers);

/*---------------------------------------------------------------------------*/
void
rime_sniffer_add(struct rime_sniffer *s)
{
  list_add(sniffers, s);
}
/*---------------------------------------------------------------------------*/
void
rime_sniffer_remove(struct rime_sniffer *s)
{
  list_remove(sniffers, s);
}
/*---------------------------------------------------------------------------*/
static void
input(const struct mac_driver *r)
{
  int len;
  struct rime_sniffer *s;
  len = rime_mac->read();
  if(len > 0) {
    for(s = list_head(sniffers); s != NULL; s = s->next) {
      if(s->input_callback != NULL) {
	s->input_callback();
      }
    }
    RIMESTATS_ADD(rx);
    abc_input_packet();
  }
}
/*---------------------------------------------------------------------------*/
void
rime_init(const struct mac_driver *m)
{
  queuebuf_init();
  route_init();
  rimebuf_clear();
  neighbor_init();
  rime_mac = m;
  rime_mac->set_receive_function(input);
}
/*---------------------------------------------------------------------------*/
void
rime_output(void)
{
  struct rime_sniffer *s;
    
  RIMESTATS_ADD(tx);
  rimebuf_compact();

  for(s = list_head(sniffers); s != NULL; s = s->next) {
    if(s->output_callback != NULL) {
      s->output_callback();
    }
  }
  
  if(rime_mac) {
    rime_mac->send();
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
