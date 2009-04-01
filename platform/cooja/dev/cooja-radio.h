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
 * $Id: cooja-radio.h,v 1.5 2009/04/01 13:44:34 fros4943 Exp $
 */

#ifndef __COOJA_RADIO_H__
#define __COOJA_RADIO_H__

#include "contiki.h"
#include "contiki-conf.h"

#include "dev/radio.h"

#include "net/uip.h"
#include "net/uip-fw.h"

#define COOJA_RADIO_BUFSIZE UIP_BUFSIZE
#define COOJA_RADIO_DROPPED UIP_FW_DROPPED
#define COOJA_RADIO_TOOLARGE UIP_FW_TOOLARGE
#define COOJA_RADIO_ZEROLEN UIP_FW_ZEROLEN
#define COOJA_RADIO_OK UIP_FW_OK

extern const struct radio_driver cooja_radio;

/**
 * Turn radio hardware on.
 */
int
radio_on(void);

/**
 * Turn radio hardware off.
 */
int
radio_off(void);

/**
 * Set radio receiver.
 */
void
radio_set_receiver(void (* recv)(const struct radio_driver *));

/**
 * Set radio channel.
 */
void
radio_set_channel(int channel);

/**
 * Set transmission power of transceiver.
 *
 * \param p The power of the tranceiver, between 1 (lowest) and 100
 * (highest).
 */
void
radio_set_txpower(unsigned char p);

/**
 * Send a packet from the given buffer with the given length.
 */
int
radio_send(const void *payload, unsigned short payload_len);

/**
 * Check if an incoming packet has been received.
 *
 * This function checks the receive buffer to see if an entire packet
 * has been received.
 *
 * \return The length of the received packet, or 0 if no packet has
 * been received.
 */
int
radio_read(void *buf, unsigned short bufsize);

/**
 * This function returns the signal strength of the last
 * received packet. This function typically is called when a packet
 * has been received.
 */
int radio_sstrength(void);

/**
 * This function measures and returns the signal strength.
 */
int radio_current_sstrength(void);

/**
 * Internal COOJA function, calls the current receiver function.
 */
void radio_call_receiver(void);

#endif /* __COOJA_RADIO_H__ */
