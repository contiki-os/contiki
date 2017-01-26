/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup saker
 * @{
 *
 * \file
 *      Configuration for the Saker platform
 */
#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Include project specific configuration file */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */
/*---------------------------------------------------------------------------*/
/**
 * \name Compiler configuration and platform-specific type definitions
 *
 * These values are not meant to be modified by the user.
 * @{
 */
#define CLOCK_CONF_SECOND               128

/* Compiler configurations */
#define CCIF
#define CLIF

/* Platform typedefs */
typedef uint32_t clock_time_t;
typedef uint32_t uip_stats_t;

/*
 * rtimer.h typedefs rtimer_clock_t as unsigned short. We need to define
 * RTIMER_CLOCK_DIFF to override this
 */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_DIFF(a, b)     ((int32_t)((a) - (b)))
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Serial Boot Loader Backdoor configuration
 *
 * @{
 */
#ifndef FLASH_CCA_CONF_BOOTLDR_BACKDOOR
/** Enable the boot loader backdoor */
#define FLASH_CCA_CONF_BOOTLDR_BACKDOOR             1
#endif

#ifndef FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN
/** Pin PA5 activates the boot loader */
#define FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN  5
#endif

#ifndef FLASH_CCA_CONF_BOOTLDR_BACKDOOR_ACTIVE_HIGH
/** A logic low level activates the boot loader */
#define FLASH_CCA_CONF_BOOTLDR_BACKDOOR_ACTIVE_HIGH 0
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Processor speed and crystal configuration
 *
 * Per default, the system clock is run at maximum speed (32 MHz) and uses the
 * external 32kHz crystal.
 * @{
 */
#ifndef SYS_CTRL_CONF_SYS_DIV
#define SYS_CTRL_CONF_SYS_DIV           SYS_CTRL_CLOCK_CTRL_SYS_DIV_32MHZ
#endif /* SYS_CTRL_CONF_SYS_DIV */

#ifndef SYS_CTRL_CONF_IO_DIV
#define SYS_CTRL_CONF_IO_DIV            SYS_CTRL_CLOCK_CTRL_IO_DIV_32MHZ
#endif /* SYS_CTRL_CONF_IO_DIV */
/*---------------------------------------------------------------------------*/

#define SYS_CTRL_CONF_OSC32K_USE_XTAL   1 /**<  Use the external 32kHz watch
                                             crystal */
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name Flash Memory configuration
 *
 * Please make sure to load the binary to this address when you flash the
 * device.
 * @{
 */
#ifndef FLASH_CONF_ORIGIN
#define FLASH_CONF_ORIGIN  0x00200000
#endif

#ifndef FLASH_CONF_SIZE
#define FLASH_CONF_SIZE    0x00080000 /* 512 KiB */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Watchdog Timer configuration
 *
 * @{
 */
#ifndef WATCHDOG_CONF_ENABLE
#define WATCHDOG_CONF_ENABLE        1 /**< Enable the watchdog timer */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB 'core' configuration
 *
 * Those values are not meant to be modified by the user, except where stated
 * otherwise.
 * @{
 */
#define CTRL_EP_SIZE                8
#define USB_EP1_SIZE               32
#define USB_EP2_SIZE               64
#define USB_EP3_SIZE               64
#define USB_ARCH_WRITE_NOTIFY       0

#ifndef USB_ARCH_CONF_DMA
#define USB_ARCH_CONF_DMA           1 /**< Change to Enable/Disable USB DMA */

#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Generic Configuration directives
 *
 * @{
 */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON            0 /**< Energest Module */
#endif

#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        1 /**< Set to 0 to decrease startup verbosity */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name uDMA Configuration and channel allocations
 *
 * @{
 */
#define USB_ARCH_CONF_RX_DMA_CHAN   0 /**< USB -> RAM DMA channel */
#define USB_ARCH_CONF_TX_DMA_CHAN   1 /**< RAM -> USB DMA channel */
#define CC2538_RF_CONF_TX_DMA_CHAN  2 /**< RF -> RAM DMA channel */
#define CC2538_RF_CONF_RX_DMA_CHAN  3 /**< RAM -> RF DMA channel */
#define UDMA_CONF_MAX_CHANNEL       CC2538_RF_CONF_RX_DMA_CHAN
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

#ifndef SLIP_ARCH_CONF_USB
#define SLIP_ARCH_CONF_USB          0 /**< SLIP over UART by default */
#endif

#ifndef CC2538_RF_CONF_SNIFFER_USB
#define CC2538_RF_CONF_SNIFFER_USB  0 /**< Sniffer out over UART by default */
#endif

#ifndef DBG_CONF_USB
#define DBG_CONF_USB                0 /**< All debugging over UART by default */
#endif

#ifndef SERIAL_LINE_CONF_UART
#define SERIAL_LINE_CONF_UART       0 /**< UART to use with serial line */
#endif

#if !SLIP_ARCH_CONF_USB
#ifndef SLIP_ARCH_CONF_UART
#define SLIP_ARCH_CONF_UART         0 /**< UART to use with SLIP */
#endif
#endif

#if !CC2538_RF_CONF_SNIFFER_USB
#ifndef CC2538_RF_CONF_SNIFFER_UART
#define CC2538_RF_CONF_SNIFFER_UART 0 /**< UART to use with sniffer */
#endif
#endif

#if !DBG_CONF_USB
#ifndef DBG_CONF_UART
#define DBG_CONF_UART               0 /**< UART to use for debugging */
#endif
#endif

#ifndef UART1_CONF_UART
#define UART1_CONF_UART             0 /**< UART to use for examples relying on
                                           the uart1_* API */
#endif

/**
 * \brief Configure the device to act as a sniffer.
 *
 * When set, the radio turns off address filtering and sends all captured
 * frames down a peripheral (UART or USB, depending on the value of
 * CC2538_RF_CONF_SNIFFER_USB).
 */
#ifndef CC2538_RF_CONF_SNIFFER
#define CC2538_RF_CONF_SNIFFER      0
#endif

#ifndef CC1200_CONF_SNIFFER
#define CC1200_CONF_SNIFFER         0
#endif /* CC1200_CONF_SNIFFER */

/**
 * \brief Define this as 1 to build a headless node.
 *
 * The UART will not be initialised its clock will be gated, offering some
 * energy savings. The USB will not be initialised either.
 */
#ifndef CC2538_CONF_QUIET
#define CC2538_CONF_QUIET           0
#endif

/* CC2538_CONF_QUIET is hard and overrides all other related defines */
#if CC2538_CONF_QUIET
#undef USB_SERIAL_CONF_ENABLE
#define USB_SERIAL_CONF_ENABLE      0

#undef UART_CONF_ENABLE
#define UART_CONF_ENABLE            0

#undef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        0

/* Little sanity check: We can't have quiet sniffers */
#if CC2538_RF_CONF_SNIFFER
#error "CC2538_RF_CONF_SNIFFER == 1 and CC2538_CONF_QUIET == 1"
#error "These values are conflicting. Please set either to 0"
#endif
#endif /* CC2538_CONF_QUIET */

/**
 * \brief If not defined by the user, the USB Core will automatically only be
 * enabled if it is needed by other enabled components.
 */
#ifndef USB_SERIAL_CONF_ENABLE
#define USB_SERIAL_CONF_ENABLE \
  ((SLIP_ARCH_CONF_USB & SLIP_ARCH_CONF_ENABLED) | \
   DBG_CONF_USB | \
   (CC2538_RF_CONF_SNIFFER & CC2538_RF_CONF_SNIFFER_USB))
#endif

/*
 * If debugging and SLIP use the same peripheral, this will be 1. Don't modify
 * this
 */
#if SLIP_ARCH_CONF_ENABLED
#define DBG_CONF_SLIP_MUX (SLIP_ARCH_CONF_USB == DBG_CONF_USB && \
                           (SLIP_ARCH_CONF_USB || \
                            SLIP_ARCH_CONF_UART == DBG_CONF_UART))
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
/* board.h assumes that basic configuration is done */
#include "board.h"
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/**
 * \name Radio selection
 *
 * The main CC2538 processor drives the 2.4GHz antenna, for Sub-GHz operation we
 * use an additional CC1200 chip that the CC2538 controls via SPI. Only one of
 * these configurations can be operated at a time. To enable the CC1200 and
 * disable the CC2538 radio core, set PLATFORM_CONF_USE_CC1200 to 1.
 * @{
 */
/*
 * The CC1200 driver offers a test mode, see cc1200.c. Enabling that settles the
 * question which radio chip to use.
 */
#if CC1200_RF_TESTMODE
#undef PLATFORM_CONF_USE_CC1200
#define PLATFORM_CONF_USE_CC1200        1
#endif

#ifdef PLATFORM_CONF_USE_CC1200
#define PLATFORM_USE_CC1200             PLATFORM_CONF_USE_CC1200
#else
#define PLATFORM_USE_CC1200             0
#endif
/*---------------------------------------------------------------------------*/
#if PLATFORM_USE_CC1200
#define NETSTACK_CONF_RADIO             cc1200_driver
#else /* PLATFORM_USE_CC1200 */
#define NETSTACK_CONF_RADIO             cc2538_rf_driver
#endif /* PLATFORM_USE_CC1200 */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 *  \name RF configuration for the CC1200 radio chip
 * @{
 */
/*---------------------------------------------------------------------------*/
/** \brief Frequency fine-tuning.
 *
 * Please refer to equations 27 and 28 in the user guide and default values in
 * the 802154g configuration file of the CC1200. This offset slightly raises the
 * frequency to balance out crystal inaccuracies under load and was determined
 * by measurement. Please don't touch.
 */
#define CC1200_CONF_FREQ_OFFSET         691

#ifndef CC1200_CONF_USE_GPIO2
#define CC1200_CONF_USE_GPIO2           1
#endif /* #CC1200_CONF_USE_GPIO2 */

/*
 * We go with a size that is 2 bytes smaller than the FIFO, 2 bytes being the
 * size of the Packet header in 802.15.4g. Could support up to 2047 bytes
 * because the platform has all the necessary interrupt lines connected to keep
 * the FIFO from overflowing, but that can be very hard on RAM as all the
 * necessary software buffers have to be increased to accommodate the packets.
 */
#ifndef CC1200_CONF_MAX_PAYLOAD_LEN
#define CC1200_CONF_MAX_PAYLOAD_LEN     126
#endif /* CC1200_CONF_MAX_PAYLOAD_LEN */

#define PACKETBUF_CONF_SIZE             (CC1200_CONF_MAX_PAYLOAD_LEN + 2)

#ifndef CC1200_CONF_802154G
#define CC1200_CONF_802154G             1
#endif /* #CC1200_CONF_802154G */

#ifndef CC1200_CONF_TX_LEDS
#define CC1200_CONF_TX_LEDS             LEDS_GREEN
#endif /* CC1200_CONF_TX_LEDS */

#ifndef CC1200_CONF_RX_LEDS
#define CC1200_CONF_RX_LEDS             LEDS_RED
#endif /* CC1200_CONF_TX_LEDS */

#ifndef CC1200_CONF_DEFAULT_CHANNEL
#define CC1200_CONF_DEFAULT_CHANNEL     25
#endif /* CC1200_CONF_DEFAULT_CHANNEL */

/** @} */
/*---------------------------------------------------------------------------*/
/**
 *  \name RF configuration for the CC2538 radio core
 * @{
 */
/*---------------------------------------------------------------------------*/
#ifndef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL          25
#endif

#ifndef CC2538_RF_CONF_AUTOACK
/* RF H/W generates ACKs */
#define CC2538_RF_CONF_AUTOACK          1
#endif

#ifndef CC2538_RF_CONF_TX_USE_DMA
/* RF TX over DMA */
#define CC2538_RF_CONF_TX_USE_DMA       1
#endif

#ifndef CC2538_RF_CONF_RX_USE_DMA
/* RF RX over DMA */
#define CC2538_RF_CONF_RX_USE_DMA       1
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Network Stack Configuration
 *
 * @{
 */
/*---------------------------------------------------------------------------*/
#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID                   0xABCD
#endif

#ifndef NETSTACK_CONF_NETWORK
#if NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_NETWORK                   sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK                   rime_driver
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* NETSTACK_CONF_NETWORK */

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC                       csma_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC                       contikimac_driver
#endif

/* Configure NullRDC for when it's selected in project-conf.h */
#define NULLRDC_802154_AUTOACK                  1
#define NULLRDC_802154_AUTOACK_HW               1

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8
#endif

#ifndef NETSTACK_CONF_FRAMER
#if NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_FRAMER                    framer_802154
#else /* NETSTACK_CONF_WITH_IPV6 */
#define NETSTACK_CONF_FRAMER                    contikimac_framer
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* NETSTACK_CONF_FRAMER */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Contikimac configuration
 *
 * If the CC1200 is selected, we use considerably higher values for all
 * variables that control Contikimac's timing. That is to account for the slower
 * transmit rate of 50 kbps on the 868MHz band.
 * @{
 */
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 1
/*---------------------------------------------------------------------------*/
#if PLATFORM_USE_CC1200
/*---------------------------------------------------------------------------*/
#define WITH_FAST_SLEEP                         0

/* Default in contikimac.c is fine, just listed for documentation. */
/* #ifndef CONTIKIMAC_CONF_CYCLE_TIME */
/* #define CONTIKIMAC_CONF_CYCLE_TIME              RTIMER_ARCH_SECOND/8 */
/* #endif *//* CONTIKIMAC_CONF_CYCLE_TIME */

#ifndef CONTIKIMAC_CONF_CCA_CHECK_TIME
#define CONTIKIMAC_CONF_CCA_CHECK_TIME          RTIMER_ARCH_SECOND / 1000
#endif /* CONTIKIMAC_CONF_CCA_CHECK_TIME */

#ifndef CONTIKIMAC_CONF_CCA_SLEEP_TIME
#define CONTIKIMAC_CONF_CCA_SLEEP_TIME          RTIMER_ARCH_SECOND / 190
#endif /* CONTIKIMAC_CONF_CCA_SLEEP_TIME */

#ifndef CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED
#define CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED  RTIMER_ARCH_SECOND / 20
#endif /* CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED */

/**
 * The CC1200 driver automatically sends auto ack per default. Contikimac's own
 * autoacks are not necessary.
 */
#ifndef CONTIKIMAC_CONF_SEND_SW_ACK
#define CONTIKIMAC_CONF_SEND_SW_ACK             0
#endif /* CONTIKIMAC_CONF_SEND_SW_ACK */

#ifndef CONTIKIMAC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#define CONTIKIMAC_CONF_AFTER_ACK_DETECTED_WAIT_TIME RTIMER_ARCH_SECOND / 400
#endif /* CONTIKIMAC_CONF_AFTER_ACK_DETECTED_WAIT_TIME */

#ifndef CONTIKIMAC_CONF_INTER_PACKET_INTERVAL
#define CONTIKIMAC_CONF_INTER_PACKET_INTERVAL   RTIMER_ARCH_SECOND / 200
#endif /* CONTIKIMAC_CONF_INTER_PACKET_INTERVAL */

#ifndef CONTIKIMAC_CONF_INTER_PACKET_DEADLINE
#define CONTIKIMAC_CONF_INTER_PACKET_DEADLINE   RTIMER_ARCH_SECOND / 20
#endif /* CONTIKIMAC_CONF_INTER_PACKET_DEADLINE */

#ifndef CONTIKIMAC_CONF_GUARD_TIME
#define CONTIKIMAC_CONF_GUARD_TIME              RTIMER_ARCH_SECOND / 12
#endif /* CONTIKIMAC_CONF_GUARD_TIME */

#ifndef CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME
#define CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME   RTIMER_ARCH_SECOND / 10
#endif /* CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME */

#define NULLRDC_CONF_ACK_WAIT_TIME                (RTIMER_SECOND / 400)
#define NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME (RTIMER_SECOND / 1000)
/*---------------------------------------------------------------------------*/
#else /* PLATFORM_USE_CC1200 */
/*---------------------------------------------------------------------------*/

#ifndef CONTIKIMAC_CONF_SEND_SW_ACK
#define CONTIKIMAC_CONF_SEND_SW_ACK             !CC2538_RF_CONF_AUTOACK
#endif /* CONTIKIMAC_CONF_SEND_SW_ACK */

#ifndef CONTIKIMAC_CONF_INTER_PACKET_INTERVAL
#define CONTIKIMAC_CONF_INTER_PACKET_INTERVAL   RTIMER_ARCH_SECOND / 2100
#endif /* CONTIKIMAC_CONF_INTER_PACKET_INTERVAL */

#ifndef CONTIKIMAC_CONF_GUARD_TIME
#define CONTIKIMAC_CONF_GUARD_TIME              RTIMER_ARCH_SECOND / 80
#endif /* CONTIKIMAC_CONF_GUARD_TIME */

#ifndef CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME
#define CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME   RTIMER_ARCH_SECOND / 40
#endif /* CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME */
/*---------------------------------------------------------------------------*/
#endif /* PLATFORM_USE_CC1200 */
/*---------------------------------------------------------------------------*/
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name LPM configuration
 * @{
 */
#ifndef LPM_CONF_ENABLE
#define LPM_CONF_ENABLE       1 /**< Set to 0 to disable LPM entirely */
#endif

/**
 * \brief Maximum PM
 *
 * The SoC will never drop to a Power Mode deeper than the one specified here.
 * 0 for PM0, 1 for PM1 and 2 for PM2
 */
#ifndef LPM_CONF_MAX_PM
#define LPM_CONF_MAX_PM       1
#endif

#ifndef LPM_CONF_STATS
#define LPM_CONF_STATS        1 /**< Set to 1 to enable LPM-related stats */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name IEEE address configuration
 *
 * Used to generate our RIME & IPv6 address
 * @{
 */
/**
 * \brief Location of the IEEE address: 0 => Read from InfoPage,
 * 1 => Use a hardcoded address, configured by IEEE_ADDR_CONF_ADDRESS
 */
#ifndef IEEE_ADDR_CONF_HARDCODED
#define IEEE_ADDR_CONF_HARDCODED             0
#endif

/**
 * \brief The hardcoded IEEE address to be used when IEEE_ADDR_CONF_HARDCODED
 * is defined as 1
 */
#ifndef IEEE_ADDR_CONF_ADDRESS
#define IEEE_ADDR_CONF_ADDRESS { 0x00, 0x12, 0x4B, 0x00, 0x89, 0xAB, 0xCD, 0xEF }
#endif

/**
 * \brief Location of the IEEE address in the InfoPage when
 * IEEE_ADDR_CONF_HARDCODED is defined as 0:
 * 0 => Use the primary address location
 * 1 => Use the secondary address location
 */
#ifndef IEEE_ADDR_CONF_USE_SECONDARY_LOCATION
#define IEEE_ADDR_CONF_USE_SECONDARY_LOCATION 0
#endif
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name IPv6, RIME and network buffer configuration
 *
 * @{
 */

/* Don't let contiki-default-conf.h decide if we are an IPv6 build */
#ifndef NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_WITH_IPV6                 0
#endif

#if NETSTACK_CONF_WITH_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define LINKADDR_CONF_SIZE                      8
#define UIP_CONF_LL_802154                      1
#define UIP_CONF_LLH_LEN                        0
#define UIP_CONF_NETIF_MAX_ADDRESSES            3

/* TCP, UDP, ICMP */
#ifndef UIP_CONF_TCP
#define UIP_CONF_TCP                            1
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS                        64
#endif
#define UIP_CONF_UDP                            1
#define UIP_CONF_UDP_CHECKSUMS                  1
#define UIP_CONF_ICMP6                          1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                         1
#endif

#define UIP_CONF_ND6_SEND_RA                    0
#define UIP_CONF_IP_FORWARD                     0
#define RPL_CONF_STATS                          0
#define RPL_CONF_MAX_DAG_ENTRIES                1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF                             rpl_mrhof
#endif

#define UIP_CONF_ND6_REACHABLE_TIME             600000
#define UIP_CONF_ND6_RETRANS_TIMER              10000

#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES                     32
#endif
#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS            32
#endif

/* uIP */
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE                    1300
#endif

#define UIP_CONF_IPV6_QUEUE_PKT                 0
#define UIP_CONF_IPV6_CHECKS                    1
#define UIP_CONF_IPV6_REASSEMBLY                0
#define UIP_CONF_MAX_LISTENPORTS                8

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD   63
#endif
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#endif
#define SICSLOWPAN_CONF_MAXAGE                  8

/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       1
#ifndef SICSLOWPAN_CONF_ADDR_CONTEXT_0
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 {                   \
    addr_contexts[0].prefix[0] = UIP_DS6_DEFAULT_PREFIX_0; \
    addr_contexts[0].prefix[1] = UIP_DS6_DEFAULT_PREFIX_1; \
}
#endif
/*---------------------------------------------------------------------------*/
#else /* NETSTACK_CONF_WITH_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                     1

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE                    108
#endif

#define RIME_CONF_NO_POLITE_ANNOUCEMENTS        0

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                       8
#endif

#endif /* NETSTACK_CONF_WITH_IPV6 */
/** @} */

/*---------------------------------------------------------------------------*/

#endif /* CONTIKI_CONF_H_ */

/** @} */
