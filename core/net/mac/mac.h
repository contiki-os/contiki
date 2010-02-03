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
 * $Id: mac.h,v 1.11 2010/02/03 16:45:12 adamdunkels Exp $
 */

/**
 * \file
 *         MAC driver header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __MAC_H__
#define __MAC_H__

#include "contiki-conf.h"
#include "dev/radio.h"

/**
 * The structure of a MAC protocol driver in Contiki.
 */
struct mac_driver {
  char *name;

  /** Initialize the MAC driver */
  const struct mac_driver *(* init)(const struct radio_driver *r);

  /** Send a packet from the Rime buffer  */
  int (* send)(void);

  /** Read a received packet into the Rime buffer. */
  int (* read)(void);

  /** Set a function to be called when a packet has been received. */
  void (* set_receive_function)(void (*f)(const struct mac_driver *d));

  /** Turn the MAC layer on. */
  int (* on)(void);

  /** Turn the MAC layer off. */
  int (* off)(int keep_radio_on);

  /** Returns the channel check interval, expressed in clock_time_t ticks. */
  unsigned short (* channel_check_interval)(void);
};

/* Generic MAC return values. */
enum {
  /**< The MAC layer transmission was OK. */
  MAC_TX_OK,

  /**< The MAC layer transmission could not be performed due to a
     collision. */
  MAC_TX_COLLISION,

  /**< The MAC layer did not get an acknowledgement for the packet. */
  MAC_TX_NOACK,

  /**< The MAC layer transmission could not be performed because of an
     error. The upper layer may try again later. */
  MAC_TX_ERR,

  /**< The MAC layer transmission could not be performed because of a
     fatal error. The upper layer does not need to try again, as the
     error will be fatal then as well. */
  MAC_TX_ERR_FATAL,
};
#ifndef MAC_CHANNEL_CHECK_RATE
#ifdef MAC_CONF_CHANNEL_CHECK_RATE
#define MAC_CHANNEL_CHECK_RATE MAC_CONF_CHANNEL_CHECK_RATE
#else /* MAC_CONF_CHANNEL_CHECK_RATE */
#define MAC_CHANNEL_CHECK_RATE 4
#endif /* MAC_CONF_CHANNEL_CHECK_RATE */
#endif /* MAC_CHANNEL_CHECK_RATE */

#if (MAC_CHANNEL_CHECK_RATE & (MAC_CHANNEL_CHECK_RATE - 1)) != 0
#error MAC_CONF_CHANNEL_CHECK_RATE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change MAC_CONF_CHANNEL_CHECK_RATE in contiki-conf.h or in your Makefile.
#endif

#endif /* __MAC_H__ */
