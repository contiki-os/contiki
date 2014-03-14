/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimemultihop Best-effort multihop forwarding
 * @{
 *
 * The multihop module implements a multihop forwarding mechanism. Routes
 * must have already been setup with the route_add() function. Setting
 * up routes is done with another Rime module such as the \ref
 * routediscovery "route-discovery module".
 *
 * The multihop sends a packet to an identified node in the network by
 * using multi-hop forwarding at each node in the network.  The
 * application or protocol that uses the multihop primitive supplies a
 * routing function for selecting the next-hop neighbor.  If the
 * multihop primitive is requested to send a packet for which no
 * suitable next hop neighbor is found, the caller is immediately
 * notified of this and may choose to initiate a route discovery
 * process.
 *
 *
 * \section channels Channels
 *
 * The multihop module uses 1 channel.
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
 */

/**
 * \file
 *         Multihop forwarding header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef MULTIHOP_H_
#define MULTIHOP_H_

#include "net/rime/unicast.h"
#include "net/linkaddr.h"

struct multihop_conn;

#define MULTIHOP_ATTRIBUTES   { PACKETBUF_ADDR_ESENDER, PACKETBUF_ADDRSIZE }, \
                              { PACKETBUF_ADDR_ERECEIVER, PACKETBUF_ADDRSIZE }, \
                              { PACKETBUF_ATTR_HOPS, PACKETBUF_ATTR_BIT * 5 }, \
                                UNICAST_ATTRIBUTES



struct multihop_callbacks {
  void (* recv)(struct multihop_conn *ptr,
		const linkaddr_t *sender,
		const linkaddr_t *prevhop,
		uint8_t hops);
  linkaddr_t *(* forward)(struct multihop_conn *ptr,
			  const linkaddr_t *originator,
			  const linkaddr_t *dest,
			  const linkaddr_t *prevhop,
			  uint8_t hops);
};

struct multihop_conn {
  struct unicast_conn c;
  const struct multihop_callbacks *cb;
};

void multihop_open(struct multihop_conn *c, uint16_t channel,
	     const struct multihop_callbacks *u);
void multihop_close(struct multihop_conn *c);
int multihop_send(struct multihop_conn *c, const linkaddr_t *to);
void multihop_resend(struct multihop_conn *c, const linkaddr_t *nexthop);

#endif /* MULTIHOP_H_ */
/** @} */
/** @} */
