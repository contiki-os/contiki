/*
 * Copyright (c) 2006, Technical University of Munich
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
 * @(#)$$
 */

/**
 * \file
 *         Configuration for sample STK 501 Contiki kernel
 *
 * \author
 *         Simon Barner <barner@in.tum.de
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

/*
 * MCU and clock rate. Various MCUs can be inserted in the ZIF socket.
 */
 /* Platform name, type, and MCU clock rate */
#define PLATFORM_NAME  "STK501"
#define PLATFORM_TYPE  STK501
#ifndef MCU
#define MCU      atmega128
#endif
#ifndef F_CPU
#define F_CPU          16000000UL
#endif

#define HAVE_STDINT_H
#include "avrdef.h"

/* The AVR tick interrupt usually is done with an 8 bit counter around 128 Hz.
 * 125 Hz needs slightly more overhead during the interrupt, as does a 32 bit
 * clock_time_t.
 */
 /* Clock ticks per second */
#define CLOCK_CONF_SECOND 125
#if 1
/* 16 bit counter overflows every ~10 minutes */
typedef unsigned short clock_time_t;
#define CLOCK_LT(a,b)  ((signed short)((a)-(b)) < 0)
#define INFINITE_TIME 0xffff
#define RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME INFINITE_TIME/CLOCK_CONF_SECOND /* Default uses 600 */
#define COLLECT_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME INFINITE_TIME/CLOCK_CONF_SECOND /* Default uses 600 */
#else
typedef unsigned long clock_time_t;
#define CLOCK_LT(a,b)  ((signed long)((a)-(b)) < 0)
#define INFINITE_TIME 0xffffffff
#endif
/* These routines are not part of the contiki core but can be enabled in cpu/avr/clock.c */
void clock_delay_msec(uint16_t howlong);
void clock_adjust_ticks(clock_time_t howmany);

/* COM port to be used for SLIP connection */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256

/* Use the following address for code received via the codeprop
 * facility
 */
#define EEPROMFS_ADDR_CODEPROP 0x8000

#define CCIF
#define CLIF

#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 4
#define UIP_CONF_BUFFER_SIZE     100
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_FWCACHE_SIZE    2
#define UIP_CONF_BROADCAST       1
#define UIP_UDP			 1

#define HAVE_STDINT_H
#include "avrdef.h"

typedef unsigned short uip_stats_t;
typedef unsigned long off_t;

#endif /* __CONTIKI_CONF_H__ */
