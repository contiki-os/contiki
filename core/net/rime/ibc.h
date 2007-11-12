/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeibc Identified best-effort local area broadcast
 * @{
 *
 * The ibc module sends packets to all local area neighbors with an a
 * header that identifies the sender.
 *
 * \section channels Channels
 *
 * The ibc module uses 1 channel.
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
 * $Id: ibc.h,v 1.9 2007/11/12 12:34:29 nvt-se Exp $
 */

/**
 * \file
 *         Header file for identified best-effort local area broadcast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __IBC_H__
#define __IBC_H__

#include "net/rime/abc.h"
#include "net/rime/rimeaddr.h"

struct ibc_conn;

/**
 * \brief     Callback structure for ibc
 *
 */
struct ibc_callbacks {
  /** Called when a packet has been received by the ibc module. */
  void (* recv)(struct ibc_conn *ptr, rimeaddr_t *sender);
};

struct ibc_conn {
  struct abc_conn c;
  const struct ibc_callbacks *u;
};

/**
 * \brief      Set up an identified best-effort broadcast connection
 * \param c    A pointer to a struct ibc_conn
 * \param channel The channel on which the connection will operate
 * \param u    A struct ibc_callbacks with function pointers to functions that will be called when a packet has been received
 *
 *             This function sets up an ibc connection on the
 *             specified channel. The caller must have allocated the
 *             memory for the struct ibc_conn, usually by declaring it
 *             as a static variable.
 *
 *             The struct ibc_callbacks pointer must point to a structure
 *             containing a pointer to a function that will be called
 *             when a packet arrives on the channel.
 *
 */
void ibc_open(struct ibc_conn *c, u16_t channel,
	       const struct ibc_callbacks *u);

/**
 * \brief      Close an ibc connection
 * \param c    A pointer to a struct ibc_conn
 *
 *             This function closes an ibc connection that has
 *             previously been opened with ibc_open().
 *
 *             This function typically is called as an exit handler.
 *
 */
void ibc_close(struct ibc_conn *c);

/**
 * \brief      Send an identified best-effort broadcast packet
 * \param c    The ibc connection on which the packet should be sent
 * \retval     Non-zero if the packet could be sent, zero otherwise
 *
 *             This function sends an identified best-effort broadcast
 *             packet. The packet must be present in the rimebuf
 *             before this function is called.
 *
 *             The parameter c must point to an ibc connection that
 *             must have previously been set up with ibc_open().
 *
 */
int ibc_send(struct ibc_conn *c);

#endif /* __IBC_H__ */
/** @} */
/** @} */
