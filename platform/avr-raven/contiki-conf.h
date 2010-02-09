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

#include <stdint.h>

typedef int32_t s32_t;

/*
 * MCU and clock rate
 */
#define MCU_MHZ 8
#define PLATFORM PLATFORM_AVR

/* Clock ticks per second */
#define CLOCK_CONF_SECOND 125

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
#define PACKETBUF_CONF_HDR_SIZE    0           //RF230 handles headers internally

/* 0 for IPv6, or 1 for HC1, 2 for HC01 */
#define SICSLOWPAN_CONF_COMPRESSION_IPV6 0 
#define SICSLOWPAN_CONF_COMPRESSION_HC1  1 
#define SICSLOWPAN_CONF_COMPRESSION_HC01 2

#define SICSLOWPAN_CONF_COMPRESSION       SICSLOWPAN_CONF_COMPRESSION_HC01 
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS 2
#define SICSLOWPAN_CONF_FRAG              1

/* Below will prevent fragmentation of TCP packets, undef for faster page loads, simpler wireshark captures */
//#define UIP_CONF_TCP_MSS 48

/* Fragmentation uses queuebuf.c to save packets */
#define QUEUEBUF_CONF_NUM 1
#define QUEUEBUF_CONF_REF_NUM 1

/* Logging adds 200 bytes to program size */
#define LOG_CONF_ENABLED 1

/* RF230BB reduces program size by 6.5KB, RAM by 500 bytes */
#ifdef RF230BB
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC    1   //for barebones driver, sicslowpan calls radio->read function
#undef PACKETBUT_CONF_HDR_SIZE                  //RF230BB takes the packetbuf default for header size
//#undef SICSLOWPAN_CONF_FRAG
//#define SICSLOWPAN_CONF_FRAG              0   //for sky equivalence with barebones driver
//#define UIP_CONF_BUFFER_SIZE            256   //for sky equivalence with barebones driver
//#define UIP_CONF_ROUTER 1
#define UIP_CONF_ROUTER 0 
#endif

#define SICSLOWPAN_CONF_MAXAGE 5

#define UIP_CONF_LL_802154       1
#define UIP_CONF_LLH_LEN         0

#define UIP_CONF_MAX_CONNECTIONS 2
#define UIP_CONF_MAX_LISTENPORTS 2
#define UIP_CONF_UDP_CONNS       2

#define UIP_CONF_IP_FORWARD      0
#define UIP_CONF_FWCACHE_SIZE    0

#define UIP_CONF_IPV6            1
#define UIP_CONF_IPV6_CHECKS     1
#define UIP_CONF_IPV6_QUEUE_PKT  1
#define UIP_CONF_IPV6_REASSEMBLY 0
#define UIP_CONF_NETIF_MAX_ADDRESSES  3
#define UIP_CONF_ND6_MAX_PREFIXES     3
#define UIP_CONF_ND6_MAX_NEIGHBORS    4  
#define UIP_CONF_ND6_MAX_DEFROUTERS   2
#define UIP_CONF_ICMP6           1

#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1

#define UIP_CONF_TCP             1
#define UIP_CONF_TCP_SPLIT       1


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
