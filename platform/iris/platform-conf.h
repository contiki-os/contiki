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
#define PLATFORM PLATFORM_AVR

#define HARWARE_REVISION IRIS

/* Clock ticks per second */
#define CLOCK_CONF_SECOND 128


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


#define NETSTACK_CONF_RADIO   rf230_driver


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

#define CSN            0

#endif /* __PLATFORM_CONF_H__ */
