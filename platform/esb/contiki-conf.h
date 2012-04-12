#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define PLATFORM_HAS_LEDS    1
#define PLATFORM_HAS_BUTTON  1
#define PLATFORM_HAS_BATTERY 1

/* DCO speed resynchronization for more robust UART, etc. */
#define DCOSYNCH_CONF_ENABLED 1
#define DCOSYNCH_CONF_PERIOD 30

#define SERIAL_LINE_CONF_BUFSIZE 64

#define TIMESYNCH_CONF_ENABLED 0
#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 1

#define HAVE_STDINT_H
#include "msp430def.h"


#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 0

/* CPU target speed in Hz */
#define F_CPU 2457600uL

/* Our clock resolution, this is the same as Unix HZ. Must be a power
   of two (see clock.c for details). */
#define CLOCK_CONF_SECOND 64UL

#define NODE_ID_EEPROM_OFFSET     0x0010 /* - 0x0014 */
#define CFS_EEPROM_CONF_OFFSET    0x0040

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1

#define CC_CONF_VA_ARGS                1

#define CCIF
#define CLIF

typedef unsigned long clock_time_t;

#define LOG_CONF_ENABLED 0

#define PACKETBUF_CONF_ATTRS_INLINE 1
#define NETSTACK_CONF_RADIO   tr1001_driver

#if WITH_UIP
/* Network setup for IPv4 */

#define NETSTACK_CONF_NETWORK uip_driver
#define NETSTACK_CONF_MAC     nullmac_driver
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_CONF_FRAMER  framer_nullmac

#define QUEUEBUF_CONF_NUM     0
#define QUEUEBUF_CONF_REF_NUM 0
#define ROUTE_CONF_ENTRIES    0

#else /* WITH_UIP */

/* Network setup for non-IPv4 (rime). */

#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC     nullmac_driver
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_CONF_FRAMER  framer_nullmac

#define QUEUEBUF_CONF_NUM 1
#define QUEUEBUF_CONF_REF_NUM 1
#define ROUTE_CONF_ENTRIES 4

#endif /* WITH_UIP */

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_IP_FORWARD      1
#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_BUFFER_SIZE     110
#define UIP_CONF_RECEIVE_WINDOW  (UIP_CONF_BUFFER_SIZE - 40)
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 4
#define UIP_CONF_UDP_CONNS       3
#define UIP_CONF_FWCACHE_SIZE    1
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0
#define UIP_CONF_RESOLV_ENTRIES  1

#define UIP_CONF_TCP_SPLIT       0

#define LOADER_CONF_ARCH "loader/loader-arch.h"

#define ELFLOADER_CONF_TEXT_IN_ROM 1
#define ELFLOADER_CONF_DATAMEMORY_SIZE 100
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x1000

#define WEBSERVER_CONF_CGI_CONNS 1

/* LEDs ports. */
#define LEDS_PxDIR P2DIR
#define LEDS_PxOUT P2OUT
#define LEDS_CONF_RED    0x01
#define LEDS_CONF_GREEN  0x02
#define LEDS_CONF_YELLOW 0x04

#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* __CONTIKI_CONF_H__ */
