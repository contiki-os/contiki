#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#define HAVE_STDINT_H
#include "msp430def.h"

#define WITH_SD				0

#define NETSTACK_CONF_RADIO		cc1020_driver
#define NETSTACK_CONF_RDC		lpp_driver
#define NETSTACK_CONF_MAC		csma_driver
#define NETSTACK_CONF_NETWORK		rime_driver
#define NETSTACK_CONF_FRAMER		framer_nullmac

#define MAC_CONF_CHANNEL_CHECK_RATE	8

#define ENERGEST_CONF_ON		1

#define IRQ_PORT1			0x01
#define IRQ_PORT2			0x02
#define IRQ_ADC				0x03

/* MSP430 information memory */
#define INFOMEM_START			0x1000
#define INFOMEM_BLOCK_SIZE		128
#define INFOMEM_NODE_ID			0x0000 /* - 0x0004 */

#define CC_CONF_REGISTER_ARGS		1
#define CC_CONF_FUNCTION_POINTER_ARGS	1
#define CC_CONF_INLINE			inline
#define CC_CONF_VA_ARGS			1

#define LPP_CONF_LISTEN_TIME		2
#define LPP_CONF_OFF_TIME		(CLOCK_SECOND - (LPP_CONF_LISTEN_TIME))
#define QUEUEBUF_CONF_NUM		4


#define CCIF
#define CLIF

/* Clear channel assessment timeout for sending with the CC1020 radio. (ms) */
#define CC1020_CONF_CCA_TIMEOUT		10

/* Clock */
typedef unsigned clock_time_t;
#define CLOCK_CONF_SECOND		64
#define F_CPU				2457600uL /* CPU target speed in Hz. */

#define BAUD2UBR(baud)			(F_CPU/(baud))

#include "ctk/ctk-vncarch.h"

#define LOG_CONF_ENABLED		0

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef uint16_t uip_stats_t;

typedef int bool;
#define	TRUE				1
#define FALSE 				0

#define UIP_CONF_ICMP_DEST_UNREACH	1
#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN		0
#define UIP_CONF_BUFFER_SIZE		116
#define UIP_CONF_RECEIVE_WINDOW		(UIP_CONF_BUFFER_SIZE - 40)
#define UIP_CONF_MAX_CONNECTIONS	4
#define UIP_CONF_MAX_LISTENPORTS	8
#define UIP_CONF_UDP_CONNS		8
#define UIP_CONF_FWCACHE_SIZE		20
#define UIP_CONF_BROADCAST		1
#define UIP_ARCH_IPCHKSUM		1
#define UIP_CONF_UDP_CHECKSUMS		1
#define UIP_CONF_PINGADDRCONF		0
#define UIP_CONF_LOGGING		0

#define LOADER_CONF_ARCH "loader/loader-arch.h"

#define ELFLOADER_CONF_DATAMEMORY_SIZE	100
#define ELFLOADER_CONF_TEXTMEMORY_SIZE	0x1000

/* LEDs ports MSB430 */
#define LEDS_PxDIR P5DIR
#define LEDS_PxOUT P5OUT
#define LEDS_CONF_RED			0x80
#define LEDS_CONF_GREEN			0x00
#define LEDS_CONF_YELLOW		0x00

#endif /* !CONTIKI_CONF_H */
