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
 *         Configuration for RZRAVEN USB stick "jackdaw"
 *
 * \author
 *         Simon Barner <barner@in.tum.de>
 *         David Kopf <dak664@embarqmail.com>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* ************************************************************************** */
//#pragma mark Basic Configuration
/* ************************************************************************** */

/* MCU and clock rate */
#define MCU_MHZ 8
#define PLATFORM PLATFORM_AVR
#define RAVEN_REVISION	 RAVENUSB_C

/* Clock ticks per second */
#define CLOCK_CONF_SECOND 125

/* Mac address, RF channel, PANID from EEPROM settings manager */
/* Generate random MAC address on first startup */
/* Random number from radio clock skew or ADC noise */
#define JACKDAW_CONF_USE_SETTINGS		1
#define JACKDAW_CONF_RANDOM_MAC         0
#define RNG_CONF_USE_RADIO_CLOCK	1
//#define RNG_CONF_USE_ADC	1

/* Since clock_time_t is 16 bits, maximum interval is 524 seconds */
#define RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME CLOCK_CONF_SECOND * 524UL /*Default uses 600*/

/* Maximum time interval (used for timers) */
#define INFINITE_TIME 0xffff

/* COM port to be used for SLIP connection */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256

/* Use the following address for code received via the codeprop
 * facility
 */
#define EEPROMFS_ADDR_CODEPROP 0x8000

/* Use Atmel 'Route Under MAC', currently just in sniffer mode! */
//#define UIP_CONF_USE_RUM  1

#define CCIF
#define CLIF

/* ************************************************************************** */
//#pragma mark USB Ethernet Hooks
/* ************************************************************************** */

#ifndef USB_ETH_HOOK_IS_READY_FOR_INBOUND_PACKET
#if RF230BB
#define	USB_ETH_HOOK_IS_READY_FOR_INBOUND_PACKET()		rf230_is_ready_to_send()
#else
static inline uint8_t radio_is_ready_to_send_() {
	switch(radio_get_trx_state()) {
		case BUSY_TX:
		case BUSY_TX_ARET:
			return 0;
	}
	return 1;
}
#define	USB_ETH_HOOK_IS_READY_FOR_INBOUND_PACKET()		radio_is_ready_to_send_()
#endif
#endif

#ifndef USB_ETH_HOOK_HANDLE_INBOUND_PACKET
#define USB_ETH_HOOK_HANDLE_INBOUND_PACKET(buffer,len)	do { uip_len = len ; mac_ethernetToLowpan(buffer); } while(0)
#endif

#ifndef USB_ETH_HOOK_SET_PROMISCIOUS_MODE
#if RF230BB
#define USB_ETH_HOOK_SET_PROMISCIOUS_MODE(value)	rf230_set_promiscuous_mode(value)
#else		
#define USB_ETH_HOOK_SET_PROMISCIOUS_MODE(value)	radio_set_trx_state(value?RX_ON:RX_AACK_ON)
#endif
#endif

#ifndef USB_ETH_HOOK_INIT
#define USB_ETH_HOOK_INIT()		mac_ethernetSetup()
#endif

/* ************************************************************************** */
//#pragma mark RF230BB Hooks
/* ************************************************************************** */

//#define RF230BB_HOOK_RADIO_OFF()	Led1_off()
//#define RF230BB_HOOK_RADIO_ON()		Led1_on()
#define RF230BB_HOOK_TX_PACKET(buffer,total_len) mac_log_802_15_4_tx(buffer,total_len)
#define RF230BB_HOOK_RX_PACKET(buffer,total_len) mac_log_802_15_4_rx(buffer,total_len)
#define	RF230BB_HOOK_IS_SEND_ENABLED()	mac_is_send_enabled()
extern bool mac_is_send_enabled(void);
extern void mac_log_802_15_4_tx(const uint8_t* buffer, size_t total_len);
extern void mac_log_802_15_4_rx(const uint8_t* buffer, size_t total_len);


/* ************************************************************************** */
//#pragma mark USB CDC-ACM (UART) Hooks
/* ************************************************************************** */

#define USB_CDC_ACM_HOOK_TX_END(char)			vcptx_end_led()
#define USB_CDC_ACM_HOOK_CLS_CHANGED(state)		vcptx_end_led()
#define USB_CDC_ACM_HOOK_CONFIGURED()			vcptx_end_led()

/* ************************************************************************** */
//#pragma mark UIP Settings
/* ************************************************************************** */

#define UIP_CONF_LL_802154       1
#define UIP_CONF_LLH_LEN         14
#define UIP_CONF_BUFSIZE		UIP_LINK_MTU + UIP_LLH_LEN + 4   // +4 for vlan on macosx

#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 4

#define UIP_CONF_IP_FORWARD      0
#define UIP_CONF_FWCACHE_SIZE    0

#define UIP_CONF_IPV6            1
#define UIP_CONF_IPV6_CHECKS     1
#define UIP_CONF_IPV6_QUEUE_PKT  0
#define UIP_CONF_IPV6_REASSEMBLY 0
#define UIP_CONF_NETIF_MAX_ADDRESSES  3
#define UIP_CONF_ND6_MAX_PREFIXES     3
#define UIP_CONF_ND6_MAX_NEIGHBORS    4
#define UIP_CONF_ND6_MAX_DEFROUTERS   2

#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1

#define UIP_CONF_TCP             1
#define UIP_CONF_TCP_SPLIT       1

#define UIP_CONF_STATISTICS      1

/* ************************************************************************** */
//#pragma mark Serial Port Settings
/* ************************************************************************** */
/* Set USB_CONF_MACINTOSH to prefer CDC-ECM+DEBUG enumeration for Mac/Linux 
 * Leave undefined to prefer RNDIS+DEBUG enumeration for Windows/Linux
 * TODO:Serial port would enumerate in all cases and prevent falling through to
 * the supported network interface if USB_CONF_MACINTOSH is used with Windows
 * or vice versa. The Mac configuration is set up to still enumerate as RNDIS-ONLY
 * on Windows (without the serial port). 
 * At present the Windows configuration will not enumerate on the Mac at all,
 * since it wants a custom descriptor for USB composite devices.
 */ 
#define USB_CONF_MACINTOSH 0

/* Set USB_CONF_SERIAL to enable the USB serial port that allows control of the
 * run-time configuration (COMx on Windows, ttyACMx on Linux, tty.usbmodemx on Mac)
 * Debug printfs will go to this port unless USB_CONF_RS232 is set.
 */
#define USB_CONF_SERIAL          1
 
/* RS232 debugs have less effect on network timing and are less likely
 * to be dropped due to buffer overflow. Only tx is implemented at present.
 * The tx pad is the middle one behind the jackdaw leds.
 * RS232 output will work with or without enabling the USB serial port
 */
#define USB_CONF_RS232           0

/* Disable mass storage enumeration for more program space */
//#define USB_CONF_STORAGE         1   /* TODO: Mass storage is currently broken */
 
/* ************************************************************************** */
//#pragma mark RIME Settings
/* ************************************************************************** */

#define RIMEADDR_CONF_SIZE       8

/* ************************************************************************** */
//#pragma mark SICSLOWPAN Settings
/* ************************************************************************** */

#define SICSLOWPAN_CONF_COMPRESSION       SICSLOWPAN_COMPRESSION_HC06
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS 2
#ifdef RF230BB
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC    1   //for barebones driver, sicslowpan calls radio->read function

/* ************************************************************************** */
//#pragma mark NETSTACK Settings
/* ************************************************************************** */

#if 1       /* Network setup */
/* No radio cycling */
#define NETSTACK_CONF_NETWORK     sicslowpan_driver
#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         sicslowmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define RF230_CONF_AUTOACK        1
#define RF230_CONF_AUTORETRIES    2
#define QUEUEBUF_CONF_NUM         1
#define QUEUEBUF_CONF_REF_NUM     1
#define SICSLOWPAN_CONF_FRAG      1
#define SICSLOWPAN_CONF_MAXAGE    5

#elif 0
/* Contiki-mac radio cycling */
#define NETSTACK_CONF_NETWORK     sicslowpan_driver
#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         contikimac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define RF230_CONF_AUTOACK        0
#define RF230_CONF_AUTORETRIES    0

#elif 1
/* cx-mac radio cycling */
#define NETSTACK_CONF_NETWORK     sicslowpan_driver
#define NETSTACK_CONF_MAC         nullmac_driver
#define NETSTACK_CONF_RDC         cxmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define RF230_CONF_AUTOACK        0
#define RF230_CONF_AUTORETRIES    0
#define MAC_CONF_CHANNEL_CHECK_RATE 8
#define SICSLOWPAN_CONF_FRAG      1
#define QUEUEBUF_CONF_NUM         3
#define QUEUEBUF_CONF_REF_NUM     1
#define SICSLOWPAN_CONF_MAXAGE    5
//following gives 50% duty cycle, undef for default 5%
#define CXMAC_CONF_ON_TIME (RTIMER_ARCH_SECOND / 16)

#else
#error Network configuration not specified!
#endif   /* Network setup */


/* ************************************************************************** */
//#pragma mark RPL Settings
/* ************************************************************************** */

#define UIP_CONF_IPV6_RPL               0

#if UIP_CONF_IPV6_RPL

/* Not completely working yet. Link local pings work but address prefixes do not get assigned */
/* RPL requires the uip stack. Change #CONTIKI_NO_NET=1 to UIP_CONF_IPV6=1 in the examples makefile,
or include the needed source files in /plaftorm/avr-ravenusb/Makefile.avr-ravenusb */

#define UIP_CONF_ROUTER  1
#define RPL_CONF_STATS    0
#define PROCESS_CONF_NO_PROCESS_NAMES  0
#undef UIP_CONF_TCP            //TCP needed to serve RPL neighbor web page
#define UIP_CONF_TCP             1
//#undef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
#undef UIP_CONF_MAX_CONNECTIONS
#define UIP_CONF_MAX_CONNECTIONS 1
//#undef UIP_CONF_MAX_LISTENPORTS
//#define UIP_CONF_MAX_LISTENPORTS 10
//#define UIP_CONF_BUFFER_SIZE 256
#define UIP_CONF_TCP_MSS         512

#if 0  //too much RAM!
/* Handle 10 neighbors */
#define UIP_CONF_DS6_NBR_NBU     10
/* Handle 10 routes    */
#define UIP_CONF_DS6_ROUTE_NBU   10

#define UIP_CONF_ND6_SEND_RA		0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000
#undef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT         1
//#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      4
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#define UIP_CONF_BUFFER_SIZE		240
#define UIP_CONF_ICMP_DEST_UNREACH 1
#define UIP_CONF_DHCP_LIGHT

#undef UIP_CONF_LLH_LEN
#define UIP_CONF_LLH_LEN         0
//#define UIP_CONF_RECEIVE_WINDOW  48
//#define UIP_CONF_TCP_MSS         48
#undef UIP_CONF_UDP_CONNS
#define UIP_CONF_UDP_CONNS       12
#undef UIP_CONF_FWCACHE_SIZE
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0
#endif
#endif /* UIP_CONF_IPV6_RPL */
#endif /* RF230BB */

/* ************************************************************************** */
//#pragma mark Other Settings
/* ************************************************************************** */

/* Route-Under-MAC uses 16-bit short addresses */
#if UIP_CONF_USE_RUM
#undef  UIP_CONF_LL_802154
#define UIP_DATA_RUM_OFFSET      5
#endif

#include <stdint.h>

typedef int32_t s32_t;
typedef unsigned short clock_time_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef unsigned short uip_stats_t;
typedef unsigned long off_t;

void clock_delay(unsigned int us2);
void clock_wait(int ms10);
void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);

#endif /* __CONTIKI_CONF_H__ */
