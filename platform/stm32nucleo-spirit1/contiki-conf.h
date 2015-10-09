/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__
/*---------------------------------------------------------------------------*/
#include "platform-conf.h"
/*---------------------------------------------------------------------------*/
#define SLIP_BRIDGE_CONF_NO_PUTCHAR 1

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#define NULLRDC_CONF_802154_AUTOACK 0
#define NETSTACK_CONF_FRAMER  framer_802154
#define NETSTACK_CONF_NETWORK sicslowpan_driver

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_RDC_HEADER_LEN 0

#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC csma_driver
#define NETSTACK_MAC_HEADER_LEN 0

#define SICSLOWPAN_CONF_MAC_MAX_PAYLOAD \
  (NETSTACK_RADIO_MAX_PAYLOAD_LEN - NETSTACK_MAC_HEADER_LEN - \
   NETSTACK_RDC_HEADER_LEN)

#define RIMESTATS_CONF_ENABLED 0
#define RIMESTATS_CONF_ON 0

/* Network setup for IPv6 */
#define CXMAC_CONF_ANNOUNCEMENTS         0

/* A trick to resolve a compilation error with IAR. */
#ifdef __ICCARM__
#define UIP_CONF_DS6_AADDR_NBU              1
#endif

/* radio driver blocks until ACK is received */
#define NULLRDC_CONF_ACK_WAIT_TIME          (0)
#define CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT 0
#define IEEE802154_CONF_PANID       0xABCD

#define AODV_COMPLIANCE

#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1
/*#define PROCESS_CONF_FASTPOLL    4*/

#define LINKADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_ROUTER                 1

/* configure number of neighbors and routes */
#ifndef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   30
#endif /* UIP_CONF_DS6_ROUTE_NBU */

#define UIP_CONF_ND6_SEND_RA    0
#define UIP_CONF_ND6_REACHABLE_TIME    600000 /* 90000// 600000 */
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define UIP_CONF_IPV6                   1
#ifndef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT         0
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#define UIP_CONF_IP_FORWARD             0
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    280
/* #define UIP_CONF_BUFFER_SIZE    600 */
#endif

#define SICSLOWPAN_CONF_MAXAGE                  4
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2

#ifndef SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS
#define SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS   5
#endif /* SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS */

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  150
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS         UIP_CONF_RECEIVE_WINDOW
#endif
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        0
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_TCP     1
/*---------------------------------------------------------------------------*/
/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */
/*---------------------------------------------------------------------------*/
#endif /* CONTIKI_CONF_H */
/*---------------------------------------------------------------------------*/
