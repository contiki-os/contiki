/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 * @(#)$$
 */

/**
 * \file
 *         Rime initialization for the MICAz port.
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#include "contiki.h"
#include "node-id.h"
#include "dev/cc2420.h"
#include "net/mac/nullmac.h"
#include "net/mac/lpp.h"
#include "net/rime.h"

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif

void
init_net(void)
{
  rimeaddr_t rimeaddr;
  cc2420_init();
  cc2420_set_channel(RF_CHANNEL);
#if WITH_NULLMAC
  rime_init(nullmac_init(&cc2420_driver));
#else
  rime_init(lpp_init(&cc2420_driver));
#endif
  rimeaddr.u8[0] = node_id & 0xff;
  rimeaddr.u8[1] = node_id >> 8;
  rimeaddr_set_node_addr(&rimeaddr);
}
