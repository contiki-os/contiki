/*
 * Copyright (c) 2013, KTH, Royal Institute of Technology
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 */

#ifndef __CONTIKI_CONF_H_
#define __CONTIKI_CONF_H_

#include <stdint.h>
#include "lpc1768.h"

#define CCIF
#define CLIF

#define CLOCK_CONF_SECOND 100

/* Defined as 0 for UART0 and 1 for UART1 */
#define DEBUG_UART      0

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

#ifndef BV
#define BV(x) (1<<(x))
#endif

/* Define the MAC address of the device */
#define EMAC_ADDR0              0x10
#define EMAC_ADDR1              0x1F
#define EMAC_ADDR2              0xE0
#define EMAC_ADDR3              0x12
#define EMAC_ADDR4              0x1D
#define EMAC_ADDR5              0x0C

/* uIP configuration */
/* Ethernet LLH(Link Level Header) size is 14 bytes */
#define UIP_CONF_LLH_LEN	14
#define UIP_CONF_BROADCAST	1
#define UIP_CONF_LOGGING	1
#define UIP_CONF_BUFFER_SIZE	1024
#define UIP_CONF_TCP_FORWARD	1
#define UIP_CONF_ICMP6		1
#define UIP_CONF_LL_802154	1

/* Prefix for relocation sections in ELF files */
#define REL_SECT_PREFIX ".rel"

#define CC_BYTE_ALIGNED __attribute__ ((packed, aligned(1)))

#define USB_EP1_SIZE 64
#define USB_EP2_SIZE 64

#define RAND_MAX 0x7fff

#endif /* __CONTIKI_CONF_H__CDBB4VIH3I__ */
