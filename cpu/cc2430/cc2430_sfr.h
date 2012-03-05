/**
 *
 * \file cc2430_sfr.h
 * \brief CC2430 registers header file for CC2430.
 *
 *  Definitions for CC2430 SFR registers.
 *   
 *	
 */

#ifndef REG_CC2430_H
#define REG_CC2430_H

#include "8051def.h"

/*  BYTE Register  */

__sfr __at (0x80) P0   ;
/* P0 */
__sbit __at (0x87) P0_7 ;
__sbit __at (0x86) P0_6 ;
__sbit __at (0x85) P0_5 ;
__sbit __at (0x84) P0_4 ;
__sbit __at (0x83) P0_3 ;
__sbit __at (0x82) P0_2 ;
__sbit __at (0x81) P0_1 ;
__sbit __at (0x80) P0_0 ;

__sfr __at (0x81) SP   ;
__sfr __at (0x82) DPL0  ;
__sfr __at (0x83) DPH0  ;
/*DPL and DPH correspond DPL0 and DPH0 (82-83)*/
__sfr __at (0x84)   DPL1;
__sfr __at (0x85)   DPH1;
__sfr __at (0x86)   U0CSR;
#define U_MODE	0x80
#define U_RE	0x40
#define U_SLAVE	0x20
#define U_FE	0x10
#define U_ERR	0x08
#define U_RXB	0x04
#define	U_TXB	0x02
#define U_ACTIVE 0x01

__sfr __at (0x87) PCON ;
/*  PCON (0x87) */
#define IDLE 0x01

__sfr __at (0x88) TCON ;
/*  TCON (0x88) */
__sbit __at (0x8F) TCON_URX1IF;
/*__sbit __at (0x8E) RES;*/
__sbit __at (0x8D) TCON_ADCIF;
/*__sbit __at (0x8C) RES;*/
__sbit __at (0x8B) TCON_URX0IF;
__sbit __at (0x8A) TCON_IT1;
__sbit __at (0x89) TCON_RFERRIF;
__sbit __at (0x88) TCON_IT0;


__sfr __at (0x89)   P0IFG;
__sfr __at (0x8A)   P1IFG;
__sfr __at (0x8B)   P2IFG;
__sfr __at (0x8C)   PICTL;
/*PICTL bits*/
#define PADSC	0x40
#define P2IEN	0x20
#define P0IENH	0x10
#define P0IENL	0x08
#define P2ICON	0x04
#define P1ICON	0x02
#define P0ICON	0x01

__sfr __at (0x8D)   P1IEN;
__sfr __at (0x8F)   P0INP;

__sfr __at (0x90) P1   ;
/* P1 */
__sbit __at (0x90) P1_0 ;
__sbit __at (0x91) P1_1 ;
__sbit __at (0x92) P1_2 ;
__sbit __at (0x93) P1_3 ;
__sbit __at (0x94) P1_4 ;
__sbit __at (0x95) P1_5 ;
__sbit __at (0x96) P1_6 ;
__sbit __at (0x97) P1_7 ;

__sfr __at (0x91)   RFIM;
__sfr __at (0x92)   DPS;
__sfr __at (0x93)   _XPAGE; /*MPAGE as paging register for sdcc*/
__sfr __at (0x94)   T2CMP;
__sfr __at (0x95)   ST0;
__sfr __at (0x96)   ST1;
__sfr __at (0x97)   ST2;
__sfr __at (0x98)   S0CON ;

__sbit __at (0x99) S0CON_ENCIF_1;
__sbit __at (0x98) S0CON_ENCIF_0;

__sfr __at (0x99)   HSRC;
__sfr __at (0x9A)   IEN2;
/*IEN2 bits*/
#define WDTIE	0x20
#define P1IE	0x10
#define UTX1IE	0x08
#define UTX0IE	0x04
#define P2IE	0x02
#define RFIE	0x01
__sfr __at (0x9B)   S1CON;
/*S1CON bits*/
#define RFIF_1	0x02
#define RFIF_0	0x01
__sfr __at (0x9C)   T2PEROF0;
__sfr __at (0x9D)   T2PEROF1;
__sfr __at (0x9E)   T2PEROF2;
/*T2PEROF2 bits*/
#define CMPIM	0x80
#define PERIM	0x40
#define OFCMPIM	0x20

#define PEROF23	0x08
#define PEROF22	0x04
#define PEROF21	0x02
#define PEROF20	0x01

__sfr __at (0x9F) FMAP;
__sfr __at (0x9F) PSBANK;

__sfr __at (0xA0) P2   ;
/* P2 */
__sbit __at (0xA0) P2_0 ;
__sbit __at (0xA1) P2_1 ;
__sbit __at (0xA2) P2_2 ;
__sbit __at (0xA3) P2_3 ;
__sbit __at (0xA4) P2_4 ;
/*__sbit __at (0xA5) P2_5 ;
__sbit __at (0xA6) P2_6 ;
__sbit __at (0xA7) P2_7 ;*/

__sfr __at (0xA1)   T2OF0;
__sfr __at (0xA2)   T2OF1;
__sfr __at (0xA3)   T2OF2;
__sfr __at (0xA4)   T2CAPLPL;
__sfr __at (0xA5)   T2CAPHPH;
__sfr __at (0xA6)   T2TLD;
__sfr __at (0xA7)   T2THD;

__sfr __at (0xA8) IE   ;
__sfr __at (0xA8)   IEN0;
/*IEN0 bits*/
#define IEN0_EA_MASK	0x80
#define STIE	0x20
#define ENCIE	0x10
#define URX1IE	0x08
#define URX0IE	0x04
#define ADCIE	0x02
#define RFERRIE	0x01
/*  IEN0 (0xA8) */
__sbit __at (0xAF) EA;
__sbit __at (0xAF) IEN0_EA;
/*__sbit __at (0xAE) RES;*/
__sbit __at (0xAD) IEN0_STIE;
__sbit __at (0xAC) IEN0_ENCIE;
__sbit __at (0xAB) IEN0_URX1IE;
__sbit __at (0xAA) IEN0_URX0IE;
__sbit __at (0xA9) IEN0_ADCIE;
__sbit __at (0xA8) IEN0_RFERRIE;

__sfr __at (0xA9)   IP0;
/*IP0 bits*/
#define IP0_5	0x20
#define IP0_4	0x10
#define IP0_3	0x08
#define IP0_2	0x04
#define IP0_1	0x02
#define IP0_0	0x01
__sfr __at (0xAB)   FWT;
__sfr __at (0xAC)   FADDRL;
__sfr __at (0xAD)   FADDRH;

__sfr __at (0xAE)   FCTL;
#define F_BUSY	0x80
#define F_SWBSY	0x40
#define F_CONTRD 0x10
#define F_WRITE 0x02
#define F_ERASE	0x01
__sfr __at (0xAF)   FWDATA;

/*No port 3 (0xB0)*/
__sfr __at (0xB1)   ENCDI;
__sfr __at (0xB2)   ENCDO;
__sfr __at (0xB3)   ENCCS;
#define CCS_MODE2	0x40
#define CCS_MODE1	0x20
#define CCS_MODE0	0x10
#define CCS_RDY		0x08
#define CCS_CMD1	0x04
#define CCS_CMD0	0x02
#define CCS_ST		0x01
__sfr __at (0xB4)   ADCCON1;
/*ADCCON1 bits*/
#define ADEOC	0x80
#define ADST	0x40
#define ADSTS1	0x20
#define ADSTS0	0x10
#define ADRCTRL1	0x08
#define ADRCTRL0	0x04
__sfr __at (0xB5)   ADCCON2;
/*ADCCON2 bits*/
#define ADSREF1	0x80
#define ADSREF0	0x40
#define ADSDIV1	0x20
#define ADSDIV0	0x10
#define ADSCH3	0x08
#define ADSCH2	0x04
#define ADSCH1	0x02
#define ADSCH0	0x01
__sfr __at (0xB6)   ADCCON3;
/*ADCCON3 bits*/
#define ADEREF1	0x80
#define ADEREF0	0x40
#define ADEDIV1	0x20
#define ADEDIV0	0x10
#define ADECH3	0x08
#define ADECH2	0x04
#define ADECH1	0x02
#define ADECH0	0x01

__sfr __at (0xB7)   RCCTL;
#undef IP  /*this is 0xb8 in base core*/

__sfr __at (0xB8)   IEN1;
/*IEN1 bits*/
#define P0IE	0x20
#define T4IE	0x10
#define T3IE	0x08
#define T2IE	0x04
#define T1IE	0x02
#define DMAIE	0x01
/*  IEN1 (0xB8) */
/*__sbit __at (0xBF) IEN1_RES;*/
/*__sbit __at (0xBE) RES;*/
__sbit __at (0xBD) IEN1_P0IE;
__sbit __at (0xBC) IEN1_T4IE;
__sbit __at (0xBB) IEN1_T3IE;
__sbit __at (0xBA) IEN1_T2IE;
__sbit __at (0xB9) IEN1_T1IE;
__sbit __at (0xB8) IEN1_DMAIE;

__sfr __at (0xB9)   IP1;
/*IP1 bits*/
#define IP1_5	0x20
#define IP1_4	0x10
#define IP1_3	0x08
#define IP1_2	0x04
#define IP1_1	0x02
#define IP1_0	0x01

__sfr __at (0xBA)   ADCL;
__sfr __at (0xBB)   ADCH;
__sfr __at (0xBC)   RNDL;
__sfr __at (0xBD)   RNDH;

__sfr __at (0xBE)   SLEEP;
#define OSC32K_CALDIS  0x80
#define XOSC_STB	0x40
#define HFRC_STB	0x20
#define RST1		0x10
#define RST0		0x08
#define OSC_PD		0x04
#define SLEEP_MODE1	0x02
#define SLEEP_MODE0	0x01

__sfr __at (0xC0)   IRCON;
/*IRCON bits*/
#define STIF	0x80
#define P0IF	0x20
#define T4IF	0x10
#define T3IF	0x08
#define T2IF	0x04
#define T1IF	0x02
#define DMAIF	0x01
/* IRCON */
__sbit __at (0xC7) IRCON_STIF ;
/*__sbit __at (0x86) IRCON_6 ;*/
__sbit __at (0xC5) IRCON_P0IF;
__sbit __at (0xC4) IRCON_T4IF;
__sbit __at (0xC3) IRCON_T3IF;
__sbit __at (0xC2) IRCON_T2IF;
__sbit __at (0xC1) IRCON_T1IF;
__sbit __at (0xC0) IRCON_DMAIF;

__sfr __at (0xC1)   U0BUF;

__sfr __at (0xC2)   U0BAUD;
__sfr __at (0xC3)   T2CNF;
/*T2SEL bits*/
#define CMPIF	0x80
#define PERIF	0x40
#define OFCMPIF	0x20

#define CMSEL	0x08

#define SYNC	0x02
#define RUN	0x01

__sfr __at (0xC4)   U0UCR;
#define U_FLUSH		0x80
#define U_FLOW		0x40
#define U_D9		0x20
#define U_BIT9		0x10
#define U_PARITY	0x08
#define U_SPB		0x04
#define	U_STOP		0x02
#define U_START 	0x01

__sfr __at (0xC5)   U0GCR;
#define U_CPOL		0x80
#define U_CPHA		0x40
#define U_ORDER		0x20
#define U_BAUD_E4	0x10
#define U_BAUD_E3	0x08
#define U_BAUD_E2	0x04
#define U_BAUD_E1	0x02
#define U_BAUD_E0	0x01

__sfr __at (0xC6)   CLKCON;
#define OSC32K		0x80
#define OSC		0x40
#define TICKSPD2	0x20
#define TICKSPD1	0x10
#define TICKSPD0	0x08
#define CLKSPD		0x01

__sfr __at (0xC7)   MEMCTR;
#define MUNIF    0x40
__sfr __at (0xC8)   T2CON;

__sfr __at (0xC9)   WDCTL;
#define WDT_CLR3 0x80
#define WDT_CLR2 0x40
#define WDT_CLR1 0x20
#define WDT_CLR0 0x10
#define WDT_EN   0x08
#define WDT_MODE 0x04
#define WDT_INT1 0x02
#define WDT_INT0 0x01

__sfr __at (0xCA)   T3CNT;

__sfr __at (0xCB)   T3CTL;
/*T3CTL bits*/
#define T3DIV2	0x80
#define T3DIV1	0x40
#define T3DIV0	0x20
#define T3START	0x10
#define T3OVFIM	0x08
#define T3CLR	0x04
#define T3MODE1	0x02
#define T3MODE0	0x01

__sfr __at (0xCC)   T3CCTL0;
/*T3CCTL0 bits*/
#define T3IM	0x40
#define T3CMP2	0x20
#define T3CMP1	0x10
#define T3CMP0	0x08
#define T3MODE	0x04
#define T3CAP1	0x02
#define T3CAP0	0x01

__sfr __at (0xCD)   T3CC0;
__sfr __at (0xCE)   T3CCTL1;
/*T3CCTL0 bits apply*/
__sfr __at (0xCF)   T3CC1;

__sfr __at (0xD0) PSW  ;
/*  PSW   */
__sbit __at (0xD0) P    ;
__sbit __at (0xD1) F1   ;
__sbit __at (0xD2) OV   ;
__sbit __at (0xD3) RS0  ;
__sbit __at (0xD4) RS1  ;
__sbit __at (0xD5) F0   ;
__sbit __at (0xD6) AC   ;
__sbit __at (0xD7) CY   ;

__sfr __at (0xD1)   DMAIRQ;
/*DMAIRQ bits*/
#define DMAIF4	0x10
#define DMAIF3	0x08
#define DMAIF2	0x04
#define DMAIF1	0x02
#define DMAIF0	0x01

__sfr __at (0xD2)   DMA1CFGL;
__sfr __at (0xD3)   DMA1CFGH;
__sfr __at (0xD4)   DMA0CFGL;
__sfr __at (0xD5)   DMA0CFGH;

__sfr __at (0xD6)   DMAARM;
/*DMAARM bits*/
#define ABORT	0x80
#define DMAARM4	0x10
#define DMAARM3	0x08
#define DMAARM2	0x04
#define DMAARM1	0x02
#define DMAARM0	0x01

__sfr __at (0xD7)   DMAREQ;
/*DMAREQ bits*/
#define DMAREQ4	0x10
#define DMAREQ3	0x08
#define DMAREQ2	0x04
#define DMAREQ1	0x02
#define DMAREQ0	0x01

__sfr __at (0xD8)   TIMIF;
/*TIMIF bits*/
#define OVFIM	0x40
#define T4CH1IF	0x20
#define T4CH0IF	0x10
#define T4OVFIF	0x08
#define T3CH1IF	0x04
#define T3CH0IF	0x02
#define T3OVFIF	0x01

__sfr __at (0xD9)   RFD;
__sfr __at (0xDA)   T1CC0L;
__sfr __at (0xDB)   T1CC0H;
__sfr __at (0xDC)   T1CC1L;
__sfr __at (0xDD)   T1CC1H;
__sfr __at (0xDE)   T1CC2L;
__sfr __at (0xDF)   T1CC2H;

__sfr __at (0xE0)   ACC;
__sfr __at (0xE1)   RFST;
__sfr __at (0xE2)   T1CNTL;
__sfr __at (0xE3)   T1CNTH;

__sfr __at (0xE4)   T1CTL;
/*T1CTL bits*/
#define CH2IF	0x80
#define CH1IF	0x40
#define CH0IF	0x20
#define OVFIF	0x10
#define T1DIV1	0x08
#define T1DIV0	0x04
#define T1MODE1	0x02
#define T1MODE0	0x01

__sfr __at (0xE5)   T1CCTL0;
/*T1CCTL0 bits*/
#define T1CPSEL	0x80
#define T1IM	0x40
#define T1CMP2	0x20
#define T1CMP1	0x10
#define T1CMP0	0x08
#define T1MODE	0x04
#define T1CAP1	0x02
#define T1CAP0	0x01

__sfr __at (0xE6)   T1CCTL1;
/*Bits defined in T1CCTL0 */
__sfr __at (0xE7)   T1CCTL2;
/*Bits defined in T1CCTL0 */
__sfr __at (0xE8)   IRCON2;
/*IRCON2 bits*/
#define WDTIF	0x10
#define P1IF	0x08
#define UTX1IF	0x04
#define UTX0IF	0x02
#define P2IF	0x01
/* IRCON 2 */ 
/*__sbit __at (0xEF) IRCON2_P1_7 ;
__sbit __at (0xEE) IRCON2_P1_6 ;
__sbit __at (0xED) IRCON2_P1_5 ;*/
__sbit __at (0xEC) IRCON2_WDTIF ;
__sbit __at (0xEB) IRCON2_P1IF ;
__sbit __at (0xEA) IRCON2_UTX1IF ;
__sbit __at (0xE9) IRCON2_UTX0IF ;
__sbit __at (0xE8) IRCON2_P2IF;


__sfr __at (0xE9)   RFIF;
/*RFIF bits*/
#define IRQ_RREG_ON	0x80
#define IRQ_TXDONE	0x40
#define IRQ_FIFOP	0x20
#define IRQ_SFD		0x10
#define IRQ_CCA		0x08
#define IRQ_CSP_WT	0x04
#define IRQ_CSP_STOP	0x02
#define IRQ_CSP_INT	0x01

__sfr __at (0xEA)   T4CNT;
__sfr __at (0xEB)   T4CTL;
/*T4CTL bits*/
#define T4DIV2	0x80
#define T4DIV1	0x40
#define T4DIV0	0x20
#define T4START	0x10
#define T4OVFIM	0x08
#define T4CLR	0x04
#define T4MODE1	0x02
#define T4MODE0	0x01

__sfr __at (0xEC)   T4CCTL0;
/*T4CCTL0 bits*/
#define T4IM	0x40
#define T4CMP2	0x20
#define T4CMP1	0x10
#define T4CMP0	0x08
#define T4MODE	0x04
#define T4CAP1	0x02
#define T4CAP0	0x01

__sfr __at (0xED)   T4CC0;
__sfr __at (0xEE)   T4CCTL1;
/*T4CCTL0 bits apply*/
__sfr __at (0xEF)   T4CC1;

__sfr __at (0xF0) B    ;
__sfr __at (0xF1)   PERCFG;
/*PERCFG bits*/
#define T1CFG	0x40
#define T3CFG	0x20
#define T4CFG	0x10
#define U1CFG	0x02
#define U0CFG	0x01

__sfr __at (0xF2)   ADCCFG;
/*ADCCFG bits*/
#define ADC7EN	0x80
#define ADC6EN	0x40
#define ADC5EN	0x20
#define ADC4EN	0x10
#define ADC3EN	0x08
#define ADC2EN	0x04
#define ADC1EN	0x02
#define ADC0EN	0x01

__sfr __at (0xF3)   P0SEL;
__sfr __at (0xF4)   P1SEL;
__sfr __at (0xF5)   P2SEL;
/*P2SEL bits*/
#define PRI3P1	0x40
#define PRI2P1	0x20
#define PRI1P1	0x10
#define PRI0P1	0x08
#define SELP2_4	0x04
#define SELP2_3	0x02
#define SELP2_0	0x01

__sfr __at (0xF6)   P1INP;

__sfr __at (0xF7)   P2INP;
/*P2INP bits*/
#define PDUP2	0x80
#define PDUP1	0x40
#define PDUP0	0x20
#define MDP2_4	0x10
#define MDP2_3	0x08
#define MDP2_2	0x04
#define MDP2_1	0x02
#define MDP2_0	0x01

__sfr __at (0xF8)   U1CSR;
__sfr __at (0xF9)   U1BUF;
__sfr __at (0xFA)   U1BAUD;
__sfr __at (0xFB)   U1UCR;
__sfr __at (0xFC)   U1GCR;
__sfr __at (0xFD)   P0DIR;
__sfr __at (0xFE)   P1DIR;

__sfr __at (0xFF)   P2DIR;
/*P2DIR bits*/
#define PRI1P0	0x80
#define PRI0P0	0x40
#define DIRP2_4	0x10
#define DIRP2_3	0x08
#define DIRP2_2	0x04
#define DIRP2_1	0x02
#define DIRP2_0	0x01

/*  IEN0  */
/*__sbit __at (0xA8) EA   ;
__sbit __at (0x99) TI   ;
__sbit __at (0x9A) RB8  ;
__sbit __at (0x9B) TB8  ;
__sbit __at (0x9C) REN  ;
__sbit __at (0x9D) SM2  ;
__sbit __at (0x9E) SM1  ;
__sbit __at (0x9F) SM0  ;*/



/* Interrupt numbers: address = (number * 8) + 3 */
/*#undef IE0_VECTOR
#undef TF0_VECTOR
#undef IE1_VECTOR
#undef TF1_VECTOR
#undef SI0_VECTOR*/

/* CC2430 interrupt vectors */
#define RFERR_VECTOR	0
#define ADC_VECTOR	1
#define URX0_VECTOR	2
#define URX1_VECTOR	3
#define ENC_VECTOR	4
#define ST_VECTOR	5
#define P2INT_VECTOR	6
#define UTX0_VECTOR	7
#define DMA_VECTOR	8
#define T1_VECTOR	9
#define T2_VECTOR	10
#define T3_VECTOR	11
#define T4_VECTOR	12
#define P0INT_VECTOR	13
#define UTX1_VECTOR	14
#define P1INT_VECTOR	15
#define RF_VECTOR	16
#define WDT_VECTOR	17

/* RF control registers*/
__xdata __at (0xDF02) unsigned char MDMCTRL0H;
__xdata __at (0xDF03) unsigned char MDMCTRL0L;
__xdata __at (0xDF04) unsigned char MDMCTRL1H;
__xdata __at (0xDF05) unsigned char MDMCTRL1L;
__xdata __at (0xDF06) unsigned char RSSIH;
__xdata __at (0xDF07) unsigned char RSSIL;
__xdata __at (0xDF08) unsigned char SYNCWORDH;
__xdata __at (0xDF09) unsigned char SYNCWORDL;
__xdata __at (0xDF0A) unsigned char TXCTRLH;
__xdata __at (0xDF0B) unsigned char TXCTRLL;
__xdata __at (0xDF0C) unsigned char RXCTRL0H;
__xdata __at (0xDF0D) unsigned char RXCTRL0L;
__xdata __at (0xDF0E) unsigned char RXCTRL1H;
__xdata __at (0xDF0F) unsigned char RXCTRL1L;
__xdata __at (0xDF10) unsigned char FSCTRLH;
__xdata __at (0xDF11) unsigned char FSCTRLL;
__xdata __at (0xDF12) unsigned char CSPX;
__xdata __at (0xDF13) unsigned char CSPY;
__xdata __at (0xDF14) unsigned char CSPZ;
__xdata __at (0xDF15) unsigned char CSPCTRL;
__xdata __at (0xDF16) unsigned char CSPT;
__xdata __at (0xDF17) unsigned char RFPWR;
#define ADI_RADIO_PD	0x10
#define RREG_RADIO_PD	0x08
#define RREG_DELAY_MASK 0x07

__xdata __at (0xDF20) unsigned char FSMTCH;
__xdata __at (0xDF21) unsigned char FSMTCL;
__xdata __at (0xDF22) unsigned char MANANDH;
__xdata __at (0xDF23) unsigned char MANANDL;
__xdata __at (0xDF24) unsigned char MANORH;
__xdata __at (0xDF25) unsigned char MANORL;
__xdata __at (0xDF26) unsigned char AGCCTRLH;
__xdata __at (0xDF27) unsigned char AGCCTRLL;

__xdata __at (0xDF39) unsigned char FSMSTATE;
__xdata __at (0xDF3A) unsigned char ADCTSTH;
__xdata __at (0xDF3B) unsigned char ADCTSTL;
__xdata __at (0xDF3C) unsigned char DACTSTH;
__xdata __at (0xDF3D) unsigned char DACTSTL;

__xdata __at (0xDF43) unsigned char IEEE_ADDR0;
__xdata __at (0xDF44) unsigned char IEEE_ADDR1;
__xdata __at (0xDF45) unsigned char IEEE_ADDR2;
__xdata __at (0xDF46) unsigned char IEEE_ADDR3;
__xdata __at (0xDF47) unsigned char IEEE_ADDR4;
__xdata __at (0xDF48) unsigned char IEEE_ADDR5;
__xdata __at (0xDF49) unsigned char IEEE_ADDR6;
__xdata __at (0xDF4A) unsigned char IEEE_ADDR7;
__xdata __at (0xDF4B) unsigned char PANIDH;
__xdata __at (0xDF4C) unsigned char PANIDL;
__xdata __at (0xDF4D) unsigned char SHORTADDRH;
__xdata __at (0xDF4E) unsigned char SHORTADDRL;
__xdata __at (0xDF4F) unsigned char IOCFG0;
__xdata __at (0xDF50) unsigned char IOCFG1;
__xdata __at (0xDF51) unsigned char IOCFG2;
__xdata __at (0xDF52) unsigned char IOCFG3;
__xdata __at (0xDF53) unsigned char RXFIFOCNT;
__xdata __at (0xDF54) unsigned char FSMTC1;
#define ABORTRX_ON_SRXON 0x20
#define RX_INTERRUPTED	0x10
#define AUTO_TX2RX_OFF	0x08
#define RX2RX_TIME_OFF	0x04
#define PENDING_OR 	0x02
#define ACCEPT_ACKPKT	0x01

__xdata __at (0xDF60) unsigned char CHVER;
__xdata __at (0xDF61) unsigned char CHIPID;
__xdata __at (0xDF62) unsigned char RFSTATUS;
#define TX_ACTIVE	0x10
#define FIFO		0x08
#define FIFOP		0x04
#define SFD		0x02
#define CCA		0x01

__xdata __at (0xDFC1) unsigned char   U0BUF_SHADOW;

__xdata __at (0xDFD9) unsigned char RFD_SHADOW;

__xdata __at (0xDFF9) unsigned char U1BUF_SHADOW;

__xdata __at (0xDFBA) unsigned int ADC_SHADOW;

#endif /*REG_CC2430*/
