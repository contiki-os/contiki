/*
 * Copyright (c) 2006, Technical University of Munich
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
 * @(#)$$
 */

/**
 * \file
 *         Configuration for Atmel Raven
 *
 * \author
 *         Simon Barner <barner@in.tum.de>
 *         David Kopf <dak664@embarqmail.com>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

/* Platform name, type, and MCU clock rate */
#define PLATFORM_NAME  "Raven"
#define PLATFORM_TYPE  RAVEN_D
#ifndef F_CPU
#define F_CPU          8000000UL
#endif

/* MCU_CONF_LOW_WEAR will remove the signature and eeprom from the .elf file */
/* This reduces reprogramming wear during development */
//#define MCU_CONF_LOW_WEAR 1

#include <stdint.h>

/* The AVR tick interrupt usually is done with an 8 bit counter around 128 Hz.
 * 125 Hz needs slightly more overhead during the interrupt, as does a 32 bit
 * clock_time_t.
 */
/* Clock ticks per second */
#define CLOCK_CONF_SECOND 128
#if 1
/* 16 bit counter overflows every ~10 minutes */
typedef unsigned short clock_time_t;
#define CLOCK_LT(a,b)  ((signed short)((a)-(b)) < 0)
#define INFINITE_TIME 0xffff
#define RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME INFINITE_TIME/CLOCK_CONF_SECOND /* Default uses 600 */
#define COLLECT_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME INFINITE_TIME/CLOCK_CONF_SECOND /* Default uses 600 */
#else
typedef unsigned long clock_time_t;
#define CLOCK_LT(a,b)  ((signed long)((a)-(b)) < 0)
#define INFINITE_TIME 0xffffffff
#endif
/* These routines are not part of the contiki core but can be enabled in cpu/avr/clock.c */
void clock_delay_msec(uint16_t howlong);
void clock_adjust_ticks(clock_time_t howmany);

/* The 1284p can use TIMER2 with the external 32768Hz crystal to keep time. Else TIMER0 is used. */
/* The sleep timer in raven-lcd.c also uses the crystal and adds a TIMER2 interrupt routine if not already define by clock.c */
/* If F_CPU is 0x800000 the clock tick interrupt routine will (attempt to) keep the cpu clock phase locked to the crystal. */
#define AVR_CONF_USE32KCRYSTAL 1

/* Rtimer is implemented through the 16 bit Timer1, clocked at F_CPU through a 1024 prescaler. */
/* This gives 7812 counts per second, 128 microsecond precision and maximum interval 8.388 seconds. */
/* Change clock source and prescaler for greater precision and shorter maximum interval. */
/* 0 will disable the Rtimer code */
//#define RTIMER_ARCH_PRESCALER 256UL /*0, 1, 8, 64, 256, 1024 */

/* COM port to be used for SLIP connection. Not tested on Raven */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
/* Default is 4096. Currently used only when elfloader is present. Not tested on Raven */
//#define MMEM_CONF_SIZE 256

/* Starting address for code received via the codeprop facility. Not tested on Raven */
typedef unsigned long off_t;
//#define EEPROMFS_ADDR_CODEPROP 0x8000

/* RADIO_CONF_CALIBRATE_INTERVAL is used in rf230bb and clock.c. If nonzero a 256 second interval is used */
/* Calibration is automatic when the radio wakes so is not necessary when the radio periodically sleeps */
//#define RADIO_CONF_CALIBRATE_INTERVAL 256

/* RADIOSTATS is used in rf230bb, clock.c and the webserver cgi to report radio usage */
#define RADIOSTATS                1

/* More extensive stats */
#define ENERGEST_CONF_ON          1

/* Packet statistics */
typedef unsigned short uip_stats_t;
#define UIP_STATISTICS            0


/* Possible watchdog timeouts depend on mcu. Default is WDTO_2S. -1 Disables the watchdog. */
/* AVR Studio simulator tends to reboot due to clocking the WD 8 times too fast */
//#define WATCHDOG_CONF_TIMEOUT -1

/* Debugflow macro, useful for tracing path through mac and radio interrupts */
//#define DEBUGFLOWSIZE 128

/* Define MAX_*X_POWER to reduce tx power and ignore weak rx packets for testing a miniature multihop network.
 * Leave undefined for full power and sensitivity.
 * tx=0 (3dbm, default) to 15 (-17.2dbm)
 * RF230_CONF_AUTOACK sets the extended mode using the energy-detect register with rx=0 (-91dBm) to 84 (-7dBm)
 *   else the rssi register is used having range 0 (91dBm) to 28 (-10dBm)
 *   For simplicity RF230_MIN_RX_POWER is based on the energy-detect value and divided by 3 when autoack is not set.
 * On the RF230 a reduced rx power threshold will not prevent autoack if enabled and requested.
 * These numbers applied to both Raven and Jackdaw give a maximum communication distance of about 15 cm
 * and a 10 meter range to a full-sensitivity RF230 sniffer.
#define RF230_MAX_TX_POWER 15
#define RF230_MIN_RX_POWER 30
 */
  /* The rf231 and atmega128rfa1 can use an rssi threshold for triggering rx_busy that saves 0.5ma in rx mode */
/* 1 - 15 maps into -90 to -48 dBm; the register is written with RF230_MIN_RX_POWER/6 + 1. Undefine for -100dBm sensitivity */
//#define RF230_MIN_RX_POWER        0

/* Network setup. The new NETSTACK interface requires RF230BB (as does ip4) */
#if RF230BB
#undef PACKETBUF_CONF_HDR_SIZE                  //Use the packetbuf default for header size
/* TX routine passes the cca/ack result in the return parameter */
#define RDC_CONF_HARDWARE_ACK      1
/* TX routine does automatic cca and optional backoff */
#define RDC_CONF_HARDWARE_CSMA     1
/* Allow MCU sleeping between channel checks */
#define RDC_CONF_MCU_SLEEP         0
#else
#define PACKETBUF_CONF_HDR_SIZE    0            //RF230 combined driver/mac handles headers internally
#endif /*RF230BB */

#if UIP_CONF_IPV6
#define RIMEADDR_CONF_SIZE        8
#define UIP_CONF_ICMP6            1
#define UIP_CONF_UDP              1
#define UIP_CONF_TCP              1
//#define UIP_CONF_IPV6_RPL         0
#define NETSTACK_CONF_NETWORK       sicslowpan_driver
#define SICSLOWPAN_CONF_COMPRESSION SICSLOWPAN_COMPRESSION_HC06
#else
/* ip4 should build but is largely untested */
#define RIMEADDR_CONF_SIZE        2
#define NETSTACK_CONF_NETWORK     rime_driver
#endif /* UIP_CONF_IPV6 */

#define UIP_CONF_LL_802154       1
#define UIP_CONF_LLH_LEN         0

/* 10 bytes per stateful address context - see sicslowpan.c */
/* Default is 1 context with prefix aaaa::/64 */
/* These must agree with all the other nodes or there will be a failure to communicate! */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS 1
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 {addr_contexts[0].prefix[0]=0xaa;addr_contexts[0].prefix[1]=0xaa;}
#define SICSLOWPAN_CONF_ADDR_CONTEXT_1 {addr_contexts[1].prefix[0]=0xbb;addr_contexts[1].prefix[1]=0xbb;}
#define SICSLOWPAN_CONF_ADDR_CONTEXT_2 {addr_contexts[2].prefix[0]=0x20;addr_contexts[2].prefix[1]=0x01;addr_contexts[2].prefix[2]=0x49;addr_contexts[2].prefix[3]=0x78,addr_contexts[2].prefix[4]=0x1d;addr_contexts[2].prefix[5]=0xb1;}

/* Take the default TCP maximum segment size for efficiency and simpler wireshark captures */
/* Use this to prevent 6LowPAN fragmentation (whether or not fragmentation is enabled) */
//#define UIP_CONF_TCP_MSS      48

#define UIP_CONF_IP_FORWARD      0
#define UIP_CONF_FWCACHE_SIZE    0

#define UIP_CONF_IPV6_CHECKS     1
#define UIP_CONF_IPV6_QUEUE_PKT  1
#define UIP_CONF_IPV6_REASSEMBLY 0

#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_DHCP_LIGHT      1

#if 1 /* No radio cycling */

#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         sicslowmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
#define RADIO_CONF_CALIBRATE_INTERVAL 256
/* AUTOACK receive mode gives better rssi measurements, even if ACK is never requested */
#define RF230_CONF_AUTOACK        1
/* Request 802.15.4 ACK on all packets sent (else autoretry). This is primarily for testing. */
#define SICSLOWPAN_CONF_ACK_ALL   0
/* Number of auto retry attempts+1, 1-16. Set zero to disable extended TX_ARET_ON mode with CCA) */
#define RF230_CONF_FRAME_RETRIES    3
/* Number of CSMA attempts 0-7. 802.15.4 2003 standard max is 5. */
#define RF230_CONF_CSMA_RETRIES    5
/* CCA theshold energy -91 to -61 dBm (default -77). Set this smaller than the expected minimum rssi to avoid packet collisions */
/* The Jackdaw menu 'm' command is helpful for determining the smallest ever received rssi */
#define RF230_CONF_CCA_THRES    -85
/* Allow 6lowpan fragments (needed for large TCP maximum segment size) */
#define SICSLOWPAN_CONF_FRAG      1
/* Most browsers reissue GETs after 3 seconds which stops fragment reassembly so a longer MAXAGE does no good */
#define SICSLOWPAN_CONF_MAXAGE    3
/* How long to wait before terminating an idle TCP connection. Smaller to allow faster sleep. Default is 120 seconds */
#define UIP_CONF_WAIT_TIMEOUT     5
/* 211 bytes per queue buffer */
#define QUEUEBUF_CONF_NUM         8
/* 54 bytes per queue ref buffer */
#define QUEUEBUF_CONF_REF_NUM     2
/* Allocate remaining RAM as desired */
/* 30 bytes per TCP connection */
/* 6LoWPAN does not do well with concurrent TCP streams, as new browser GETs collide with packets coming */
/* from previous GETs, causing decreased throughput, retransmissions, and timeouts. Increase to study this. */
/* ACKs to other ports become interleaved with computation-intensive GETs, so ACKs are particularly missed. */
/* Increasing the number of packet receive buffers in RAM helps to keep ACKs from being lost */
#define UIP_CONF_MAX_CONNECTIONS  4
/* 2 bytes per TCP listening port */
#define UIP_CONF_MAX_LISTENPORTS  4
/* 25 bytes per UDP connection */
#define UIP_CONF_UDP_CONNS       10
/* See uip-ds6.h */
#define UIP_CONF_DS6_NBR_NBU      20
#define UIP_CONF_DS6_DEFRT_NBU    2
#define UIP_CONF_DS6_PREFIX_NBU   3
#define UIP_CONF_DS6_ROUTE_NBU    20
#define UIP_CONF_DS6_ADDR_NBU     3
#define UIP_CONF_DS6_MADDR_NBU    0
#define UIP_CONF_DS6_AADDR_NBU    0

#elif 1  /* Contiki-mac radio cycling */
//#define NETSTACK_CONF_MAC         nullmac_driver
/* csma needed for burst mode at present. Webserver won't work without it */
#define NETSTACK_CONF_MAC         csma_driver
#define NETSTACK_CONF_RDC         contikimac_driver
/* Default is two CCA separated by 500 usec */
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE   8
/* Wireshark won't decode with the header, but padded packets will fail ipv6 checksum */
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER 0
/* So without the header this needed for RPL mesh to form */
#define CONTIKIMAC_CONF_SHORTEST_PACKET_SIZE   43-18  //multicast RPL DIS length
/* Not tested much yet */
#define WITH_PHASE_OPTIMIZATION                0
#define CONTIKIMAC_CONF_COMPOWER               1
#define RIMESTATS_CONF_ON                      1
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
/* The radio needs to interrupt during an rtimer interrupt */
#define RTIMER_CONF_NESTED_INTERRUPTS 1
#define RF230_CONF_AUTOACK        1
/* A 0 here means non-extended mode; 1 means extended mode with no retry, >1 for retrys */
#define RF230_CONF_FRAME_RETRIES    1
/* A 0 here means cca but no retry, >1= for backoff retrys */
#define RF230_CONF_CSMA_RETRIES    1
#define SICSLOWPAN_CONF_FRAG      1
#define SICSLOWPAN_CONF_MAXAGE    3
/* 211 bytes per queue buffer. Contikimac burst mode needs 15 for a 1280 byte MTU */
#define QUEUEBUF_CONF_NUM         15
/* 54 bytes per queue ref buffer */
#define QUEUEBUF_CONF_REF_NUM     2
/* Allocate remaining RAM. Not much left due to queuebuf increase  */
#define UIP_CONF_MAX_CONNECTIONS  2
#define UIP_CONF_MAX_LISTENPORTS  2
#define UIP_CONF_UDP_CONNS        4
#define UIP_CONF_DS6_NBR_NBU     10
#define UIP_CONF_DS6_DEFRT_NBU    2
#define UIP_CONF_DS6_PREFIX_NBU   2
#define UIP_CONF_DS6_ROUTE_NBU    4
#define UIP_CONF_DS6_ADDR_NBU     3
#define UIP_CONF_DS6_MADDR_NBU    0
#define UIP_CONF_DS6_AADDR_NBU    0

#elif 1  /* cx-mac radio cycling */
/* RF230 does clear-channel assessment in extended mode (frame retries>0) */
#define RF230_CONF_FRAME_RETRIES    1
#if RF230_CONF_FRAME_RETRIES
#define NETSTACK_CONF_MAC         nullmac_driver
#else
#define NETSTACK_CONF_MAC         csma_driver
#endif
#define NETSTACK_CONF_RDC         cxmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
#define RF230_CONF_AUTOACK        1
#define SICSLOWPAN_CONF_FRAG      1
#define SICSLOWPAN_CONF_MAXAGE    3
#define CXMAC_CONF_ANNOUNCEMENTS  0
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
/* 211 bytes per queue buffer. Burst mode will need 15 for a 1280 byte MTU */
#define QUEUEBUF_CONF_NUM         15
/* 54 bytes per queue ref buffer */
#define QUEUEBUF_CONF_REF_NUM     2
/* Allocate remaining RAM. Not much left due to queuebuf increase  */
#define UIP_CONF_MAX_CONNECTIONS  2
#define UIP_CONF_MAX_LISTENPORTS  4
#define UIP_CONF_UDP_CONNS        5
#define UIP_CONF_DS6_NBR_NBU      4
#define UIP_CONF_DS6_DEFRT_NBU    2
#define UIP_CONF_DS6_PREFIX_NBU   3
#define UIP_CONF_DS6_ROUTE_NBU    4
#define UIP_CONF_DS6_ADDR_NBU     3
#define UIP_CONF_DS6_MADDR_NBU    0
#define UIP_CONF_DS6_AADDR_NBU    0
//Below gives 10% duty cycle, undef for default 5%
//#define CXMAC_CONF_ON_TIME (RTIMER_ARCH_SECOND / 80)
//Below gives 50% duty cycle
//#define CXMAC_CONF_ON_TIME (RTIMER_ARCH_SECOND / 16)


#else
#error Network configuration not specified!
#endif   /* Network setup */

/* Logging adds 200 bytes to program size */
#define LOG_CONF_ENABLED         1

/* ************************************************************************** */
//#pragma mark RPL Settings
/* ************************************************************************** */
#if UIP_CONF_IPV6_RPL

#define UIP_CONF_ROUTER                 1
#define UIP_CONF_ND6_SEND_RA		    0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000
/* For slow slip connections, to prevent buffer overruns */
//#define UIP_CONF_RECEIVE_WINDOW 300
#undef UIP_CONF_FWCACHE_SIZE
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

#endif /* RPL */

#define CCIF
#define CLIF

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* __CONTIKI_CONF_H__ */
