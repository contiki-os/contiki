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
 *         Configuration for Atmel Raven
 *
 * \author
 *         Simon Barner <barner@in.tum.de>
 *         David Kopf <dak664@embarqmail.com>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

/* MCU and clock rate */
#define PLATFORM       PLATFORM_AVR
#define RAVEN_REVISION RAVEN_D
#ifndef F_CPU
#define F_CPU          8000000UL
#endif

/* Clock ticks per second */
#define CLOCK_CONF_SECOND 125

/* Since clock_time_t is 16 bits, maximum interval is 524 seconds */
#define RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME CLOCK_CONF_SECOND * 524UL /*Default uses 600*/

/* Maximum time interval (used for timers) */
#define INFINITE_TIME 0xffff

/* COM port to be used for SLIP connection. Not tested on Raven */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
/* Default is 4096. Currently used only when elfloader is present. Not tested on Raven */
//#define MMEM_CONF_SIZE 256
/* Starting address for code received via the codeprop facility. Not tested on Raven */
//#define EEPROMFS_ADDR_CODEPROP 0x8000

#define CCIF
#define CLIF

/* Network setup. The new NETSTACK interface requires RF230BB (as does ip4) */
#if RF230BB
//#define SICSLOWPAN_CONF_CONVENTIONAL_MAC  1     //sicslowpan calls radio->read function
#undef PACKETBUF_CONF_HDR_SIZE                  //using the packetbuf default for header size
#else
#define PACKETBUF_CONF_HDR_SIZE    0            //RF230 combined driver/mac handles headers internally
#endif /*RF230BB */

#if UIP_CONF_IPV6
#define RIMEADDR_CONF_SIZE        8
#define UIP_CONF_ICMP6            1
#define UIP_CONF_UDP              1
#define UIP_CONF_TCP              1
#define UIP_CONF_IPV6_RPL         0
#define NETSTACK_CONF_NETWORK       sicslowpan_driver
#define SICSLOWPAN_CONF_COMPRESSION SICSLOWPAN_COMPRESSION_HC06
#else
/* ip4 should build but is largely untested */
#define RIMEADDR_CONF_SIZE        2
#define NETSTACK_CONF_NETWORK     rime_driver
#endif /* UIP_CONF_IPV6 */

/* See uip-ds6.h */
#define UIP_CONF_DS6_NBR_NBU      20
#define UIP_CONF_DS6_DEFRT_NBU    2
#define UIP_CONF_DS6_PREFIX_NBU   3
#define UIP_CONF_DS6_ROUTE_NBU    20
#define UIP_CONF_DS6_ADDR_NBU     3
#define UIP_CONF_DS6_MADDR_NBU    0
#define UIP_CONF_DS6_AADDR_NBU    0

#define UIP_CONF_LL_802154       1
#define UIP_CONF_LLH_LEN         0

/* 10 bytes per stateful address context - see sicslowpan.c */
/* These must agree with all the other nodes or there will be a failure to communicate! */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS 3
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 {addr_contexts[0].prefix[0]=0xaa;addr_contexts[0].prefix[1]=0xaa;}
#define SICSLOWPAN_CONF_ADDR_CONTEXT_1 {addr_contexts[1].prefix[0]=0xbb;addr_contexts[1].prefix[1]=0xbb;}
#define SICSLOWPAN_CONF_ADDR_CONTEXT_2 {addr_contexts[2].prefix[0]=0x20;addr_contexts[2].prefix[1]=0x01;addr_contexts[2].prefix[2]=0x49;addr_contexts[2].prefix[3]=0x78,addr_contexts[2].prefix[4]=0x1d;addr_contexts[2].prefix[5]=0xb1;}

/* 211 bytes per queue buffer */
#define QUEUEBUF_CONF_NUM         8

/* 54 bytes per queue ref buffer */
#define QUEUEBUF_CONF_REF_NUM     8

#define UIP_CONF_MAX_CONNECTIONS 8
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       4

#define UIP_CONF_IP_FORWARD      0
#define UIP_CONF_FWCACHE_SIZE    0

#define UIP_CONF_IPV6_CHECKS     1
#define UIP_CONF_IPV6_QUEUE_PKT  1
#define UIP_CONF_IPV6_REASSEMBLY 0

#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_DHCP_LIGHT      1


#if 1 /* No radio cycling */

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

#elif 0  /* Contiki-mac radio cycling */
#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         contikimac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
#define RF230_CONF_AUTOACK        0
#define RF230_CONF_AUTORETRIES    0
#define SICSLOWPAN_CONF_FRAG      1
#define SICSLOWPAN_CONF_MAXAGE    3

#elif 0  /* cx-mac radio cycling */
#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         cxmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
#define RF230_CONF_AUTOACK        0
#define RF230_CONF_AUTORETRIES    0
#define MAC_CONF_CHANNEL_CHECK_RATE 8
#define SICSLOWPAN_CONF_FRAG      1
#define SICSLOWPAN_CONF_MAXAGE    3

//Below will prevent fragmentation of TCP packets, undef for faster page loads, simpler wireshark captures
//#define UIP_CONF_TCP_MSS          48
//Below gives 10% duty cycle, undef for default 5%
//#define CXMAC_CONF_ON_TIME (RTIMER_ARCH_SECOND / 80)
//Below gives 50% duty cycle
//#define CXMAC_CONF_ON_TIME (RTIMER_ARCH_SECOND / 16)

#else
#error Network configuration not specified!
#endif   /* Network setup */

/* Logging adds 200 bytes to program size */
#define LOG_CONF_ENABLED         1

/* ************************************************************************** */
//#pragma mark RPL Settings
/* ************************************************************************** */
#if UIP_CONF_IPV6_RPL

/* Define MAX_*X_POWER to reduce tx power and ignore weak rx packets for testing a miniature multihop network.
 * Leave undefined for full power and sensitivity.
 * tx=0 (3dbm, default) to 15 (-17.2dbm)
 * RF230_CONF_AUTOACK sets the extended mode using the energy-detect register with rx=0 (-91dBm) to 84 (-7dBm)
 *   else the rssi register is used having range 0 (91dBm) to 28 (-10dBm)
 *   For simplicity RF230_MIN_RX_POWER is based on the energy-detect value and divided by 3 when autoack is not set.
 * On the RF230 a reduced rx power threshold will not prevent autoack if enabled and requested.
 * These numbers applied to both Raven and Jackdaw give a maximum communication distance of about 15 cm
 * and a 10 meter range to a full-sensitivity RF230 sniffer.
#define RF230_MAX_TX_POWER 15
#define RF230_MIN_RX_POWER 30
 */

#define UIP_CONF_ROUTER                 1
#define UIP_CONF_ND6_SEND_RA		    0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#undef UIP_CONF_UDP_CONNS
#define UIP_CONF_UDP_CONNS       12
#undef UIP_CONF_FWCACHE_SIZE
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

#endif /* RPL */

#include <stdint.h>

typedef int32_t s32_t;
typedef unsigned short clock_time_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef unsigned short uip_stats_t;
typedef unsigned long off_t;

void clock_delay(unsigned int us2);
void clock_wait(int ms10);
void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);

#endif /* __CONTIKI_CONF_H__ */
