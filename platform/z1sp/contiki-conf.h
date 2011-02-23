/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H


#include "platform-conf.h"

#define XMAC_CONF_COMPOWER          1
#define CXMAC_CONF_COMPOWER         1

#if WITH_UIP6

/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK sicslowpan_driver
/* #define NETSTACK_CONF_MAC nullmac_driver */
/* #define NETSTACK_CONF_RDC sicslowmac_driver */
#define NETSTACK_CONF_MAC     csma_driver
#define NETSTACK_CONF_RDC     contikimac_driver
#define NETSTACK_CONF_RADIO   cc2420_driver
#define NETSTACK_CONF_FRAMER  framer_802154

#define CC2420_CONF_AUTOACK              1
#define NETSTACK_RDC_CHANNEL_CHECK_RATE  8
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 0
#define CXMAC_CONF_ANNOUNCEMENTS         0
#define XMAC_CONF_ANNOUNCEMENTS          0

#define QUEUEBUF_CONF_NUM                4

#else /* WITH_UIP6 */

/* Network setup for non-IPv6 (rime). */

#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC     csma_driver
/* #define NETSTACK_CONF_RDC     contikimac_driver */
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_CONF_FRAMER  framer_802154

#define CC2420_CONF_AUTOACK              0
#define MAC_CONF_CHANNEL_CHECK_RATE      8

#define COLLECT_CONF_ANNOUNCEMENTS       1
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 0
#define CXMAC_CONF_ANNOUNCEMENTS         0
#define XMAC_CONF_ANNOUNCEMENTS          0
#define CONTIKIMAC_CONF_ANNOUNCEMENTS    0

#define CONTIKIMAC_CONF_COMPOWER         1
#define XMAC_CONF_COMPOWER               1
#define CXMAC_CONF_COMPOWER              1

#define COLLECT_NEIGHBOR_CONF_MAX_NEIGHBORS      32

#define QUEUEBUF_CONF_NUM          8

#endif /* WITH_UIP6 */

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif /* RF_CHANNEL */

#define IEEE802154_CONF_PANID       0xABCD

#define SHELL_VARS_CONF_RAM_BEGIN 0x1100
#define SHELL_VARS_CONF_RAM_END 0x2000


#define CFS_CONF_OFFSET_TYPE	long

#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 0

#define ELFLOADER_CONF_TEXT_IN_ROM 0
#define ELFLOADER_CONF_DATAMEMORY_SIZE 0x400
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x800

#define CCIF
#define CLIF

#define CC_CONF_INLINE inline

#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES 32

#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1
/*#define PROCESS_CONF_FASTPOLL    4*/


#define UART0_CONF_TX_WITH_INTERRUPT 0 // So far, printfs without interrupt.

#ifdef WITH_UIP6

#define RIMEADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_ROUTER                 1
#define UIP_CONF_IPV6_RPL               1

/* Handle 10 neighbors */
#define UIP_CONF_DS6_NBR_NBU     15
/* Handle 10 routes    */
#define UIP_CONF_DS6_ROUTE_NBU   15

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
#define UIP_CONF_BUFFER_SIZE		140

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
#define UIP_CONF_BUFFER_SIZE     108
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


#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */



#endif /* CONTIKI_CONF_H */
