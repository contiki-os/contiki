/*
 * This file is part of HiKoB Openlab.
 *
 * HiKoB Openlab is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, version 3.
 *
 * HiKoB Openlab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HiKoB Openlab. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2011,2012 HiKoB.
 */

/**
 * \file
 *         Configuration for HiKoB OpenLab platforms
 *
 * \author
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 *         Gaëtan Harter <gaetan.harter.at.inria.fr>
 *
 */

#ifndef OPENLAB_CONTIKI_CONF_H_
#define OPENLAB_CONTIKI_CONF_H_

#include <stdint.h>

/* ---------------------------------------- */
/*
 *  Clock module and rtimer support
 *
 */

#define CLOCK_CONF_SECOND 100

typedef unsigned int   clock_time_t;
typedef unsigned short rtimer_clock_t;

/* ---------------------------------------- */
/*
 * Cortex M3 / ARM
 *
 */

typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t   s8_t;
typedef int16_t  s16_t;
typedef int32_t  s32_t;

#define CC_BYTE_ALIGNED __attribute__ ((packed, aligned(1)))
/* Prefix for relocation sections in ELF files */
#define REL_SECT_PREFIX ".rel"

/* Delay between GO signal and SFD
 * TODO: the current value is only a guess, needs actual measurement */
#define RADIO_DELAY_BEFORE_TX ((unsigned)US_TO_RTIMERTICKS(182))
/* Delay between GO signal and start listening
 * TODO: the current value is only a guess, needs actual measurement */
#define RADIO_DELAY_BEFORE_RX ((unsigned)US_TO_RTIMERTICKS(150))
/* Delay between the SFD finishes arriving and it is detected in software */
#define RADIO_DELAY_BEFORE_DETECT ((unsigned)US_TO_RTIMERTICKS(0))

/* ---------------------------------------- */

/*
 * Networking configuration inpired by cooja/contiki-conf.h
 */

#ifndef SLIP_ARCH_CONF_ENABLE
#define SLIP_ARCH_CONF_ENABLE 0
#endif
#define RF2XX_DEVICE rf231
#define SLIP_ARCH_CONF_UART uart_print

/*
 * Global Configuration networking
 */
typedef unsigned int uip_stats_t;
#define LINKADDR_CONF_SIZE          8

#define NETSTACK_CONF_RADIO         rf2xx_driver
/* Max payload of rf2xx is 125 bytes (128 -1 for length -2 for CRC) */
#define PACKETBUF_CONF_SIZE         125

/* Use big conf buffer size as there is plenty of ram */
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE       1500
#endif /* UIP_CONF_BUFFER_SIZE */


#ifdef NETSTACK_CONF_H
/*
 * These header overrides the below default configuration
 */
#define NETSTACK__QUOTEME(s) NETSTACK_QUOTEME(s)
#define NETSTACK_QUOTEME(s) #s
#include NETSTACK__QUOTEME(NETSTACK_CONF_H)
#else /* NETSTACK_CONF_H */


/* Default network config */
#if NETSTACK_CONF_WITH_IPV6
/*
 * Network setup for IPv6
 */
#ifndef NETSTACK_CONF_NETWORK
#define NETSTACK_CONF_NETWORK       sicslowpan_driver
#endif
#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC           csma_driver
#endif
#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER        framer_802154
#endif
/* NETSTACK_CONF_RDC */
#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC           contikimac_driver
#endif

#define UIP_CONF_ICMP6              1
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06

#endif /* NETSTACK_CONF_WITH_IPV6 */


#if NETSTACK_CONF_WITH_IPV4
/*
 * Network setup for IPv4
 */
#error NETSTACK_CONF_WITH_IPV4 not tested
// Cooja config:
#define NETSTACK_CONF_NETWORK rime_driver
#define UIP_CONF_IP_FORWARD           1
#define NETSTACK_CONF_MAC nullmac_driver


#endif /* NETSTACK_CONF_WITH_IPV4 */

#if NETSTACK_CONF_WITH_RIME
/*
 * Network setup for Rime
 */
// Rime needs default LINKADDR_SIZE 2
#undef LINKADDR_CONF_SIZE
#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC csma_driver

#endif /* #if NETSTACK_CONF_WITH_RIME */
#endif /* NETSTACK_CONF_H */

/* ---------------------------------------- */
/*
 * Networking
 *
 */


/*
 * MAC configuration:
 *
 * * has to do the csma
 * * has to receive the ack
 * * has to send the ack
 * * should not add additionnal header
 *
 */

/* Outgoing packet queue */
 #undef QUEUEBUF_CONF_NUM
 #define QUEUEBUF_CONF_NUM 16

/* contikimac */
#define RDC_CONF_HARDWARE_CSMA 0
#define RDC_CONF_HARDWARE_ACK 0
#define CONTIKIMAC_CONF_SEND_SW_ACK 1
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER 0

/* Other configs, no idea why (not checked) */
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 64
#define CONTIKIMAC_CONF_CCA_COUNT_MAX 16
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
#define CONTIKIMAC_CONF_INTER_PACKET_INTERVAL (RTIMER_ARCH_SECOND / 1000)

/*
 * UIP config
 *
 * * Addd logging support by default
 * * Interface is 802.15.4
 * * No link layer headers
 * * Add TCP/UDP support
 *
 */

#define UIP_CONF_LOGGING            1
#define UIP_CONF_LL_802154          1
#define UIP_CONF_LLH_LEN            0
#define UIP_CONF_UDP                1
#define UIP_CONF_TCP                1

/*
 * Non IPv6 Configuration ?
 * Network not tested
 */


/* ---------------------------------------- */
/*
 * Contiki internals
 *
 */
#define WITH_ASCII                      1
#define CCIF
#define CLIF

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* OPENLAB_CONTIKI_CONF_H_ */
