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
 *         Header file with definitions of bit masks for some cc2530 SFRs
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */


#ifndef SFR_BITS_H_
#define SFR_BITS_H_

/* CLKCON */
#define CLKCONCMD_OSC32K    0x80
#define CLKCONCMD_OSC       0x40
#define CLKCONCMD_TICKSPD2  0x20
#define CLKCONCMD_TICKSPD1  0x10
#define CLKCONCMD_TICKSPD0  0x08
#define CLKCONCMD_CLKSPD2   0x04
#define CLKCONCMD_CLKSPD1   0x02
#define CLKCONCMD_CLKSPD0   0x01

/* SLEEPCMD and SLEEPSTA */
#define SLEEP_OSC32K_CALDIS  0x80
#define SLEEP_XOSC_STB       0x40
#define SLEEP_HFRC_STB       0x20
#define SLEEP_RST1           0x10 /* SLEEPSTA only */
#define SLEEP_RST0           0x08 /* SLEEPSTA only */
#define SLEEP_OSC_PD         0x04
#define SLEEP_MODE1          0x02
#define SLEEP_MODE0          0x01

/* PCON */
#define PCON_IDLE 0x01

/* T1CTL */
#define T1CTL_DIV1  0x08
#define T1CTL_DIV0  0x04
#define T1CTL_MODE1 0x02
#define T1CTL_MODE0 0x01

/* T1CCTLx */
#define T1CCTL_RFIRQ 0x80
#define T1CCTL_IM    0x40
#define T1CCTL_CMP2  0x20
#define T1CCTL_CMP1  0x10
#define T1CCTL_CMP0  0x08
#define T1CCTL_MODE  0x04
#define T1CCTL_CAP1  0x02
#define T1CCTL_CAP0  0x01

/* T1STAT */
#define T1STAT_OVFIF   0x20
#define T1STAT_CH4IF   0x10
#define T1STAT_CH3IF   0x08
#define T1STAT_CH2IF   0x04
#define T1STAT_CH1IF   0x02
#define T1STAT_CH0IF   0x01

/* WDCTL */
#define WDCTL_CLR3  0x80
#define WDCTL_CLR2  0x40
#define WDCTL_CLR1  0x20
#define WDCTL_CLR0  0x10
#define WDCTL_MODE1 0x08
#define WDCTL_MODE0 0x04
#define WDCTL_INT1  0x02
#define WDCTL_INT0  0x01

/* ADCCON1 */
#define ADCCON1_EOC     0x80
#define ADCCON1_ST      0x40
#define ADCCON1_STSEL1  0x20
#define ADCCON1_STSEL0  0x10
/* ADCCON1 - RNG bits */
#define ADCCON1_RCTRL1  0x08
#define ADCCON1_RCTRL0  0x04

/* ADCCON3 */
#define ADCCON3_EREF1 0x80
#define ADCCON3_EREF0 0x40
#define ADCCON3_EDIV1 0x20
#define ADCCON3_EDIV0 0x10
#define ADCCON3_ECH3  0x08
#define ADCCON3_ECH2  0x04
#define ADCCON3_ECH1  0x02
#define ADCCON3_ECH0  0x01

/* PERCFG */
#define PERCFG_T1CFG 0x40
#define PERCFG_T3CFG 0x20
#define PERCFG_T4CFG 0x10
#define PERCFG_U1CFG 0x02
#define PERCFG_U0CFG 0x01

/* UxCSR */
#define UCSR_MODE    0x80
#define UCSR_RE      0x40
#define UCSR_SLAVE   0x20
#define UCSR_FE      0x10
#define UCSR_ERR     0x08
#define UCSR_RX_BYTE 0x04
#define UCSR_TX_BYTE 0x02
#define UCSR_ACTIVE  0x01

/* IEN2 */
#define IEN2_WDTIE   0x20
#define IEN2_P1IE    0x10
#define IEN2_UTX1IE  0x08
#define IEN2_UTX0IE  0x04
#define IEN2_P2IE    0x02
#define IEN2_RFIE    0x01

/* PICTL */
#define PICTL_PADSC   0x40
#define PICTL_P2ICON  0x08
#define PICTL_P1ICONH 0x04
#define PICTL_P1ICONL 0x02
#define PICTL_P0ICON  0x01

/* DMAARM */
#define DMAARM_ABORT   0x80
#define DMAARM_DMAARM4 0x10
#define DMAARM_DMAARM3 0x08
#define DMAARM_DMAARM2 0x04
#define DMAARM_DMAARM1 0x02
#define DMAARM_DMAARM0 0x01

/* DMAREQ */
#define DMAREQ_DMAREQ4 0x10
#define DMAREQ_DMAREQ3 0x08
#define DMAREQ_DMAREQ2 0x04
#define DMAREQ_DMAREQ1 0x02
#define DMAREQ_DMAREQ0 0x01

/* DMAIRQ */
#define DMAIRQ_DMAIF4  0x10
#define DMAIRQ_DMAIF3  0x08
#define DMAIRQ_DMAIF2  0x04
#define DMAIRQ_DMAIF1  0x02
#define DMAIRQ_DMAIF0  0x01
/*---------------------------------------------------------------------------
 * XREG bits, excluding RF and USB
 *---------------------------------------------------------------------------*/
/* FCTL */
#define FCTL_BUSY     0x80
#define FCTL_FULL     0x40
#define FCTL_ABORT    0x20
#define FCTL_CM1      0x08
#define FCTL_CM0      0x04
#define FCTL_WRITE    0x02
#define FCTL_ERASE    0x01
/*---------------------------------------------------------------------------
 * Radio Register Bits
 *---------------------------------------------------------------------------*/
/* FRMFILT0 */
#define FRMFILT0_FRAME_FILTER_EN    0x01

/* FRMCTRL0 */
#define FRMCTRL0_APPEND_DATA_MODE   0x80
#define FRMCTRL0_AUTOCRC            0x40
#define FRMCTRL0_AUTOACK            0x20
#define FRMCTRL0_ENERGY_SCAN        0x10
#define FRMCTRL0_RX_MODE1           0x08
#define FRMCTRL0_RX_MODE0           0x04
#define FRMCTRL0_TX_MODE1           0x02
#define FRMCTRL0_TX_MODE0           0x01

/* FRMCTRL1 */
#define FRMCTRL1_PENDING_OR         0x04
#define FRMCTRL1_IGNORE_TX_UNDERF   0x02
#define FRMCTRL1_SET_RXENMASK_ON_TX 0x01

/* FSMSTAT1 */
#define FSMSTAT1_FIFO               0x80
#define FSMSTAT1_FIFOP              0x40
#define FSMSTAT1_SFD                0x20
#define FSMSTAT1_CCA                0x10
#define FSMSTAT1_TX_ACTIVE          0x02
#define FSMSTAT1_RX_ACTIVE          0x01

/* RSSISTAT */
#define RSSISTAT_RSSI_VALID         0x01

/* RFRND */
#define RFRND_QRND                  0x02
#define RFRND_IRND                  0x01

#endif /* SFR_BITS_H_ */
