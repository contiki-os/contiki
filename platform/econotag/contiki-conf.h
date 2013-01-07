/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
 *
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
 *         Configuration for Econotag
 *
 * \author
 *         Original by:
 *         Simon Barner <barner@in.tum.de>
 *         This version by:
 *         Mariano Alvira <mar@devl.org>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <stdint.h>

/* mc1322x files */
#include "contiki-mc1322x-conf.h"

/* Econotag I tune parameters */
#define ECONOTAG_CTUNE_4PF 1
/* Coarse tune: add 0-15 pf (CTUNE is 4 bits) */
#define ECONOTAG_CTUNE 11
/* Fine tune: add FTUNE * 156fF (FTUNE is 5bits) */
#define ECONOTAG_FTUNE 7

/* M12 tune parameters */
#define M12_CTUNE_4PF 1
#define M12_CTUNE 3
#define M12_FTUNE 3

/* the econotag platform will correctly detect an Econotag I (no M12) vs. Econotag II (w/M12) */
/* and trim the main crystal accordingly */
/* this detection will be incorrect if you populate the 32.768kHz crystal on the Econotag I */
/* In that case, you should FORCE_ECONOTAG_I below */
#define FORCE_ECONOTAG_I 0

/* if you define a serial number then it will be used to comput the mac address */
/* otherwise, a random mac address in the Redwire development IAB will be used */
/* #define M12_CONF_SERIAL 0x000000 */

/* Clock ticks per second */
#define CLOCK_CONF_SECOND 100

#define CCIF
#define CLIF

#define CONSOLE_UART UART1
#define CONSOLE_BAUD 115200

#define dbg_putchar(x) uart1_putc(x)

#define USE_FORMATTED_STDIO         1
#define MACA_DEBUG                  0
#define CONTIKI_MACA_RAW_MODE       0

#define BLOCKING_TX 1
#define MACA_AUTOACK 1
#define NULLRDC_CONF_802154_AUTOACK_HW 1

#define USE_WDT 0

#ifndef WDT_TIMEOUT
#define WDT_TIMEOUT 5000 /* watchdog timeout in ms */
#endif

/* end of mc1322x specific config. */

/* start of conitki config. */
#define PLATFORM_HAS_LEDS 1
#define PLATFORM_HAS_BUTTON 1

/* Core rtimer.h defaults to 16 bit timer unless RTIMER_CLOCK_LT is defined */
typedef unsigned long rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((signed long)((a)-(b)) < 0)

#define RIMEADDR_CONF_SIZE              8

#if WITH_UIP6
/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#define NETSTACK_CONF_MAC     nullmac_driver 
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_CONF_RADIO   contiki_maca_driver
#define NETSTACK_CONF_FRAMER  framer_802154

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE      8
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 0
#define CXMAC_CONF_ANNOUNCEMENTS         0
#define XMAC_CONF_ANNOUNCEMENTS          0

#else /* WITH_UIP6 */
/* Network setup for non-IPv6 (rime). */

#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC     csma_driver
#define NETSTACK_CONF_RDC     sicslowmac_driver
#define NETSTACK_CONF_RADIO   contiki_maca_driver
#define NETSTACK_CONF_FRAMER  framer_802154

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE      8

#define COLLECT_CONF_ANNOUNCEMENTS       1
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 0
#define CXMAC_CONF_ANNOUNCEMENTS         0
#define XMAC_CONF_ANNOUNCEMENTS          0
#define CONTIKIMAC_CONF_ANNOUNCEMENTS    0

#define CONTIKIMAC_CONF_COMPOWER         0
#define XMAC_CONF_COMPOWER               0
#define CXMAC_CONF_COMPOWER              0

#define COLLECT_NEIGHBOR_CONF_MAX_NEIGHBORS      32

#endif /* WITH_UIP6 */

#define QUEUEBUF_CONF_NUM          16

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif /* RF_CHANNEL */

#define CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT 0

#define IEEE802154_CONF_PANID       0xABCD

#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 0

#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES 32

#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1

#ifdef WITH_UIP6

#define RIMEADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_ROUTER                 1  
#define UIP_CONF_IPV6_RPL               1

#define UIP_CONF_DS6_NBR_NBU     30
#define UIP_CONF_DS6_ROUTE_NBU   30

#define UIP_CONF_ND6_SEND_RA		0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define UIP_CONF_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         0
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      4
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#define UIP_CONF_BUFFER_SIZE		1300
#define SICSLOWPAN_CONF_FRAG            1
#define SICSLOWPAN_CONF_MAXAGE          8

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC	1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#else /* WITH_UIP6 */
#define UIP_CONF_IP_FORWARD      1
#define UIP_CONF_BUFFER_SIZE     1300
#endif /* WITH_UIP6 */

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_RECEIVE_WINDOW  48
#define UIP_CONF_TCP_MSS         48
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

#define UIP_CONF_TCP_SPLIT       0

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* __CONTIKI_CONF_H__ */
