/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 *         Configuration for MICAz platform.
 *
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define HAVE_STDINT_H
#include "avrdef.h"


/*
 * MCU and clock rate.
 * MICAZ runs on 7.3728 MHz clock.
 */
#define MCU_MHZ 7

#define PLATFORM PLATFORM_AVR

/* Clock ticks per second */
#define CLOCK_CONF_SECOND 128

/* COM port to be used for SLIP connection */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256

/* Use the following address for code received via the codeprop
 * facility
 */
#define EEPROMFS_ADDR_CODEPROP 0x8000

#define EEPROM_NODE_ID_START 0x00

#define TIMESYNCH_CONF_ENABLED 1
#define CC2420_CONF_TIMESTAMPS 1

#define WITH_NULLMAC 0

#define CCIF
#define CLIF


#ifdef WITH_UIP6

#define RIMEADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         1
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      4
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICS_LOWPAN_CONF_COMPRESSION_HC1
#define SICSLOWPAN_CONF_FRAG                    0
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC	1
#else
#define UIP_CONF_IP_FORWARD      1
#endif /* WITH_UIP6 */

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_BUFFER_SIZE     110
#define UIP_CONF_RECEIVE_WINDOW  (UIP_CONF_BUFFER_SIZE - 40)
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

#define UIP_CONF_TCP_SPLIT       0





/* LEDs ports. */
#define LEDS_PxDIR DDRA // port direction register
#define LEDS_PxOUT PORTA // port register
#define LEDS_CONF_RED    0x04 //red led
#define LEDS_CONF_GREEN  0x02 // green led
#define LEDS_CONF_YELLOW 0x01 // yellow led


/*
 * SPI bus configuration for the MicaZ.
 */

/* SPI input/output registers. */
#define SPI_TXBUF SPDR
#define SPI_RXBUF SPDR

#define BV(bitno) _BV(bitno)

#define	SPI_WAITFOREOTx() do { while (!(SPSR & BV(SPIF))); } while (0)
#define	SPI_WAITFOREORx() do { while (!(SPSR & BV(SPIF))); } while (0)

#define SCK            1  /* - Output: SPI Serial Clock (SCLK) - ATMEGA128 PORTB, PIN1 */
#define MOSI           2  /* - Output: SPI Master out - slave in (MOSI) - ATMEGA128 PORTB, PIN2 */
#define MISO           3  /* - Input:  SPI Master in - slave out (MISO) - ATMEGA128 PORTB, PIN3 */

/*
 * SPI bus - CC2420 pin configuration.
 */

#define FIFO_P         6  /* - Input: FIFOP from CC2420 - ATMEGA128 PORTE, PIN6 */
#define FIFO           7  /* - Input: FIFO from CC2420 - ATMEGA128 PORTB, PIN7 */
#define CCA            6  /* - Input: CCA from CC2420 - ATMEGA128 PORTD, PIN6 */

#define SFD            4  /* - Input:  SFD from CC2420 - ATMEGA128 PORTD, PIN4 */
#define CSN            0  /* - Output: SPI Chip Select (CS_N) - ATMEGA128 PORTB, PIN0 */
#define VREG_EN        5  /* - Output: VREG_EN to CC2420 - ATMEGA128 PORTA, PIN5 */
#define RESET_N        6  /* - Output: RESET_N to CC2420 - ATMEGA128 PORTA, PIN6 */

/* Pin status. */

#define FIFO_IS_1       (!!(PINB & BV(FIFO)))
#define CCA_IS_1        (!!(PIND & BV(CCA) ))
#define RESET_IS_1      (!!(PINA & BV(RESET_N)))
#define VREG_IS_1       (!!(PINA & BV(VREG_EN)))
#define FIFOP_IS_1      (!!(PINE & BV(FIFO_P)))
#define SFD_IS_1        (!!(PIND & BV(SFD)))

/* The CC2420 reset pin. */
#define SET_RESET_INACTIVE()    ( PORTA |= BV(RESET_N) )
#define SET_RESET_ACTIVE()      ( PORTA &= ~BV(RESET_N) )

/* CC2420 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       ( PORTA |= BV(VREG_EN) )
#define SET_VREG_INACTIVE()     ( PORTA &= ~BV(VREG_EN) )

/* CC2420 rising edge trigger for external interrupt 6 (FIFOP).
 * Enable the external interrupt request for INT6.
 * See Atmega128 datasheet about EICRB Register
 */
#define FIFOP_INT_INIT() do {\
  EICRB |= 0x30; \
  CLEAR_FIFOP_INT(); \
} while (0)

/* FIFOP on external interrupt 6. */
#define ENABLE_FIFOP_INT()          do { EIMSK |= 0x40; } while (0)
#define DISABLE_FIFOP_INT()         do { EIMSK &= ~0x40; } while (0)
#define CLEAR_FIFOP_INT()           do { EIFR = 0x40; } while (0)

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

typedef unsigned short clock_time_t;
typedef unsigned short uip_stats_t;
typedef unsigned long off_t;

void clock_delay(unsigned int us2);
void clock_wait(int ms10);
void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);

#endif /* __CONTIKI_CONF_H__ */
