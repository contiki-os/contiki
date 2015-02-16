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
 *         Header file for the Rime module Anonymous BroadCast (abc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeabc Anonymous best-effort local area broadcast
 * @{
 *
 * The abc module sends packets to all local area neighbors. The abc
 * module adds no headers to outgoing packets.
 *
 * \section abc-channels Channels
 *
 * The abc module uses 1 channel.
 *
 */

#ifndef ABC_H_
#define ABC_H_

#include "net/packetbuf.h"
#include "net/rime/channel.h"

struct abc_conn;

#define ABC_ATTRIBUTES

/**
 * \brief     Callback structure for abc
 *
 */
struct abc_callbacks {
  /** Called when a packet has been received by the abc module. */
  void (* recv)(struct abc_conn *ptr);
  void (* sent)(struct abc_conn *ptr, int status, int num_tx);
};

struct abc_conn {
  struct channel channel;
  const struct abc_callbacks *u;
};

/**
 * \brief      Set up an anonymous best-effort broadcast connection
 * \param c    A pointer to a struct abc_conn
 * \param channel The channel on which the connection will operate
 * \param u    A struct abc_callbacks with function pointers to functions that will be called when a packet has been received
 *
 *             This function sets up an abc connection on the
 *             specified channel. The caller must have allocated the
 *             memory for the struct abc_conn, usually by declaring it
 *             as a static variable.
 *
 *             The struct abc_callbacks pointer must point to a structure
 *             containing a pointer to a function that will be called
 *             when a packet arrives on the channel.
 *
 */
void abc_open(struct abc_conn *c, uint16_t channel,
	       const struct abc_callbacks *u);

/**
 * \brief      Close an abc connection
 * \param c    A pointer to a struct abc_conn
 *
 *             This function closes an abc connection that has
 *             previously been opened with abc_open().
 *
 *             This function typically is called as an exit handler.
 *
 */
void abc_close(struct abc_conn *c);

/**
 * \brief      Send an anonymous best-effort broadcast packet
 * \param c    The abc connection on which the packet should be sent
 * \retval     Non-zero if the packet could be sent, zero otherwise
 *
 *             This function sends an anonymous best-effort broadcast
 *             packet. The packet must be present in the packetbuf
 *             before this function is called.
 *
 *             The parameter c must point to an abc connection that
 *             must have previously been set up with abc_open().
 *
 */
int abc_send(struct abc_conn *c);

/**
 * \brief      Internal Rime function: Pass a packet to the abc layer
 *
 *             This function is used internally by Rime to pass
 *             packets to the abc layer. Should never be called
 *             directly.
 *
 */

void abc_input(struct channel *channel);

void abc_sent(struct channel *channel, int status, int num_tx);

#endif /* ABC_H_ */
/** @} */
/** @} */
