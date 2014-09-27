#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <stdint.h>

#define CCIF
#define CLIF

typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long bool_t;
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;

#define CLOCK_CONF_SECOND 64

#define CFS_CONF_OFFSET_TYPE  long

#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

/* Tell example applications that there are leds on board that can be controlled from software. */
#define PLATFORM_HAS_LEDS    1
/* Radio RSSI sensor using values from rf230 driver */
#define PLATFORM_HAS_RADIO   1
/* Battery voltage sensor using Mulle voltage API */
#define PLATFORM_HAS_BATTERY 1

#define BAUD2UBR(b) b

#define LEDS_CONF_RED (1 << 15)
#define LEDS_CONF_GREEN (1 << 13)
#define LEDS_CONF_YELLOW (1 << 14)

#if WITH_UIP6
#ifndef NETSTACK_CONF_NETWORK
#define NETSTACK_CONF_NETWORK       sicslowpan_driver
#endif /* NETSTACK_CONF_NETWORK */
#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC           csma_driver
#endif /* NETSTACK_CONF_MAC */
#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC           nullrdc_driver
#endif /* NETSTACK_CONF_RDC */
#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER        framer_802154
#endif /* NETSTACK_CONF_FRAMER */
#ifndef NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO         rf230_driver
#endif /* NETSTACK_CONF_RADIO */

#define SICSLOWPAN_CONF_MAXAGE      1
#define RF230_CONF_RX_BUFFERS       10
#define RF230_CONF_AUTOACK          0
#define RF230_CONF_AUTORETRIES      3
#define LINKADDR_CONF_SIZE          8

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE        1514
#endif /* UIP_CONF_BUFFER_SIZE */

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM           16
#endif /* QUEUEBUF_CONF_NUM */

#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                 1
#endif /* UIP_CONF_ROUTER */

#ifndef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL               1
#endif /* UIP_CONF_IPV6_RPL */

#define SICSLOWPAN_CONF_COMPRESSION_IPV6    0
#define SICSLOWPAN_CONF_COMPRESSION_HC1     1
#define SICSLOWPAN_CONF_COMPRESSION_HC01    2
#define SICSLOWPAN_CONF_COMPRESSION         SICSLOWPAN_CONF_COMPRESSION_HC01
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS   0
#define SICSLOWPAN_CONF_FRAG                1

#define UIP_CONF_UDP                1
#define UIP_CONF_UDP_CHECKSUMS      1

/* configure number of neighbors and routes */
#define UIP_CONF_DS6_NBR_NBU        15
#define UIP_CONF_DS6_ROUTE_NBU      15
#define NEIGHBOR_CONF_MAX_NEIGHBORS 15

#ifndef UIP_CONF_ND6_SEND_RA
#define UIP_CONF_ND6_SEND_RA            0
#endif
#ifndef UIP_CONF_ND6_REACHABLE_TIME
/* RFC 4861 default value (REACHABLE_TIME): 30000 ms */
#define UIP_CONF_ND6_REACHABLE_TIME 600000
#endif
#ifndef UIP_CONF_ND6_RETRANS_TIMER
/* RFC 4861 default value (RETRANS_TIMER): 1000 ms */
#define UIP_CONF_ND6_RETRANS_TIMER  10000
#endif

#define UIP_CONF_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         1
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      5
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0

#define UIP_CONF_ICMP_DEST_UNREACH  1
#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LL_802154          1

#ifndef UIP_CONF_LLH_LEN
#define UIP_CONF_LLH_LEN            0
#endif /* UIP_CONF_LLH_LEN */

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW     60
#endif /* UIP_CONF_RECEIVE_WINDOW */

#define UIP_CONF_TCP_MSS            48
#define UIP_CONF_MAX_CONNECTIONS    4
#define UIP_CONF_MAX_LISTENPORTS    8
#define UIP_CONF_UDP_CONNS          15
#define UIP_CONF_FWCACHE_SIZE       15
#define UIP_CONF_BROADCAST          1
#define UIP_CONF_UDP                1
#define UIP_CONF_UDP_CHECKSUMS      1
#define UIP_CONF_PINGADDRCONF       0
#define UIP_CONF_LOGGING            0

#define UIP_CONF_TCP_SPLIT          1
#ifndef UIP_CONF_TCP
#define UIP_CONF_TCP                0
#endif

#endif /* WITH_UIP6 */

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL                  0
#endif /* RF_CHANNEL */

#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID       0x777
#endif /* IEEE802154_CONF_PANID */

#ifndef COAP_MAX_OPEN_TRANSACTIONS
/* Multiplies with chunk size, be aware of memory constraints. */
#define COAP_MAX_OPEN_TRANSACTIONS   11
#endif /* COAP_MAX_OPEN_TRANSACTIONS */

/* Must be <= open transaction number. */
#define COAP_MAX_OBSERVERS      (COAP_MAX_OPEN_TRANSACTIONS - 1)

/* Tell Contiki that we are not using PETSCII (retarded default) */
#define WITH_ASCII 1

#endif /* __CONTIKI_CONF_H__ */
