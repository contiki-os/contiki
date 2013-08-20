#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include<stdint.h>

/* Platform name, type, and MCU clock rate */
#define PLATFORM_NAME  "STK500"
#define PLATFORM_TYPE  STK500
#ifndef F_CPU
#define F_CPU          8000000UL
#endif

#define CCIF
#define CLIF

/* The AVR tick interrupt usually is done with an 8 bit counter around 128 Hz.
 * 125 Hz needs slightly more overhead during the interrupt, as does a 32 bit
 * clock_time_t.
 */
 /* Clock ticks per second */
#define CLOCK_CONF_SECOND 125
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

#define SLIP_PORT 0

#if UIP_CONF_IPV6
#define RIMEADDR_CONF_SIZE        8
#define UIP_CONF_ICMP6            1
#define UIP_CONF_UDP              1
#define UIP_CONF_TCP              1
//#define UIP_CONF_IPV6_RPL         0

/* See uip-ds6.h */
#define NBR_TABLE_CONF_MAX_NEIGHBORS      20
#define UIP_CONF_DS6_DEFRT_NBU    2
#define UIP_CONF_DS6_PREFIX_NBU   3
#define UIP_CONF_MAX_ROUTES    20
#define UIP_CONF_DS6_ADDR_NBU     3
#define UIP_CONF_DS6_MADDR_NBU    0
#define UIP_CONF_DS6_AADDR_NBU    0

#define NETSTACK_CONF_NETWORK     sicslowpan_driver
#define NETSTACK_CONF_MAC         nullmac_driver
#define SICSLOWPAN_CONF_COMPRESSION SICSLOWPAN_COMPRESSION_HC06
#define NETSTACK_CONF_RDC         sicslowmac_driver
#define NETSTACK_CONF_FRAMER      framer_802154
#define NETSTACK_CONF_RADIO       rf230_driver
#define CHANNEL_802_15_4          26
/* AUTOACK receive mode gives better rssi measurements, even if ACK is never requested */
#define RF230_CONF_AUTOACK        1
/* Request 802.15.4 ACK on all packets sent (else autoretry). This is primarily for testing. */
#define SICSLOWPAN_CONF_ACK_ALL   0
/* Number of auto retry attempts 0-15 (0 implies don't use extended TX_ARET_ON mode with CCA) */
#define RF230_CONF_AUTORETRIES    2
#define SICSLOWPAN_CONF_FRAG      1
/* Most browsers reissue GETs after 3 seconds which stops fragment reassembly so a longer MAXAGE does no good */
#define SICSLOWPAN_CONF_MAXAGE    3
/* How long to wait before terminating an idle TCP connection. Smaller to allow faster sleep. Default is 120 seconds */
#define UIP_CONF_WAIT_TIMEOUT     5

#else
/* ip4 should build but is largely untested */
#define RIMEADDR_CONF_SIZE        2
#define NETSTACK_CONF_NETWORK     rime_driver
#endif /* UIP_CONF_IPV6 */

typedef unsigned short uip_stats_t;

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef int8_t s8_t;
typedef uint16_t u16_t;
typedef int16_t s16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;

#endif /* __CONTIKI_CONF_H__ */
