/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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

/**
 * \addtogroup cc3200-launchxl
 * @{
 *
 * \file
 *  Configuration for the cc3200-launchxl platform
 */
#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

/*---------------------------------------------------------------------------*/
/**
 * \name Compiler configuration and platform-specific type definitions
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define CLOCK_CONF_SECOND 125

/* Compiler configurations */
#define CCIF
#define CLIF

/* Platform typedefs */
typedef uint32_t clock_time_t;
typedef uint32_t uip_stats_t;

/*
 * rtimer.h typedefs rtimer_clock_t as unsigned short. We need to define
 * RTIMER_CLOCK_LT to override this
 */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)	((int32_t)((a)-(b)) < 0)

/*---------------------------------------------------------------------------*/
/**
 * \name Memory configuration
 *
 * @{
 */
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
#ifndef HEAP_CONF_SIZE
#define HEAP_CONF_SIZE  			0x00010000 /* 64 KiB */
#endif

#ifndef MTARCH_CONF_STACKSIZE
#define MTARCH_CONF_STACKSIZE		0x00000800 /*  2 KiB */
#endif
#else
#ifndef HEAP_CONF_SIZE
#define HEAP_CONF_SIZE  			0x00002000 /*  8 KiB */
#endif
#endif

#ifndef CONTIKI_CONF_STACKSIZE
#define CONTIKI_CONF_STACKSIZE		0x00008000 /* 32 KiB */
#endif
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name Watchdog Timer configuration
 *
 * @{
 */
#ifndef WATCHDOG_CONF_ENABLE
#define WATCHDOG_CONF_ENABLE		1 /**< Enable the watchdog timer */
#endif

/*---------------------------------------------------------------------------*/
/**
 * \name Generic Configuration directives
 *
 * @{
 */
#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        1 /**< Set to 0 to decrease startup verbosity */
#endif

#define LOG_CONF_ENABLED 			1
#define UIP_CONF_LOGGING 			1

/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name Character I/O Configuration
 *
 * @{
 */
#ifndef UART_CONF_ENABLE
#define UART_CONF_ENABLE            1 /**< Enable/Disable UART I/O */
#endif

#ifndef UART0_CONF_BAUD_RATE
#define UART0_CONF_BAUD_RATE   115200 /**< Default UART0 baud rate */
#endif

#ifndef UART1_CONF_BAUD_RATE
#define UART1_CONF_BAUD_RATE   115200 /**< Default UART1 baud rate */
#endif

#ifndef SERIAL_LINE_CONF_UART
#define SERIAL_LINE_CONF_UART       1 /**< UART to use with serial line */
#endif

#ifndef SLIP_ARCH_CONF_UART
#define SLIP_ARCH_CONF_UART         1 /**< UART to use with SLIP */
#endif

#ifndef CC2520_RF_CONF_SNIFFER_UART
#define CC2520_RF_CONF_SNIFFER_UART 1 /**< UART to use with sniffer */
#endif

#ifndef DBG_CONF_UART
#define DBG_CONF_UART               1 /**< UART to use for debugging */
#endif

#ifndef UART1_CONF_UART
#define UART1_CONF_UART             0 /**< UART to use for examples relying on
                                           the uart1_* API */
#endif

/* Turn off example-provided putchars */
#define SLIP_BRIDGE_CONF_NO_PUTCHAR 1
#define SLIP_RADIO_CONF_NO_PUTCHAR  1

#ifndef SLIP_ARCH_CONF_ENABLED
/*
 * Determine whether we need SLIP
 * This will keep working while UIP_FALLBACK_INTERFACE and CMD_CONF_OUTPUT
 * keep using SLIP
 */
#if defined (UIP_FALLBACK_INTERFACE) || defined (CMD_CONF_OUTPUT)
#define SLIP_ARCH_CONF_ENABLED      1
#endif
#endif

/*
 * When set, the radio turns off address filtering and sends all captured
 * frames down a peripheral
 */
#ifndef CC2520_RF_CONF_SNIFFER
#define CC2520_RF_CONF_SNIFFER      0
#endif

/**
 * \brief Define this as 1 to build a headless node.
 *
 * The UART will not be initialised its clock will be gated, offering some
 * energy savings. The USB will not be initialised either
 */
#ifndef CC32XX_CONF_QUIET
#define CC32XX_CONF_QUIET           0
#endif

/* CC32XX_CONF_QUIET is hard and overrides all other related defines */
#if CC32XX_CONF_QUIET

#undef UART_CONF_ENABLE
#define UART_CONF_ENABLE            0

#undef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        0

/* Little sanity check: We can't have quiet sniffers */
#if CC2520_RF_CONF_SNIFFER
#error "CC2520_RF_CONF_SNIFFER == 1 and CC32XX_CONF_QUIET == 1"
#error "These values are conflicting. Please set either to 0"
#endif
#endif /* CC2538_CONF_QUIET */

/*
 * If debugging and SLIP use the same peripheral, this will be 1. Don't modify
 * this
 */
#if SLIP_ARCH_CONF_ENABLED
#define DBG_CONF_SLIP_MUX (SLIP_ARCH_CONF_UART == DBG_CONF_UART)
#endif

/*
 * Automatic detection of whether a specific UART is in use
 */
#define UART_IN_USE_BY_SERIAL_LINE(u) (SERIAL_LINE_CONF_UART == (u))
#define UART_IN_USE_BY_SLIP(u)        (SLIP_ARCH_CONF_ENABLED && \
                                       !SLIP_ARCH_CONF_USB && \
                                       SLIP_ARCH_CONF_UART == (u))
#define UART_IN_USE_BY_RF_SNIFFER(u)  (CC2538_RF_CONF_SNIFFER && \
                                       !CC2538_RF_CONF_SNIFFER_USB && \
                                       CC2538_RF_CONF_SNIFFER_UART == (u))
#define UART_IN_USE_BY_DBG(u)         (!DBG_CONF_USB && DBG_CONF_UART == (u))
#define UART_IN_USE_BY_UART1(u)       (UART1_CONF_UART == (u))

#define UART_IN_USE(u) ( \
  UART_CONF_ENABLE && \
  (UART_IN_USE_BY_SERIAL_LINE(u) || \
   UART_IN_USE_BY_SLIP(u) || \
   UART_IN_USE_BY_RF_SNIFFER(u) || \
   UART_IN_USE_BY_DBG(u) || \
   UART_IN_USE_BY_UART1(u)) \
)
/** @} */
/*---------------------------------------------------------------------------*/
/* platform-conf.h assumes that basic configuration is done */
#include "platform-conf.h"
/*---------------------------------------------------------------------------*/

/**
 * \name Network Stack Configuration
 *
 * @{
 */
//#ifndef NETSTACK_CONF_NETWORK
//#if NETSTACK_CONF_WITH_IPV6
//#define NETSTACK_CONF_NETWORK sicslowpan_driver
//#else
//#define NETSTACK_CONF_NETWORK rime_driver
//#endif /* NETSTACK_CONF_WITH_IPV6 */
//#endif /* NETSTACK_CONF_NETWORK */
//
//#ifndef NETSTACK_CONF_MAC
///* #define NETSTACK_CONF_MAC     csma_driver */
//#define NETSTACK_CONF_MAC     csma_driver
//#endif
//
//#ifndef NETSTACK_CONF_RDC
///* #define NETSTACK_CONF_RDC     contikimac_driver */
//#define NETSTACK_CONF_RDC     contikimac_driver
//#endif
//
//#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
//#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8
//#endif
//
///* Configure NullRDC for when it's selected */
//#define NULLRDC_802154_AUTOACK                  1
//#define NULLRDC_802154_AUTOACK_HW               1
//
///* Configure ContikiMAC for when it's selected */
//#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
//#define WITH_FAST_SLEEP                         1
//
//#ifndef NETSTACK_CONF_FRAMER
//#if NETSTACK_CONF_WITH_IPV6
//#define NETSTACK_CONF_FRAMER  framer_802154
//#else /* NETSTACK_CONF_WITH_IPV6 */
//#define NETSTACK_CONF_FRAMER  contikimac_framer
//#endif /* NETSTACK_CONF_WITH_IPV6 */
//#endif /* NETSTACK_CONF_FRAMER */
//
//#define NETSTACK_CONF_RADIO   cc2520_driver

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name RF configuration
 *
 * @{
 */
/* RF Config */
#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID           0x5449 /**< Default PAN ID: TI */
#endif

#ifndef CC2520_RF_CONF_CHANNEL
#define CC2520_RF_CONF_CHANNEL              25
#endif /* CC2538_RF_CONF_CHANNEL */

#ifndef CC2520_CONF_AUTOACK
#define CC2520_CONF_AUTOACK                  1 /**< RF H/W generates ACKs */
#endif /* CC2520_CONF_AUTOACK */

/* WIFI Config */
#define UIP_CONF_LLH_LEN                    14 /**< Standard size of Ethernet Header */
#define UIP_CONF_BUFFER_SIZE              1500 /**< Standard MTU for Ethernet */

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name IPv6, RIME and network buffer configuration
 *
 * @{
 */

/* Don't let contiki-default-conf.h decide if we are an IPv6 build */
#ifndef NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_WITH_IPV6              0
#endif

#if NETSTACK_CONF_WITH_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define LINKADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_NETIF_MAX_ADDRESSES         4

/* TCP, UDP, ICMP */
#ifndef UIP_CONF_TCP
#define UIP_CONF_TCP                         1
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS                    64
#endif
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1
#define UIP_CONF_ICMP6                       1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif

#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#endif

#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS        32
#endif
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES                 20
#endif

#define UIP_CONF_IPV6_QUEUE_PKT              0
#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0
#define UIP_CONF_MAX_LISTENPORTS             8

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 63
#endif
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 1
#endif
#define SICSLOWPAN_CONF_MAXAGE               8

/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#ifndef SICSLOWPAN_CONF_ADDR_CONTEXT_0
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}
#endif

#define MAC_CONF_CHANNEL_CHECK_RATE          8

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    8
#endif
/*---------------------------------------------------------------------------*/
#else /* NETSTACK_CONF_WITH_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1

#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                   16
#endif

#endif /* NETSTACK_CONF_WITH_IPV6 */
/** @} */
/*---------------------------------------------------------------------------*/

#endif /* CONTIKI_CONF_H_ */

/** @} */
