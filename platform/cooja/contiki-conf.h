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
 * $Id: contiki-conf.h,v 1.17 2010/03/31 10:22:33 fros4943 Exp $
 *
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 0
#define LOG_CONF_ENABLED 1

#define COOJA 1

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

/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK uip_driver
#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_RADIO cooja_radio_driver
#define UIP_CONF_IPV6 1
#define UIP_CONF_IP_FORWARD           0

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
#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_RADIO cooja_radio_driver
/*#define NETSTACK_CONF_FRAMER framer_nullmac*/

#endif /* WITH_UIP */
#endif /* WITH_UIP6 */

#endif /* NETSTACK_CONF_H */


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

#define UIP_CONF_MAX_LISTENPORTS      40
#define UIP_CONF_MAX_CONNECTIONS      40
#define UIP_CONF_BYTE_ORDER           UIP_LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT            0
#define UIP_CONF_LOGGING              0
#define UIP_CONF_UDP_CHECKSUMS        0
#define UIP_CONF_BROADCAST            1

#define UIP_CONF_UDP                  1
#define UIP_CONF_TCP                  1

#if UIP_CONF_IPV6
#define RIMEADDR_CONF_SIZE            8
#define UIP_CONF_IPV6_QUEUE_PKT       1
#define UIP_CONF_IPV6_CHECKS          1
#define UIP_CONF_IPV6_REASSEMBLY      1
#define UIP_CONF_NETIF_MAX_ADDRESSES  3
#define UIP_CONF_ND6_MAX_PREFIXES     3
#define UIP_CONF_ND6_MAX_NEIGHBORS    4
#define UIP_CONF_ND6_MAX_DEFROUTERS   2
#endif /* UIP_CONF_IPV6 */

#define CFS_CONF_OFFSET_TYPE	long

#endif /* __CONTIKI_CONF_H__ */
