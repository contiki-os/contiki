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
 * $Id: ipolite.h,v 1.6 2009/02/07 16:15:37 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for Ipolite best effort local Broadcast (ipolite)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __IPOLITE_H__
#define __IPOLITE_H__

#include "net/rime/broadcast.h"
#include "net/rime/ctimer.h"
#include "net/rime/queuebuf.h"

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
  void (* recv)(struct ipolite_conn *c, rimeaddr_t *from); 

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
};


/**
 * \brief      Open an ipolite connection
 * \param c    A pointer to a struct ipolite_conn.
 * \param channel The channel number to be used for this connection
 * \param cb   A pointer to the callbacks used for this connection
 *
 *             This function opens an ipolite connection on the
 *             specified channel. The callbacks are called when a
 *             packet is received, or when another event occurs on the
 *             connection (see \ref "struct ipolite_callbacks").
 */
void ipolite_open(struct ipolite_conn *c, uint16_t channel,
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
 *             This function sends a packet from the rimebuf on the
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

#endif /* __IPOLITE_H__ */

/** @} */
/** @} */
