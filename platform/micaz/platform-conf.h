/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: platform-conf.h,v 1.1 2010/06/23 10:25:54 joxe Exp $
 */

/**
 * \file
 *         A brief description of what this file is
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */
 /* Platform name, type, and MCU clock rate */
#define PLATFORM_NAME  "MicaZ"
#define PLATFORM_TYPE  MICAZ
#ifndef F_CPU
#define F_CPU          7372800UL
#endif

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

/* LED ports */
#define LEDS_PxDIR DDRA // port direction register
#define LEDS_PxOUT PORTA // port register
#define LEDS_CONF_RED    0x04 //red led
#define LEDS_CONF_GREEN  0x02 // green led
#define LEDS_CONF_YELLOW 0x01 // yellow led

/* COM port to be used for SLIP connection */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256

/* Use the following address for code received via the codeprop
 * facility
 */
#define EEPROMFS_ADDR_CODEPROP 0x8000

#define EEPROM_NODE_ID_START 0x00


#define NETSTACK_CONF_RADIO   cc2420_driver


/*
 * SPI bus configuration for the TMote Sky.
 */

/* SPI input/output registers. */
#define SPI_TXBUF SPDR
#define SPI_RXBUF SPDR

#define BV(bitno) _BV(bitno)

#define SPI_WAITFOREOTx() do { while (!(SPSR & BV(SPIF))); } while (0)
#define SPI_WAITFOREORx() do { while (!(SPSR & BV(SPIF))); } while (0)

#define SCK            1  /* - Output: SPI Serial Clock (SCLK) - ATMEGA128 PORTB, PIN1 */
#define MOSI           2  /* - Output: SPI Master out - slave in (MOSI) - ATMEGA128 PORTB, PIN2 */
#define MISO           3  /* - Input:  SPI Master in - slave out (MISO) - ATMEGA128 PORTB, PIN3 */

/*
 * SPI bus - M25P80 external flash configuration.
 */

#define FLASH_PWR       3       /* P4.3 Output */
#define FLASH_CS        4       /* P4.4 Output */
#define FLASH_HOLD      7       /* P4.7 Output */

/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  ( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() ( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()                ( P4OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()              ( P4OUT |=  BV(FLASH_HOLD) )

/*
 * SPI bus - CC2420 pin configuration.
 */

#define CC2420_CONF_SYMBOL_LOOP_COUNT 500

/*
 * SPI bus - CC2420 pin configuration.
 */

#define FIFO_P         6
#define FIFO           7
#define CCA            6

#define SFD            4
#define CSN            0
#define VREG_EN        5
#define RESET_N        6


/* - Input: FIFOP from CC2420 - ATMEGA128 PORTE, PIN6 */
#define CC2420_FIFOP_PORT(type)   P##type##E
#define CC2420_FIFOP_PIN          6
/* - Input: FIFO from CC2420 - ATMEGA128 PORTB, PIN7 */
#define CC2420_FIFO_PORT(type)     P##type##B
#define CC2420_FIFO_PIN            7
/* - Input: CCA from CC2420 - ATMEGA128 PORTD, PIN6 */
#define CC2420_CCA_PORT(type)      P##type##D
#define CC2420_CCA_PIN             6
/* - Input:  SFD from CC2420 - ATMEGA128 PORTD, PIN4 */
#define CC2420_SFD_PORT(type)      P##type##D
#define CC2420_SFD_PIN             4
/* - Output: SPI Chip Select (CS_N) - ATMEGA128 PORTB, PIN0 */
#define CC2420_CSN_PORT(type)      P##type##B
#define CC2420_CSN_PIN             0
/* - Output: VREG_EN to CC2420 - ATMEGA128 PORTA, PIN5 */
#define CC2420_VREG_PORT(type)     P##type##A
#define CC2420_VREG_PIN            5
/* - Output: RESET_N to CC2420 - ATMEGA128 PORTA, PIN6 */
#define CC2420_RESET_PORT(type)    P##type##A
#define CC2420_RESET_PIN           6

#define CC2420_IRQ_VECTOR INT6_vect

/* Pin status. */
#define CC2420_FIFOP_IS_1 (!!(CC2420_FIFOP_PORT(IN) & BV(CC2420_FIFOP_PIN)))
#define CC2420_FIFO_IS_1  (!!(CC2420_FIFO_PORT(IN) & BV(CC2420_FIFO_PIN)))
#define CC2420_CCA_IS_1   (!!(CC2420_CCA_PORT(IN) & BV(CC2420_CCA_PIN)))
#define CC2420_SFD_IS_1   (!!(CC2420_SFD_PORT(IN) & BV(CC2420_SFD_PIN)))

/* The CC2420 reset pin. */
#define SET_RESET_INACTIVE()   (CC2420_RESET_PORT(ORT) |=  BV(CC2420_RESET_PIN))
#define SET_RESET_ACTIVE()     (CC2420_RESET_PORT(ORT) &= ~BV(CC2420_RESET_PIN))

/* CC2420 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       (CC2420_VREG_PORT(ORT) |=  BV(CC2420_VREG_PIN))
#define SET_VREG_INACTIVE()     (CC2420_VREG_PORT(ORT) &= ~BV(CC2420_VREG_PIN))

/* CC2420 rising edge trigger for external interrupt 6 (FIFOP).
 * Enable the external interrupt request for INT6.
 * See Atmega128 datasheet about EICRB Register
 */
#define CC2420_FIFOP_INT_INIT() do {\
  EICRB |= 0x30; \
  CC2420_CLEAR_FIFOP_INT(); \
} while (0)

/* FIFOP on external interrupt 6. */
#define CC2420_ENABLE_FIFOP_INT()          do { EIMSK |= 0x40; } while (0)
#define CC2420_DISABLE_FIFOP_INT()         do { EIMSK &= ~0x40; } while (0)
#define CC2420_CLEAR_FIFOP_INT()           do { EIFR = 0x40; } while (0)

/*
 * Enables/disables CC2420 access to the SPI bus (not the bus).
 * (Chip Select)
 */
#define CC2420_SPI_ENABLE() (PORTB &= ~BV(CSN)) /* ENABLE CSn (active low) */
#define CC2420_SPI_DISABLE() (PORTB |=  BV(CSN)) /* DISABLE CSn (active low) */

#endif /* __PLATFORM_CONF_H__ */
