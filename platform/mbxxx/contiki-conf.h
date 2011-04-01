/*
 * Copyright (c) 2010, STMicroelectronics.
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
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			contiki-conf.h for MBXXX.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/
/*---------------------------------------------------------------------------*/


#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include PLATFORM_HEADER

#include <inttypes.h>
#include <string.h>  // For memcpm().

#define CC_CONF_REGISTER_ARGS          0
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline

#define CCIF
#define CLIF

typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;
typedef unsigned short uip_stats_t;


//#define FIXED_NET_ADDRESS             1
//#define NET_ADDR_A                    0x2001
//#define NET_ADDR_B                    0xdb8
//#define NET_ADDR_C                    0xbbbb
//#define NET_ADDR_D                    0xabcd

#define UART1_CONF_TX_WITH_INTERRUPT		0
#define WITH_SERIAL_LINE_INPUT			1
#define ENERGEST_CONF_ON			0
#define TELNETD_CONF_NUMLINES			6

#define QUEUEBUF_CONF_NUM			2       


#define NETSTACK_CONF_RADIO		stm32w_radio_driver

#if WITH_UIP6

/* No radio cycling */
#define NETSTACK_CONF_NETWORK		sicslowpan_driver
#define NETSTACK_CONF_MAC		nullmac_driver
#define NETSTACK_CONF_RDC		sicslowmac_driver
#define NETSTACK_CONF_FRAMER		framer_802154

#define RIMEADDR_CONF_SIZE              8
#define UIP_CONF_LL_802154              1

#define UIP_CONF_ROUTER				1
#define UIP_CONF_IPV6_RPL			1
#define UIP_CONF_ND6_SEND_RA			0
//#define RPL_BORDER_ROUTER			0

/* A trick to resolve a compilation error with IAR. */
#ifdef __ICCARM__
#define UIP_CONF_DS6_AADDR_NBU			1
#endif

#define UIP_CONF_IPV6				1
#define UIP_CONF_IPV6_QUEUE_PKT			0   // This is a very costly feature as it increases the RAM usage by approximately UIP_ND6_MAX_NEIGHBORS * UIP_LINK_MTU bytes.
#define UIP_CONF_IPV6_CHECKS			1
#define UIP_CONF_IPV6_REASSEMBLY		0
#define UIP_CONF_ND6_MAX_PREFIXES		2
#define UIP_CONF_ND6_MAX_NEIGHBORS		2
#define UIP_CONF_ND6_MAX_DEFROUTERS		1
#define UIP_CONF_IP_FORWARD			0
#define UIP_CONF_BUFFER_SIZE			140
#define UIP_CONF_MAX_CONNECTIONS		6
#define UIP_CONF_MAX_LISTENPORTS		6
#define UIP_CONF_UDP_CONNS			3

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC06        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_CONF_COMPRESSION_HC06
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#define SICSLOWPAN_CONF_MAXAGE                  2

#define UIP_CONF_ICMP6				0
#endif /* WITH_UIP6 */

#define UIP_CONF_UDP				1
#define UIP_CONF_TCP				1

#define IEEE802154_CONF_PANID		0x1234
#define STM32W_NODE_ID			0x5678   // to be deleted
#define RF_CHANNEL			16
#define RADIO_RXBUFS                    2   // Set to a number greater than 1 to decrease packet loss probability at high rates (e.g, with fragmented packets)
#define UIP_CONF_LLH_LEN                0

typedef unsigned long clock_time_t;

#define CLOCK_CONF_SECOND 1000

typedef unsigned long long rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((signed short)((a)-(b)) < 0)

/* LEDs ports MB8xxx */

#define LEDS_CONF_GREEN			LED_D1
#define LEDS_CONF_YELLOW        LED_D3
#define LEDS_CONF_RED           LED_D3


#define UIP_ARCH_ADD32           1
#define UIP_ARCH_CHKSUM          0

#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN


#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */


#endif /* __CONTIKI_CONF_H__ */
