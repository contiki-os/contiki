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
 * BasicClockModule
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;
import se.sics.mspsim.util.Utils;

public class BasicClockModule extends ClockSystem {

  private static final int DCOCTL = 0x56; // 0x60
  private static final int BCSCTL1 = 0x57; // 0x84 
  private static final int BCSCTL2 = 0x58;

  private static final int ACLK_FRQ = 32768;
  // DCO_FRQ what default frq is the DCO running at???
  private static final int DCO_FRQ = 2500000;
  // What frequency steps to take for the DCO?
  // We have 8 bits + 3 => 11 bits => 2048 combinations...
  // => What is lowest frq??? (zero)
  // Max speed is 8Mhz (CPU limits it) - is max DCO 8Mhz?
  // Based on the scatterweb code it looks like less than
  // 5Mhz is more correct...
  private static final int MAX_DCO_FRQ = 4915200;
  private static final int MIN_DCO_FRQ = 1000;
  private static final int DCO_FACTOR = (MAX_DCO_FRQ - MIN_DCO_FRQ) / 2048;

  private Timer[] timers;

  private int dcoFrequency;
  private int dcoModulator;
  private int resistorSel;
  // These will give =>
  private int calcDCOFrq;
  private int divAclk = 1;
  private int lfxt1Mode;
  private int xt2Off;
  private int mclkSel;
  private int divMclk = 1;
  private int smclSel;
  private int divSMclk = 1;
  private int dcoResitorSel;

  /**
   * Creates a new <code>BasicClockModule</code> instance.
   *
   */
  public BasicClockModule(MSP430Core core, int[] memory, int offset, Timer[] timers) {
    super("BasicClockModule", core, memory, offset);
    this.timers = timers;
    //    reset(0);
  }

  public int getMaxDCOFrequency() {
    return MAX_DCO_FRQ;
  }

  public int getAddressRangeMin() {
    return DCOCTL;
  }

  public int getAddressRangeMax() {
    return BCSCTL2;
  }

  public void reset(int type) {
    write(DCOCTL, 0x60, false, cpu.cycles);
    write(BCSCTL1, 0x84, false, cpu.cycles);
    write(BCSCTL2, 0, false, cpu.cycles);
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
    if (DEBUG) log("Write to BasicClockModule: " +
		       Utils.hex16(address) + " => " + Utils.hex16(data));

    memory[address] = data & 0xff;
    if (word) memory[address + 1] = (data >> 8) & 0xff;


    switch (address) {
    case DCOCTL:
      dcoFrequency = (data >> 5) & 0x7;
      dcoModulator = data & 0x1f;
      if (DEBUG) log("Write: BCM DCOCTL0: DCO Frq:" + dcoFrequency +
			 "  dcoMod:" + dcoModulator);
      break;
    case BCSCTL1:
      resistorSel = data & 0x7;
      divAclk = 1 << ((data >> 4) & 3);
      lfxt1Mode = (data >> 6) & 1;
      xt2Off = (data >> 7) & 1;
      if (DEBUG) log("Write: BCM BCSCTL1: RSel:" + resistorSel +
			 " DivACLK:" + divAclk + " ACLKFrq: " +
			 ACLK_FRQ / divAclk);
      cpu.setACLKFrq(ACLK_FRQ / divAclk);
      updateTimers(cycles);
      break;
    case BCSCTL2:
      mclkSel = (data >> 6) & 3;
      divMclk = 1 << ((data >> 4) & 3);
      smclSel = (data >> 3) & 1;
      divSMclk = 1 << ((data >> 2) & 3);
      dcoResitorSel = data & 1;
      if (DEBUG) log("Write: BCM BCSCTL2: SMCLKDIV: " +
			 divSMclk + " SMCLK_SEL: "
			 + smclSel + " MCLKSel: " + mclkSel + " divMclk: " +
			 divMclk + " DCOResitorSel: " + dcoResitorSel);
      break;
    }

    // resistor selects three bits gives the highest impact on the DCO_FACTOR
    // then dcoFrq and last dcoModulator
    int newcalcDCOFrq = ((dcoFrequency << 5) + dcoModulator +
			 (resistorSel << 8)) * DCO_FACTOR + MIN_DCO_FRQ;
    if (newcalcDCOFrq != calcDCOFrq) {
      calcDCOFrq = newcalcDCOFrq;
      if (DEBUG) log("BCM  DCO_Speed: " + calcDCOFrq);
      cpu.setDCOFrq(calcDCOFrq, calcDCOFrq / divSMclk);
      updateTimers(cycles);
    }
  }

  private void updateTimers(long cycles) {
    if (timers != null) {
      for(int i = 0; i < timers.length; i++) {
        timers[i].resetCounter(cycles);
      }
    }
  }
  
  public void interruptServiced(int vector) {
  }
}
