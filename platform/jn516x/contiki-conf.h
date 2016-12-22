/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 */

#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#ifdef PLATFORM_CONF_H
#include PLATFORM_CONF_H
#else
#include "platform-conf.h"
#endif /* PLATFORM_CONF_H */

#ifndef CCM_STAR_CONF
#define CCM_STAR_CONF ccm_star_driver_jn516x
#endif /* CCM_STAR_CONF */

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#endif /* NETSTACK_CONF_MAC */

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#endif /* NETSTACK_CONF_RDC */

#ifndef NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO   micromac_radio_driver
#endif /* NETSTACK_CONF_RADIO */

#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif /* NETSTACK_CONF_FRAMER */

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID       0xABCD
#endif /* IEEE802154_CONF_PANID */

#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON 1
#endif /* ENERGEST_CONF_ON */

#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1

#if !defined NETSTACK_CONF_WITH_IPV6 && !defined NETSTACK_CONF_WITH_IPV4 && !defined NETSTACK_CONF_WITH_RIME
#define NETSTACK_CONF_WITH_IPV6 1
#endif /* NETSTACK_CONF_ not defined */

/* Network setup for IP */
#if NETSTACK_CONF_WITH_IPV4 || NETSTACK_CONF_WITH_IPV6

#define LINKADDR_CONF_SIZE              8

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM               16
#endif

/* Network setup for non-IP (rime). */
#else

#define LINKADDR_CONF_SIZE              2

#ifndef COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS
#define COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS     32
#endif /* COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS */

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM               16
#endif /* QUEUEBUF_CONF_NUM */

#endif /* NETSTACK_CONF_WITH_IPV4 || NETSTACK_CONF_WITH_IPV6 */

/* Network setup for IPv6 */
#if NETSTACK_CONF_WITH_IPV6

/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#define UIP_CONF_BROADCAST 1

/* Configure NullRDC for when it is selected */
#define NULLRDC_CONF_802154_AUTOACK_HW  1

/* Configure ContikiMAC for when it's selected */
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER  0
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
#define WITH_FAST_SLEEP                         1

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8
#endif
#define RDC_CONF_HARDWARE_ACK        1

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_ROUTER                 1
#ifndef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL               1
#endif /* UIP_CONF_IPV6_RPL */

/* configure number of neighbors and routes */
#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS     20
#endif /* NBR_TABLE_CONF_MAX_NEIGHBORS */
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES   20
#endif /* UIP_CONF_MAX_ROUTES */

#define UIP_CONF_ND6_SEND_RA            0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#ifndef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT         0
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE           1280
#endif

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  48
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS         48
#endif
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_CHKSUM          0
#define UIP_ARCH_ADD32           0
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0
#define LOG_CONF_ENABLED         0

#define UIP_CONF_TCP_SPLIT       0

#define UIP_CONF_BYTE_ORDER      UIP_BIG_ENDIAN
#define UIP_CONF_LOGGING         0

#endif /* NETSTACK_CONF_WITH_IPV6 */

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* CONTIKI_CONF_H_ */
