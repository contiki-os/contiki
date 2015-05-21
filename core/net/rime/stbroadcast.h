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
 *         Header file for the Rime module Stubborn Anonymous BroadCast (stbroadcast)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rime
 * @{
*/

/**
 * \defgroup rimestbroadcast Stubborn best-effort local area broadcast
 * @{
 *
 * The stbroadcast module provides stubborn anonymous best-effort local area
 * broadcast. A message sent with the stbroadcast module is repeated until
 * either the message is canceled or a new message is sent. Messages
 * sent with the stbroadcast module are not identified with a sender ID.
 *
 * \section stbroadcast-channels Channels
 *
 * The stbroadcast module uses 1 channel.
 *
 */

#ifndef STBROADCAST_H_
#define STBROADCAST_H_

#include "sys/ctimer.h"

#include "net/rime/broadcast.h"
#include "net/queuebuf.h"

struct stbroadcast_conn;

struct stbroadcast_callbacks {
  void (* recv)(struct stbroadcast_conn *c);
  void (* sent)(struct stbroadcast_conn *c);
};

/**
 * A stbroadcast connection. This is an opaque structure with no user-visible
 * fields. The stbroadcast_open() function is used for setting up a stbroadcast
 * connection.
 */
struct stbroadcast_conn {
  struct broadcast_conn c;
  struct ctimer t;
  struct queuebuf *buf;
  const struct stbroadcast_callbacks *u;
};


/**
 * \brief      Set up a stbroadcast connection.
 * \param c    A pointer to a user-supplied struct stbroadcast variable.
 * \param channel The Rime channel on which messages should be sent.
 * \param u    Pointer to the upper layer functions that should be used
 *             for this connection.
 *
 *             This function sets up a stbroadcast connection on the
 *             specified channel. No checks are made if the channel is
 *             currently used by another connection.
 *
 *             This function must be called before any other function
 *             that operates on the connection is called.
 *
 */
void stbroadcast_open(struct stbroadcast_conn *c, uint16_t channel,
		const struct stbroadcast_callbacks *u);
void stbroadcast_close(struct stbroadcast_conn *c);

/**
 * \brief      Send a stubborn message.
 * \param c    A stbroadcast connection that must have been previously set up
 *             with stbroadcast_open()
 * \param t    The time between message retransmissions.
 *
 *             This function sends a message from the Rime buffer. The
 *             message must have been previously constructed in the
 *             Rime buffer. When this function returns, the message
 *             has been copied into a queue buffer.
 *
 *             If another message has previously been sent, the old
 *             message is canceled.
 *
 */
int stbroadcast_send_stubborn(struct stbroadcast_conn *c, clock_time_t t);

/**
 * \brief      Cancel the current stubborn message.
 * \param c    A stbroadcast connection that must have been previously set up
 *             with stbroadcast_open()
 *
 *             This function cancels a stubborn message that has
 *             previously been sent with the stbroadcast_send_stubborn()
 *             function.
 *
 */
void stbroadcast_cancel(struct stbroadcast_conn *c);



/**
 * \brief      Set the retransmission time of the current stubborn message.
 * \param c    A stbroadcast connection that must have been previously set up
 *             with stbroadcast_open()
 * \param t    The new time between message retransmissions.
 *
 *             This function sets the retransmission timer for the
 *             current stubborn message to a new value.
 *
 */
void stbroadcast_set_timer(struct stbroadcast_conn *c, clock_time_t t);

#endif /* STBROADCAST_H_ */

/** @} */
/** @} */
