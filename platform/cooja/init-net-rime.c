/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 * @(#)$Id: init-net-rime.c,v 1.7 2009/04/01 13:50:12 fros4943 Exp $
 */

#include "contiki.h"
#include <stdio.h>
#include <string.h>

#include "net/rime.h"
#include "net/mac/nullmac.h"

#include "node-id.h"
#include "dev/cooja-radio.h"

void
init_net(void)
{
  int i;
  rimeaddr_t rimeaddr;

  /* Init Rime */
  ctimer_init();
  rimeaddr.u8[0] = node_id & 0xff;
  rimeaddr.u8[1] = node_id >> 8;
  rimeaddr_set_node_addr(&rimeaddr);
  printf("Rime started with address: ");
  for(i = 0; i < sizeof(rimeaddr_node_addr.u8) - 1; i++) {
    printf("%d.", rimeaddr_node_addr.u8[i]);
  }
  printf("%d\n", rimeaddr_node_addr.u8[i]);

  /* Rime <-> nullMAC <-> COOJA's packet radio */
  nullmac_init(&cooja_radio);
  rime_init(&nullmac_driver);
}
