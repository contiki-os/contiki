/* -*- C -*- */
/* @(#)$Id: contiki-conf.h,v 1.5 2010/06/22 06:39:44 joxe Exp $ */

#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include <avr/interrupt.h>

#define HAVE_STDINT_H
#include "avrdef.h"

/* #define CB_GATEWAY */

#define CCIF
#define CLIF

#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES 32

void clock_delay(unsigned int us2);
void clock_wait(int ms10);
void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);

#define WITH_UIP 1
#define WITH_ASCII 1

#define PROCESS_CONF_FASTPOLL    4

/* Our clock resolution, this is the same as Unix HZ. */
/* Fix clock.c XXX/bg */
#define CLOCK_CONF_SECOND 62

/* CPU target speed in Hz */
#define F_CPU 8000000L

/* The +1 and divide by 2 is to achieve rounding. */
#define BAUD2UBR(baud) ((u16_t)((F_CPU/baud/8uL + 1)/2 - 1))

/* UART configs */
//#define MCU_MHZ 8
//#define MCU atmega128
//#define SLIP_PORT RS232_PORT_1


#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_BUFFER_SIZE     116
#define UIP_CONF_RECEIVE_WINDOW  (UIP_CONF_BUFFER_SIZE - 40)
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
//#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */

#ifdef CB_GATEWAY
/* LED port E */
#define LEDS_CONF_GREEN  _BV(2)	/* PE.2 - Output */
#define LEDS_CONF_YELLOW _BV(3)	/* PE.3 - Output */
#else
#define LEDS_ORANGE 8
/* LED port B */
#define LEDS_CONF_ORANGE _BV(4)	/* PB.4 - Output */
#define LEDS_CONF_GREEN  _BV(7) /* PB.7 - Output */
/* LED port E */
#define LEDS_CONF_RED    _BV(3)	/* PE.3 - Output */
#define LEDS_CONF_YELLOW _BV(4)	/* PE.4 - Output */
#endif

typedef u16_t uip_stats_t;
typedef u16_t clock_time_t;

typedef u32_t off_t;
#define ROM_ERASE_UNIT_SIZE  SPM_PAGESIZE
#define XMEM_ERASE_UNIT_SIZE 8

/* Use the first 64k of external flash for codeprop. */
#define EEPROMFS_ADDR_CODEPROP  (128 * XMEM_ERASE_UNIT_SIZE)

#define CC2420_RADIO
/* LOOP count for waiting 20 symbols in the CC2420 code */
#define CC2420_CONF_SYMBOL_LOOP_COUNT 500
/*
 * SPI bus configuration for the CC2420DBK.
 */

/* SPI input/output registers. */
#define SPI_TXBUF SPDR
#define SPI_RXBUF SPDR

#define BV(bitno) _BV(bitno)

#define	SPI_WAITFOREOTx() do { while (!(SPSR & BV(SPIF))); } while (0)
#define	SPI_WAITFOREORx() do { while (!(SPSR & BV(SPIF))); } while (0)

#define SCK            1  /* - Output: SPI Serial Clock (SCLK) */
#define MOSI           2  /* - Output: SPI Master out - slave in (MOSI) */
#define MISO           3  /* - Input:  SPI Master in - slave out (MISO) */

/*
 * SPI bus - CC2420 pin configuration.
 */

#define FIFO_P         0  /* - Input: FIFOP from CC2420 */
#define FIFO           1  /* - Input: FIFO from CC2420 */
#define CCA            6  /* - Input: CCA from CC2420 */

#define SFD            4  /* - Input:  SFD from CC2420 */
#define CSN            0  /* - Output: SPI Chip Select (CS_N) */
#define VREG_EN        5  /* - Output: VREG_EN to CC2420 */
#define RESET_N        6  /* - Output: RESET_N to CC2420 */

/* Pin status. */

#define FIFO_IS_1       (!!(PIND & BV(FIFO)))
#define CCA_IS_1        (!!(PIND & BV(CCA) ))
#define RESET_IS_1      (!!(PINB & BV(RESET_N)))
#define VREG_IS_1       (!!(PINB & BV(VREG_EN)))
#define FIFOP_IS_1      (!!(PIND & BV(FIFO_P)))
#define SFD_IS_1        (!!(PIND & BV(SFD)))

/* The CC2420 reset pin. */
#define SET_RESET_INACTIVE()    ( PORTB |= BV(RESET_N) )    
#define SET_RESET_ACTIVE()      ( PORTB &= ~BV(RESET_N) )

/* CC2420 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       ( PORTB |= BV(VREG_EN) )
#define SET_VREG_INACTIVE()     ( PORTB &= ~BV(VREG_EN) )

/* CC2420 rising edge trigger for external interrupt 0 (FIFOP). */
#define FIFOP_INT_INIT() do {\
  EICRA |= 0x03; \
  CLEAR_FIFOP_INT(); \
} while (0)

/* FIFOP on external interrupt 0. */
#define ENABLE_FIFOP_INT()          do { EIMSK |= 0x01; } while (0)
#define DISABLE_FIFOP_INT()         do { EIMSK &= ~0x01; } while (0)
#define CLEAR_FIFOP_INT()           do { EIFR = 0x01; } while (0)

/* Enables/disables CC2420 access to the SPI bus (not the bus).
 *
 * These guys should really be renamed but are compatible with the
 * original Chipcon naming.
 *
 * SPI_CC2420_ENABLE/SPI_CC2420_DISABLE???
 * CC2420_ENABLE_SPI/CC2420_DISABLE_SPI???
 */

#define SPI_ENABLE()    ( PORTB &= ~BV(CSN) ) /* ENABLE CSn (active low) */
#define SPI_DISABLE()   ( PORTB |=  BV(CSN) ) /* DISABLE CSn (active low) */

#endif /* CONTIKI_CONF_H */
