/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeipolite Ipolite best effort local broadcast
 * @{
 *
 * The ipolite module sends one local area broadcast packet within one
 * time interval. If a packet with the same header is received from a
 * neighbor within the interval, the packet is not sent.
 *
 * The polite primitive is a generalization of the polite gossip
 * algorithm from Trickle (Levis et al, NSDI 2004).  The polite gossip
 * algorithm is designed to reduce the total amount of packet
 * transmissions by not repeating a message that other nodes have
 * already sent.  The purpose of the polite broadcast primitive is to
 * avoid that multiple copies of a specific set of packet attributes
 * is sent on a specified logical channel in the local neighborhood
 * during a time interval.
 *
 * The polite broadcast primitive is useful for implementing broadcast
 * protocols that use, e.g., negative acknowledgements.  If many nodes
 * need to send the negative acknowledgement to a sender, it is enough
 * if only a single message is delivered to the sender.
 *
 * The upper layer protocol or application that uses the polite
 * broadcast primitive provides an interval time, and message along
 * with a list of packet attributes for which multiple copies should
 * be avoided.  The polite broadcast primitive stores the outgoing
 * message in a queue buffer, stores the list of packet attributes,
 * and sets up a timer.  The timer is set to a random time during the
 * second half of the interval time.
 *
 * During the first half of the time interval, the sender listens for
 * other transmissions.  If it hears a packet that matches the
 * attributes provided by the upper layer protocol or application, the
 * sender drops the packet.  The send timer has been set to a random
 * time some time during the second half of the interval.  When the
 * timer fires, and the sender has not yet heard a transmission of the
 * same packet attributes, the sender broadcasts its packet to all its
 * neighbors.
 *
 * The polite broadcast module does not add any packet attributes to
 * outgoing packets apart from those added by the upper layer.
 *
 * \section channels Channels
 *
 * The ipolite module uses 1 channel.
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
 */

/**
 * \file
 *         Header file for Ipolite best effort local Broadcast (ipolite)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef IPOLITE_H_
#define IPOLITE_H_

#include "sys/ctimer.h"

#include "net/rime/broadcast.h"
#include "net/queuebuf.h"

struct ipolite_conn;

#define IPOLITE_ATTRIBUTES IBC_ATTRIBUTES

/**
 * \brief      A structure with callback functions for an ipolite connection.
 *
 *             This structure holds a list of callback functions used
 *             a an ipolite connection. The functions are called when
 *             events occur on the connection.
 *
 */
struct ipolite_callbacks {
  /**
   * Called when a packet is received on the connection.
   */
  void (* recv)(struct ipolite_conn *c, const linkaddr_t *from);

  /**
   * Called when a packet is sent on the connection.
   */
  void (* sent)(struct ipolite_conn *c);

  /**
   * Called when a packet is dropped because a packet was heard from a
   * neighbor.
   */
  void (* dropped)(struct ipolite_conn *c);
};

/**
 * An opaque structure with no user-visible elements that holds the
 * state of an ipolite connection,
 */
struct ipolite_conn {
  struct broadcast_conn c;
  const struct ipolite_callbacks *cb;
  struct ctimer t;
  struct queuebuf *q;
  uint8_t hdrsize;
  uint8_t maxdups;
  uint8_t dups;
};


/**
 * \brief      Open an ipolite connection
 * \param c    A pointer to a struct ipolite_conn.
 * \param channel The channel number to be used for this connection
 * \param maxdups The number of duplicates that are allowed to be heard before suppressing
 * \param cb   A pointer to the callbacks used for this connection
 *
 *             This function opens an ipolite connection on the
 *             specified channel. The callbacks are called when a
 *             packet is received, or when another event occurs on the
 *             connection (see \ref "struct ipolite_callbacks").
 */
void ipolite_open(struct ipolite_conn *c, uint16_t channel, uint8_t maxdups,
		  const struct ipolite_callbacks *cb);

/**
 * \brief      Close an ipolite connection
 * \param c    A pointer to a struct ipolite_conn that has previously been opened with ipolite_open().
 *
 *             This function closes an ipolite connection that has
 *             previously been opened with ipolite_open().
 */
void ipolite_close(struct ipolite_conn *c);

/**
 * \brief      Send a packet on an ipolite connection.
 * \param c    A pointer to a struct ipolite_conn that has previously been opened with ipolite_open().
 * \param interval The timer interval in which the packet should be sent.
 * \param hdrsize The size of the header that should be unique within the time interval.
 *
 *             This function sends a packet from the packetbuf on the
 *             ipolite connection. The packet is sent some time during
 *             the time interval, but only if no other packet is
 *             received with the same header.
 *
 */
int  ipolite_send(struct ipolite_conn *c, clock_time_t interval,
		  uint8_t hdrsize);

/**
 * \brief      Cancel a pending packet
 * \param c    A pointer to a struct ipolite_conn that has previously been opened with ipolite_open().
 *
 *             This function cancels a pending transmission that has
 *             previously been started with ipolite_send().
 */
void ipolite_cancel(struct ipolite_conn *c);

#endif /* IPOLITE_H_ */

/** @} */
/** @} */
