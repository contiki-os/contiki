/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeibc Best-effort local area broadcast
 * @{
 *
 * The broadcast module sends packets to all local area neighbors with an a
 * header that identifies the sender.
 *
 * The broadcast module sends a packet to all local neighbors.  The
 * module adds the single-hop sender address as a packet attribute to
 * outgoing packets.  All Rime primitives that need the identity of
 * the sender in the outgoing packets use the broadcast primitive,
 * either directly or indirectly through any of the other
 * communication primitives that are based on the broadcast primitive.
 *
 * \section channels Channels
 *
 * The broadcast module uses 1 channel.
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
 * $Id: broadcast.h,v 1.5 2010/02/23 18:38:05 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for identified best-effort local area broadcast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __BROADCAST_H__
#define __BROADCAST_H__

#include "net/rime/abc.h"
#include "net/rime/rimeaddr.h"

struct broadcast_conn;

#define BROADCAST_ATTRIBUTES  { PACKETBUF_ADDR_SENDER, PACKETBUF_ADDRSIZE }, \
                        ABC_ATTRIBUTES

/**
 * \brief     Callback structure for broadcast
 *
 */
struct broadcast_callbacks {
  /** Called when a packet has been received by the broadcast module. */
  void (* recv)(struct broadcast_conn *ptr, const rimeaddr_t *sender);
  void (* sent)(struct broadcast_conn *ptr, int status, int num_tx);
};

struct broadcast_conn {
  struct abc_conn c;
  const struct broadcast_callbacks *u;
};

/**
 * \brief      Set up an identified best-effort broadcast connection
 * \param c    A pointer to a struct broadcast_conn
 * \param channel The channel on which the connection will operate
 * \param u    A struct broadcast_callbacks with function pointers to functions that will be called when a packet has been received
 *
 *             This function sets up a broadcast connection on the
 *             specified channel. The caller must have allocated the
 *             memory for the struct broadcast_conn, usually by declaring it
 *             as a static variable.
 *
 *             The struct broadcast_callbacks pointer must point to a structure
 *             containing a pointer to a function that will be called
 *             when a packet arrives on the channel.
 *
 */
void broadcast_open(struct broadcast_conn *c, uint16_t channel,
	       const struct broadcast_callbacks *u);

/**
 * \brief      Close a broadcast connection
 * \param c    A pointer to a struct broadcast_conn
 *
 *             This function closes a broadcast connection that has
 *             previously been opened with broadcast_open().
 *
 *             This function typically is called as an exit handler.
 *
 */
void broadcast_close(struct broadcast_conn *c);

/**
 * \brief      Send an identified best-effort broadcast packet
 * \param c    The broadcast connection on which the packet should be sent
 * \retval     Non-zero if the packet could be sent, zero otherwise
 *
 *             This function sends an identified best-effort broadcast
 *             packet. The packet must be present in the packetbuf
 *             before this function is called.
 *
 *             The parameter c must point to a broadcast connection that
 *             must have previously been set up with broadcast_open().
 *
 */
int broadcast_send(struct broadcast_conn *c);

#endif /* __BROADCAST_H__ */
/** @} */
/** @} */
