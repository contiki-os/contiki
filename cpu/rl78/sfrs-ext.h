/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Ian Martin <martini@redwirellc.com>
 */

#ifndef SFRS_EXT_H
#define SFRS_EXT_H

#define ADM2     (*(volatile unsigned char  *)0xF0010)
#define ADUL     (*(volatile unsigned char  *)0xF0011)
#define ADLL     (*(volatile unsigned char  *)0xF0012)
#define ADTES    (*(volatile unsigned char  *)0xF0013)
#define PU0      (*(volatile unsigned char  *)0xF0030)
#define PU1      (*(volatile unsigned char  *)0xF0031)
#define PU3      (*(volatile unsigned char  *)0xF0033)
#define PU4      (*(volatile unsigned char  *)0xF0034)
#define PU5      (*(volatile unsigned char  *)0xF0035)
#define PU6      (*(volatile unsigned char  *)0xF0036)
#define PU7      (*(volatile unsigned char  *)0xF0037)
#define PU8      (*(volatile unsigned char  *)0xF0038)
#define PU9      (*(volatile unsigned char  *)0xF0039)
#define PU10     (*(volatile unsigned char  *)0xF003A)
#define PU11     (*(volatile unsigned char  *)0xF003B)
#define PU12     (*(volatile unsigned char  *)0xF003C)
#define PU14     (*(volatile unsigned char  *)0xF003E)
#define PIM0     (*(volatile unsigned char  *)0xF0040)
#define PIM1     (*(volatile unsigned char  *)0xF0041)
#define PIM4     (*(volatile unsigned char  *)0xF0044)
#define PIM5     (*(volatile unsigned char  *)0xF0045)
#define PIM8     (*(volatile unsigned char  *)0xF0048)
#define PIM14    (*(volatile unsigned char  *)0xF004E)
#define POM0     (*(volatile unsigned char  *)0xF0050)
#define POM1     (*(volatile unsigned char  *)0xF0051)
#define POM4     (*(volatile unsigned char  *)0xF0054)
#define POM5     (*(volatile unsigned char  *)0xF0055)
#define POM7     (*(volatile unsigned char  *)0xF0057)
#define POM8     (*(volatile unsigned char  *)0xF0058)
#define POM9     (*(volatile unsigned char  *)0xF0059)
#define POM14    (*(volatile unsigned char  *)0xF005E)
#define PMC0     (*(volatile unsigned char  *)0xF0060)
#define PMC3     (*(volatile unsigned char  *)0xF0063)
#define PMC10    (*(volatile unsigned char  *)0xF006A)
#define PMC11    (*(volatile unsigned char  *)0xF006B)
#define PMC12    (*(volatile unsigned char  *)0xF006C)
#define PMC14    (*(volatile unsigned char  *)0xF006E)
#define NFEN0    (*(volatile unsigned char  *)0xF0070)
#define NFEN1    (*(volatile unsigned char  *)0xF0071)
#define NFEN2    (*(volatile unsigned char  *)0xF0072)
#define ISC      (*(volatile unsigned char  *)0xF0073)
#define TIS0     (*(volatile unsigned char  *)0xF0074)
#define ADPC     (*(volatile unsigned char  *)0xF0076)
#define PIOR     (*(volatile unsigned char  *)0xF0077)
#define IAWCTL   (*(volatile unsigned char  *)0xF0078)
#define GDIDIS   (*(volatile unsigned char  *)0xF007D)
#define PRDSEL   (*(volatile unsigned short *)0xF007E)
#define TOOLEN   (*(volatile unsigned char  *)0xF0080)
#define BPAL0    (*(volatile unsigned char  *)0xF0081)
#define BPAH0    (*(volatile unsigned char  *)0xF0082)
#define BPAS0    (*(volatile unsigned char  *)0xF0083)
#define BACDVL0  (*(volatile unsigned char  *)0xF0084)
#define BACDVH0  (*(volatile unsigned char  *)0xF0085)
#define BACDML0  (*(volatile unsigned char  *)0xF0086)
#define BACDMH0  (*(volatile unsigned char  *)0xF0087)
#define MONMOD   (*(volatile unsigned char  *)0xF0088)
#define DFLCTL   (*(volatile unsigned char  *)0xF0090)
#define HIOTRM   (*(volatile unsigned char  *)0xF00A0)
#define BECTL    (*(volatile unsigned char  *)0xF00A1)
#define HOCODIV  (*(volatile unsigned char  *)0xF00A8)
#define TEMPCAL0 (*(volatile unsigned char  *)0xF00AC)
#define TEMPCAL1 (*(volatile unsigned char  *)0xF00AD)
#define TEMPCAL2 (*(volatile unsigned char  *)0xF00AE)
#define TEMPCAL3 (*(volatile unsigned char  *)0xF00AF)
#define FLSEC    (*(volatile unsigned short *)0xF00B0)
#define FLFSWS   (*(volatile unsigned short *)0xF00B2)
#define FLFSWE   (*(volatile unsigned short *)0xF00B4)
#define FSSET    (*(volatile unsigned char  *)0xF00B6)
#define FSSE     (*(volatile unsigned char  *)0xF00B7)
#define FLFADL   (*(volatile unsigned short *)0xF00B8)
#define FLFADH   (*(volatile unsigned char  *)0xF00BA)
#define PFCMD    (*(volatile unsigned char  *)0xF00C0)
#define PFS      (*(volatile unsigned char  *)0xF00C1)
#define FLRL     (*(volatile unsigned short *)0xF00C2)
#define FLRH     (*(volatile unsigned short *)0xF00C4)
#define FLWE     (*(volatile unsigned char  *)0xF00C6)
#define FLRE     (*(volatile unsigned char  *)0xF00C7)
#define FLTMS    (*(volatile unsigned short *)0xF00C8)
#define DFLMC    (*(volatile unsigned short *)0xF00CA)
#define FLMCL    (*(volatile unsigned short *)0xF00CC)
#define FLMCH    (*(volatile unsigned char  *)0xF00CE)
#define FSCTL    (*(volatile unsigned char  *)0xF00CF)
#define ICEADR   (*(volatile unsigned short *)0xF00D0)
#define ICEDAT   (*(volatile unsigned short *)0xF00D2)
#define MDCL     (*(volatile unsigned short *)0xF00E0)
#define MDCH     (*(volatile unsigned short *)0xF00E2)
#define MDUC     (*(volatile unsigned char  *)0xF00E8)
#define PER0     (*(volatile unsigned char  *)0xF00F0)
#define OSMC     (*(volatile unsigned char  *)0xF00F3)
#define RMC      (*(volatile unsigned char  *)0xF00F4)
#define RPECTL   (*(volatile unsigned char  *)0xF00F5)
#define BCDADJ   (*(volatile unsigned char  *)0xF00FE)
#define VECTCTRL (*(volatile unsigned char  *)0xF00FF)
#define SSR00    (*(volatile unsigned short *)0xF0100)
#define SSR00L   (*(volatile unsigned char  *)0xF0100)
#define SSR01    (*(volatile unsigned short *)0xF0102)
#define SSR01L   (*(volatile unsigned char  *)0xF0102)
#define SSR02    (*(volatile unsigned short *)0xF0104)
#define SSR02L   (*(volatile unsigned char  *)0xF0104)
#define SSR03    (*(volatile unsigned short *)0xF0106)
#define SSR03L   (*(volatile unsigned char  *)0xF0106)
#define SIR00    (*(volatile unsigned short *)0xF0108)
#define SIR00L   (*(volatile unsigned char  *)0xF0108)
#define SIR01    (*(volatile unsigned short *)0xF010A)
#define SIR01L   (*(volatile unsigned char  *)0xF010A)
#define SIR02    (*(volatile unsigned short *)0xF010C)
#define SIR02L   (*(volatile unsigned char  *)0xF010C)
#define SIR03    (*(volatile unsigned short *)0xF010E)
#define SIR03L   (*(volatile unsigned char  *)0xF010E)
#define SMR00    (*(volatile unsigned short *)0xF0110)
#define SMR01    (*(volatile unsigned short *)0xF0112)
#define SMR02    (*(volatile unsigned short *)0xF0114)
#define SMR03    (*(volatile unsigned short *)0xF0116)
#define SCR00    (*(volatile unsigned short *)0xF0118)
#define SCR01    (*(volatile unsigned short *)0xF011A)
#define SCR02    (*(volatile unsigned short *)0xF011C)
#define SCR03    (*(volatile unsigned short *)0xF011E)
#define SE0      (*(volatile unsigned short *)0xF0120)
#define SE0L     (*(volatile unsigned char  *)0xF0120)
#define SS0      (*(volatile unsigned short *)0xF0122)
#define SS0L     (*(volatile unsigned char  *)0xF0122)
#define ST0      (*(volatile unsigned short *)0xF0124)
#define ST0L     (*(volatile unsigned char  *)0xF0124)
#define SPS0     (*(volatile unsigned short *)0xF0126)
#define SPS0L    (*(volatile unsigned char  *)0xF0126)
#define SO0      (*(volatile unsigned short *)0xF0128)
#define SOE0     (*(volatile unsigned short *)0xF012A)
#define SOE0L    (*(volatile unsigned char  *)0xF012A)
#define EDR00    (*(volatile unsigned short *)0xF012C)
#define EDR00L   (*(volatile unsigned char  *)0xF012C)
#define EDR01    (*(volatile unsigned short *)0xF012E)
#define EDR01L   (*(volatile unsigned char  *)0xF012E)
#define EDR02    (*(volatile unsigned short *)0xF0130)
#define EDR02L   (*(volatile unsigned char  *)0xF0130)
#define EDR03    (*(volatile unsigned short *)0xF0132)
#define EDR03L   (*(volatile unsigned char  *)0xF0132)
#define SOL0     (*(volatile unsigned short *)0xF0134)
#define SOL0L    (*(volatile unsigned char  *)0xF0134)
#define SSC0     (*(volatile unsigned short *)0xF0138)
#define SSC0L    (*(volatile unsigned char  *)0xF0138)
#define SSR10    (*(volatile unsigned short *)0xF0140)
#define SSR10L   (*(volatile unsigned char  *)0xF0140)
#define SSR11    (*(volatile unsigned short *)0xF0142)
#define SSR11L   (*(volatile unsigned char  *)0xF0142)
#define SSR12    (*(volatile unsigned short *)0xF0144)
#define SSR12L   (*(volatile unsigned char  *)0xF0144)
#define SSR13    (*(volatile unsigned short *)0xF0146)
#define SSR13L   (*(volatile unsigned char  *)0xF0146)
#define SIR10    (*(volatile unsigned short *)0xF0148)
#define SIR10L   (*(volatile unsigned char  *)0xF0148)
#define SIR11    (*(volatile unsigned short *)0xF014A)
#define SIR11L   (*(volatile unsigned char  *)0xF014A)
#define SIR12    (*(volatile unsigned short *)0xF014C)
#define SIR12L   (*(volatile unsigned char  *)0xF014C)
#define SIR13    (*(volatile unsigned short *)0xF014E)
#define SIR13L   (*(volatile unsigned char  *)0xF014E)
#define SMR10    (*(volatile unsigned short *)0xF0150)
#define SMR11    (*(volatile unsigned short *)0xF0152)
#define SMR12    (*(volatile unsigned short *)0xF0154)
#define SMR13    (*(volatile unsigned short *)0xF0156)
#define SCR10    (*(volatile unsigned short *)0xF0158)
#define SCR11    (*(volatile unsigned short *)0xF015A)
#define SCR12    (*(volatile unsigned short *)0xF015C)
#define SCR13    (*(volatile unsigned short *)0xF015E)
#define SE1      (*(volatile unsigned short *)0xF0160)
#define SE1L     (*(volatile unsigned char  *)0xF0160)
#define SS1      (*(volatile unsigned short *)0xF0162)
#define SS1L     (*(volatile unsigned char  *)0xF0162)
#define ST1      (*(volatile unsigned short *)0xF0164)
#define ST1L     (*(volatile unsigned char  *)0xF0164)
#define SPS1     (*(volatile unsigned short *)0xF0166)
#define SPS1L    (*(volatile unsigned char  *)0xF0166)
#define SO1      (*(volatile unsigned short *)0xF0168)
#define SOE1     (*(volatile unsigned short *)0xF016A)
#define SOE1L    (*(volatile unsigned char  *)0xF016A)
#define EDR10    (*(volatile unsigned short *)0xF016C)
#define EDR10L   (*(volatile unsigned char  *)0xF016C)
#define EDR11    (*(volatile unsigned short *)0xF016E)
#define EDR11L   (*(volatile unsigned char  *)0xF016E)
#define EDR12    (*(volatile unsigned short *)0xF0170)
#define EDR12L   (*(volatile unsigned char  *)0xF0170)
#define EDR13    (*(volatile unsigned short *)0xF0172)
#define EDR13L   (*(volatile unsigned char  *)0xF0172)
#define SOL1     (*(volatile unsigned short *)0xF0174)
#define SOL1L    (*(volatile unsigned char  *)0xF0174)
#define SSC1     (*(volatile unsigned short *)0xF0178)
#define SSC1L    (*(volatile unsigned char  *)0xF0178)
#define TCR00    (*(volatile unsigned short *)0xF0180)
#define TCR01    (*(volatile unsigned short *)0xF0182)
#define TCR02    (*(volatile unsigned short *)0xF0184)
#define TCR03    (*(volatile unsigned short *)0xF0186)
#define TCR04    (*(volatile unsigned short *)0xF0188)
#define TCR05    (*(volatile unsigned short *)0xF018A)
#define TCR06    (*(volatile unsigned short *)0xF018C)
#define TCR07    (*(volatile unsigned short *)0xF018E)
#define TMR00    (*(volatile unsigned short *)0xF0190)
#define TMR01    (*(volatile unsigned short *)0xF0192)
#define TMR02    (*(volatile unsigned short *)0xF0194)
#define TMR03    (*(volatile unsigned short *)0xF0196)
#define TMR04    (*(volatile unsigned short *)0xF0198)
#define TMR05    (*(volatile unsigned short *)0xF019A)
#define TMR06    (*(volatile unsigned short *)0xF019C)
#define TMR07    (*(volatile unsigned short *)0xF019E)
#define TSR00    (*(volatile unsigned short *)0xF01A0)
#define TSR00L   (*(volatile unsigned char  *)0xF01A0)
#define TSR01    (*(volatile unsigned short *)0xF01A2)
#define TSR01L   (*(volatile unsigned char  *)0xF01A2)
#define TSR02    (*(volatile unsigned short *)0xF01A4)
#define TSR02L   (*(volatile unsigned char  *)0xF01A4)
#define TSR03    (*(volatile unsigned short *)0xF01A6)
#define TSR03L   (*(volatile unsigned char  *)0xF01A6)
#define TSR04    (*(volatile unsigned short *)0xF01A8)
#define TSR04L   (*(volatile unsigned char  *)0xF01A8)
#define TSR05    (*(volatile unsigned short *)0xF01AA)
#define TSR05L   (*(volatile unsigned char  *)0xF01AA)
#define TSR06    (*(volatile unsigned short *)0xF01AC)
#define TSR06L   (*(volatile unsigned char  *)0xF01AC)
#define TSR07    (*(volatile unsigned short *)0xF01AE)
#define TSR07L   (*(volatile unsigned char  *)0xF01AE)
#define TE0      (*(volatile unsigned short *)0xF01B0)
#define TE0L     (*(volatile unsigned char  *)0xF01B0)
#define TS0      (*(volatile unsigned short *)0xF01B2)
#define TS0L     (*(volatile unsigned char  *)0xF01B2)
#define TT0      (*(volatile unsigned short *)0xF01B4)
#define TT0L     (*(volatile unsigned char  *)0xF01B4)
#define TPS0     (*(volatile unsigned short *)0xF01B6)
#define TO0      (*(volatile unsigned short *)0xF01B8)
#define TO0L     (*(volatile unsigned char  *)0xF01B8)
#define TOE0     (*(volatile unsigned short *)0xF01BA)
#define TOE0L    (*(volatile unsigned char  *)0xF01BA)
#define TOL0     (*(volatile unsigned short *)0xF01BC)
#define TOL0L    (*(volatile unsigned char  *)0xF01BC)
#define TOM0     (*(volatile unsigned short *)0xF01BE)
#define TOM0L    (*(volatile unsigned char  *)0xF01BE)
#define TCR10    (*(volatile unsigned short *)0xF01C0)
#define TCR11    (*(volatile unsigned short *)0xF01C2)
#define TCR12    (*(volatile unsigned short *)0xF01C4)
#define TCR13    (*(volatile unsigned short *)0xF01C6)
#define TCR14    (*(volatile unsigned short *)0xF01C8)
#define TCR15    (*(volatile unsigned short *)0xF01CA)
#define TCR16    (*(volatile unsigned short *)0xF01CC)
#define TCR17    (*(volatile unsigned short *)0xF01CE)
#define TMR10    (*(volatile unsigned short *)0xF01D0)
#define TMR11    (*(volatile unsigned short *)0xF01D2)
#define TMR12    (*(volatile unsigned short *)0xF01D4)
#define TMR13    (*(volatile unsigned short *)0xF01D6)
#define TMR14    (*(volatile unsigned short *)0xF01D8)
#define TMR15    (*(volatile unsigned short *)0xF01DA)
#define TMR16    (*(volatile unsigned short *)0xF01DC)
#define TMR17    (*(volatile unsigned short *)0xF01DE)
#define TSR10    (*(volatile unsigned short *)0xF01E0)
#define TSR10L   (*(volatile unsigned char  *)0xF01E0)
#define TSR11    (*(volatile unsigned short *)0xF01E2)
#define TSR11L   (*(volatile unsigned char  *)0xF01E2)
#define TSR12    (*(volatile unsigned short *)0xF01E4)
#define TSR12L   (*(volatile unsigned char  *)0xF01E4)
#define TSR13    (*(volatile unsigned short *)0xF01E6)
#define TSR13L   (*(volatile unsigned char  *)0xF01E6)
#define TSR14    (*(volatile unsigned short *)0xF01E8)
#define TSR14L   (*(volatile unsigned char  *)0xF01E8)
#define TSR15    (*(volatile unsigned short *)0xF01EA)
#define TSR15L   (*(volatile unsigned char  *)0xF01EA)
#define TSR16    (*(volatile unsigned short *)0xF01EC)
#define TSR16L   (*(volatile unsigned char  *)0xF01EC)
#define TSR17    (*(volatile unsigned short *)0xF01EE)
#define TSR17L   (*(volatile unsigned char  *)0xF01EE)
#define TE1      (*(volatile unsigned short *)0xF01F0)
#define TE1L     (*(volatile unsigned char  *)0xF01F0)
#define TS1      (*(volatile unsigned short *)0xF01F2)
#define TS1L     (*(volatile unsigned char  *)0xF01F2)
#define TT1      (*(volatile unsigned short *)0xF01F4)
#define TT1L     (*(volatile unsigned char  *)0xF01F4)
#define TPS1     (*(volatile unsigned short *)0xF01F6)
#define TO1      (*(volatile unsigned short *)0xF01F8)
#define TO1L     (*(volatile unsigned char  *)0xF01F8)
#define TOE1     (*(volatile unsigned short *)0xF01FA)
#define TOE1L    (*(volatile unsigned char  *)0xF01FA)
#define TOL1     (*(volatile unsigned short *)0xF01FC)
#define TOL1L    (*(volatile unsigned char  *)0xF01FC)
#define TOM1     (*(volatile unsigned short *)0xF01FE)
#define TOM1L    (*(volatile unsigned char  *)0xF01FE)
#define DSA2     (*(volatile unsigned char  *)0xF0200)
#define DSA3     (*(volatile unsigned char  *)0xF0201)
#define DRA2     (*(volatile unsigned short *)0xF0202)
#define DRA2L    (*(volatile unsigned char  *)0xF0202)
#define DRA2H    (*(volatile unsigned char  *)0xF0203)
#define DRA3     (*(volatile unsigned short *)0xF0204)
#define DRA3L    (*(volatile unsigned char  *)0xF0204)
#define DRA3H    (*(volatile unsigned char  *)0xF0205)
#define DBC2     (*(volatile unsigned short *)0xF0206)
#define DBC2L    (*(volatile unsigned char  *)0xF0206)
#define DBC2H    (*(volatile unsigned char  *)0xF0207)
#define DBC3     (*(volatile unsigned short *)0xF0208)
#define DBC3L    (*(volatile unsigned char  *)0xF0208)
#define DBC3H    (*(volatile unsigned char  *)0xF0209)
#define DMC2     (*(volatile unsigned char  *)0xF020A)
#define DMC3     (*(volatile unsigned char  *)0xF020B)
#define DRC2     (*(volatile unsigned char  *)0xF020C)
#define DRC3     (*(volatile unsigned char  *)0xF020D)
#define DWAITALL (*(volatile unsigned char  *)0xF020F)
#define IICCTL00 (*(volatile unsigned char  *)0xF0230)
#define IICCTL01 (*(volatile unsigned char  *)0xF0231)
#define IICWL0   (*(volatile unsigned char  *)0xF0232)
#define IICWH0   (*(volatile unsigned char  *)0xF0233)
#define SVA0     (*(volatile unsigned char  *)0xF0234)
#define IICSE0   (*(volatile unsigned char  *)0xF0235)
#define IICCTL10 (*(volatile unsigned char  *)0xF0238)
#define IICCTL11 (*(volatile unsigned char  *)0xF0239)
#define IICWL1   (*(volatile unsigned char  *)0xF023A)
#define IICWH1   (*(volatile unsigned char  *)0xF023B)
#define SVA1     (*(volatile unsigned char  *)0xF023C)
#define IICSE1   (*(volatile unsigned char  *)0xF023D)
#define CRC0CTL  (*(volatile unsigned char  *)0xF02F0)
#define PGCRCL   (*(volatile unsigned short *)0xF02F2)
#define CRCD     (*(volatile unsigned short *)0xF02FA)


struct ADM2struct {
	unsigned char  adtyp     : 1;
	unsigned char            : 1;
	unsigned char  awc       : 1;
	unsigned char  adrck     : 1;
};

struct ADULstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ADLLstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ADTESstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU3struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU4struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU5struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU6struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU7struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU8struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU9struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU10struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU11struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU12struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PU14struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIM0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIM1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIM4struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIM5struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIM8struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIM14struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM4struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM5struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM7struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM8struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM9struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct POM14struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PMC0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PMC3struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PMC10struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PMC11struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PMC12struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PMC14struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct NFEN0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct NFEN1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct NFEN2struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ISCstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TIS0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ADPCstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PIORstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct IAWCTLstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct GDIDISstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PRDSELstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOOLENstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BPAL0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BPAH0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BPAS0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BACDVL0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BACDVH0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BACDML0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BACDMH0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct MONMODstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DFLCTLstruct {
	unsigned char  dflen     : 1;
};

struct HIOTRMstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct BECTLstruct {
	unsigned char  brsam     : 1;
};

struct HOCODIVstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TEMPCAL0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TEMPCAL1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TEMPCAL2struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TEMPCAL3struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct FLSECstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLFSWSstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLFSWEstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FSSETstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct FSSEstruct {
	unsigned char            : 1;
	unsigned char  esqst     : 1;
};

struct FLFADLstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLFADHstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PFCMDstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct PFSstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct FLRLstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLRHstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLWEstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct FLREstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct FLTMSstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct DFLMCstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLMCLstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct FLMCHstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct FSCTLstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ICEADRstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct ICEDATstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct MDCLstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct MDCHstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct MDUCstruct {
	unsigned char  divst     : 1;
	unsigned char  macsf     : 1;
	unsigned char  macof     : 1;
	unsigned char  mdsm      : 1;
	unsigned char            : 1;
	unsigned char  macmode   : 1;
	unsigned char  divmode   : 1;
};

struct PER0struct {
	unsigned char  tau0en    : 1;
	unsigned char  tau1en    : 1;
	unsigned char  sau0en    : 1;
	unsigned char  sau1en    : 1;
	unsigned char  iica0en   : 1;
	unsigned char  adcen     : 1;
	unsigned char  iica1en   : 1;
	unsigned char  rtcen     : 1;
};

struct OSMCstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct RMCstruct {
	unsigned char  paenb     : 1;
	unsigned char            : 1;
	unsigned char  wdvol     : 1;
};

struct RPECTLstruct {
	unsigned char  rpef      : 1;
	unsigned char            : 1;
	unsigned char  rperdis   : 1;
};

struct BCDADJstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct VECTCTRLstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR00Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR01Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR02Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR03Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR00Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR01Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR02Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR03Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SMR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SMR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SMR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SMR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SE0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SE0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SS0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SS0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ST0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct ST0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SPS0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SPS0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SO0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SOE0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SOE0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR00Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR01Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR02Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR03Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SOL0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SOL0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSC0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSC0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR10Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR11Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR12Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSR13Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR10Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR11Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR12Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SIR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SIR13Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SMR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SMR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SMR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SMR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SCR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SE1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SE1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SS1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SS1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct ST1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct ST1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SPS1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SPS1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SO1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SOE1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SOE1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR10Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR11Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR12Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct EDR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct EDR13Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SOL1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SOL1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SSC1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct SSC1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TCR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR04struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR05struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR06struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR07struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR04struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR05struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR06struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR07struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR00struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR00Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR01struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR01Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR02struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR02Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR03struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR03Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR04struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR04Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR05struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR05Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR06struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR06Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR07struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR07Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TE0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TE0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TS0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TS0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TT0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TT0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TPS0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TO0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TO0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TOE0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOE0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TOL0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOL0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TOM0struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOM0Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TCR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR14struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR15struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR16struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TCR17struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR14struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR15struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR16struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TMR17struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR10struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR10Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR11struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR11Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR12struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR12Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR13struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR13Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR14struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR14Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR15struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR15Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR16struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR16Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TSR17struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TSR17Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TE1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TE1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TS1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TS1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TT1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TT1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TPS1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TO1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TO1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TOE1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOE1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TOL1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOL1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct TOM1struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct TOM1Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DSA2struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DSA3struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DRA2struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct DRA2Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DRA2Hstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DRA3struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct DRA3Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DRA3Hstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DBC2struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct DBC2Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DBC2Hstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DBC3struct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct DBC3Lstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DBC3Hstruct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct DMC2struct {
	unsigned char            : 1;
	unsigned char  dwait2    : 1;
	unsigned char  ds2       : 1;
	unsigned char  drs2      : 1;
	unsigned char  stg2      : 1;
};

struct DMC3struct {
	unsigned char            : 1;
	unsigned char  dwait3    : 1;
	unsigned char  ds3       : 1;
	unsigned char  drs3      : 1;
	unsigned char  stg3      : 1;
};

struct DRC2struct {
	unsigned char  dst2      : 1;
	unsigned char            : 1;
	unsigned char  den2      : 1;
};

struct DRC3struct {
	unsigned char  dst3      : 1;
	unsigned char            : 1;
	unsigned char  den3      : 1;
};

struct DWAITALLstruct {
	unsigned char  dwaitall0 : 1;
	unsigned char            : 1;
	unsigned char  prvari    : 1;
};

struct IICCTL00struct {
	unsigned char  spt0      : 1;
	unsigned char  stt0      : 1;
	unsigned char  acke0     : 1;
	unsigned char  wtim0     : 1;
	unsigned char  spie0     : 1;
	unsigned char  wrel0     : 1;
	unsigned char  lrel0     : 1;
	unsigned char  iice0     : 1;
};

struct IICCTL01struct {
	unsigned char  prs0      : 1;
	unsigned char            : 1;
	unsigned char  dfc0      : 1;
	unsigned char  smc0      : 1;
	unsigned char  dad0      : 1;
	unsigned char  cld0      : 1;
	unsigned char            : 1;
	unsigned char  wup0      : 1;
};

struct IICWL0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct IICWH0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SVA0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct IICSE0struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct IICCTL10struct {
	unsigned char  spt1      : 1;
	unsigned char  stt1      : 1;
	unsigned char  acke1     : 1;
	unsigned char  wtim1     : 1;
	unsigned char  spie1     : 1;
	unsigned char  wrel1     : 1;
	unsigned char  lrel1     : 1;
	unsigned char  iice1     : 1;
};

struct IICCTL11struct {
	unsigned char  prs1      : 1;
	unsigned char            : 1;
	unsigned char  dfc1      : 1;
	unsigned char  smc1      : 1;
	unsigned char  dad1      : 1;
	unsigned char  cld1      : 1;
	unsigned char            : 1;
	unsigned char  wup1      : 1;
};

struct IICWL1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct IICWH1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct SVA1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct IICSE1struct {
	unsigned char  bit0      : 1;
	unsigned char  bit1      : 1;
	unsigned char  bit2      : 1;
	unsigned char  bit3      : 1;
	unsigned char  bit4      : 1;
	unsigned char  bit5      : 1;
	unsigned char  bit6      : 1;
	unsigned char  bit7      : 1;
};

struct CRC0CTLstruct {
	unsigned char            : 1;
	unsigned char  crc0en    : 1;
};

struct PGCRCLstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};

struct CRCDstruct {
	unsigned short bit0      : 1;
	unsigned short bit1      : 1;
	unsigned short bit2      : 1;
	unsigned short bit3      : 1;
	unsigned short bit4      : 1;
	unsigned short bit5      : 1;
	unsigned short bit6      : 1;
	unsigned short bit7      : 1;
	unsigned short bit8      : 1;
	unsigned short bit9      : 1;
	unsigned short bit10     : 1;
	unsigned short bit11     : 1;
	unsigned short bit12     : 1;
	unsigned short bit13     : 1;
	unsigned short bit14     : 1;
	unsigned short bit15     : 1;
};



#define ADM2bits     (*(volatile struct ADM2struct    *)0xF0010)
#define ADULbits     (*(volatile struct ADULstruct    *)0xF0011)
#define ADLLbits     (*(volatile struct ADLLstruct    *)0xF0012)
#define ADTESbits    (*(volatile struct ADTESstruct   *)0xF0013)
#define PU0bits      (*(volatile struct PU0struct     *)0xF0030)
#define PU1bits      (*(volatile struct PU1struct     *)0xF0031)
#define PU3bits      (*(volatile struct PU3struct     *)0xF0033)
#define PU4bits      (*(volatile struct PU4struct     *)0xF0034)
#define PU5bits      (*(volatile struct PU5struct     *)0xF0035)
#define PU6bits      (*(volatile struct PU6struct     *)0xF0036)
#define PU7bits      (*(volatile struct PU7struct     *)0xF0037)
#define PU8bits      (*(volatile struct PU8struct     *)0xF0038)
#define PU9bits      (*(volatile struct PU9struct     *)0xF0039)
#define PU10bits     (*(volatile struct PU10struct    *)0xF003A)
#define PU11bits     (*(volatile struct PU11struct    *)0xF003B)
#define PU12bits     (*(volatile struct PU12struct    *)0xF003C)
#define PU14bits     (*(volatile struct PU14struct    *)0xF003E)
#define PIM0bits     (*(volatile struct PIM0struct    *)0xF0040)
#define PIM1bits     (*(volatile struct PIM1struct    *)0xF0041)
#define PIM4bits     (*(volatile struct PIM4struct    *)0xF0044)
#define PIM5bits     (*(volatile struct PIM5struct    *)0xF0045)
#define PIM8bits     (*(volatile struct PIM8struct    *)0xF0048)
#define PIM14bits    (*(volatile struct PIM14struct   *)0xF004E)
#define POM0bits     (*(volatile struct POM0struct    *)0xF0050)
#define POM1bits     (*(volatile struct POM1struct    *)0xF0051)
#define POM4bits     (*(volatile struct POM4struct    *)0xF0054)
#define POM5bits     (*(volatile struct POM5struct    *)0xF0055)
#define POM7bits     (*(volatile struct POM7struct    *)0xF0057)
#define POM8bits     (*(volatile struct POM8struct    *)0xF0058)
#define POM9bits     (*(volatile struct POM9struct    *)0xF0059)
#define POM14bits    (*(volatile struct POM14struct   *)0xF005E)
#define PMC0bits     (*(volatile struct PMC0struct    *)0xF0060)
#define PMC3bits     (*(volatile struct PMC3struct    *)0xF0063)
#define PMC10bits    (*(volatile struct PMC10struct   *)0xF006A)
#define PMC11bits    (*(volatile struct PMC11struct   *)0xF006B)
#define PMC12bits    (*(volatile struct PMC12struct   *)0xF006C)
#define PMC14bits    (*(volatile struct PMC14struct   *)0xF006E)
#define NFEN0bits    (*(volatile struct NFEN0struct   *)0xF0070)
#define NFEN1bits    (*(volatile struct NFEN1struct   *)0xF0071)
#define NFEN2bits    (*(volatile struct NFEN2struct   *)0xF0072)
#define ISCbits      (*(volatile struct ISCstruct     *)0xF0073)
#define TIS0bits     (*(volatile struct TIS0struct    *)0xF0074)
#define ADPCbits     (*(volatile struct ADPCstruct    *)0xF0076)
#define PIORbits     (*(volatile struct PIORstruct    *)0xF0077)
#define IAWCTLbits   (*(volatile struct IAWCTLstruct  *)0xF0078)
#define GDIDISbits   (*(volatile struct GDIDISstruct  *)0xF007D)
#define PRDSELbits   (*(volatile struct PRDSELstruct  *)0xF007E)
#define TOOLENbits   (*(volatile struct TOOLENstruct  *)0xF0080)
#define BPAL0bits    (*(volatile struct BPAL0struct   *)0xF0081)
#define BPAH0bits    (*(volatile struct BPAH0struct   *)0xF0082)
#define BPAS0bits    (*(volatile struct BPAS0struct   *)0xF0083)
#define BACDVL0bits  (*(volatile struct BACDVL0struct *)0xF0084)
#define BACDVH0bits  (*(volatile struct BACDVH0struct *)0xF0085)
#define BACDML0bits  (*(volatile struct BACDML0struct *)0xF0086)
#define BACDMH0bits  (*(volatile struct BACDMH0struct *)0xF0087)
#define MONMODbits   (*(volatile struct MONMODstruct  *)0xF0088)
#define DFLCTLbits   (*(volatile struct DFLCTLstruct  *)0xF0090)
#define HIOTRMbits   (*(volatile struct HIOTRMstruct  *)0xF00A0)
#define BECTLbits    (*(volatile struct BECTLstruct   *)0xF00A1)
#define HOCODIVbits  (*(volatile struct HOCODIVstruct *)0xF00A8)
#define TEMPCAL0bits (*(volatile struct TEMPCAL0struct*)0xF00AC)
#define TEMPCAL1bits (*(volatile struct TEMPCAL1struct*)0xF00AD)
#define TEMPCAL2bits (*(volatile struct TEMPCAL2struct*)0xF00AE)
#define TEMPCAL3bits (*(volatile struct TEMPCAL3struct*)0xF00AF)
#define FLSECbits    (*(volatile struct FLSECstruct   *)0xF00B0)
#define FLFSWSbits   (*(volatile struct FLFSWSstruct  *)0xF00B2)
#define FLFSWEbits   (*(volatile struct FLFSWEstruct  *)0xF00B4)
#define FSSETbits    (*(volatile struct FSSETstruct   *)0xF00B6)
#define FSSEbits     (*(volatile struct FSSEstruct    *)0xF00B7)
#define FLFADLbits   (*(volatile struct FLFADLstruct  *)0xF00B8)
#define FLFADHbits   (*(volatile struct FLFADHstruct  *)0xF00BA)
#define PFCMDbits    (*(volatile struct PFCMDstruct   *)0xF00C0)
#define PFSbits      (*(volatile struct PFSstruct     *)0xF00C1)
#define FLRLbits     (*(volatile struct FLRLstruct    *)0xF00C2)
#define FLRHbits     (*(volatile struct FLRHstruct    *)0xF00C4)
#define FLWEbits     (*(volatile struct FLWEstruct    *)0xF00C6)
#define FLREbits     (*(volatile struct FLREstruct    *)0xF00C7)
#define FLTMSbits    (*(volatile struct FLTMSstruct   *)0xF00C8)
#define DFLMCbits    (*(volatile struct DFLMCstruct   *)0xF00CA)
#define FLMCLbits    (*(volatile struct FLMCLstruct   *)0xF00CC)
#define FLMCHbits    (*(volatile struct FLMCHstruct   *)0xF00CE)
#define FSCTLbits    (*(volatile struct FSCTLstruct   *)0xF00CF)
#define ICEADRbits   (*(volatile struct ICEADRstruct  *)0xF00D0)
#define ICEDATbits   (*(volatile struct ICEDATstruct  *)0xF00D2)
#define MDCLbits     (*(volatile struct MDCLstruct    *)0xF00E0)
#define MDCHbits     (*(volatile struct MDCHstruct    *)0xF00E2)
#define MDUCbits     (*(volatile struct MDUCstruct    *)0xF00E8)
#define PER0bits     (*(volatile struct PER0struct    *)0xF00F0)
#define OSMCbits     (*(volatile struct OSMCstruct    *)0xF00F3)
#define RMCbits      (*(volatile struct RMCstruct     *)0xF00F4)
#define RPECTLbits   (*(volatile struct RPECTLstruct  *)0xF00F5)
#define BCDADJbits   (*(volatile struct BCDADJstruct  *)0xF00FE)
#define VECTCTRLbits (*(volatile struct VECTCTRLstruct*)0xF00FF)
#define SSR00bits    (*(volatile struct SSR00struct   *)0xF0100)
#define SSR00Lbits   (*(volatile struct SSR00Lstruct  *)0xF0100)
#define SSR01bits    (*(volatile struct SSR01struct   *)0xF0102)
#define SSR01Lbits   (*(volatile struct SSR01Lstruct  *)0xF0102)
#define SSR02bits    (*(volatile struct SSR02struct   *)0xF0104)
#define SSR02Lbits   (*(volatile struct SSR02Lstruct  *)0xF0104)
#define SSR03bits    (*(volatile struct SSR03struct   *)0xF0106)
#define SSR03Lbits   (*(volatile struct SSR03Lstruct  *)0xF0106)
#define SIR00bits    (*(volatile struct SIR00struct   *)0xF0108)
#define SIR00Lbits   (*(volatile struct SIR00Lstruct  *)0xF0108)
#define SIR01bits    (*(volatile struct SIR01struct   *)0xF010A)
#define SIR01Lbits   (*(volatile struct SIR01Lstruct  *)0xF010A)
#define SIR02bits    (*(volatile struct SIR02struct   *)0xF010C)
#define SIR02Lbits   (*(volatile struct SIR02Lstruct  *)0xF010C)
#define SIR03bits    (*(volatile struct SIR03struct   *)0xF010E)
#define SIR03Lbits   (*(volatile struct SIR03Lstruct  *)0xF010E)
#define SMR00bits    (*(volatile struct SMR00struct   *)0xF0110)
#define SMR01bits    (*(volatile struct SMR01struct   *)0xF0112)
#define SMR02bits    (*(volatile struct SMR02struct   *)0xF0114)
#define SMR03bits    (*(volatile struct SMR03struct   *)0xF0116)
#define SCR00bits    (*(volatile struct SCR00struct   *)0xF0118)
#define SCR01bits    (*(volatile struct SCR01struct   *)0xF011A)
#define SCR02bits    (*(volatile struct SCR02struct   *)0xF011C)
#define SCR03bits    (*(volatile struct SCR03struct   *)0xF011E)
#define SE0bits      (*(volatile struct SE0struct     *)0xF0120)
#define SE0Lbits     (*(volatile struct SE0Lstruct    *)0xF0120)
#define SS0bits      (*(volatile struct SS0struct     *)0xF0122)
#define SS0Lbits     (*(volatile struct SS0Lstruct    *)0xF0122)
#define ST0bits      (*(volatile struct ST0struct     *)0xF0124)
#define ST0Lbits     (*(volatile struct ST0Lstruct    *)0xF0124)
#define SPS0bits     (*(volatile struct SPS0struct    *)0xF0126)
#define SPS0Lbits    (*(volatile struct SPS0Lstruct   *)0xF0126)
#define SO0bits      (*(volatile struct SO0struct     *)0xF0128)
#define SOE0bits     (*(volatile struct SOE0struct    *)0xF012A)
#define SOE0Lbits    (*(volatile struct SOE0Lstruct   *)0xF012A)
#define EDR00bits    (*(volatile struct EDR00struct   *)0xF012C)
#define EDR00Lbits   (*(volatile struct EDR00Lstruct  *)0xF012C)
#define EDR01bits    (*(volatile struct EDR01struct   *)0xF012E)
#define EDR01Lbits   (*(volatile struct EDR01Lstruct  *)0xF012E)
#define EDR02bits    (*(volatile struct EDR02struct   *)0xF0130)
#define EDR02Lbits   (*(volatile struct EDR02Lstruct  *)0xF0130)
#define EDR03bits    (*(volatile struct EDR03struct   *)0xF0132)
#define EDR03Lbits   (*(volatile struct EDR03Lstruct  *)0xF0132)
#define SOL0bits     (*(volatile struct SOL0struct    *)0xF0134)
#define SOL0Lbits    (*(volatile struct SOL0Lstruct   *)0xF0134)
#define SSC0bits     (*(volatile struct SSC0struct    *)0xF0138)
#define SSC0Lbits    (*(volatile struct SSC0Lstruct   *)0xF0138)
#define SSR10bits    (*(volatile struct SSR10struct   *)0xF0140)
#define SSR10Lbits   (*(volatile struct SSR10Lstruct  *)0xF0140)
#define SSR11bits    (*(volatile struct SSR11struct   *)0xF0142)
#define SSR11Lbits   (*(volatile struct SSR11Lstruct  *)0xF0142)
#define SSR12bits    (*(volatile struct SSR12struct   *)0xF0144)
#define SSR12Lbits   (*(volatile struct SSR12Lstruct  *)0xF0144)
#define SSR13bits    (*(volatile struct SSR13struct   *)0xF0146)
#define SSR13Lbits   (*(volatile struct SSR13Lstruct  *)0xF0146)
#define SIR10bits    (*(volatile struct SIR10struct   *)0xF0148)
#define SIR10Lbits   (*(volatile struct SIR10Lstruct  *)0xF0148)
#define SIR11bits    (*(volatile struct SIR11struct   *)0xF014A)
#define SIR11Lbits   (*(volatile struct SIR11Lstruct  *)0xF014A)
#define SIR12bits    (*(volatile struct SIR12struct   *)0xF014C)
#define SIR12Lbits   (*(volatile struct SIR12Lstruct  *)0xF014C)
#define SIR13bits    (*(volatile struct SIR13struct   *)0xF014E)
#define SIR13Lbits   (*(volatile struct SIR13Lstruct  *)0xF014E)
#define SMR10bits    (*(volatile struct SMR10struct   *)0xF0150)
#define SMR11bits    (*(volatile struct SMR11struct   *)0xF0152)
#define SMR12bits    (*(volatile struct SMR12struct   *)0xF0154)
#define SMR13bits    (*(volatile struct SMR13struct   *)0xF0156)
#define SCR10bits    (*(volatile struct SCR10struct   *)0xF0158)
#define SCR11bits    (*(volatile struct SCR11struct   *)0xF015A)
#define SCR12bits    (*(volatile struct SCR12struct   *)0xF015C)
#define SCR13bits    (*(volatile struct SCR13struct   *)0xF015E)
#define SE1bits      (*(volatile struct SE1struct     *)0xF0160)
#define SE1Lbits     (*(volatile struct SE1Lstruct    *)0xF0160)
#define SS1bits      (*(volatile struct SS1struct     *)0xF0162)
#define SS1Lbits     (*(volatile struct SS1Lstruct    *)0xF0162)
#define ST1bits      (*(volatile struct ST1struct     *)0xF0164)
#define ST1Lbits     (*(volatile struct ST1Lstruct    *)0xF0164)
#define SPS1bits     (*(volatile struct SPS1struct    *)0xF0166)
#define SPS1Lbits    (*(volatile struct SPS1Lstruct   *)0xF0166)
#define SO1bits      (*(volatile struct SO1struct     *)0xF0168)
#define SOE1bits     (*(volatile struct SOE1struct    *)0xF016A)
#define SOE1Lbits    (*(volatile struct SOE1Lstruct   *)0xF016A)
#define EDR10bits    (*(volatile struct EDR10struct   *)0xF016C)
#define EDR10Lbits   (*(volatile struct EDR10Lstruct  *)0xF016C)
#define EDR11bits    (*(volatile struct EDR11struct   *)0xF016E)
#define EDR11Lbits   (*(volatile struct EDR11Lstruct  *)0xF016E)
#define EDR12bits    (*(volatile struct EDR12struct   *)0xF0170)
#define EDR12Lbits   (*(volatile struct EDR12Lstruct  *)0xF0170)
#define EDR13bits    (*(volatile struct EDR13struct   *)0xF0172)
#define EDR13Lbits   (*(volatile struct EDR13Lstruct  *)0xF0172)
#define SOL1bits     (*(volatile struct SOL1struct    *)0xF0174)
#define SOL1Lbits    (*(volatile struct SOL1Lstruct   *)0xF0174)
#define SSC1bits     (*(volatile struct SSC1struct    *)0xF0178)
#define SSC1Lbits    (*(volatile struct SSC1Lstruct   *)0xF0178)
#define TCR00bits    (*(volatile struct TCR00struct   *)0xF0180)
#define TCR01bits    (*(volatile struct TCR01struct   *)0xF0182)
#define TCR02bits    (*(volatile struct TCR02struct   *)0xF0184)
#define TCR03bits    (*(volatile struct TCR03struct   *)0xF0186)
#define TCR04bits    (*(volatile struct TCR04struct   *)0xF0188)
#define TCR05bits    (*(volatile struct TCR05struct   *)0xF018A)
#define TCR06bits    (*(volatile struct TCR06struct   *)0xF018C)
#define TCR07bits    (*(volatile struct TCR07struct   *)0xF018E)
#define TMR00bits    (*(volatile struct TMR00struct   *)0xF0190)
#define TMR01bits    (*(volatile struct TMR01struct   *)0xF0192)
#define TMR02bits    (*(volatile struct TMR02struct   *)0xF0194)
#define TMR03bits    (*(volatile struct TMR03struct   *)0xF0196)
#define TMR04bits    (*(volatile struct TMR04struct   *)0xF0198)
#define TMR05bits    (*(volatile struct TMR05struct   *)0xF019A)
#define TMR06bits    (*(volatile struct TMR06struct   *)0xF019C)
#define TMR07bits    (*(volatile struct TMR07struct   *)0xF019E)
#define TSR00bits    (*(volatile struct TSR00struct   *)0xF01A0)
#define TSR00Lbits   (*(volatile struct TSR00Lstruct  *)0xF01A0)
#define TSR01bits    (*(volatile struct TSR01struct   *)0xF01A2)
#define TSR01Lbits   (*(volatile struct TSR01Lstruct  *)0xF01A2)
#define TSR02bits    (*(volatile struct TSR02struct   *)0xF01A4)
#define TSR02Lbits   (*(volatile struct TSR02Lstruct  *)0xF01A4)
#define TSR03bits    (*(volatile struct TSR03struct   *)0xF01A6)
#define TSR03Lbits   (*(volatile struct TSR03Lstruct  *)0xF01A6)
#define TSR04bits    (*(volatile struct TSR04struct   *)0xF01A8)
#define TSR04Lbits   (*(volatile struct TSR04Lstruct  *)0xF01A8)
#define TSR05bits    (*(volatile struct TSR05struct   *)0xF01AA)
#define TSR05Lbits   (*(volatile struct TSR05Lstruct  *)0xF01AA)
#define TSR06bits    (*(volatile struct TSR06struct   *)0xF01AC)
#define TSR06Lbits   (*(volatile struct TSR06Lstruct  *)0xF01AC)
#define TSR07bits    (*(volatile struct TSR07struct   *)0xF01AE)
#define TSR07Lbits   (*(volatile struct TSR07Lstruct  *)0xF01AE)
#define TE0bits      (*(volatile struct TE0struct     *)0xF01B0)
#define TE0Lbits     (*(volatile struct TE0Lstruct    *)0xF01B0)
#define TS0bits      (*(volatile struct TS0struct     *)0xF01B2)
#define TS0Lbits     (*(volatile struct TS0Lstruct    *)0xF01B2)
#define TT0bits      (*(volatile struct TT0struct     *)0xF01B4)
#define TT0Lbits     (*(volatile struct TT0Lstruct    *)0xF01B4)
#define TPS0bits     (*(volatile struct TPS0struct    *)0xF01B6)
#define TO0bits      (*(volatile struct TO0struct     *)0xF01B8)
#define TO0Lbits     (*(volatile struct TO0Lstruct    *)0xF01B8)
#define TOE0bits     (*(volatile struct TOE0struct    *)0xF01BA)
#define TOE0Lbits    (*(volatile struct TOE0Lstruct   *)0xF01BA)
#define TOL0bits     (*(volatile struct TOL0struct    *)0xF01BC)
#define TOL0Lbits    (*(volatile struct TOL0Lstruct   *)0xF01BC)
#define TOM0bits     (*(volatile struct TOM0struct    *)0xF01BE)
#define TOM0Lbits    (*(volatile struct TOM0Lstruct   *)0xF01BE)
#define TCR10bits    (*(volatile struct TCR10struct   *)0xF01C0)
#define TCR11bits    (*(volatile struct TCR11struct   *)0xF01C2)
#define TCR12bits    (*(volatile struct TCR12struct   *)0xF01C4)
#define TCR13bits    (*(volatile struct TCR13struct   *)0xF01C6)
#define TCR14bits    (*(volatile struct TCR14struct   *)0xF01C8)
#define TCR15bits    (*(volatile struct TCR15struct   *)0xF01CA)
#define TCR16bits    (*(volatile struct TCR16struct   *)0xF01CC)
#define TCR17bits    (*(volatile struct TCR17struct   *)0xF01CE)
#define TMR10bits    (*(volatile struct TMR10struct   *)0xF01D0)
#define TMR11bits    (*(volatile struct TMR11struct   *)0xF01D2)
#define TMR12bits    (*(volatile struct TMR12struct   *)0xF01D4)
#define TMR13bits    (*(volatile struct TMR13struct   *)0xF01D6)
#define TMR14bits    (*(volatile struct TMR14struct   *)0xF01D8)
#define TMR15bits    (*(volatile struct TMR15struct   *)0xF01DA)
#define TMR16bits    (*(volatile struct TMR16struct   *)0xF01DC)
#define TMR17bits    (*(volatile struct TMR17struct   *)0xF01DE)
#define TSR10bits    (*(volatile struct TSR10struct   *)0xF01E0)
#define TSR10Lbits   (*(volatile struct TSR10Lstruct  *)0xF01E0)
#define TSR11bits    (*(volatile struct TSR11struct   *)0xF01E2)
#define TSR11Lbits   (*(volatile struct TSR11Lstruct  *)0xF01E2)
#define TSR12bits    (*(volatile struct TSR12struct   *)0xF01E4)
#define TSR12Lbits   (*(volatile struct TSR12Lstruct  *)0xF01E4)
#define TSR13bits    (*(volatile struct TSR13struct   *)0xF01E6)
#define TSR13Lbits   (*(volatile struct TSR13Lstruct  *)0xF01E6)
#define TSR14bits    (*(volatile struct TSR14struct   *)0xF01E8)
#define TSR14Lbits   (*(volatile struct TSR14Lstruct  *)0xF01E8)
#define TSR15bits    (*(volatile struct TSR15struct   *)0xF01EA)
#define TSR15Lbits   (*(volatile struct TSR15Lstruct  *)0xF01EA)
#define TSR16bits    (*(volatile struct TSR16struct   *)0xF01EC)
#define TSR16Lbits   (*(volatile struct TSR16Lstruct  *)0xF01EC)
#define TSR17bits    (*(volatile struct TSR17struct   *)0xF01EE)
#define TSR17Lbits   (*(volatile struct TSR17Lstruct  *)0xF01EE)
#define TE1bits      (*(volatile struct TE1struct     *)0xF01F0)
#define TE1Lbits     (*(volatile struct TE1Lstruct    *)0xF01F0)
#define TS1bits      (*(volatile struct TS1struct     *)0xF01F2)
#define TS1Lbits     (*(volatile struct TS1Lstruct    *)0xF01F2)
#define TT1bits      (*(volatile struct TT1struct     *)0xF01F4)
#define TT1Lbits     (*(volatile struct TT1Lstruct    *)0xF01F4)
#define TPS1bits     (*(volatile struct TPS1struct    *)0xF01F6)
#define TO1bits      (*(volatile struct TO1struct     *)0xF01F8)
#define TO1Lbits     (*(volatile struct TO1Lstruct    *)0xF01F8)
#define TOE1bits     (*(volatile struct TOE1struct    *)0xF01FA)
#define TOE1Lbits    (*(volatile struct TOE1Lstruct   *)0xF01FA)
#define TOL1bits     (*(volatile struct TOL1struct    *)0xF01FC)
#define TOL1Lbits    (*(volatile struct TOL1Lstruct   *)0xF01FC)
#define TOM1bits     (*(volatile struct TOM1struct    *)0xF01FE)
#define TOM1Lbits    (*(volatile struct TOM1Lstruct   *)0xF01FE)
#define DSA2bits     (*(volatile struct DSA2struct    *)0xF0200)
#define DSA3bits     (*(volatile struct DSA3struct    *)0xF0201)
#define DRA2bits     (*(volatile struct DRA2struct    *)0xF0202)
#define DRA2Lbits    (*(volatile struct DRA2Lstruct   *)0xF0202)
#define DRA2Hbits    (*(volatile struct DRA2Hstruct   *)0xF0203)
#define DRA3bits     (*(volatile struct DRA3struct    *)0xF0204)
#define DRA3Lbits    (*(volatile struct DRA3Lstruct   *)0xF0204)
#define DRA3Hbits    (*(volatile struct DRA3Hstruct   *)0xF0205)
#define DBC2bits     (*(volatile struct DBC2struct    *)0xF0206)
#define DBC2Lbits    (*(volatile struct DBC2Lstruct   *)0xF0206)
#define DBC2Hbits    (*(volatile struct DBC2Hstruct   *)0xF0207)
#define DBC3bits     (*(volatile struct DBC3struct    *)0xF0208)
#define DBC3Lbits    (*(volatile struct DBC3Lstruct   *)0xF0208)
#define DBC3Hbits    (*(volatile struct DBC3Hstruct   *)0xF0209)
#define DMC2bits     (*(volatile struct DMC2struct    *)0xF020A)
#define DMC3bits     (*(volatile struct DMC3struct    *)0xF020B)
#define DRC2bits     (*(volatile struct DRC2struct    *)0xF020C)
#define DRC3bits     (*(volatile struct DRC3struct    *)0xF020D)
#define DWAITALLbits (*(volatile struct DWAITALLstruct*)0xF020F)
#define IICCTL00bits (*(volatile struct IICCTL00struct*)0xF0230)
#define IICCTL01bits (*(volatile struct IICCTL01struct*)0xF0231)
#define IICWL0bits   (*(volatile struct IICWL0struct  *)0xF0232)
#define IICWH0bits   (*(volatile struct IICWH0struct  *)0xF0233)
#define SVA0bits     (*(volatile struct SVA0struct    *)0xF0234)
#define IICSE0bits   (*(volatile struct IICSE0struct  *)0xF0235)
#define IICCTL10bits (*(volatile struct IICCTL10struct*)0xF0238)
#define IICCTL11bits (*(volatile struct IICCTL11struct*)0xF0239)
#define IICWL1bits   (*(volatile struct IICWL1struct  *)0xF023A)
#define IICWH1bits   (*(volatile struct IICWH1struct  *)0xF023B)
#define SVA1bits     (*(volatile struct SVA1struct    *)0xF023C)
#define IICSE1bits   (*(volatile struct IICSE1struct  *)0xF023D)
#define CRC0CTLbits  (*(volatile struct CRC0CTLstruct *)0xF02F0)
#define PGCRCLbits   (*(volatile struct PGCRCLstruct  *)0xF02F2)
#define CRCDbits     (*(volatile struct CRCDstruct    *)0xF02FA)


#define ADTYP     (ADM2bits.adtyp)
#define AWC       (ADM2bits.awc)
#define ADRCK     (ADM2bits.adrck)
#define DFLEN     (DFLCTLbits.dflen)
#define BRSAM     (BECTLbits.brsam)
#define ESQST     (FSSEbits.esqst)
#define DIVST     (MDUCbits.divst)
#define MACSF     (MDUCbits.macsf)
#define MACOF     (MDUCbits.macof)
#define MDSM      (MDUCbits.mdsm)
#define MACMODE   (MDUCbits.macmode)
#define DIVMODE   (MDUCbits.divmode)
#define TAU0EN    (PER0bits.tau0en)
#define TAU1EN    (PER0bits.tau1en)
#define SAU0EN    (PER0bits.sau0en)
#define SAU1EN    (PER0bits.sau1en)
#define IICA0EN   (PER0bits.iica0en)
#define ADCEN     (PER0bits.adcen)
#define IICA1EN   (PER0bits.iica1en)
#define RTCEN     (PER0bits.rtcen)
#define PAENB     (RMCbits.paenb)
#define WDVOL     (RMCbits.wdvol)
#define RPEF      (RPECTLbits.rpef)
#define RPERDIS   (RPECTLbits.rperdis)
#define DWAIT2    (DMC2bits.dwait2)
#define DS2       (DMC2bits.ds2)
#define DRS2      (DMC2bits.drs2)
#define STG2      (DMC2bits.stg2)
#define DWAIT3    (DMC3bits.dwait3)
#define DS3       (DMC3bits.ds3)
#define DRS3      (DMC3bits.drs3)
#define STG3      (DMC3bits.stg3)
#define DST2      (DRC2bits.dst2)
#define DEN2      (DRC2bits.den2)
#define DST3      (DRC3bits.dst3)
#define DEN3      (DRC3bits.den3)
#define DWAITALL0 (DWAITALLbits.dwaitall0)
#define PRVARI    (DWAITALLbits.prvari)
#define SPT0      (IICCTL00bits.spt0)
#define STT0      (IICCTL00bits.stt0)
#define ACKE0     (IICCTL00bits.acke0)
#define WTIM0     (IICCTL00bits.wtim0)
#define SPIE0     (IICCTL00bits.spie0)
#define WREL0     (IICCTL00bits.wrel0)
#define LREL0     (IICCTL00bits.lrel0)
#define IICE0     (IICCTL00bits.iice0)
#define PRS0      (IICCTL01bits.prs0)
#define DFC0      (IICCTL01bits.dfc0)
#define SMC0      (IICCTL01bits.smc0)
#define DAD0      (IICCTL01bits.dad0)
#define CLD0      (IICCTL01bits.cld0)
#define WUP0      (IICCTL01bits.wup0)
#define SPT1      (IICCTL10bits.spt1)
#define STT1      (IICCTL10bits.stt1)
#define ACKE1     (IICCTL10bits.acke1)
#define WTIM1     (IICCTL10bits.wtim1)
#define SPIE1     (IICCTL10bits.spie1)
#define WREL1     (IICCTL10bits.wrel1)
#define LREL1     (IICCTL10bits.lrel1)
#define IICE1     (IICCTL10bits.iice1)
#define PRS1      (IICCTL11bits.prs1)
#define DFC1      (IICCTL11bits.dfc1)
#define SMC1      (IICCTL11bits.smc1)
#define DAD1      (IICCTL11bits.dad1)
#define CLD1      (IICCTL11bits.cld1)
#define WUP1      (IICCTL11bits.wup1)
#define CRC0EN    (CRC0CTLbits.crc0en)


#define PIOR5 (PIORbits.bit5)
#define PMC02 (PMC0bits.bit2)
#define PM02  (PM0bits.bit2)
#define P02   (P0bits.bit2)

#define PMC03 (PMC0bits.bit3)
#define PM03  (PM0bits.bit3)
#define P03   (P0bits.bit3)

#define PMC04 (PMC0bits.bit4)
#define PM04  (PM0bits.bit4)
#define P04   (P0bits.bit4)

#define P05   (P0bits.bit5)
#define P06   (P0bits.bit6)
#define P16   (P1bits.bit6)
#define P30   (P3bits.bit0)
#define P43   (P4bits.bit3)
#define P50   (P5bits.bit0)
#define P120  (P12bits.bit0)

#define TCR (&TCR00)
#define TMR (&TMR00)

#endif // SFRS_EXT_H
