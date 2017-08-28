/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <stdint.h>

#include "aducrf101-contiki.h"
#include "platform-conf.h"

/* Clock ticks per second */
#define CLOCK_CONF_SECOND               1000

#define CCIF
#define CLIF

/* start of conitki config. */
#define PLATFORM_HAS_LEDS               1
#define PLATFORM_HAS_BUTTON             1

#define LINKADDR_CONF_SIZE              8

#if NETSTACK_CONF_WITH_IPV6
/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK           sicslowpan_driver
#define NETSTACK_CONF_MAC               nullmac_driver
#define NETSTACK_CONF_RDC               nullrdc_driver
#define NETSTACK_CONF_RADIO             aducrf101_radio_driver
#define NETSTACK_CONF_FRAMER            framer_802154

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS        0
#define CXMAC_CONF_ANNOUNCEMENTS                0
#define XMAC_CONF_ANNOUNCEMENTS                 0

#else /* NETSTACK_CONF_WITH_IPV6 */

/* Network setup for non-IPv6 (rime). */
#define NETSTACK_CONF_NETWORK           rime_driver
#define NETSTACK_CONF_MAC               csma_driver
#define NETSTACK_CONF_RDC               nullrdc_driver
#define NETSTACK_CONF_RADIO             aducrf101_radio_driver
#define NETSTACK_CONF_FRAMER            framer_802154

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8

#define COLLECT_CONF_ANNOUNCEMENTS              1
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS        0
#define CXMAC_CONF_ANNOUNCEMENTS                0
#define XMAC_CONF_ANNOUNCEMENTS                 0
#define CONTIKIMAC_CONF_ANNOUNCEMENTS           0

#define CONTIKIMAC_CONF_COMPOWER                0
#define XMAC_CONF_COMPOWER                      0
#define CXMAC_CONF_COMPOWER                     0

#define COLLECT_NBR_TABLE_CONF_MAX_NEIGHBORS    16

#endif /* NETSTACK_CONF_WITH_IPV6 */

#define QUEUEBUF_CONF_NUM               4

#define PACKETBUF_CONF_ATTRS_INLINE     1

#ifndef RF_CHANNEL
#define RF_CHANNEL                      868000000
#endif /* RF_CHANNEL */

#define CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT    0

#define IEEE802154_CONF_PANID           0xABCD

#define PROFILE_CONF_ON                 0
#define ENERGEST_CONF_ON                0

#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES             16

#define WITH_ASCII                      1

#define PROCESS_CONF_NUMEVENTS          8
#define PROCESS_CONF_STATS              1

#ifdef NETSTACK_CONF_WITH_IPV6

#define LINKADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                 1
#endif

#define NBR_TABLE_CONF_MAX_NEIGHBORS    16
#define UIP_CONF_MAX_ROUTES             16

#define UIP_CONF_ND6_SEND_RA            0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define NETSTACK_CONF_WITH_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         0
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_IP_FORWARD             0
#define UIP_CONF_BUFFER_SIZE            240
#define SICSLOWPAN_CONF_FRAG            1
#define SICSLOWPAN_CONF_MAXAGE          8

#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#else /* NETSTACK_CONF_WITH_IPV6 */
#define UIP_CONF_IP_FORWARD             1
#define UIP_CONF_BUFFER_SIZE            140
#endif /* NETSTACK_CONF_WITH_IPV6 */

#define UIP_CONF_ICMP_DEST_UNREACH      1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN                0
#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW         48
#endif
#define UIP_CONF_TCP_MSS                48
#define UIP_CONF_MAX_CONNECTIONS        4
#define UIP_CONF_MAX_LISTENPORTS        4
#define UIP_CONF_UDP_CONNS              8
#define UIP_CONF_FWCACHE_SIZE           16
#define UIP_CONF_BROADCAST              1
#define UIP_CONF_UDP                    1
#define UIP_CONF_UDP_CHECKSUMS          1
#define UIP_CONF_PINGADDRCONF           0
#define UIP_CONF_LOGGING                0

#define UIP_CONF_TCP_SPLIT              0

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* __CONTIKI_CONF_H__ */
