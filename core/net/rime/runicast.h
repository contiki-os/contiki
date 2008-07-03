/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimerunicast Single-hop reliable unicast
 * @{
 *
 * The runicast module implements a single-hop reliable unicast mechanism.
 *
 * \section channels Channels
 *
 * The runicast module uses 1 channel.
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
 * $Id: runicast.h,v 1.1 2008/07/03 21:52:25 adamdunkels Exp $
 */

/**
 * \file
 *         Reliable unicast header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RUNICAST_H__
#define __RUNICAST_H__

#include "net/rime/stunicast.h"

struct runicast_conn;

#define RUNICAST_ATTRIBUTES  { RIMEBUF_ATTR_PACKET_TYPE, RIMEBUF_ATTR_BIT }, \
                        { RIMEBUF_ATTR_PACKET_ID, RIMEBUF_ATTR_BIT * 2 }, \
                        STUNICAST_ATTRIBUTES
struct runicast_callbacks {
  void (* recv)(struct runicast_conn *c, rimeaddr_t *from, uint8_t seqno);
  void (* sent)(struct runicast_conn *c, rimeaddr_t *to, uint8_t retransmissions);
  void (* timedout)(struct runicast_conn *c, rimeaddr_t *to, uint8_t retransmissions);
};

struct runicast_conn {
  struct stunicast_conn c;
  const struct runicast_callbacks *u;
  uint8_t sndnxt;
  uint8_t rxmit;
  uint8_t max_rxmit;
};

void runicast_open(struct runicast_conn *c, uint16_t channel,
	       const struct runicast_callbacks *u);
void runicast_close(struct runicast_conn *c);

int runicast_send(struct runicast_conn *c, rimeaddr_t *receiver, uint8_t max_retransmissions);

#endif /* __RUNICAST_H__ */
/** @} */
/** @} */
