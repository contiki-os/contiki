/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: tr1001.h,v 1.9 2010/03/02 22:40:39 nifi Exp $
 */
#ifndef __TR1001_H__
#define __TR1001_H__

#include "contiki-net.h"
#include "dev/radio.h"

#include "contiki-conf.h"

/**
 * Radio driver for TR1001
 */
extern const struct radio_driver tr1001_driver;

/**
 * Initialize the radio transceiver.
 *
 * Turns on reception of bytes and installs the receive interrupt
 * handler.
 */
int tr1001_init(void);

/**
 * Set the speed of the TR1001 radio device.
 *
 * This function sets the speed of the TR1001 radio transceiver. Both
 * the sender and the receiver must have the same speed for
 * communication to work.
 *
 * \param speed The speed of the TR1001 radio: TR1001_19200,
 * TR1001_38400, TR1001_57600 or TR1001_115200.
 *
 */
void tr1001_set_speed(unsigned char s);
#define TR1001_19200  1
#define TR1001_38400  2
#define TR1001_57600  3
#define TR1001_115200 4

/**
 * Set the transmission power of the transceiver.
 *
 * The sensor board is equipped with a DS1804 100 position trimmer
 * potentiometer which is used to set the transmission input current
 * to the radio transceiver chip, thus setting the transmission power
 * of the radio transceiver.
 *
 * This function sets the trimmer potentiometer to a value between 1
 * and 100.
 *
 * \param p The power of the transceiver, between 1 (lowest) and 100
 * (highest).
 */
void tr1001_set_txpower(unsigned char p);

/**
 * \brief   The highest transmission power
 */
#define TR1001_TXPOWER_HIGHEST 100

/**
 * \brief   The lowest transmission power
 */
#define TR1001_TXPOWER_LOWEST  1

/**
 * Send a packet.
 *
 * This function causes a packet to be sent out after a small random
 * delay, but without doing any MAC layer collision detection or
 * back-offs. The packet is sent with a 4 byte header that contains a
 * a "type" identifier, an 8-bit packet ID field and the length of the
 * packet in network byte order.
 *
 * This function should normally not be called from user
 * programs. Rather, the uIP TCP/IP or Rime stack should be used.
 */
int tr1001_send(const void *packet, unsigned short len);

/**
 * Check if an incoming packet has been received.
 *
 * This function checks the receive buffer to see if an entire packet
 * has been received. The actual reception is handled by an interrupt
 * handler.
 *
 * This function should normally not be called from user
 * programs. Rather, the uIP TCP/IP or Rime stack should be used.
 *
 * \return The length of the received packet, or 0 if no packet has
 * been received.
 */
int tr1001_read(void *buf, unsigned short bufsize);

extern unsigned char tr1001_rxbuf[];
extern volatile unsigned char tr1001_rxstate;

/**
 * Calculate the signal strength of a received packet.
 *
 * This function calculates the received signal strength of the last
 * received packet. This function typically is called when a packet
 * has been received.
 */
unsigned short tr1001_sstrength(void);

#endif /* __TR1001_H__ */
