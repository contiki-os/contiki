/* -*- C -*- */
/* @(#)$Id: contiki-conf.h,v 1.71 2010/02/08 22:12:29 adamdunkels Exp $ */

#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H



/* Specifies the default MAC driver */
#define MAC_CONF_CSMA               1

#define XMAC_CONF_COMPOWER          1
#define CXMAC_CONF_COMPOWER         1


#if WITH_UIP6
#define MAC_CONF_DRIVER             cxmac_driver
#define MAC_CONF_CHANNEL_CHECK_RATE 8
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 0
#define CXMAC_CONF_ANNOUNCEMENTS    0
#define XMAC_CONF_ANNOUNCEMENTS     0
#else /* WITH_UIP6 */
#define MAC_CONF_DRIVER             xmac_driver
#define MAC_CONF_CHANNEL_CHECK_RATE 4
#define TIMESYNCH_CONF_ENABLED 1
#define CC2420_CONF_TIMESTAMPS 1
#define CC2420_CONF_CHECKSUM   0
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS 1
#define XMAC_CONF_ANNOUNCEMENTS     1
#define CXMAC_CONF_ANNOUNCEMENTS    1
#endif /* WITH_UIP6 */

#define QUEUEBUF_CONF_NUM          16

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif /* RF_CHANNEL */

#define IEEE802154_CONF_PANID       0xABCD

#define SHELL_VARS_CONF_RAM_BEGIN 0x1100
#define SHELL_VARS_CONF_RAM_END 0x2000

/* DCO speed resynchronization for more robust UART, etc. */
#define DCOSYNCH_CONF_ENABLED 1
#define DCOSYNCH_CONF_PERIOD 30


#define CFS_CONF_OFFSET_TYPE	long

#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 1

#define HAVE_STDINT_H
#define MSP430_MEMCPY_WORKAROUND 1
#include "msp430def.h"

#define ELFLOADER_CONF_TEXT_IN_ROM 0
#define ELFLOADER_CONF_DATAMEMORY_SIZE 0x400
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x800

#define IRQ_PORT1 0x01
#define IRQ_PORT2 0x02
#define IRQ_ADC   0x03

#define CCIF
#define CLIF

#define CC_CONF_INLINE inline

#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES 32

#define TMOTE_SKY 1
#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1
/*#define PROCESS_CONF_FASTPOLL    4*/

/* CPU target speed in Hz */
#define F_CPU 3900000uL /*2457600uL*/

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 128

#define BAUD2UBR(baud) ((F_CPU/baud))

#ifdef WITH_UIP6

#define RIMEADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER			0
#endif

#define UIP_CONF_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         1
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      4
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#define UIP_CONF_BUFFER_SIZE		240

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_CONF_COMPRESSION_HC01
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC	1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#else /* WITH_UIP6 */
#define UIP_CONF_IP_FORWARD      1
#define UIP_CONF_BUFFER_SIZE     108
#endif /* WITH_UIP6 */

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_RECEIVE_WINDOW  48
#define UIP_CONF_TCP_MSS         48
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

#define UIP_CONF_TCP_SPLIT       0

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */

/* LED ports */
#define LEDS_PxDIR P5DIR
#define LEDS_PxOUT P5OUT
#define LEDS_CONF_RED    0x10
#define LEDS_CONF_GREEN  0x20
#define LEDS_CONF_YELLOW 0x40

/* Button sensors. */
#define IRQ_PORT2 0x02

typedef unsigned short uip_stats_t;
typedef unsigned short clock_time_t;

typedef unsigned long off_t;
#define ROM_ERASE_UNIT_SIZE  512
#define XMEM_ERASE_UNIT_SIZE (64*1024L)

/* Use the first 64k of external flash for node configuration */
#define NODE_ID_XMEM_OFFSET     (0 * XMEM_ERASE_UNIT_SIZE)

/* Use the second 64k of external flash for codeprop. */
#define EEPROMFS_ADDR_CODEPROP  (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_XMEM_CONF_OFFSET    (2 * XMEM_ERASE_UNIT_SIZE)
#define CFS_XMEM_CONF_SIZE      (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_RAM_CONF_SIZE 4096

/*
 * SPI bus configuration for the TMote Sky.
 */

/* SPI input/output registers. */
#define SPI_TXBUF U0TXBUF
#define SPI_RXBUF U0RXBUF

				/* USART0 Tx ready? */
#define	SPI_WAITFOREOTx() while ((U0TCTL & TXEPT) == 0)
				/* USART0 Rx ready? */
#define	SPI_WAITFOREORx() while ((IFG1 & URXIFG0) == 0)
				/* USART0 Tx buffer ready? */
#define SPI_WAITFORTxREADY() while ((IFG1 & UTXIFG0) == 0)

#define SCK            1  /* P3.1 - Output: SPI Serial Clock (SCLK) */
#define MOSI           2  /* P3.2 - Output: SPI Master out - slave in (MOSI) */
#define MISO           3  /* P3.3 - Input:  SPI Master in - slave out (MISO) */

/*
 * SPI bus - M25P80 external flash configuration.
 */

#define FLASH_PWR	3	/* P4.3 Output */
#define FLASH_CS	4	/* P4.4 Output */
#define FLASH_HOLD	7	/* P4.7 Output */

/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  ( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() ( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()		( P4OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()		( P4OUT |=  BV(FLASH_HOLD) )

/*
 * SPI bus - CC2420 pin configuration.
 */

#define FIFO_P         0  /* P1.0 - Input: FIFOP from CC2420 */
#define FIFO           3  /* P1.3 - Input: FIFO from CC2420 */
#define CCA            4  /* P1.4 - Input: CCA from CC2420 */

#define SFD            1  /* P4.1 - Input:  SFD from CC2420 */
#define CSN            2  /* P4.2 - Output: SPI Chip Select (CS_N) */
#define VREG_EN        5  /* P4.5 - Output: VREG_EN to CC2420 */
#define RESET_N        6  /* P4.6 - Output: RESET_N to CC2420 */

/* Pin status. */

#define FIFO_IS_1       (!!(P1IN & BV(FIFO)))
#define CCA_IS_1        (!!(P1IN & BV(CCA) ))
#define RESET_IS_1      (!!(P4IN & BV(RESET_N)))
#define VREG_IS_1       (!!(P4IN & BV(VREG_EN)))
#define FIFOP_IS_1      (!!(P1IN & BV(FIFO_P)))
#define SFD_IS_1        (!!(P4IN & BV(SFD)))

/* The CC2420 reset pin. */
#define SET_RESET_INACTIVE()    ( P4OUT |=  BV(RESET_N) )
#define SET_RESET_ACTIVE()      ( P4OUT &= ~BV(RESET_N) )

/* CC2420 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       ( P4OUT |=  BV(VREG_EN) )
#define SET_VREG_INACTIVE()     ( P4OUT &= ~BV(VREG_EN) )

/* CC2420 rising edge trigger for external interrupt 0 (FIFOP). */
#define FIFOP_INT_INIT() do {\
  P1IES &= ~BV(FIFO_P);\
  CLEAR_FIFOP_INT();\
} while (0)

/* FIFOP on external interrupt 0. */
#define ENABLE_FIFOP_INT()          do { P1IE |= BV(FIFO_P); } while (0)
#define DISABLE_FIFOP_INT()         do { P1IE &= ~BV(FIFO_P); } while (0)
#define CLEAR_FIFOP_INT()           do { P1IFG &= ~BV(FIFO_P); } while (0)

/* Enables/disables CC2420 access to the SPI bus (not the bus).
 *
 * These guys should really be renamed but are compatible with the
 * original Chipcon naming.
 *
 * SPI_CC2420_ENABLE/SPI_CC2420_DISABLE???
 * CC2420_ENABLE_SPI/CC2420_DISABLE_SPI???
 */

#define SPI_ENABLE()    ( P4OUT &= ~BV(CSN) ) /* ENABLE CSn (active low) */
#define SPI_DISABLE()   ( P4OUT |=  BV(CSN) ) /* DISABLE CSn (active low) */
#define SPI_IS_ENABLED()   ( (P4OUT & BV(CSN)) != BV(CSN) )

#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */



#endif /* CONTIKI_CONF_H */
