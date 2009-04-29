/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimecollect Tree-based hop-by-hop reliable data collection
 * @{
 *
 * The collect module implements a hop-by-hop reliable data collection
 * mechanism.
 *
 * \section channels Channels
 *
 * The collect module uses 2 channels; one for neighbor discovery and one
 * for data packets.
 *
 */

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
 * $Id: collect.h,v 1.11 2009/04/29 20:48:57 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for hop-by-hop reliable data collection
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __COLLECT_H__
#define __COLLECT_H__

#include "net/rime/announcement.h"
#include "net/rime/runicast.h"

#define COLLECT_ATTRIBUTES  { PACKETBUF_ADDR_ESENDER,    PACKETBUF_ADDRSIZE }, \
                            { PACKETBUF_ATTR_EPACKET_ID, PACKETBUF_ATTR_BIT * 4 }, \
                            { PACKETBUF_ATTR_TTL,        PACKETBUF_ATTR_BIT * 4 }, \
                            { PACKETBUF_ATTR_HOPS,       PACKETBUF_ATTR_BIT * 4 }, \
                            { PACKETBUF_ATTR_MAX_REXMIT, PACKETBUF_ATTR_BIT * 3 }, \
                            RUNICAST_ATTRIBUTES

struct collect_callbacks {
  void (* recv)(const rimeaddr_t *originator, uint8_t seqno,
		uint8_t hops);
};

struct collect_conn {
  struct runicast_conn runicast_conn;
  struct announcement announcement;
  const struct collect_callbacks *cb;
  struct ctimer t;
  uint16_t rtmetric;
  uint8_t forwarding;
  uint8_t seqno;
};

void collect_open(struct collect_conn *c, uint16_t channels,
	       const struct collect_callbacks *callbacks);
void collect_close(struct collect_conn *c);

int collect_send(struct collect_conn *c, int rexmits);

void collect_set_sink(struct collect_conn *c, int should_be_sink);

int collect_depth(struct collect_conn *c);

#define COLLECT_MAX_DEPTH 255

#endif /* __COLLECT_H__ */
/** @} */
/** @} */
