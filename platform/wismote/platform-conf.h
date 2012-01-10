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
 */

/**
 * \file
 *         Platform configuration for the wismote platform.
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */

#define PLATFORM_HAS_LEDS   1
#define PLATFORM_HAS_BUTTON 1

/* CPU target speed in Hz */
#define F_CPU 16000000uL

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 128UL
#define RTIMER_CONF_SECOND (4096U*8)

#define BAUD2UBR(baud) (baud)

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#include "msp430def.h"


/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long off_t;

/* the low-level radio driver */
#define NETSTACK_CONF_RADIO   cc2520_driver

#define ROM_ERASE_UNIT_SIZE  512
#define XMEM_ERASE_UNIT_SIZE (64*1024L)


#define CFS_CONF_OFFSET_TYPE    long


/* Use the first 64k of external flash for node configuration */
#define NODE_ID_XMEM_OFFSET     (0 * XMEM_ERASE_UNIT_SIZE)

/* Use the second 64k of external flash for codeprop. */
#define EEPROMFS_ADDR_CODEPROP  (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_XMEM_CONF_OFFSET    (2 * XMEM_ERASE_UNIT_SIZE)
#define CFS_XMEM_CONF_SIZE      (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_RAM_CONF_SIZE 4096

/*
 * SPI bus configuration for the wismote
 */

/* SPI input/output registers. */
#define SPI_TXBUF UCB0TXBUF
#define SPI_RXBUF UCB0RXBUF

                                /* USART0 Tx ready? */
#define SPI_WAITFOREOTx() while ((UCB0STAT & UCBUSY) != 0)
                                /* USART0 Rx ready? */
#define SPI_WAITFOREORx() while ((UCB0IFG & UCRXIFG) == 0)
                                /* USART0 Tx buffer ready? */
#define SPI_WAITFORTxREADY() while ((UCB0IFG & UCTXIFG) == 0)
/*                                 /\* USART0 Tx ready? *\/ */
/* #define SPI_WAITFOREOTx() while (!(UCB0IFG & UCRXIFG)) */
/*                                 /\* USART0 Rx ready? *\/ */
/* #define SPI_WAITFOREORx() while (!(UCB0IFG & UCRXIFG)) */
/*                                 /\* USART0 Tx buffer ready? *\/ */
/* #define SPI_WAITFORTxREADY() while (!(UCB0IFG & UCRXIFG)) */
/* #define SPI_BUSY_WAIT() 		while ((UCB0STAT & UCBUSY) == 1) */

#define MOSI           1  /* P3.1 - Output: SPI Master out - slave in (MOSI) */
#define MISO           2  /* P3.2 - Input:  SPI Master in - slave out (MISO) */
#define SCK            3  /* P3.3 - Output: SPI Serial Clock (SCLK) */
/* #define SCK            1  /\* P3.1 - Output: SPI Serial Clock (SCLK) *\/ */
/* #define MOSI           2  /\* P3.2 - Output: SPI Master out - slave in (MOSI) *\/ */
/* #define MISO           3  /\* P3.3 - Input:  SPI Master in - slave out (MISO) *\/ */

/*
 * SPI bus - M25P80 external flash configuration.
 */

#define FLASH_PWR       //3       /* P4.3 Output */
#define FLASH_CS        //4       /* P4.4 Output */
#define FLASH_HOLD      //7       /* P4.7 Output */

/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  //( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() //( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()               // ( P4OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()              //( P4OUT |=  BV(FLASH_HOLD) )

/*
 * SPI bus - CC2520 pin configuration.
 */

#define CC2520_CONF_SYMBOL_LOOP_COUNT 2604      /* 326us msp430X @ 16MHz */

/* P1.6 - Input: FIFOP from CC2520 */
#define CC2520_FIFOP_PORT(type)    P1##type
#define CC2520_FIFOP_PIN           6
/* P1.5 - Input: FIFO from CC2520 */
#define CC2520_FIFO_PORT(type)     P1##type
#define CC2520_FIFO_PIN            5
/* P1.7 - Input: CCA from CC2520 */
#define CC2520_CCA_PORT(type)      P1##type
#define CC2520_CCA_PIN             7
/* P2.0 - Input:  SFD from CC2520 */
#define CC2520_SFD_PORT(type)      P2##type
#define CC2520_SFD_PIN             0
/* P3.0 - Output: SPI Chip Select (CS_N) */
#define CC2520_CSN_PORT(type)      P3##type
#define CC2520_CSN_PIN             0
/* P4.3 - Output: VREG_EN to CC2520 */
#define CC2520_VREG_PORT(type)     P4##type
#define CC2520_VREG_PIN            3
/* P4.4 - Output: RESET_N to CC2520 */
#define CC2520_RESET_PORT(type)    P4##type
#define CC2520_RESET_PIN           4

#define CC2520_IRQ_VECTOR PORT1_VECTOR

/* Pin status.CC2520 */
#define CC2520_FIFOP_IS_1 (!!(CC2520_FIFOP_PORT(IN) & BV(CC2520_FIFOP_PIN)))
#define CC2520_FIFO_IS_1  (!!(CC2520_FIFO_PORT(IN) & BV(CC2520_FIFO_PIN)))
#define CC2520_CCA_IS_1   (!!(CC2520_CCA_PORT(IN) & BV(CC2520_CCA_PIN)))
#define CC2520_SFD_IS_1   (!!(CC2520_SFD_PORT(IN) & BV(CC2520_SFD_PIN)))

/* The CC2520 reset pin. */
#define SET_RESET_INACTIVE()   (CC2520_RESET_PORT(OUT) |=  BV(CC2520_RESET_PIN))
#define SET_RESET_ACTIVE()     (CC2520_RESET_PORT(OUT) &= ~BV(CC2520_RESET_PIN))

/* CC2520 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       (CC2520_VREG_PORT(OUT) |=  BV(CC2520_VREG_PIN))
#define SET_VREG_INACTIVE()     (CC2520_VREG_PORT(OUT) &= ~BV(CC2520_VREG_PIN))

/* CC2520 rising edge trigger for external interrupt 0 (FIFOP). */
#define CC2520_FIFOP_INT_INIT() do {                  \
    CC2520_FIFOP_PORT(IES) &= ~BV(CC2520_FIFOP_PIN);  \
    CC2520_CLEAR_FIFOP_INT();                         \
  } while(0)

/* FIFOP on external interrupt 0. */
/* FIFOP on external interrupt 0. */
#define CC2520_ENABLE_FIFOP_INT()          do { P1IE |= BV(CC2520_FIFOP_PIN); } while (0)
#define CC2520_DISABLE_FIFOP_INT()         do { P1IE &= ~BV(CC2520_FIFOP_PIN); } while (0)
#define CC2520_CLEAR_FIFOP_INT()           do { P1IFG &= ~BV(CC2520_FIFOP_PIN); } while (0)

/*
 * Enables/disables CC2520 access to the SPI bus (not the bus).
 * (Chip Select)
 */

 /* ENABLE CSn (active low) */
#define CC2520_SPI_ENABLE()     do{ UCB0CTL1 &= ~UCSWRST;  clock_delay(5); P3OUT &= ~BIT0;clock_delay(5);}while(0)
 /* DISABLE CSn (active low) */
#define CC2520_SPI_DISABLE()    do{clock_delay(5);UCB0CTL1 |= UCSWRST;clock_delay(1); P3OUT |= BIT0;clock_delay(5);}while(0)
#define CC2520_SPI_IS_ENABLED() ((CC2520_CSN_PORT(OUT) & BV(CC2520_CSN_PIN)) != BV(CC2520_CSN_PIN))

#endif /* __PLATFORM_CONF_H__ */
