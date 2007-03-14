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
 * $Id: abc.h,v 1.3 2007/03/14 00:29:05 adamdunkels Exp $
 */

/**
 * \addtogroup rime
 * @{
 */

/**
 * \file
 *         Header file for the Rime module Anonymous BroadCast (abc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __ABC_H__
#define __ABC_H__

#include "contiki-net.h"
#include "net/rime/rimebuf.h"

struct abc_conn;

struct abc_ulayer {
  void (* recv)(struct abc_conn *ptr);
};

struct abc_conn {
  struct abc_conn *next;
  u16_t channel;
  const struct abc_ulayer *u;
};

/**
 * \brief      Set up an anonymous best-effort broadcast connection
 * \param c    A pointer to a struct abc_conn
 * \param channel The channel on which the connection will operate
 * \param u    A struct abc_ulayer with function pointers to functions that will be called when a packet has been received
 *
 *             This function sets up an abc connection on the
 *             specified channel. The caller must have allocated the
 *             memory for the struct abc_conn, usually by declaring it
 *             as a static variable.
 *
 *             The struct abc_ulayer pointer must point to a structure
 *             containing a pointer to a function that will be called
 *             when a packet arrives on the channel.
 *
 */
void abc_setup(struct abc_conn *c, u16_t channel,
	       const struct abc_ulayer *u);

/**
 * \brief      Send an anonymous best-effort broadcast packet
 * \param c    The abc connection on which the packet should be sent
 * \retval     Non-zero if the packet could be sent, zero otherwise
 *
 *             This function sends an anonymous best-effort broadcast
 *             packet. The packet must be present in the rimebuf
 *             before this function is called.
 *
 *             The parameter c must point to an abc connection that
 *             must have previously been set up with abc_setup().
 *
 */
int abc_send(struct abc_conn *c);

/**
 * \brief      Pass a packet to the abc layer
 *
 *             This function is used by a device driver to pass an
 *             incoming packet to the abc layer. The packet must be
 *             present in the rimebuf buffer when this function is
 *             called.
 */
void abc_input_packet(void);

/**
 * \brief      This function, which must be implemented by the driver, is called to send out a packet
 *
 *             This function is implemented by the driver running
 *             below abc and is called by abc to send out a
 *             packet. The packet is contained in the rimebuf. The
 *             packet is consecutive in the rimebuf and a pointer to
 *             the first byte is gotten from the rimebuf_hdrptr()
 *             function. The length of the packet to send is gotten
 *             with the rimebuf_totlen() function.
 *
 *             The driver, which typically is a MAC protocol, may
 *             queue the packet by using the queuebuf functions.
 *
 */
void abc_driver_send(void);

#endif /* __BC_H__ */
/** @} */
