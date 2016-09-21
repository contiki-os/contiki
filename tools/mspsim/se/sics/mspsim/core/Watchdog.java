/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: $
 *
 * -----------------------------------------------------------------
 *
 * Watchdog
 *
 * Author  : Joakim Eriksson
 * Created : 22 apr 2008
 * Updated : $Date:$
 *           $Revision:$
 */
package se.sics.mspsim.core;

import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.util.Utils;

/**
 * @author joakim
 *
 */
public class Watchdog extends IOUnit implements SFRModule {
  
  private static final int WDTHOLD = 0x80;
  private static final int WDTCNTCL = 0x08;
  private static final int WDTMSEL = 0x10;
  private static final int WDTSSEL = 0x04;
  private static final int WDTISx = 0x03;
  
  private static final int WATCHDOG_VECTOR = 10;
  private static final int WATCHDOG_INTERRUPT_BIT = 0;
  private static final int WATCHDOG_INTERRUPT_VALUE = 1 << WATCHDOG_INTERRUPT_BIT;
  
  private static final int[] DELAY = {
    32768, 8192, 512, 64
  };

  private int resetVector = 15;

  private int wdtctl = 0x4;
  private int offset;
  
  public boolean wdtOn = true;
  private boolean hold = false;

  // The current "delay" when started/clered (or hold)
  private int delay;
  // The target time for this timer
  private long targetTime;
  // Timer ACLK
  private boolean sourceACLK = false;

  // Timer or WDT mode
  private boolean timerMode = false;
  
  private TimeEvent wdtTrigger = new TimeEvent(0, "Watchdog") {
    public void execute(long t) {
//      System.out.println(getName() + " **** executing update timers at " + t + " cycles=" + core.cycles);
      triggerWDT(t);
    }
  };

  public Watchdog(MSP430Core cpu, int address) {
    super("Watchdog", cpu, cpu.memory, address);

    resetVector = cpu.MAX_INTERRUPT;
    
    this.offset = address;
    cpu.getSFR().registerSFDModule(0, WATCHDOG_INTERRUPT_BIT, this, WATCHDOG_VECTOR);
  }
   
  public void interruptServiced(int vector) {
    cpu.flagInterrupt(vector, this, false);
  }

  public void reset(int type) {
      super.reset(type);
      wdtctl = 0x4;
  }

  private void triggerWDT(long time) {
      // Here the WDT triggered!!!
      if (timerMode) {
          SFR sfr = cpu.getSFR();
          sfr.setBitIFG(0, WATCHDOG_INTERRUPT_VALUE);
          scheduleTimer();
          System.out.println("WDT trigger - will set interrupt flag (no reset)");
          cpu.generateTrace(System.out);
      } else {
          System.out.println("WDT trigger - will reset node!");
          cpu.generateTrace(System.out);
          cpu.flagInterrupt(resetVector, this, true);
      }
  }
  
  public int read(int address, boolean word, long cycles) {
	  return wdtctl | 0x6900;
  }

  public void write(int address, int value, boolean word, long cycles) {
    if (address == offset) {
      if ((value >> 8) == 0x5a) {
        wdtctl = value & 0xff;
        if (DEBUG) log("Wrote to WDTCTL: " + Utils.hex8(wdtctl) + " from $" + Utils.hex(cpu.getPC(), 4));
        
        // Is it on?
        wdtOn = (value & 0x80) == 0;
//        boolean lastACLK = sourceACLK;
        sourceACLK = (value & WDTSSEL) != 0;
        if ((value & WDTCNTCL) != 0) {
          // Clear timer => reset the delay
          delay = DELAY[value & WDTISx];
        }
        timerMode = (value & WDTMSEL) != 0;
        // Start it if it should be started!
        if (wdtOn) {
          if (DEBUG) log("Setting WDTCNT to count: " + delay);
          scheduleTimer();
        } else {
          // Stop it and remember current "delay" left!
          wdtTrigger.remove();
        }
      } else {
        // Trigger reset!!
        logw(WarningType.EXECUTION, "illegal write to WDTCTL (" + value + ") from $" + Utils.hex(cpu.getPC(), 4)
            + " - reset!!!!");
        cpu.flagInterrupt(resetVector, this, true);
      }
    }
  }

  private void scheduleTimer() {
      if (sourceACLK) {
          if (DEBUG) log("setting delay in ms (ACLK): " + 1000.0 * delay / cpu.aclkFrq);
          targetTime = cpu.scheduleTimeEventMillis(wdtTrigger, 1000.0 * delay / cpu.aclkFrq);
      } else {
          if (DEBUG) log("setting delay in cycles");
          cpu.scheduleCycleEvent(wdtTrigger, targetTime = cpu.cycles + delay);
      }
  }

  public void enableChanged(int reg, int bit, boolean enabled) {
      if (DEBUG) log("*** Watchdog module enabled: " + enabled);
  }
}
