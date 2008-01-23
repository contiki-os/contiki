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
 * $Id: simple-cc2420.h,v 1.9 2008/01/23 14:57:19 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SIMPLE_CC2420_H__
#define __SIMPLE_CC2420_H__

#include "contiki.h"
#include "dev/radio.h"

void simple_cc2420_init(void);

#define SIMPLE_CC2420_MAX_PACKET_LEN      127

void simple_cc2420_set_channel(int channel);
int simple_cc2420_get_channel(void);

void simple_cc2420_set_pan_addr(unsigned pan,
				unsigned addr,
				const uint8_t *ieee_addr);

extern signed char simple_cc2420_last_rssi;
extern uint8_t simple_cc2420_last_correlation;

int simple_cc2420_rssi(void);

extern const struct radio_driver simple_cc2420_driver;

/**
 * \param power Between 1 and 31.
 */
void simple_cc2420_set_txpower(uint8_t power);
int simple_cc2420_get_txpower(void);
#define SIMPLE_CC2420_TXPOWER_MAX  31
#define SIMPLE_CC2420_TXPOWER_MIN   0

/**
 * Interrupt function, called from the simple-cc2420-arch driver.
 *
 */
int simple_cc2420_interrupt(void);

/* XXX hack: these will be made as Chameleon packet attributes */
extern rtimer_clock_t simple_cc2420_time_of_arrival,
  simple_cc2420_time_of_departure;
extern int simple_cc2420_authority_level_of_sender;

int simple_cc2420_on(void);
int simple_cc2420_off(void);


#endif /* __SIMPLE_CC2420_H__ */
