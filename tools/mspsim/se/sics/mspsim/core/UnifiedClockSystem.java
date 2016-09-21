/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * UnifiedClockSystem
 *
 * Author  : Joakim Eriksson
 * Author  : Adam Dunkels
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;
import se.sics.mspsim.util.Utils;

public class UnifiedClockSystem extends ClockSystem {

  private static final int UCSCTL0 = 0x0160;
  private static final int UCSCTL1 = 0x0162;
  private static final int UCSCTL2 = 0x0164;
  private static final int UCSCTL3 = 0x0166;
  private static final int UCSCTL4 = 0x0168;
  private static final int UCSCTL5 = 0x016a;
  private static final int UCSCTL6 = 0x016c;
  private static final int UCSCTL7 = 0x016e;
  private static final int UCSCTL8 = 0x0170;

/* UCSCTL0 Control Bits */
//  private static final int RESERVED            = 0x0001;    /* RESERVED */
//  private static final int RESERVED            = 0x0002;    /* RESERVED */
//  private static final int RESERVED            = 0x0004;    /* RESERVED */
  private static final int MOD_BITPOS          = 3;
  private static final int MOD_BITWIDTH        = 5;
  private static final int MOD0                = 0x0008;    /* Modulation Bit Counter Bit : 0 */
  private static final int MOD1                = 0x0010;    /* Modulation Bit Counter Bit : 1 */
  private static final int MOD2                = 0x0020;    /* Modulation Bit Counter Bit : 2 */
  private static final int MOD3                = 0x0040;    /* Modulation Bit Counter Bit : 3 */
  private static final int MOD4                = 0x0080;    /* Modulation Bit Counter Bit : 4 */

  private static final int DCO_BITPOS          = 8;
  private static final int DCO_BITWIDTH        = 5;
  private static final int DCO0                = 0x0100;    /* DCO TAP Bit : 0 */
  private static final int DCO1                = 0x0200;    /* DCO TAP Bit : 1 */
  private static final int DCO2                = 0x0400;    /* DCO TAP Bit : 2 */
  private static final int DCO3                = 0x0800;    /* DCO TAP Bit : 3 */
  private static final int DCO4                = 0x1000;    /* DCO TAP Bit : 4 */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
//  private static final int RESERVED            = 0x4000;    /* RESERVED */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */


/* UCSCTL1 Control Bits */
  private static final int DISMOD              = 0x0001;    /* Disable Modulation */
//  private static final int RESERVED            = 0x0002;    /* RESERVED */
//  private static final int RESERVED            = 0x0004;    /* RESERVED */
//  private static final int RESERVED            = 0x0008;    /* RESERVED */
  private static final int DCORSEL_BITPOS      = 4;
  private static final int DCORSEL_BITWIDTH    = 3;
  private static final int DCORSEL0            = 0x0010;    /* DCO Freq. Range Select Bit : 0 */
  private static final int DCORSEL1            = 0x0020;    /* DCO Freq. Range Select Bit : 1 */
  private static final int DCORSEL2            = 0x0040;    /* DCO Freq. Range Select Bit : 2 */
//  private static final int RESERVED            = 0x0080;    /* RESERVED */
//  private static final int RESERVED            = 0x0100;    /* RESERVED */
//  private static final int RESERVED            = 0x0200;    /* RESERVED */
//  private static final int RESERVED            = 0x0400;    /* RESERVED */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
//  private static final int RESERVED            = 0x1000;    /* RESERVED */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
//  private static final int RESERVED            = 0x4000;    /* RESERVED */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */


/* UCSCTL2 Control Bits */
  private static final int FLLN_BITPOS         = 0;
  private static final int FLLN_BITWIDTH       = 10;
  private static final int FLLN0               = 0x0001;    /* FLL Multipier Bit : 0 */
  private static final int FLLN1               = 0x0002;    /* FLL Multipier Bit : 1 */
  private static final int FLLN2               = 0x0004;    /* FLL Multipier Bit : 2 */
  private static final int FLLN3               = 0x0008;    /* FLL Multipier Bit : 3 */
  private static final int FLLN4               = 0x0010;    /* FLL Multipier Bit : 4 */
  private static final int FLLN5               = 0x0020;    /* FLL Multipier Bit : 5 */
  private static final int FLLN6               = 0x0040;    /* FLL Multipier Bit : 6 */
  private static final int FLLN7               = 0x0080;    /* FLL Multipier Bit : 7 */
  private static final int FLLN8               = 0x0100;    /* FLL Multipier Bit : 8 */
  private static final int FLLN9               = 0x0200;    /* FLL Multipier Bit : 9 */
//  private static final int RESERVED            = 0x0400;    /* RESERVED */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
  private static final int FLLD_BITPOS         = 12;
  private static final int FLLD_BITWIDTH       = 3;
  private static final int FLLD0               = 0x1000;    /* Loop Divider Bit : 0 */
  private static final int FLLD1               = 0x2000;    /* Loop Divider Bit : 1 */
  private static final int FLLD2               = 0x4000;    /* Loop Divider Bit : 1 */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */


/* UCSCTL3 Control Bits */
  private static final int FLLREFDIV_BITPOS    = 0;
  private static final int FLLREFDIV_BITWIDTH  = 3;
  private static final int FLLREFDIV0          = 0x0001;    /* Reference Divider Bit : 0 */
  private static final int FLLREFDIV1          = 0x0002;    /* Reference Divider Bit : 1 */
  private static final int FLLREFDIV2          = 0x0004;    /* Reference Divider Bit : 2 */
//  private static final int RESERVED            = 0x0008;    /* RESERVED */
  private static final int SELREF_BITPOS        = 4;
  private static final int SELREF_BITWIDTH      = 3;
  private static final int SELREF0             = 0x0010;    /* FLL Reference Clock Select Bit : 0 */
  private static final int SELREF1             = 0x0020;    /* FLL Reference Clock Select Bit : 1 */
  private static final int SELREF2             = 0x0040;    /* FLL Reference Clock Select Bit : 2 */
//  private static final int RESERVED            = 0x0080;    /* RESERVED */
//  private static final int RESERVED            = 0x0100;    /* RESERVED */
//  private static final int RESERVED            = 0x0200;    /* RESERVED */
//  private static final int RESERVED            = 0x0400;    /* RESERVED */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
//  private static final int RESERVED            = 0x1000;    /* RESERVED */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
//  private static final int RESERVED            = 0x4000;    /* RESERVED */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */

/* UCSCTL3 Control Bits */

  private static final int FLLREFDIV_0         = 0x0000;    /* Reference Divider: f(LFCLK);/1 */
  private static final int FLLREFDIV_1         = 0x0001;    /* Reference Divider: f(LFCLK);/2 */
  private static final int FLLREFDIV_2         = 0x0002;    /* Reference Divider: f(LFCLK);/4 */
  private static final int FLLREFDIV_3         = 0x0003;    /* Reference Divider: f(LFCLK);/8 */
  private static final int FLLREFDIV_4         = 0x0004;    /* Reference Divider: f(LFCLK);/12 */
  private static final int FLLREFDIV_5         = 0x0005;    /* Reference Divider: f(LFCLK);/16 */
  private static final int FLLREFDIV_6         = 0x0006;    /* Reference Divider: f(LFCLK);/16 */
  private static final int FLLREFDIV_7         = 0x0007;    /* Reference Divider: f(LFCLK);/16 */
  private static final int FLLREFDIV__1        = 0x0000;    /* Reference Divider: f(LFCLK);/1 */
  private static final int FLLREFDIV__2        = 0x0001;    /* Reference Divider: f(LFCLK);/2 */
  private static final int FLLREFDIV__4        = 0x0002;    /* Reference Divider: f(LFCLK);/4 */
  private static final int FLLREFDIV__8        = 0x0003;    /* Reference Divider: f(LFCLK);/8 */
  private static final int FLLREFDIV__12       = 0x0004;    /* Reference Divider: f(LFCLK);/12 */
  private static final int FLLREFDIV__16       = 0x0005;    /* Reference Divider: f(LFCLK);/16 */
  private static final int SELREF_0            = 0x0000;    /* FLL Reference Clock Select 0 */
  private static final int SELREF_1            = 0x0010;    /* FLL Reference Clock Select 1 */
  private static final int SELREF_2            = 0x0020;    /* FLL Reference Clock Select 2 */
  private static final int SELREF_3            = 0x0030;    /* FLL Reference Clock Select 3 */
  private static final int SELREF_4            = 0x0040;    /* FLL Reference Clock Select 4 */
  private static final int SELREF_5            = 0x0050;    /* FLL Reference Clock Select 5 */
  private static final int SELREF_6            = 0x0060;    /* FLL Reference Clock Select 6 */
  private static final int SELREF_7            = 0x0070;    /* FLL Reference Clock Select 7 */
  private static final int SELREF__XT1CLK      = 0x0000;    /* Multiply Selected Loop Freq. By XT1CLK */
  private static final int SELREF__REFOCLK     = 0x0020;    /* Multiply Selected Loop Freq. By REFOCLK */
  private static final int SELREF__XT2CLK      = 0x0050;    /* Multiply Selected Loop Freq. By XT2CLK */

/* UCSCTL4 Control Bits */
  private static final int SELM0               = 0x0001;   /* MCLK Source Select Bit: 0 */
  private static final int SELM1               = 0x0002;   /* MCLK Source Select Bit: 1 */
  private static final int SELM2               = 0x0004;   /* MCLK Source Select Bit: 2 */
//  private static final int RESERVED            = 0x0008;    /* RESERVED */
  private static final int SELS0               = 0x0010;   /* SMCLK Source Select Bit: 0 */
  private static final int SELS1               = 0x0020;   /* SMCLK Source Select Bit: 1 */
  private static final int SELS2               = 0x0040;   /* SMCLK Source Select Bit: 2 */
//  private static final int RESERVED            = 0x0080;    /* RESERVED */
  private static final int SELA0               = 0x0100;   /* ACLK Source Select Bit: 0 */
  private static final int SELA1               = 0x0200;   /* ACLK Source Select Bit: 1 */
  private static final int SELA2               = 0x0400;   /* ACLK Source Select Bit: 2 */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
//  private static final int RESERVED            = 0x1000;    /* RESERVED */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
//  private static final int RESERVED            = 0x4000;    /* RESERVED */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */

/* UCSCTL4 Control Bits */
  private static final int SELM__XT1CLK        = 0x0000;   /* MCLK Source Select XT1CLK */
  private static final int SELM__VLOCLK        = 0x0001;   /* MCLK Source Select VLOCLK */
  private static final int SELM__REFOCLK       = 0x0002;   /* MCLK Source Select REFOCLK */
  private static final int SELM__DCOCLK        = 0x0003;   /* MCLK Source Select DCOCLK */
  private static final int SELM__DCOCLKDIV     = 0x0004;   /* MCLK Source Select DCOCLKDIV */
  private static final int SELM__XT2CLK        = 0x0005;   /* MCLK Source Select XT2CLK */

  private static final int SELS__XT1CLK        = 0x0000;   /* SMCLK Source Select XT1CLK */
  private static final int SELS__VLOCLK        = 0x0010;   /* SMCLK Source Select VLOCLK */
  private static final int SELS__REFOCLK       = 0x0020;   /* SMCLK Source Select REFOCLK */
  private static final int SELS__DCOCLK        = 0x0030;   /* SMCLK Source Select DCOCLK */
  private static final int SELS__DCOCLKDIV     = 0x0040;   /* SMCLK Source Select DCOCLKDIV */
  private static final int SELS__XT2CLK        = 0x0050;   /* SMCLK Source Select XT2CLK */

  private static final int SELA__XT1CLK        = 0x0000;   /* ACLK Source Select XT1CLK */
  private static final int SELA__VLOCLK        = 0x0100;   /* ACLK Source Select VLOCLK */
  private static final int SELA__REFOCLK       = 0x0200;   /* ACLK Source Select REFOCLK */
  private static final int SELA__DCOCLK        = 0x0300;   /* ACLK Source Select DCOCLK */
  private static final int SELA__DCOCLKDIV     = 0x0400;   /* ACLK Source Select DCOCLKDIV */
  private static final int SELA__XT2CLK        = 0x0500;   /* ACLK Source Select XT2CLK */

/* UCSCTL5 Control Bits */
  private static final int DIVM0               = 0x0001;   /* MCLK Divider Bit: 0 */
  private static final int DIVM1               = 0x0002;   /* MCLK Divider Bit: 1 */
  private static final int DIVM2               = 0x0004;   /* MCLK Divider Bit: 2 */
//  private static final int RESERVED            = 0x0008;    /* RESERVED */
  private static final int DIVS_BITPOS         = 4;
  private static final int DIVS_BITWIDTH       = 3;
  private static final int DIVS0               = 0x0010;   /* SMCLK Divider Bit: 0 */
  private static final int DIVS1               = 0x0020;   /* SMCLK Divider Bit: 1 */
  private static final int DIVS2               = 0x0040;   /* SMCLK Divider Bit: 2 */
//  private static final int RESERVED            = 0x0080;    /* RESERVED */
  private static final int DIVA0               = 0x0100;   /* ACLK Divider Bit: 0 */
  private static final int DIVA1               = 0x0200;   /* ACLK Divider Bit: 1 */
  private static final int DIVA2               = 0x0400;   /* ACLK Divider Bit: 2 */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
  private static final int DIVPA0              = 0x1000;   /* ACLK from Pin Divider Bit: 0 */
  private static final int DIVPA1              = 0x2000;   /* ACLK from Pin Divider Bit: 1 */
  private static final int DIVPA2              = 0x4000;   /* ACLK from Pin Divider Bit: 2 */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */

/* UCSCTL6 Control Bits */
  private static final int XT1OFF              = 0x0001;    /* High Frequency Oscillator 1 (XT1); disable */
  private static final int SMCLKOFF            = 0x0002;    /* SMCLK Off */
  private static final int XCAP0               = 0x0004;   /* XIN/XOUT Cap Bit: 0 */
  private static final int XCAP1               = 0x0008;   /* XIN/XOUT Cap Bit: 1 */
  private static final int XT1BYPASS           = 0x0010;    /* XT1 bypass mode : 0: internal 1:sourced from external pin */
  private static final int XTS                 = 0x0020;   /* 1: Selects high-freq. oscillator */
  private static final int XT1DRIVE0           = 0x0040;    /* XT1 Drive Level mode Bit 0 */
  private static final int XT1DRIVE1           = 0x0080;    /* XT1 Drive Level mode Bit 1 */
  private static final int XT2OFF              = 0x0100;    /* High Frequency Oscillator 2 (XT2); disable */
//  private static final int RESERVED            = 0x0200;    /* RESERVED */
//  private static final int RESERVED            = 0x0400;    /* RESERVED */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
  private static final int XT2BYPASS           = 0x1000;    /* XT2 bypass mode : 0: internal 1:sourced from external pin */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
  private static final int XT2DRIVE0           = 0x4000;    /* XT2 Drive Level mode Bit 0 */
  private static final int XT2DRIVE1           = 0x8000;    /* XT2 Drive Level mode Bit 1 */

/* UCSCTL7 Control Bits */
  private static final int DCOFFG              = 0x0001;    /* DCO Fault Flag */
  private static final int XT1LFOFFG           = 0x0002;    /* XT1 Low Frequency Oscillator Fault Flag */
  private static final int XT1HFOFFG           = 0x0004;    /* XT1 High Frequency Oscillator 1 Fault Flag */
  private static final int XT2OFFG             = 0x0008;    /* High Frequency Oscillator 2 Fault Flag */
//  private static final int RESERVED            = 0x0010;    /* RESERVED */
//  private static final int RESERVED            = 0x0020;    /* RESERVED */
//  private static final int RESERVED            = 0x0040;    /* RESERVED */
//  private static final int RESERVED            = 0x0080;    /* RESERVED */
//  private static final int RESERVED            = 0x0100;    /* RESERVED */
//  private static final int RESERVED            = 0x0200;    /* RESERVED */
//  private static final int RESERVED            = 0x0400;    /* RESERVED */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
//  private static final int RESERVED            = 0x1000;    /* RESERVED */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
//  private static final int RESERVED            = 0x4000;    /* RESERVED */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */

/* UCSCTL8 Control Bits */
  private static final int ACLKREQEN           = 0x0001;    /* ACLK Clock Request Enable */
  private static final int MCLKREQEN           = 0x0002;    /* MCLK Clock Request Enable */
  private static final int SMCLKREQEN          = 0x0004;    /* SMCLK Clock Request Enable */
  private static final int MODOSCREQEN         = 0x0008;    /* MODOSC Clock Request Enable */
//  private static final int RESERVED            = 0x0010;    /* RESERVED */
//  private static final int RESERVED            = 0x0020;    /* RESERVED */
//  private static final int RESERVED            = 0x0040;    /* RESERVED */
//  private static final int RESERVED            = 0x0080;    /* RESERVED */
//  private static final int RESERVED            = 0x0100;    /* RESERVED */
//  private static final int RESERVED            = 0x0200;    /* RESERVED */
//  private static final int RESERVED            = 0x0400;    /* RESERVED */
//  private static final int RESERVED            = 0x0800;    /* RESERVED */
//  private static final int RESERVED            = 0x1000;    /* RESERVED */
//  private static final int RESERVED            = 0x2000;    /* RESERVED */
//  private static final int RESERVED            = 0x4000;    /* RESERVED */
//  private static final int RESERVED            = 0x8000;    /* RESERVED */


  private static final int ACLK_FRQ = 32768;
  private static final int MAX_DCO_FRQ = 16000000;

  private final Timer[] timers;

  private int currentDcoFrequency;

  /**
   * Creates a new <code>UnifiedClockSystem</code> instance.
   *
   */
  public UnifiedClockSystem(MSP430Core cpu, int[] memory, int offset, Timer[] timers) {
    super("UnifiedClockSystem", cpu, memory, offset);
    this.timers = timers;
  }

  public int getMaxDCOFrequency() {
      return MAX_DCO_FRQ;
  }

  public int getAddressRangeMin() {
    return UCSCTL0;
  }

  public int getAddressRangeMax() {
    return UCSCTL8;
  }

  public void reset(int type) {
    // Set the reset states, according to the SLAU208h data sheet.
    write(UCSCTL0, 0x0000, true, cpu.cycles);
    write(UCSCTL1, 0x0020, true, cpu.cycles);
    write(UCSCTL2, 0x101f, true, cpu.cycles);
    write(UCSCTL3, 0x0000, true, cpu.cycles);
    write(UCSCTL4, 0x0044, true, cpu.cycles);
    write(UCSCTL5, 0x0000, true, cpu.cycles);
    write(UCSCTL6, 0xc1cd, true, cpu.cycles);
    write(UCSCTL7, 0x0703, true, cpu.cycles);
    write(UCSCTL8, 0x0707, true, cpu.cycles);
  }

  // do nothing?
  public int read(int address, boolean word, long cycles) {
    int val = memory[address];
    if (word) {
      val |= memory[(address + 1) & 0xffff] << 8;
    }
    return val;
  }

  public void write(int address, int data, boolean word, long cycles) {
    // Currently ignores the word flag...
    if (DEBUG) log("Write to UnifiedClockSystem: " +
		       Utils.hex16(address) + " => " + Utils.hex16(data));

    memory[address] = data & 0xff;
    if (word) memory[address + 1] = (data >> 8) & 0xff;

    setConfiguration(cycles);
  }

  public void interruptServiced(int vector) {
  }


  private void setConfiguration(long cycles) {
    // Read a configuration from the UCSCTL* registers and compute the timer setup

    // Read Modulation counter and DCO TAP from UCSCTL0 (currently unused)
    int modulationBitCounter = ((read(UCSCTL0, true, cycles) >> MOD_BITPOS) & ((1 << MOD_BITWIDTH) - 1));
    int dcoTap = ((read(UCSCTL0, true, cycles) >> DCO_BITPOS) & ((1 << DCO_BITWIDTH) - 1));

    // Read modulation disable bit (currently unused)
    int disableModulation = ((read(UCSCTL1, true, cycles) & DISMOD));

    // Read DCO range selection from UCSCTL1 register
    int dcoRange = ((read(UCSCTL1, true, cycles) >> DCORSEL_BITPOS) & ((1 << DCORSEL_BITWIDTH) - 1));

    // Read DCO FLL multiplier and loop divider from the UCSCTL2 register
    int dcoFLLMultiplier = (read(UCSCTL2, true, cycles) >> FLLN_BITPOS) & ((1 << FLLN_BITWIDTH) - 1);
    int dcoLoopDivider = (read(UCSCTL2, true, cycles) >> FLLD_BITPOS) & ((1 << FLLD_BITWIDTH) - 1);

    // FLL reference clock divider and selection from UCSCTL3 (currently unused)
    int fllRefDiv = (read(UCSCTL3, true, cycles) >> FLLREFDIV_BITPOS) & ((1 << FLLREFDIV_BITWIDTH) - 1);
    int selRef = (read(UCSCTL3, true, cycles) >> SELREF_BITPOS) & ((1 << SELREF_BITWIDTH) - 1);

    // SMCLK divisor
    int divSMclk = (read(UCSCTL5, true, cycles) >> DIVS_BITPOS) & ((1 << DIVS_BITWIDTH) - 1);

    int newDcoFrequency = (dcoFLLMultiplier + 1) * ACLK_FRQ;

    if (newDcoFrequency != currentDcoFrequency) {
      currentDcoFrequency = newDcoFrequency;
      cpu.setDCOFrq(currentDcoFrequency, currentDcoFrequency / (1 << divSMclk));

      if (timers != null) {
	  for(int i = 0; i < timers.length; i++) {
	      timers[i].resetCounter(cycles);
	  }
      }
    }
  }
}
