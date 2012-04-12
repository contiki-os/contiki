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
 *         Configuration for AVR Zigbit Contiki kernel
 *
 * \author
 *         Simon Barner <barner@in.tum.de>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

/* Platform name, type, and MCU clock rate */
#define PLATFORM_NAME  "Zigbit"
#define PLATFORM_TYPE  ZIGBIT
#ifndef F_CPU
#define F_CPU          8000000UL
#endif

#include <stdint.h>

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

#define RIMEADDR_CONF_SIZE       8
#define PACKETBUF_CONF_HDR_SIZE    0

//define UIP_CONF_IPV6            1 //Let the makefile do this, allows hello-world to compile
#if UIP_CONF_IPV6
#define UIP_CONF_ICMP6           1
#define UIP_CONF_UDP             1
#define UIP_CONF_TCP             1
#endif

/* The new NETSTACK interface requires RF230BB */
#if RF230BB
#define SICSLOWPAN_CONF_COMPRESSION       SICSLOWPAN_COMPRESSION_HC06
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC  1     //for barebones driver, sicslowpan calls radio->read function
#undef PACKETBUF_CONF_HDR_SIZE                  //RF230BB takes the packetbuf default for header size
#define UIP_CONF_LLH_LEN         0

/* No radio cycling */
#if UIP_CONF_IPV6
#define NETSTACK_CONF_NETWORK     sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK     rime_driver
#endif
#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         sicslowmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
#define RF230_CONF_AUTOACK        1
#define RF230_CONF_AUTORETRIES    2
#define SICSLOWPAN_CONF_FRAG      1
//Most browsers reissue GETs after 3 seconds which stops frag reassembly, longer MAXAGE does no good
#define SICSLOWPAN_CONF_MAXAGE    3
#define QUEUEBUF_CONF_NUM         1
#define QUEUEBUF_CONF_REF_NUM     1
/* Default uip_aligned_buf and sicslowpan_aligned_buf sizes of 1280 overflows RAM */
#define UIP_CONF_BUFFER_SIZE	240

#else
/* Original combined RF230/mac code will not compile with current contiki stack */
//#define PACKETBUF_CONF_HDR_SIZE    0            //RF230 handles headers internally
/* 0 for IPv6, or 1 for HC1, 2 for HC01 */
#define SICSLOWPAN_CONF_COMPRESSION_IPV6 0 
#define SICSLOWPAN_CONF_COMPRESSION_HC1  1 
#define SICSLOWPAN_CONF_COMPRESSION_HC01 2       //NB '2' is now HC06 in the core mac!
//FTH081105
#define SICSLOWPAN_CONF_COMPRESSION       SICSLOWPAN_CONF_COMPRESSION_HC01 
#define SICSLOWPAN_CONF_MAXAGE 5
#define UIP_CONF_LLH_LEN         14
#endif /*RF230BB */

#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS 2
#define SICSLOWPAN_CONF_FRAG              1 

#define UIP_CONF_LL_802154       1 

#define UIP_CONF_MAX_CONNECTIONS 2
#define UIP_CONF_MAX_LISTENPORTS 2
#define UIP_CONF_UDP_CONNS       2

#define UIP_CONF_IP_FORWARD      0
#define UIP_CONF_FWCACHE_SIZE    0

#define UIP_CONF_IPV6_CHECKS     1
#define UIP_CONF_IPV6_QUEUE_PKT  0 
#define UIP_CONF_IPV6_REASSEMBLY 0
#define UIP_CONF_NETIF_MAX_ADDRESSES  3
#define UIP_CONF_ND6_MAX_PREFIXES     3
#define UIP_CONF_ND6_MAX_NEIGHBORS    4  
#define UIP_CONF_ND6_MAX_DEFROUTERS   2
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_TCP_SPLIT       1


/* These names are deprecated, use C99 names. */
/*
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef int32_t s32_t;
*/
typedef unsigned short uip_stats_t;
typedef unsigned long off_t;


#endif /* __CONTIKI_CONF_H__ */
