/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 0
#define LOG_CONF_ENABLED 1

#define COOJA 1

#ifndef EEPROM_CONF_SIZE
#define EEPROM_CONF_SIZE				1024
#endif

#define w_memcpy memcpy

#if WITH_UIP
#if WITH_UIP6
#error WITH_UIP && WITH_IP6: Bad configuration
#endif /* WITH_UIP6 */
#endif /* WITH_UIP */

#ifdef NETSTACK_CONF_H

/* These header overrides the below default configuration */
#define NETSTACK__QUOTEME(s) NETSTACK_QUOTEME(s)
#define NETSTACK_QUOTEME(s) #s
#include NETSTACK__QUOTEME(NETSTACK_CONF_H)

#else /* NETSTACK_CONF_H */

/* Default network config */
#if WITH_UIP6

#define NULLRDC_CONF_802154_AUTOACK  0
#define NULLRDC_CONF_SEND_802154_ACK 0
#define NULLRDC_CONF_ACK_WAIT_TIME                RTIMER_SECOND / 500
#define NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME RTIMER_SECOND / 250


/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK       sicslowpan_driver
#define NETSTACK_CONF_MAC           csma_driver
#define NETSTACK_CONF_RDC           nullrdc_driver
#define NETSTACK_CONF_RADIO         cooja_radio_driver
#define NETSTACK_CONF_FRAMER        framer_802154

#else /* WITH_UIP6 */

#if WITH_UIP

/* Network setup for IPv4 */
#define NETSTACK_CONF_NETWORK rime_driver /* NOTE: uip_over_mesh. else: uip_driver */
#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_RADIO cooja_radio_driver
#define UIP_CONF_IP_FORWARD           1

#else /* WITH_UIP */

/* Network setup for Rime */
#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC csma_driver
#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_RADIO cooja_radio_driver
/*#define NETSTACK_CONF_FRAMER framer_nullmac*/

#endif /* WITH_UIP */
#endif /* WITH_UIP6 */

#endif /* NETSTACK_CONF_H */

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8

/* Default network config */
#if WITH_UIP6



/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK       sicslowpan_driver
#define NETSTACK_CONF_MAC           csma_driver
#define NETSTACK_CONF_RDC           nullrdc_driver
#define NETSTACK_CONF_RADIO         cooja_radio_driver
#define NETSTACK_CONF_FRAMER        framer_802154
#define UIP_CONF_IPV6               1

#define RIMEADDR_CONF_SIZE          8

#define UIP_CONF_LL_802154          1
#define UIP_CONF_LLH_LEN            0

#define UIP_CONF_ROUTER             1
#ifndef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL           1
#endif /* UIP_CONF_IPV6_RPL */

/* configure number of neighbors and routes */
#ifndef UIP_CONF_DS6_NBR_NBU
#define UIP_CONF_DS6_NBR_NBU     300
#endif /* UIP_CONF_DS6_NBR_NBU */
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES   300
#endif /* UIP_CONF_MAX_ROUTES */

#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1

#define RPL_CONF_DIO_INTERVAL_MIN 16

#define RPL_DIS_INTERVAL_CONF     (5 * 60)
#define RPL_CONF_DAO_LATENCY (CLOCK_SECOND * 60)

#define UIP_CONF_ND6_SEND_RA		0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define RIMEADDR_CONF_SIZE            8
#define UIP_CONF_NETIF_MAX_ADDRESSES  3
#define UIP_CONF_ND6_MAX_PREFIXES     3
#define UIP_CONF_ND6_MAX_NEIGHBORS    4
#define UIP_CONF_ND6_MAX_DEFROUTERS   2

#ifndef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT         1
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      4
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE		240
#endif

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
#ifndef SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS
#define SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS   8
#endif /* SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS */

#endif /* WITH_UIP6 */


#define PACKETBUF_CONF_ATTRS_INLINE 1

#define QUEUEBUF_CONF_NUM 16

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE inline

#define CCIF
#define CLIF

/* These names are deprecated, use C99 names. */
#include <inttypes.h>
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;

typedef unsigned short uip_stats_t;

#define CLOCK_CONF_SECOND 1000L
typedef unsigned long clock_time_t;
typedef unsigned long rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((signed long)((a)-(b)) < 0)

#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES 32

#define WITH_ASCII 1

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#ifndef  UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  48
#endif
#ifndef  UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS         48
#endif
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

#if UIP_CONF_IPV6
#endif /* UIP_CONF_IPV6 */

#define CFS_CONF_OFFSET_TYPE	long

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* __CONTIKI_CONF_H__ */
