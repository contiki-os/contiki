/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 */

/**
 * \file
 *   Definitions for TI/Chipcon cc2530, cc2531 and cc2533 SFR registers.
 *
 *   Based on information in:
 *   "CC253x System-on-Chip Solution for 2.4-GHz IEEE 802.15.4 and ZigBee®
 *    Applications"
 *   Literature Number: SWRU191B. April 2009–Revised September 2010
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef __CC253X_H__
#define __CC253X_H__

/*---------------------------------------------------------------------------*/
/* Compiler Abstraction */
#include <compiler.h>
/*---------------------------------------------------------------------------
 * Interrupt Vectors
 * (Table 2.5, page 40)
 *---------------------------------------------------------------------------*/
#define RFERR_VECTOR     0  /* RF TXFIFO underflow and RXFIFO overflow. */
#define ADC_VECTOR       1  /* ADC end of conversion */
#define URX0_VECTOR      2  /* USART 0 RX complete */
#define URX1_VECTOR      3  /* USART 1 RX complete */
#define ENC_VECTOR       4  /* AES encryption/decryption complete */
#define ST_VECTOR        5  /* Sleep Timer compare */
#define P2INT_VECTOR     6  /* Port 2 inputs/USB/I2C */
#define UTX0_VECTOR      7  /* USART 0 TX complete */
#define DMA_VECTOR       8  /* DMA transfer complete */
#define T1_VECTOR        9  /* Timer 1 (16-bit) capture/compare/overflow */
#define T2_VECTOR       10  /* Timer 2 (MAC Timer) */
#define T3_VECTOR       11  /* Timer 3 (8-bit) compare/overflow */
#define T4_VECTOR       12  /* Timer 4 (8-bit) compare/overflow */
#define P0INT_VECTOR    13  /* Port 0 inputs */
#define UTX1_VECTOR     14  /* USART1 TX complete */
#define P1INT_VECTOR    15  /* Port 1 inputs */
#define RF_VECTOR       16  /* RF general interrupts */
#define WDT_VECTOR      17  /* Watchdog overflow in timer mode */
/*---------------------------------------------------------------------------
 * Special Function Registers and BITs
 * (Table 2.1, page 27)
 *---------------------------------------------------------------------------*/
/* 8051 Internals */
SFR(P0,        0x80); /* Port 0 */
  SBIT(P0_7,     0x80, 7); /* Port 0 bit 7 */
  SBIT(P0_6,     0x80, 6); /* Port 0 bit 6 */
  SBIT(P0_5,     0x80, 5); /* Port 0 bit 5 */
  SBIT(P0_4,     0x80, 4); /* Port 0 bit 4 */
  SBIT(P0_3,     0x80, 3); /* Port 0 bit 3 */
  SBIT(P0_2,     0x80, 2); /* Port 0 bit 2 */
  SBIT(P0_1,     0x80, 1); /* Port 0 bit 1 */
  SBIT(P0_0,     0x80, 0); /* Port 0 bit 0 */
SFR(SP,        0x81); /* Stack pointer */
SFR(DPL0,      0x82); /* Data pointer 0 low byte */
SFR(DPH0,      0x83); /* Data pointer 0 high byte */
SFR(DPL1,      0x84); /* Data pointer 1 low byte */
SFR(DPH1,      0x85); /* Data pointer 1 high byte */
SFR(PCON,      0x87); /* Power mode control */
SFR(TCON,      0x88); /* Interrupt flags */
  SBIT(URX1IF,   0x88, 7); /* USART1 RX interrupt flag */
  SBIT(ADCIF,    0x88, 5); /* ADC interrupt flag */
  SBIT(URX0IF,   0x88, 3); /* USART0 RX interrupt flag */
  SBIT(IT1,      0x88, 2); /* Reserved. Must always be set to 1 */
  SBIT(RFERRIF,  0x88, 1); /* RF TXFIFO/RXFIFO interrupt flag */
  SBIT(IT0,      0x88, 0); /* Reserved. Must always be set to 1 */
SFR(P1,        0x90); /* Port 1 */
  SBIT(P1_7,     0x90, 7); /* Port 1 bit 7 */
  SBIT(P1_6,     0x90, 6); /* Port 1 bit 6 */
  SBIT(P1_5,     0x90, 5); /* Port 1 bit 5 */
  SBIT(P1_4,     0x90, 4); /* Port 1 bit 4 */
  SBIT(P1_3,     0x90, 3); /* Port 1 bit 3 */
  SBIT(P1_2,     0x90, 2); /* Port 1 bit 2 */
  SBIT(P1_1,     0x90, 1); /* Port 1 bit 1 */
  SBIT(P1_0,     0x90, 0); /* Port 1 bit 0 */
SFR(DPS,       0x92); /* Data pointer select */
SFR(S0CON,     0x98); /* Interrupt flags 2 */
  SBIT(ENCIF_1,  0x98, 1); /* AES Interrupt flag 1 */
  SBIT(ENCIF_0,  0x98, 0); /* AES Interrupt flag 0 */
SFR(IEN2,      0x9A); /* Interrupt enable 2 */
SFR(S1CON,     0x9B); /* Interrupt flags 3 */
SFR(P2,        0xA0); /* Port 2 */
  SBIT(P2_7,     0xA0, 7); /* Port 2 bit 7 */
  SBIT(P2_6,     0xA0, 6); /* Port 2 bit 6 */
  SBIT(P2_5,     0xA0, 5); /* Port 2 bit 5 */
  SBIT(P2_4,     0xA0, 4); /* Port 2 bit 4 */
  SBIT(P2_3,     0xA0, 3); /* Port 2 bit 3 */
  SBIT(P2_2,     0xA0, 2); /* Port 2 bit 2 */
  SBIT(P2_1,     0xA0, 1); /* Port 2 bit 1 */
  SBIT(P2_0,     0xA0, 0); /* Port 2 bit 0 */
SFR(IEN0,      0xA8); /* Interrupt enable 0 */
  SBIT(EA,       0xA8, 7); /* All interrupts - enable/disable */
  SBIT(STIE,     0xA8, 5); /* Sleep Timer interrupt enable */
  SBIT(ENCIE,    0xA8, 4); /* AES encryption/decryption interrupt enable */
  SBIT(URX1IE,   0xA8, 3); /* USART1 RX interrupt enable */
  SBIT(URX0IE,   0xA8, 2); /* USART0 RX interrupt enable */
  SBIT(ADCIE,    0xA8, 1); /* ADC interrupt enable */
  SBIT(RFERRIE,  0xA8, 0); /* RF TXFIFO/RXFIFO interrupt enable */
SFR(IP0,       0xA9); /* Interrupt priority 0 */
SFR(IEN1,      0xB8); /* Interrupt enable 1 */
  SBIT(P0IE,     0xB8, 5); /* Port 0 interrupt enable */
  SBIT(T4IE,     0xB8, 4); /* Timer 4 interrupt enable */
  SBIT(T3IE,     0xB8, 3); /* Timer 3 interrupt enable */
  SBIT(T2IE,     0xB8, 2); /* Timer 2 interrupt enable */
  SBIT(T1IE,     0xB8, 1); /* Timer 1 interrupt enable */
  SBIT(DMAIE,    0xB8, 0); /* DMA Transfer interrupt enable */
SFR(IP1,       0xB9); /* Interrupt priority 1 */
SFR(IRCON,     0xC0); /* Interrupt flags 4 */
  SBIT(STIF,     0xC0, 7); /* Sleep Timer interrupt flag */
  SBIT(P0IF,     0xC0, 5); /* Port 0 interrupt flag */
  SBIT(T4IF,     0xC0, 4); /* Timer 4 interrupt flag */
  SBIT(T3IF,     0xC0, 3); /* Timer 3 interrupt flag */
  SBIT(T2IF,     0xC0, 2); /* Timer 2 interrupt flag */
  SBIT(T1IF,     0xC0, 1); /* Timer 1 interrupt flag */
  SBIT(DMAIF,    0xC0, 0); /* DMA-complete interrupt flag */
SFR(PSW,       0xD0); /* Program status word */
  SBIT(CY,       0xD0, 7); /* Carry flag */
  SBIT(AC,       0xD0, 6); /* Auxiliary carry flag */
  SBIT(F0,       0xD0, 5); /* User-defined flag 1, bit addressable */
  SBIT(RS1,      0xD0, 4); /* Register bank select, bit 1 */
  SBIT(RS0,      0xD0, 3); /* Register bank select, bit 0 */
  SBIT(OV,       0xD0, 2); /* Overflow flag */
  SBIT(F1,       0xD0, 1); /* User-defined flag 0, bit addressable */
  SBIT(P,        0xD0, 0); /* Parity flag */
SFR(ACC,       0xE0); /* Accumulator */
  SBIT(ACC_7,    0xE0, 7); /* Accumulator bit 7 */
  SBIT(ACC_6,    0xE0, 6); /* Accumulator bit 6 */
  SBIT(ACC_5,    0xE0, 5); /* Accumulator bit 5 */
  SBIT(ACC_4,    0xE0, 4); /* Accumulator bit 4 */
  SBIT(ACC_3,    0xE0, 3); /* Accumulator bit 3 */
  SBIT(ACC_2,    0xE0, 2); /* Accumulator bit 2 */
  SBIT(ACC_1,    0xE0, 1); /* Accumulator bit 1 */
  SBIT(ACC_0,    0xE0, 0); /* Accumulator bit 0 */
SFR(IRCON2,    0xE8); /* Interrupt flags 5 */
  SBIT(WDTIF,    0xE8, 4); /* Watchdog Timer interrupt flag */
  SBIT(P1IF,     0xE8, 3); /* Port 1 Interrupt flag */
  SBIT(UTX1IF,   0xE8, 2); /* USART1 TX interrupt flag */
  SBIT(UTX0IF,   0xE8, 1); /* USART0 TX interrupt flag */
  SBIT(P2IF,     0xE8, 0); /* Port 2 interrupt flag */
SFR(B,         0xF0); /* B Register */
  SBIT(B_7,      0xF0, 7); /* Register B bit 7 */
  SBIT(B_6,      0xF0, 6); /* Register B bit 6 */
  SBIT(B_5,      0xF0, 5); /* Register B bit 5 */
  SBIT(B_4,      0xF0, 4); /* Register B bit 4 */
  SBIT(B_3,      0xF0, 3); /* Register B bit 3 */
  SBIT(B_2,      0xF0, 2); /* Register B bit 2 */
  SBIT(B_1,      0xF0, 1); /* Register B bit 1 */
  SBIT(B_0,      0xF0, 0); /* Register B bit 0 */

/* ADC */
SFR(ADCCON1,   0xB4); /* ADC control 1 */
SFR(ADCCON2,   0xB5); /* ADC control 2 */
SFR(ADCCON3,   0xB6); /* ADC control 3 */
SFR(ADCL,      0xBA); /* ADC data low */
SFR(ADCH,      0xBB); /* ADC data high */
SFR(RNDL,      0xBC); /* Random number generator data low */
SFR(RNDH,      0xBD); /* Random number generator data high */

/* AES Coprocessor */
SFR(ENCDI,     0xB1); /* Encryption/decryption input data */
SFR(ENCDO,     0xB2); /* Encryption/decryption output data */
SFR(ENCCS,     0xB3); /* Encryption/decryption control and status */

/* DMA Controller */
SFR(DMAIRQ,    0xD1); /* DMA interrupt flag */
SFR(DMA1CFGL,  0xD2); /* DMA channel 1–4 configuration address low */
SFR(DMA1CFGH,  0xD3); /* DMA channel 1–4 configuration address high */
SFR(DMA0CFGL,  0xD4); /* DMA channel 0 configuration address low */
SFR(DMA0CFGH,  0xD5); /* DMA channel 0 configuration address high */
SFR(DMAARM,    0xD6); /* DMA channel armed */
SFR(DMAREQ,    0xD7); /* DMA channel start request and status */

/* I/O */
SFR(P0IFG,     0x89); /* Port 0 interrupt status flag */
SFR(P1IFG,     0x8A); /* Port 1 interrupt status flag */
SFR(P2IFG,     0x8B); /* Port 2 interrupt status flag */
SFR(PICTL,     0x8C); /* Port pins interrupt mask and edge */
SFR(P0IEN,     0xAB); /* Port 0 interrupt mask */
SFR(P1IEN,     0x8D); /* Port 1 interrupt mask */
SFR(P2IEN,     0xAC); /* Port 2 interrupt mask */
SFR(P0INP,     0x8F); /* Port 0 input Mode */
SFR(PERCFG,    0xF1); /* Peripheral I/O control */
SFR(APCFG,     0xF2); /* Analog peripheral I/O configuration */
SFR(P0SEL,     0xF3); /* Port 0 function select */
SFR(P1SEL,     0xF4); /* Port 1 function select */
SFR(P2SEL,     0xF5); /* Port 2 function select */
SFR(P1INP,     0xF6); /* Port 1 input mode */
SFR(P2INP,     0xF7); /* Port 2 input mode */
SFR(P0DIR,     0xFD); /* Port 0 direction */
SFR(P1DIR,     0xFE); /* Port 1 direction */
SFR(P2DIR,     0xFF); /* Port 2 direction */
SFR(PMUX,      0xAE); /* Power-down signal mux */

/* Memory */
SFR(MPAGE,     0x93); /* Memory page select */
SFR(_XPAGE,    0x93); /* Memory page select - SDCC name */
SFR(MEMCTR,    0xC7); /* Memory system control */
SFR(FMAP,      0x9F); /* Flash-memory bank mapping */
SFR(PSBANK,    0x9F); /* Flash-memory bank mapping - SDCC name */

/* RF */
SFR(RFIRQF1,   0x91); /* RF interrupt flags MSB */
SFR(RFD,       0xD9); /* RF data */
SFR(RFST,      0xE1); /* RF command strobe */
SFR(RFIRQF0,   0xE9); /* RF interrupt flags LSB */
SFR(RFERRF,    0xBF); /* RF error interrupt flags */

/* Sleep Timer */
SFR(ST0,       0x95); /* Sleep Timer 0 */
SFR(ST1,       0x96); /* Sleep Timer 1 */
SFR(ST2,       0x97); /* Sleep Timer 2 */
SFR(STLOAD,    0xAD); /* Sleep-timer load status */
SFR(SLEEPCMD,  0xBE); /* Sleep-mode control command */
SFR(SLEEPSTA,  0x9D); /* Sleep-mode control status */

/* Power Management and Clocks */
SFR(CLKCONCMD, 0xC6); /* Clock control command */
SFR(CLKCONSTA, 0x9E); /* Clock control status */

/* Timer 1 */
SFR(T1CC0L,    0xDA); /* Timer 1 channel 0 capture/compare value low */
SFR(T1CC0H,    0xDB); /* Timer 1 channel 0 capture/compare value high */
SFR(T1CC1L,    0xDC); /* Timer 1 channel 1 capture/compare value low */
SFR(T1CC1H,    0xDD); /* Timer 1 channel 1 capture/compare value high */
SFR(T1CC2L,    0xDE); /* Timer 1 channel 2 capture/compare value low */
SFR(T1CC2H,    0xDF); /* Timer 1 channel 2 capture/compare value high */
SFR(T1CNTL,    0xE2); /* Timer 1 counter low */
SFR(T1CNTH,    0xE3); /* Timer 1 counter high */
SFR(T1CTL,     0xE4); /* Timer 1 control and status */
SFR(T1CCTL0,   0xE5); /* Timer 1 channel 0 capture/compare control */
SFR(T1CCTL1,   0xE6); /* Timer 1 channel 1 capture/compare control */
SFR(T1CCTL2,   0xE7); /* Timer 1 channel 2 capture/compare control */
SFR(T1STAT,    0xAF); /* Timer 1 status */

/* Timer 2 (MAC Timer) */
SFR(T2CTRL,    0x94); /* Timer 2 control */
SFR(T2EVTCFG,  0x9C); /* Timer 2 event configuration */
SFR(T2IRQF,    0xA1); /* Timer 2 interrupt flags */
SFR(T2M0,      0xA2); /* Timer 2 multiplexed register 0 */
SFR(T2M1,      0xA3); /* Timer 2 multiplexed register 1 */
SFR(T2MOVF0,   0xA4); /* Timer 2 multiplexed overflow register 0 */
SFR(T2MOVF1,   0xA5); /* Timer 2 multiplexed overflow register 1 */
SFR(T2MOVF2,   0xA6); /* Timer 2 multiplexed overflow register 2 */
SFR(T2IRQM,    0xA7); /* Timer 2 interrupt mask */
SFR(T2MSEL,    0xC3); /* Timer 2 multiplex select */

/* Timer 3 */
SFR(T3CNT,     0xCA); /* Timer 3 counter */
SFR(T3CTL,     0xCB); /* Timer 3 control */
SFR(T3CCTL0,   0xCC); /* Timer 3 channel 0 compare control */
SFR(T3CC0,     0xCD); /* Timer 3 channel 0 compare value */
SFR(T3CCTL1,   0xCE); /* Timer 3 channel 1 compare control */
SFR(T3CC1,     0xCF); /* Timer 3 channel 1 compare value */

/* Timer 4 */
SFR(T4CNT,     0xEA); /* Timer 4 counter */
SFR(T4CTL,     0xEB); /* Timer 4 control */
SFR(T4CCTL0,   0xEC); /* Timer 4 channel 0 compare control */
SFR(T4CC0,     0xED); /* Timer 4 channel 0 compare value */
SFR(T4CCTL1,   0xEE); /* Timer 4 channel 1 compare control */
SFR(T4CC1,     0xEF); /* Timer 4 channel 1 compare value */

/* Timer 1, 3, 4 join Interrupts */
SFR(TIMIF,     0xD8); /* Timers 1/3/4 joint interrupt mask/flags */
  SBIT(OVFIM,    0xD8, 6); /* Timer 1 overflow interrupt mask */
  SBIT(T4CH1IF,  0xD8, 5); /* Timer 4 channel 1 interrupt flag */
  SBIT(T4CH0IF,  0xD8, 4); /* Timer 4 channel 0 interrupt flag */
  SBIT(T4OVFIF,  0xD8, 3); /* Timer 4 overflow interrupt flag */
  SBIT(T3CH1IF,  0xD8, 2); /* Timer 3 channel 1 interrupt flag */
  SBIT(T3CH0IF,  0xD8, 1); /* Timer 3 channel 0 interrupt flag */
  SBIT(T3OVFIF,  0xD8, 0); /* Timer 3 overflow interrupt flag */

/* USART 0 */
SFR(U0CSR,     0x86); /* USART 0 control and status */
SFR(U0DBUF,    0xC1); /* USART 0 receive/transmit data buffer */
SFR(U0BAUD,    0xC2); /* USART 0 baud-rate control */
SFR(U0UCR,     0xC4); /* USART 0 UART control */
SFR(U0GCR,     0xC5); /* USART 0 generic control */

/* USART 1 */
SFR(U1CSR,     0xF8); /* USART 1 control and status */
  SBIT(MODE,     0xF8, 7); /* USART mode select */
  SBIT(RE,       0xF8, 6); /* UART receiver enable */
  SBIT(SLAVE,    0xF8, 5); /* SPI master- or slave mode select */
  SBIT(FE,       0xF8, 4); /* UART framing error status */
  SBIT(ERR,      0xF8, 3); /* UART parity error status */
  SBIT(RX_BYTE,  0xF8, 2); /* Receive byte status */
  SBIT(TX_BYTE,  0xF8, 1); /* Transmit byte status */
  SBIT(ACTIVE,   0xF8, 0); /* USART transmit/receive active status */
SFR(U1DBUF,    0xF9); /* USART 1 receive/transmit data buffer */
SFR(U1BAUD,    0xFA); /* USART 1 baud-rate control */
SFR(U1UCR,     0xFB); /* USART 1 UART control */
SFR(U1GCR,     0xFC); /* USART 1 Generic control */

/* Watchdog Timer */
SFR(WDCTL,     0xC9); /* Watchdog Timer Control */
/*---------------------------------------------------------------------------
 * XREG Registers (0x6000–0x63FF), excluding RF and USB registers
 * (Table 2.2, page 31)
 *---------------------------------------------------------------------------*/
SFRX(MONMUX ,    0x61A6); /* Operational amplifier mode control (cc2530/31) */
SFRX(OPAMPMC,    0x61A6); /* Battery monitor MUX (cc2533) */
/* I2C registers - cc2533 only */
SFRX(I2CCFG,     0x6230); /* I2C control */
SFRX(I2CSTAT,    0x6231); /* I2C status */
SFRX(I2CDATA,    0x6232); /* I2C data */
SFRX(I2CADDR,    0x6233); /* I2C own slave address */
SFRX(I2CWC,      0x6234); /* Wrapper Control */
SFRX(I2CIO,      0x6235); /* GPIO */
/* End I2C registers */
SFRX(OBSSEL0,    0x6243); /* Observation output control - register 0 */
SFRX(OBSSEL1,    0x6244); /* Observation output control - register 1 */
SFRX(OBSSEL2,    0x6245); /* Observation output control - register 2 */
SFRX(OBSSEL3,    0x6246); /* Observation output control - register 3 */
SFRX(OBSSEL4,    0x6247); /* Observation output control - register 4 */
SFRX(OBSSEL5,    0x6248); /* Observation output control - register 5 */
SFRX(CHVER,      0x6249); /* Chip version */
SFRX(CHIPID,     0x624A); /* Chip identification */

/* TR0 below is renamed to TESTREG0 to avoid namespace conflicts with the
 * bit-addressable TCON.TR0 on the default 8051. See SDCC bug 3513300 */
SFRX(TESTREG0,   0x624B); /* Test register 0 */

SFRX(DBGDATA,    0x6260); /* Debug interface write data */
SFRX(SRCRC,      0x6262); /* Sleep reset CRC */
SFRX(BATTMON,    0x6264); /* Battery monitor */
SFRX(IVCTRL,     0x6265); /* Analog control register */
SFRX(FCTL,       0x6270); /* Flash control */
SFRX(FADDRL,     0x6271); /* Flash address low */
SFRX(FADDRH,     0x6272); /* Flash address high */
SFRX(FWDATA,     0x6273); /* Flash write data */
SFRX(CHIPINFO0,  0x6276); /* Chip information byte 0 */
SFRX(CHIPINFO1,  0x6277); /* Chip information byte 1 */
SFRX(IRCTL,      0x6281); /* Timer 1 IR generation control */
SFRX(CLD,        0x6290); /* Clock-loss detection */
SFRX(XX_T1CCTL0, 0x62A0); /* Timer 1 channel 0 capture/compare control (additional XREG mapping of SFR) */
SFRX(XX_T1CCTL1, 0x62A1); /* Timer 1 channel 1 capture/compare control (additional XREG mapping of SFR register) */
SFRX(XX_T1CCTL2, 0x62A2); /* Timer 1 channel 2 capture/compare control (additional XREG mapping of SFR register) */
SFRX(T1CCTL3,    0x62A3); /* Timer 1 channel 3 capture/compare control */
SFRX(T1CCTL4,    0x62A4); /* Timer 1 channel 4 capture/compare control */
SFRX(XX_T1CC0L,  0x62A6); /* Timer 1 channel 0 capture/compare value low (additional XREG mapping of SFR register) */
SFRX(XX_T1CC0H,  0x62A7); /* Timer 1 channel 0 capture/compare value high (additional XREG mapping of SFR register) */
SFRX(XX_T1CC1L,  0x62A8); /* Timer 1 channel 1 capture/compare value low (additional XREG mapping of SFR register) */
SFRX(XX_T1CC1H,  0x62A9); /* Timer 1 channel 1 capture/compare value high (additional XREG mapping of SFR register) */
SFRX(XX_T1CC2L,  0x62AA); /* Timer 1 channel 2 capture/compare value low (additional XREG mapping of SFR register) */
SFRX(XX_T1CC2H,  0x62AB); /* Timer 1 channel 2 capture/compare value high (additional XREG mapping of SFR register) */
SFRX(T1CC3L,     0x62AC); /* Timer 1 channel 3 capture/compare value low */
SFRX(T1CC3H,     0x62AD); /* Timer 1 channel 3 capture/compare value high */
SFRX(T1CC4L,     0x62AE); /* Timer 1 channel 4 capture/compare value low */
SFRX(T1CC4H,     0x62AF); /* Timer 1 channel 4 capture/compare value high */
SFRX(STCC,       0x62B0); /* Sleep Timer capture control */
SFRX(STCS,       0x62B1); /* Sleep Timer capture status */
SFRX(STCV0,      0x62B2); /* Sleep Timer capture value byte 0 */
SFRX(STCV1,      0x62B3); /* Sleep Timer capture value byte 1 */
SFRX(STCV2,      0x62B4); /* Sleep Timer capture value byte 2 */
SFRX(OPAMPC,     0x62C0); /* Operational amplifier control */
SFRX(OPAMPS,     0x62C1); /* Operational amplifier status */
SFRX(CMPCTL,     0x62D0); /* Analog comparator control and status */
/*---------------------------------------------------------------------------
 * Radio Registers
 * (Sec. 23, page 211)
 *---------------------------------------------------------------------------*/
SFRX(RFCORE_RAM,      0x6000); /* RF Core Memory Map (0x6000 to 0x0617F) */
SFRX(RXFIFO,          0x6000); /* TXFIFO Direct Access (0x6000 to 0x607F) */
SFRX(TXFIFO,          0x6080); /* TXFIFO Direct Access (0x6080 to 0x60FF) */

SFRX(SRC_ADDR_TABLE,  0x6100); /* Source Address Table Start */

/* Source Address Matching Result */
SFRX(SRCRESMASK0,     0x6160); /* Extended address matching */
SFRX(SRCRESMASK1,     0x6161); /* Short address matching */
SFRX(SRCRESMASK2,     0x6162); /* Source address match - 24-bit mask */
SFRX(SRCRESINDEX,     0x6163); /* Bit index of least-significant 1 in SRCRESMASK */

/* Source Address Matching Control */
SFRX(SRCEXTPENDEN0,   0x6164); /* Ext. Address bit-mask 0 (LSB) */
SFRX(SRCEXTPENDEN1,   0x6165); /* Ext. Address bit-mask 1 */
SFRX(SRCEXTPENDEN2,   0x6166); /* Ext. Address bit-mask 2 (MSB) */
SFRX(SRCSHORTPENDEN0, 0x6167); /* Short Address bit-mask 0 (LSB) */
SFRX(SRCSHORTPENDEN1, 0x6168); /* Short Address bit-mask 1 */
SFRX(SRCSHORTPENDEN2, 0x6169); /* Short Address bit-mask 2 (MSB) */

/* Local Address Information (used during destination address filtering) */
SFRX(EXT_ADDR0,       0x616A); /* IEEE extended address 0 */
SFRX(EXT_ADDR1,       0x616B); /* IEEE extended address 1 */
SFRX(EXT_ADDR2,       0x616C); /* IEEE extended address 2 */
SFRX(EXT_ADDR3,       0x616D); /* IEEE extended address 3 */
SFRX(EXT_ADDR4,       0x616E); /* IEEE extended address 4 */
SFRX(EXT_ADDR5,       0x616F); /* IEEE extended address 5 */
SFRX(EXT_ADDR6,       0x6170); /* IEEE extended address 6 */
SFRX(EXT_ADDR7,       0x6171); /* IEEE extended address 7 */
SFRX(PAN_ID0,         0x6172); /* PAN ID 0 */
SFRX(PAN_ID1,         0x6173); /* PAN ID 1 */
SFRX(SHORT_ADDR0,     0x6174); /* Short Address 0 */
SFRX(SHORT_ADDR1,     0x6175); /* Short Address 1 */

SFRX(FRMFILT0,        0x6180); /* Frame Filtering 0 */
SFRX(FRMFILT1,        0x6181); /* Frame Filtering 1 */
SFRX(SRCMATCH,        0x6182); /* Source Address Matching and Pending Bits */
SFRX(SRCSHORTEN0,     0x6183); /* Short Address Matching 0 */
SFRX(SRCSHORTEN1,     0x6184); /* Short Address Matching 1 */
SFRX(SRCSHORTEN2,     0x6185); /* Short Address Matching 2 */
SFRX(SRCEXTEN0,       0x6186); /* Extended Address Matching 0 */
SFRX(SRCEXTEN1,       0x6187); /* Extended Address Matching 1 */
SFRX(SRCEXTEN2,       0x6188); /* Extended Address Matching 2 */
SFRX(FRMCTRL0,        0x6189); /* Frame Handling */
SFRX(FRMCTRL1,        0x618A); /* Frame Handling */
SFRX(RXENABLE,        0x618B); /* RX Enabling */
SFRX(RXMASKSET,       0x618C); /* RX Enabling */
SFRX(RXMASKCLR,       0x618D); /* RX Disabling */
SFRX(FREQTUNE,        0x618E); /* Crystal Oscillator Frequency Tuning */
SFRX(FREQCTRL,        0x618F); /* RF Frequency Control */
SFRX(TXPOWER,         0x6190); /* Controls the Output Power */
SFRX(TXCTRL,          0x6191); /* Controls the TX Settings */
SFRX(FSMSTAT0,        0x6192); /* Radio Status Register */
SFRX(FSMSTAT1,        0x6193); /* Radio Status Register */
SFRX(FIFOPCTRL,       0x6194); /* FIFOP Threshold */
SFRX(FSMCTRL,         0x6195); /* FSM Options */
SFRX(CCACTRL0,        0x6196); /* CCA Threshold */
SFRX(CCACTRL1,        0x6197); /* Other CCA Options */
SFRX(RSSI,            0x6198); /* RSSI Status Register */
SFRX(RSSISTAT,        0x6199); /* RSSI Valid Status Register */
SFRX(RXFIRST,         0x619A); /* First Byte in RXFIFO */
SFRX(RXFIFOCNT,       0x619B); /* Number of Bytes in RXFIFO */
SFRX(TXFIFOCNT,       0x619C); /* Number of Bytes in TXFIFO */
SFRX(RXFIRST_PTR,     0x619D); /* RXFIFO Pointer */
SFRX(RXLAST_PTR,      0x619E); /* RXFIFO Pointer */
SFRX(RXP1_PTR,        0x619F); /* RXFIFO Pointer */
SFRX(TXFIRST_PTR,     0x61A1); /* TXFIFO Pointer */
SFRX(TXLAST_PTR,      0x61A2); /* TXFIFO Pointer */
SFRX(RFIRQM0,         0x61A3); /* RF Interrupt Masks 0 */
SFRX(RFIRQM1,         0x61A4); /* RF Interrupt Masks 1 */
SFRX(RFERRM,          0x61A5); /* RF Error Interrupt Mask */
SFRX(RFRND,           0x61A7); /* Random Data */
SFRX(MDMCTRL0,        0x61A8); /* Controls Modem 0 */
SFRX(MDMCTRL1,        0x61A9); /* Controls Modem 1 */
SFRX(FREQEST,         0x61AA); /* Estimated RF Frequency Offset */
SFRX(RXCTRL,          0x61AB); /* Tune Receive Section */
SFRX(FSCTRL,          0x61AC); /* Tune Frequency Synthesizer */
SFRX(FSCAL0,          0x61AD); /* Tune Frequency Calibration 0 */
SFRX(FSCAL1,          0x61AE); /* Tune Frequency Calibration 1 */
SFRX(FSCAL2,          0x61AF); /* Tune Frequency Calibration 2 */
SFRX(FSCAL3,          0x61B0); /* Tune Frequency Calibration 3 */
SFRX(AGCCTRL0,        0x61B1); /* AGC Dynamic Range Control */
SFRX(AGCCTRL1,        0x61B2); /* AGC Reference Level */
SFRX(AGCCTRL2,        0x61B3); /* AGC Gain Override */
SFRX(AGCCTRL3,        0x61B4); /* AGC Control */
SFRX(ADCTEST0,        0x61B5); /* ADC Tuning 0 */
SFRX(ADCTEST1,        0x61B6); /* ADC Tuning 1 */
SFRX(ADCTEST2,        0x61B7); /* ADC Tuning 2 */
SFRX(MDMTEST0,        0x61B8); /* Test Register for Modem 0 */
SFRX(MDMTEST1,        0x61B9); /* Test Register for Modem 1 */
SFRX(DACTEST0,        0x61BA); /* DAC Override Value */
SFRX(DACTEST1,        0x61BB); /* DAC Override Value */
SFRX(DACTEST2,        0x61BC); /* DAC Test Setting */
SFRX(ATEST,           0x61BD); /* Analog Test Control */
SFRX(PTEST0,          0x61BE); /* Override Power-Down Register 0 */
SFRX(PTEST1,          0x61BF); /* Override Power-Down Register 1 */
SFRX(TXFILTCFG,       0x61FA); /*  TX Filter Configuration */
SFRX(RFC_OBS_CTRL0,   0x61EB); /* RF Observation Mux Control 0 */
SFRX(RFC_OBS_CTRL1,   0x61EC); /* RF Observation Mux Control 1 */
SFRX(RFC_OBS_CTRL2,   0x61ED); /* RF Observation Mux Control 2 */

/* Command Strobe/CSMA-CA Processor Registers */
SFRX(CSPPROG0,        0x61C0); /* CSP Program Memory, Byte 0 */
SFRX(CSPPROG1,        0x61C1); /* CSP Program Memory, Byte 1 */
SFRX(CSPPROG2,        0x61C2); /* CSP Program Memory, Byte 2 */
SFRX(CSPPROG3,        0x61C3); /* CSP Program Memory, Byte 3 */
SFRX(CSPPROG4,        0x61C4); /* CSP Program Memory, Byte 4 */
SFRX(CSPPROG5,        0x61C5); /* CSP Program Memory, Byte 5 */
SFRX(CSPPROG6,        0x61C6); /* CSP Program Memory, Byte 6 */
SFRX(CSPPROG7,        0x61C7); /* CSP Program Memory, Byte 7 */
SFRX(CSPPROG8,        0x61C8); /* CSP Program Memory, Byte 8 */
SFRX(CSPPROG9,        0x61C9); /* CSP Program Memory, Byte 9 */
SFRX(CSPPROG10,       0x61CA); /* CSP Program Memory, Byte 10 */
SFRX(CSPPROG11,       0x61CB); /* CSP Program Memory, Byte 11 */
SFRX(CSPPROG12,       0x61CC); /* CSP Program Memory, Byte 12 */
SFRX(CSPPROG13,       0x61CD); /* CSP Program Memory, Byte 13 */
SFRX(CSPPROG14,       0x61CE); /* CSP Program Memory, Byte 14 */
SFRX(CSPPROG15,       0x61CF); /* CSP Program Memory, Byte 15 */
SFRX(CSPPROG16,       0x61D0); /* CSP Program Memory, Byte 16 */
SFRX(CSPPROG17,       0x61D1); /* CSP Program Memory, Byte 17 */
SFRX(CSPPROG18,       0x61D2); /* CSP Program Memory, Byte 18 */
SFRX(CSPPROG19,       0x61D3); /* CSP Program Memory, Byte 19 */
SFRX(CSPPROG20,       0x61D4); /* CSP Program Memory, Byte 20 */
SFRX(CSPPROG21,       0x61D5); /* CSP Program Memory, Byte 21 */
SFRX(CSPPROG22,       0x61D6); /* CSP Program Memory, Byte 22 */
SFRX(CSPPROG23,       0x61D7); /* CSP Program Memory, Byte 23 */
SFRX(CSPCTRL,         0x61E0); /* CSP Control Bit */
SFRX(CSPSTAT,         0x61E1); /* CSP Status Register */
SFRX(CSPX,            0x61E2); /* CSP X Register */
SFRX(CSPY,            0x61E3); /* CSP Y Register */
SFRX(CSPZ,            0x61E4); /* CSP Z Register */
SFRX(CSPT,            0x61E5); /* CSP T Register */
/*---------------------------------------------------------------------------
 * cc2531 USB Registers
 * (Sec. 21.12, page 196)
 *---------------------------------------------------------------------------*/
SFRX(USBADDR,  0x6200); /* Function Address */
SFRX(USBPOW,   0x6201); /* Power/Control Register */
SFRX(USBIIF,   0x6202); /* IN Endpoints and EP0 Interrupt Flags */
SFRX(USBOIF,   0x6204); /* OUT-Endpoint Interrupt Flags */
SFRX(USBCIF,   0x6206); /* Common USB Interrupt Flags */
SFRX(USBIIE,   0x6207); /* IN Endpoints and EP0 Interrupt-Enable Mask */
SFRX(USBOIE,   0x6209); /* Out Endpoints Interrupt Enable Mask */
SFRX(USBCIE,   0x620B); /* Common USB Interrupt-Enable Mask */
SFRX(USBFRML,  0x620C); /* Current Frame Number (Low Byte) */
SFRX(USBFRMH,  0x620D); /* Current Frame Number (High Byte) */
SFRX(USBINDEX, 0x620E); /* Current-Endpoint Index Register */
SFRX(USBCTRL,  0x620F); /* USB Control Register */
SFRX(USBMAXI,  0x6210); /* Max. Packet Size for IN Endpoint{1–5} */
SFRX(USBCS0,   0x6211); /* EP0 Control and Status (USBINDEX = 0) */
SFRX(USBCSIL,  0x6211); /* IN EP{1–5} Control and Status, Low */
SFRX(USBCSIH,  0x6212); /* IN EP{1–5} Control and Status, High */
SFRX(USBMAXO,  0x6213); /* Max. Packet Size for OUT EP{1–5} */
SFRX(USBCSOL,  0x6214); /* OUT EP{1–5} Control and Status, Low */
SFRX(USBCSOH,  0x6215); /* OUT EP{1–5} Control and Status, High */
SFRX(USBCNT0,  0x6216); /* Number of Received Bytes in EP0 FIFO (USBINDEX = 0) */
SFRX(USBCNTL,  0x6216); /* Number of Bytes in EP{1–5} OUT FIFO, Low */
SFRX(USBCNTH,  0x6217); /* Number of Bytes in EP{1–5} OUT FIFO, High */
SFRX(USBF0,    0x6220); /* Endpoint-0 FIFO */
SFRX(USBF1,    0x6222); /* Endpoint-1 FIFO */
SFRX(USBF2,    0x6224); /* Endpoint-2 FIFO */
SFRX(USBF3,    0x6226); /* Endpoint-3 FIFO */
SFRX(USBF4,    0x6228); /* Endpoint-4 FIFO */
SFRX(USBF5,    0x622A); /* Endpoint-5 FIFO */
/*---------------------------------------------------------------------------
 * SFR Access via XDATA (0x7080 - 0x70FF)
 *---------------------------------------------------------------------------*/
SFRX(X_P0,        0x7080); /* Port 0 - Read Only */
SFRX(X_U0CSR,     0x7086); /* USART 0 control and status */
SFRX(X_P0IFG,     0x7089); /* Port 0 interrupt status flag */
SFRX(X_P1IFG,     0x708A); /* Port 1 interrupt status flag */
SFRX(X_P2IFG,     0x708B); /* Port 2 interrupt status flag */
SFRX(X_PICTL,     0x708C); /* Port pins interrupt mask and edge */
SFRX(X_P1IEN,     0x708D); /* Port 1 interrupt mask */
SFRX(X_P0INP,     0x708F); /* Port 0 input Mode */
SFRX(X_P1,        0x7090); /* Port 1 - Read Only */
SFRX(X_RFIRQF1,   0x7091); /* RF interrupt flags MSB */
SFRX(X_MPAGE,     0x7093); /* Memory page select */
SFRX(X__XPAGE,    0x7093); /* Memory page select - SDCC name */
SFRX(X_T2CTRL,    0x7094); /* Timer 2 control */
SFRX(X_ST0,       0x7095); /* Sleep Timer 0 */
SFRX(X_ST1,       0x7096); /* Sleep Timer 1 */
SFRX(X_ST2,       0x7097); /* Sleep Timer 2 */
SFRX(X_T2EVTCFG,  0x709C); /* Timer 2 event configuration */
SFRX(X_SLEEPSTA,  0x709D); /* Sleep-mode control status */
SFRX(X_CLKCONSTA, 0x709E); /* Clock control status */
SFRX(X_FMAP,      0x709F); /* Flash-memory bank mapping */
SFRX(X_PSBANK,    0x709F); /* Flash-memory bank mapping - SDCC name */
SFRX(X_P2,        0x70A0); /* Port 2 - Read Only */
SFRX(X_T2IRQF,    0x70A1); /* Timer 2 interrupt flags */
SFRX(X_T2M0,      0x70A2); /* Timer 2 multiplexed register 0 */
SFRX(X_T2M1,      0x70A3); /* Timer 2 multiplexed register 1 */
SFRX(X_T2MOVF0,   0x70A4); /* Timer 2 multiplexed overflow register 0 */
SFRX(X_T2MOVF1,   0x70A5); /* Timer 2 multiplexed overflow register 1 */
SFRX(X_T2MOVF2,   0x70A6); /* Timer 2 multiplexed overflow register 2 */
SFRX(X_T2IRQM,    0x70A7); /* Timer 2 interrupt mask */
SFRX(X_P0IEN,     0x70AB); /* Port 0 interrupt mask */
SFRX(X_P2IEN,     0x70AC); /* Port 2 interrupt mask */
SFRX(X_STLOAD,    0x70AD); /* Sleep-timer load status */
SFRX(X_PMUX,      0x70AE); /* Power-down signal mux */
SFRX(X_T1STAT,    0x70AF); /* Timer 1 status */
SFRX(X_ENCDI,     0x70B1); /* Encryption/decryption input data */
SFRX(X_ENCDO,     0x70B2); /* Encryption/decryption output data */
SFRX(X_ENCCS,     0x70B3); /* Encryption/decryption control and status */
SFRX(X_ADCCON1,   0x70B4); /* ADC control 1 */
SFRX(X_ADCCON2,   0x70B5); /* ADC control 2 */
SFRX(X_ADCCON3,   0x70B6); /* ADC control 3 */
SFRX(X_ADCL,      0x70BA); /* ADC data low */
SFRX(X_ADCH,      0x70BB); /* ADC data high */
SFRX(X_RNDL,      0x70BC); /* Random number generator data low */
SFRX(X_RNDH,      0x70BD); /* Random number generator data high */
SFRX(X_SLEEPCMD,  0x70BE); /* Sleep-mode control command */
SFRX(X_RFERRF,    0x70BF); /* RF error interrupt flags */
SFRX(X_U0DBUF,    0x70C1); /* USART 0 receive/transmit data buffer */
SFRX(X_U0BAUD,    0x70C2); /* USART 0 baud-rate control */
SFRX(X_T2MSEL,    0x70C3); /* Timer 2 multiplex select */
SFRX(X_U0UCR,     0x70C4); /* USART 0 UART control */
SFRX(X_U0GCR,     0x70C5); /* USART 0 generic control */
SFRX(X_CLKCONCMD, 0x70C6); /* Clock control command */
SFRX(X_MEMCTR,    0x70C7); /* Memory system control */
SFRX(X_WDCTL,     0x70C9); /* Watchdog Timer Control */
SFRX(X_T3CNT,     0x70CA); /* Timer 3 counter */
SFRX(X_T3CTL,     0x70CB); /* Timer 3 control */
SFRX(X_T3CCTL0,   0x70CC); /* Timer 3 channel 0 compare control */
SFRX(X_T3CC0,     0x70CD); /* Timer 3 channel 0 compare value */
SFRX(X_T3CCTL1,   0x70CE); /* Timer 3 channel 1 compare control */
SFRX(X_T3CC1,     0x70CF); /* Timer 3 channel 1 compare value */
SFRX(X_DMAIRQ,    0x70D1); /* DMA interrupt flag */
SFRX(X_DMA1CFGL,  0x70D2); /* DMA channel 1–4 configuration address low */
SFRX(X_DMA1CFGH,  0x70D3); /* DMA channel 1–4 configuration address high */
SFRX(X_DMA0CFGL,  0x70D4); /* DMA channel 0 configuration address low */
SFRX(X_DMA0CFGH,  0x70D5); /* DMA channel 0 configuration address high */
SFRX(X_DMAARM,    0x70D6); /* DMA channel armed */
SFRX(X_DMAREQ,    0x70D7); /* DMA channel start request and status */
SFRX(X_TIMIF,     0x70D8); /* Timers 1/3/4 joint interrupt mask/flags */
SFRX(X_RFD,       0x70D9); /* RF data */
SFRX(X_T1CC0L,    0x70DA); /* Timer 1 channel 0 capture/compare value low */
SFRX(X_T1CC0H,    0x70DB); /* Timer 1 channel 0 capture/compare value high */
SFRX(X_T1CC1L,    0x70DC); /* Timer 1 channel 1 capture/compare value low */
SFRX(X_T1CC1H,    0x70DD); /* Timer 1 channel 1 capture/compare value high */
SFRX(X_T1CC2L,    0x70DE); /* Timer 1 channel 2 capture/compare value low */
SFRX(X_T1CC2H,    0x70DF); /* Timer 1 channel 2 capture/compare value high */
SFRX(X_RFST,      0x70E1); /* RF command strobe */
SFRX(X_T1CNTL,    0x70E2); /* Timer 1 counter low */
SFRX(X_T1CNTH,    0x70E3); /* Timer 1 counter high */
SFRX(X_T1CTL,     0x70E4); /* Timer 1 control and status */
SFRX(X_T1CCTL0,   0x70E5); /* Timer 1 channel 0 capture/compare control */
SFRX(X_T1CCTL1,   0x70E6); /* Timer 1 channel 1 capture/compare control */
SFRX(X_T1CCTL2,   0x70E7); /* Timer 1 channel 2 capture/compare control */
SFRX(X_RFIRQF0,   0x70E9); /* RF interrupt flags LSB */
SFRX(X_T4CNT,     0x70EA); /* Timer 4 counter */
SFRX(X_T4CTL,     0x70EB); /* Timer 4 control */
SFRX(X_T4CCTL0,   0x70EC); /* Timer 4 channel 0 compare control */
SFRX(X_T4CC0,     0x70ED); /* Timer 4 channel 0 compare value */
SFRX(X_T4CCTL1,   0x70EE); /* Timer 4 channel 1 compare control */
SFRX(X_T4CC1,     0x70EF); /* Timer 4 channel 1 compare value */
SFRX(X_PERCFG,    0x70F1); /* Peripheral I/O control */
SFRX(X_APCFG,     0x70F2); /* Analog peripheral I/O configuration */
SFRX(X_P0SEL,     0x70F3); /* Port 0 function select */
SFRX(X_P1SEL,     0x70F4); /* Port 1 function select */
SFRX(X_P2SEL,     0x70F5); /* Port 2 function select */
SFRX(X_P1INP,     0x70F6); /* Port 1 input mode */
SFRX(X_P2INP,     0x70F7); /* Port 2 input mode */
SFRX(X_U1CSR,     0x70F8); /* USART 1 control and status */
SFRX(X_U1DBUF,    0x70F9); /* USART 1 receive/transmit data buffer */
SFRX(X_U1BAUD,    0x70FA); /* USART 1 baud-rate control */
SFRX(X_U1UCR,     0x70FB); /* USART 1 UART control */
SFRX(X_U1GCR,     0x70FC); /* USART 1 Generic control */
SFRX(X_P0DIR,     0x70FD); /* Port 0 direction */
SFRX(X_P1DIR,     0x70FE); /* Port 1 direction */
SFRX(X_P2DIR,     0x70FF); /* Port 2 direction */
/*---------------------------------------------------------------------------
 * Information Page (Read Only)
 *---------------------------------------------------------------------------*/
SFRX(X_INFOPAGE,  0x7800); /* Start of Information Page */
SFRX(X_IEEE_ADDR, 0x780C); /* Start of unique IEEE Address */

#endif /* __CC253X_H__ */
