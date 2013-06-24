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
 */

/**
 * \file
 *         A brief description of what this file is
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */
#define TYNDALL 1

/* CPU target speed in Hz */
#define F_CPU 8000000uL // 8MHz by default

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 128UL

#define BAUD2UBR(baud) ((F_CPU/baud))

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#define MSP430_MEMCPY_WORKAROUND 1
#include "msp430def.h"


/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long off_t;

/* the low-level radio driver */
#if (EXP5438_RFEM_CC2520EMK - 0)
#define NETSTACK_CONF_RADIO   cc2520_driver
#else
#define NETSTACK_CONF_RADIO   cc2420_driver
#endif

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */

/* DCO speed resynchronization for more robust UART, etc. */
/* Not needed from MSP430x5xx since it make use of the FLL */
#define DCOSYNCH_CONF_ENABLED 0
#define DCOSYNCH_CONF_PERIOD 30

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
 * SPI bus configuration for the TMote Sky.
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

#define MOSI           1  /* P3.1 - Output: SPI Master out - slave in (MOSI) */
#define MISO           2  /* P3.2 - Input:  SPI Master in - slave out (MISO) */
#define SCK            3  /* P3.3 - Output: SPI Serial Clock (SCLK) */

/*
 * SPI bus - M25P80 external flash configuration.
 */
//#define FLASH_PWR     3       /* P4.3 Output */ ALWAYS POWERED ON Z1
#define FLASH_CS        4       /* P4.4 Output */
#define FLASH_HOLD      7       /* P5.7 Output */

/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  ( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() ( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()                ( P5OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()              ( P5OUT |=  BV(FLASH_HOLD) )

/* RF Evaluation Module Header pin-to-MCU mapping.
 *
 * Different modules associate different functions with the RF pins.
 * These macros access the primary headers RF1 and RF2; the eZ-RF
 * header RF3 is currently not described here. */
#define RFEM_RF1_3_PORT(type)  P1##type
#define RFEM_RF1_3_PIN         4
#define RFEM_RF1_5_PORT(type)  P1##type
#define RFEM_RF1_5_PIN         2
#define RFEM_RF1_6_PORT(type)  P1##type
#define RFEM_RF1_6_PIN         5
#define RFEM_RF1_7_PORT(type)  P1##type
#define RFEM_RF1_7_PIN         5
#define RFEM_RF1_8_PORT(type)  P1##type
#define RFEM_RF1_8_PIN         6
#define RFEM_RF1_9_PORT(type)  P1##type
#define RFEM_RF1_9_PIN         6
#define RFEM_RF1_10_PORT(type) P7##type
#define RFEM_RF1_10_PIN        6
#define RFEM_RF1_12_PORT(type) P1##type
#define RFEM_RF1_12_PIN        3
#define RFEM_RF1_14_PORT(type) P3##type
#define RFEM_RF1_14_PIN        0
#define RFEM_RF1_16_PORT(type) P3##type
#define RFEM_RF1_16_PIN        3
#define RFEM_RF1_18_PORT(type) P3##type
#define RFEM_RF1_18_PIN        1
#define RFEM_RF1_20_PORT(type) P3##type
#define RFEM_RF1_20_PIN        2
#define RFEM_RF2_15_PORT(type) P1##type
#define RFEM_RF2_15_PIN        2
#define RFEM_RF2_18_PORT(type) P8##type
#define RFEM_RF2_18_PIN        1
#define RFEM_RF2_19_PORT(type) P8##type
#define RFEM_RF2_19_PIN        2

/* MSP430 headers define OUT as a macro with the value used in timer
 * capture/compare control registers to output the timer signal.  This
 * use interferes with the double macro expansion of CCFOO(OUT) passed
 * to the RFEM_RFX_Y_PORT(TYPE) macros above. */
#undef OUT

#ifndef EXP5438_RFEM_CC2520EMK
/** Define to true preprocessor value to indicate that the RFEM header
 * has a <a href="http://www.ti.com/tool/cc2520emk">CC2520 Evaluation
 * Module</a> plugged into it.  This sets the CC2520 as the radio
 * layer for Contiki on this platform. */
#define EXP5438_RFEM_CC2520EMK 0
#endif /* EXP5438_RFEM_CC2520EMK */

#if (EXP5438_RFEM_CC2520EMK - 0)

/* Default fifop signal on GPIO2 @ RF1.9.  This is P1.6, and we need
 * its interrupt. */
#define CC2520_FIFOP_PORT(type) RFEM_RF1_9_PORT(type)
#define CC2520_FIFOP_PIN RFEM_RF1_9_PIN
/* Default fifo signal on GPIO1 @ RF1.7 */
#define CC2520_FIFO_PORT(type) RFEM_RF1_7_PORT(type)
#define CC2520_FIFO_PIN RFEM_RF1_7_PIN
/* Default cca signal on GPIO3 @ RF1.12 */
#define CC2520_CCA_PORT(type) RFEM_RF1_12_PORT(type)
#define CC2520_CCA_PIN RFEM_RF1_12_PIN
/* Default sfd signal on GPIO4 @ RF2.18 */
#define CC2520_SFD_PORT(type) RFEM_RF2_18_PORT(type)
#define CC2520_SFD_PIN RFEM_RF2_18_PIN
/* SFD on P8.1 maps to TA0.CCI1B.  Record the CCIS; SFD can't actually
 * be captured until sfd-arch-sfd.c is updated to support TA0 as well
 * as TB0. */
#define CC2520_SFD_CCIS (1 * CCIS0)
/* CSn on RF1.14 */
#define CC2520_CSN_PORT(type) RFEM_RF1_14_PORT(type)
#define CC2520_CSN_PIN RFEM_RF1_14_PIN
/* VREGen on RF1.10 */
#define CC2520_VREG_PORT(type) RFEM_RF1_10_PORT(type)
#define CC2520_VREG_PIN RFEM_RF1_10_PIN
/* RESETn on RF2.15 */
#define CC2520_RESET_PORT(type) RFEM_RF2_15_PORT(type)
#define CC2520_RESET_PIN RFEM_RF2_15_PIN

#define CC2520_FIFOP_IS_1 (!!(CC2520_FIFOP_PORT(IN) & BV(CC2520_FIFOP_PIN)))
#define CC2520_FIFO_IS_1  (!!(CC2520_FIFO_PORT(IN) & BV(CC2520_FIFO_PIN)))
#define CC2520_CCA_IS_1   (!!(CC2520_CCA_PORT(IN) & BV(CC2520_CCA_PIN)))
#define CC2520_SFD_IS_1   (!!(CC2520_SFD_PORT(IN) & BV(CC2520_SFD_PIN)))

#define SET_RESET_INACTIVE() do { CC2520_RESET_PORT(OUT) |=  BV(CC2520_RESET_PIN); } while(0)
#define SET_RESET_ACTIVE()   do { CC2520_RESET_PORT(OUT) &= ~BV(CC2520_RESET_PIN); } while(0)
#define SET_VREG_ACTIVE()    do { CC2520_VREG_PORT(OUT) |=  BV(CC2520_VREG_PIN); } while(0)
#define SET_VREG_INACTIVE()  do { CC2520_VREG_PORT(OUT) &= ~BV(CC2520_VREG_PIN); } while(0)

#define CC2520_FIFOP_INT_INIT() do {                  \
    CC2520_FIFOP_PORT(IES) &= ~BV(CC2520_FIFOP_PIN);  \
    CC2520_CLEAR_FIFOP_INT();                         \
  } while(0)
#define CC2520_ENABLE_FIFOP_INT()  do { CC2520_FIFOP_PORT(IE) |= BV(CC2520_FIFOP_PIN); } while(0)
#define CC2520_DISABLE_FIFOP_INT() do { CC2520_FIFOP_PORT(IE) &= ~BV(CC2520_FIFOP_PIN); } while(0)
#define CC2520_CLEAR_FIFOP_INT()   do { CC2520_FIFOP_PORT(IFG) &= ~BV(CC2520_FIFOP_PIN); } while(0)

#define CC2520_SPI_ENABLE()     do { CC2520_CSN_PORT(OUT) &= ~BV(CC2520_CSN_PIN); } while(0)
#define CC2520_SPI_DISABLE()    do { CC2520_CSN_PORT(OUT) |=  BV(CC2520_CSN_PIN); } while(0)
#define CC2520_SPI_IS_ENABLED() ((CC2520_CSN_PORT(OUT) & BV(CC2520_CSN_PIN)) != BV(CC2520_CSN_PIN))

/* @todo@ Both these need to be fixed.  The driver shouldn't assume it
 * has sole control of the entire port register, nor should it have
 * loop values that depend on the MCU clock speed. */

/* FIFOP on RF1.9 is P1.6 ; need its interrupt vector. */
#define CC2520_IRQ_VECTOR PORT1_VECTOR
/* Magic delay */
#define CC2520_CONF_SYMBOL_LOOP_COUNT 1302      /* 326us msp430X @ 8MHz */

#else
/* Port for CC2420 support on EXP430F5438 */

/*
 * SPI bus - CC2420 pin configuration.
 */

#define CC2420_CONF_SYMBOL_LOOP_COUNT 1302      /* 326us msp430X @ 8MHz */

/* P1.2 - Input: FIFOP from CC2420 */
#define CC2420_FIFOP_PORT(type)    P1##type
#define CC2420_FIFOP_PIN           6
/* P1.3 - Input: FIFO from CC2420 */
#define CC2420_FIFO_PORT(type)     P1##type
#define CC2420_FIFO_PIN            5
/* P1.4 - Input: CCA from CC2420 */
#define CC2420_CCA_PORT(type)      P1##type
#define CC2420_CCA_PIN             7
/* P4.1 - Input:  SFD from CC2420 */
#define CC2420_SFD_PORT(type)      P1##type
#define CC2420_SFD_PIN             3
 /* P3.0 - Output: SPI Chip Select (CS_N) */
#define CC2420_CSN_PORT(type)      P3##type
#define CC2420_CSN_PIN             0
/* P4.5 - Output: VREG_EN to CC2420 */
#define CC2420_VREG_PORT(type)     P1##type
#define CC2420_VREG_PIN            4
/* P4.6 - Output: RESET_N to CC2420 */
#define CC2420_RESET_PORT(type)    P1##type
#define CC2420_RESET_PIN           2


#define CC2420_IRQ_VECTOR PORT1_VECTOR

/* Pin status. */
#define CC2420_FIFOP_IS_1 (!!(CC2420_FIFOP_PORT(IN) & BV(CC2420_FIFOP_PIN)))
#define CC2420_FIFO_IS_1  (!!(CC2420_FIFO_PORT(IN) & BV(CC2420_FIFO_PIN)))
#define CC2420_CCA_IS_1   (!!(CC2420_CCA_PORT(IN) & BV(CC2420_CCA_PIN)))
#define CC2420_SFD_IS_1   (!!(CC2420_SFD_PORT(IN) & BV(CC2420_SFD_PIN)))

/* The CC2420 reset pin. */
#define SET_RESET_INACTIVE()   (CC2420_RESET_PORT(OUT) |=  BV(CC2420_RESET_PIN))
#define SET_RESET_ACTIVE()     (CC2420_RESET_PORT(OUT) &= ~BV(CC2420_RESET_PIN))

/* CC2420 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       (CC2420_VREG_PORT(OUT) |=  BV(CC2420_VREG_PIN))
#define SET_VREG_INACTIVE()     (CC2420_VREG_PORT(OUT) &= ~BV(CC2420_VREG_PIN))

/* CC2420 rising edge trigger for external interrupt (FIFOP). */
#define CC2420_FIFOP_INT_INIT() do {                  \
    CC2420_FIFOP_PORT(IES) &= ~BV(CC2420_FIFOP_PIN);  \
    CC2420_CLEAR_FIFOP_INT();                         \
  } while(0)

/* FIFOP on external interrupt 0. */
#define CC2420_ENABLE_FIFOP_INT()  do {CC2420_FIFOP_PORT(IE) |= BV(CC2420_FIFOP_PIN);} while(0)
#define CC2420_DISABLE_FIFOP_INT() do {CC2420_FIFOP_PORT(IE) &= ~BV(CC2420_FIFOP_PIN);} while(0)
#define CC2420_CLEAR_FIFOP_INT()   do {CC2420_FIFOP_PORT(IFG) &= ~BV(CC2420_FIFOP_PIN);} while(0)


/*
 * Enables/disables CC2420 access to the SPI bus (not the bus).
 * (Chip Select)
 */

 /* ENABLE CSn (active low) */
#define CC2420_SPI_ENABLE()     (CC2420_CSN_PORT(OUT) &= ~BV(CC2420_CSN_PIN))
 /* DISABLE CSn (active low) */
#define CC2420_SPI_DISABLE()    (CC2420_CSN_PORT(OUT) |=  BV(CC2420_CSN_PIN))
#define CC2420_SPI_IS_ENABLED() ((CC2420_CSN_PORT(OUT) & BV(CC2420_CSN_PIN)) != BV(CC2420_CSN_PIN))

#endif /* RF module selection */

#endif /* __PLATFORM_CONF_H__ */
