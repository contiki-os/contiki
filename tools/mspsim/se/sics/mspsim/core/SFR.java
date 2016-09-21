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
 * SFR
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;

/**
 * SFR - emulation of special function registers
 */
public class SFR extends IOUnit {

  public static final int IE1 = 0;
  public static final int IE2 = 1;
  public static final int IFG1 = 2;
  public static final int IFG2 = 3;
  public static final int ME1 = 4;
  public static final int ME2 = 5;

  private int ie1 = 0;
  private int ie2 = 0;
  private int ifg1 = 0;
  private int ifg2 = 0;
  private int me1 = 0;
  private int me2 = 0;

  /* 64 = max number of interrupts */
  private SFRModule[] sfrModule = new SFRModule[64];
  private int[] irqVector = new int[64];
  private boolean[] irqTriggered = new boolean[64];
  private boolean[] autoclear = new boolean[64];
  private int[] irqTriggeredPos = new int[64];

  public SFR(MSP430Core cpu, int[] memory) {
    super("SFR", "Special Function Register", cpu, memory, 0);
    reset(0);
  }

  public void reset(int type) {
    ie1 = 0;
    ie2 = 0;
    ifg1 = 0;
    ifg2 = 0;
    me1 = 0;
    me2 = 0;
    /* no IRQ - but all are autoclear */
    for (int i = 0; i < irqTriggered.length; i++) {
      irqTriggered[i] = false;
      autoclear[i] = true;
    }
  }

  /* reg = 0/1
   * bit = 0-7 (LSB-MSB)
   * module = the module that will be "called"
   */
  public void registerSFDModule(int reg, int bit, SFRModule module, int irqVec) {
    int pos = reg * 8 + bit;
    sfrModule[pos] = module;
    irqVector[pos] = irqVec;
  }
  
  // write
  // write a value to the IO unit
  public void write(int address, int value, boolean word,
			     long cycles) {
    if (DEBUG) log("write to: " + address + " = " + value);
    switch (address) {
    case IE1:
    case IE2:
        updateIE(address - IE1, value);
      break;
    case IFG1:
    case IFG2:
      updateIFG(address - IFG1, value);
      break;
    case ME1:
    case ME2:
      updateME(address - ME1, value);
    }
    memory[address] = value;
  }

  // read
  // read a value from the IO unit
  public int read(int address, boolean word, long cycles) {
    if (DEBUG) log("read from: " + address);
    switch (address) {
    case IE1:
      return ie1;
    case IE2:
      return ie2;
    case IFG1:
      return ifg1;
    case IFG2:
      return ifg2;
    case ME1:
      return me1;
    case ME2:
      return me2;
    default:
      return memory[address];
    }
  }

  private void updateIE(int pos, int value) {
    int oldVal = pos == 0 ? ie1 : ie2;
    int change = oldVal ^ value;
    if (pos == 0) {
      ie1 = value;
    } else {
      ie2 = value;
    }
    updateIRQ(pos, change);
  }

  private void updateIFG(int pos, int value) {
    int oldVal = pos == 0 ? ifg1 : ifg2;
    int change = oldVal ^ value;
    if (pos == 0) {
      ifg1 = value;
    } else {
      ifg2 = value;
    }
    updateIRQ(pos, change);
  }

  private void updateME(int pos, int value) {
    int oldVal = pos == 0 ? me1 : me2;
    int change = oldVal ^ value;
    if (pos == 0) {
      me1 = value;
    } else {
      me2 = value;
    }
    int reg = pos;
    pos = pos * 8;
    for (int i = 0; i < 8; i++) {
      if ((change & 1) == 1)  {
        if (sfrModule[pos] != null) {
          if (DEBUG) log("Calling enable changed on module: " +
              sfrModule[pos].getName() + " enabled = " + (value & 1) + " bit " + i);
          sfrModule[pos].enableChanged(reg, i, (value & 1) > 0);
        }
      }
      change = change >> 1;
      value = value >> 1;
      pos++;
    }
  }
  
  private void updateIRQ(int pos, int change) {
    int ifg = pos == 0 ? ifg1 : ifg2;
    int ie = pos == 0 ? ie1 : ie2;
    pos = pos * 8;
    for (int i = 0; i < 8; i++) {
      if ((change & 1) == 1)  {
        if (sfrModule[pos] != null && !irqTriggered[irqVector[pos]]) {
          /* interrupt goes directly to the module responsible */
          if (DEBUG) log("flagging interrupt: " +
              sfrModule[pos].getName() + " pos: " + pos + " ie: " + (ie & 1) + " ifg:" + (ifg & 1) + " chg: " + change);
          if ((ie & ifg & 1) > 0) {
            int vector = irqVector[pos];
            irqTriggered[vector] = true;
            irqTriggeredPos[vector] = pos;
            cpu.flagInterrupt(vector, this, true);
          }
          //          cpu.flagInterrupt(irqVector[pos], sfrModule[pos], (ie & ifg & 1) > 0);
        }
      }
      pos++;
      change = change >> 1;
      ifg = ifg >> 1;
      ie = ie >> 1;
    }
  }
  
  public void setBitIFG(int index, int bits) {
    int value = index == 0 ? ifg1 : ifg2;
    int after = value | bits;
    int change = value ^ after;
    if (index == 0) ifg1 = after;
    else ifg2 = after;
    
    updateIRQ(index, change);
  }

  public void clrBitIFG(int index, int bits) {
    int value = index == 0 ? ifg1 : ifg2;
    int after = value & ~bits;
    int change = value ^ after;
    if (index == 0) ifg1 = after;
    else ifg2 = after;
    updateIRQ(index, change);
  }

  public boolean isIEBitsSet(int index, int flags) {
    if (index == 0) return (ie1 & flags) == flags;
    else return (ie2 & flags) == flags;
  }

  public int getIFG(int index) {
    if (index == 0) return ifg1;
    else return ifg2;
  }

  public void setAutoclear(int vector, boolean b) {
      autoclear[vector] = b;
  }

  
  public void interruptServiced(int vector) {
    irqTriggered[vector] = false;
    int pos = irqTriggeredPos[vector];
    if (autoclear[vector]) {
        /* clear the bits that correspond to this vector! */
        int bit = pos & 7;
        if (pos < 8) {
            ifg1 &= ~(1 << bit);
        } else {
            ifg2 &= ~(1 << bit);
        }
        if (DEBUG) {
            log("cleared interrupt for " + sfrModule[pos] + " vector: " + vector);
        }
    }
    cpu.flagInterrupt(vector, this, false);
    if (sfrModule[pos] != null) {
      sfrModule[pos].interruptServiced(vector);
    }
  }

} // SFR
