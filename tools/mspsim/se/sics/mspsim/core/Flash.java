/*
 * Copyright (c) 2009, Friedrich-Alexander University Erlangen, Germany
 * 
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
 * This file is part of mspsim.
 *
 */
/**
 * @author Klaus Stengel <siklsten@informatik.stud.uni-erlangen.de>
 * @author Víctor Ariño <victor.arino@tado.com>
 */
package se.sics.mspsim.core;

import java.util.Arrays;

import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.util.Utils;

public class Flash extends IOUnit {
  
  private static final int FCTL1 = 0x00;
  private static final int FCTL2 = 0x02;
  private static final int FCTL3 = 0x04;
  private static final int FCTL4 = 0x06;
  
  /* Size of the flash controller */
  public static final int SIZE = 8;

  private static final int FRKEY =   0x9600;
  private static final int FWKEY =   0xA500;
  private static final int KEYMASK = 0xff00;
  private static final int CMDMASK = 0x00ff;

  private static final int BLKWRT = 0x80;
  private static final int WRT =    0x40;
  
  private static final int ERASE_SHIFT = 1;
  private static final int ERASE_MASK = 0x06;
  
  /* Erase modes needs to be first due to usage of ordinality */
  private enum WriteMode {
    NONE,
    ERASE_SEGMENT,
    ERASE_MAIN,
    ERASE_ALL,
    WRITE_SINGLE,
    WRITE_BLOCK,
    WRITE_BLOCK_FINISH
  }
  
  private static final int EMEX =    0x20;
  private static final int LOCK =    0x10;
  private static final int WAIT =    0x08;
  private static final int ACCVIFG = 0x04;
  private static final int KEYV =    0x02;
  private static final int BUSY =    0x01;

  private static final int FSSEL_SHIFT = 6;
  private static final int FSSEL_MASK = 0xc0;

  private static final int RESET_VECTOR = 15;
  private static final int NMI_VECTOR = 14;
  private static final int ACCVIE = 1 << 5;
  
  private enum ClockSource {
    ACLK,
    MCLK,
    SMCLK
  };
  
  private static final int MASS_ERASE_TIME = 5297;
  private static final int SEGMENT_ERASE_TIME = 4819;
  
  private static final int WRITE_TIME = 35;

  private static final int BLOCKWRITE_FIRST_TIME = 30;
  private static final int BLOCKWRITE_TIME = 21;
  private static final int BLOCKWRITE_END_TIME = 6;

  private static final int FN_MASK = 0x3f;
  private static final int LOCKINFO = 0x80;

  private FlashRange main_range;
  private FlashRange info_range;
  
  private int mode;      /* FCTL1 */
  private int clockcfg;  /* FCTL2 */
  private int statusreg; /* FCTL3 */

  private boolean locked;
  private boolean wait;
  private boolean blocked_cpu;

  private WriteMode currentWriteMode;
  private int blockwriteCount;
  
  /**
   * Infomem Configurations
   */
  private int infomemcfg = 0;
  /**
   * Whether the infomem is locked or not
   */
  private boolean lockInfo = true;
  
  private TimeEvent end_process = new TimeEvent(0) {
    public void execute(long t) {
      blocked_cpu = false;
      
      switch(currentWriteMode) {
      case NONE:
        break;
        
      case ERASE_SEGMENT:
      case ERASE_MAIN:
      case ERASE_ALL:
        // Erase flags are automatically cleared after each erase
	mode = 0;
	currentWriteMode = WriteMode.NONE;
	cpu.isFlashBusy = false;
	break;
	
      case WRITE_SINGLE:
	cpu.isFlashBusy = false;
	// WRT flags are NOT automatically cleared
	break;
	
      case WRITE_BLOCK:
	blockwriteCount++;
	if (blockwriteCount == 64) {
	  // FIXME: What happens if we try to write more than 64 bytes
	  // on real hardware???
	  logw(WarningType.EXECUTION, "Last access in block mode. Forced exit?");
	  currentWriteMode = WriteMode.WRITE_BLOCK_FINISH;
	}
/*	if (DEBUG) {
	  System.out.println("Write cycle complete, flagged WAIT.");
	} */
	wait = true;
	break;
	
      case WRITE_BLOCK_FINISH:
	if (DEBUG) {
	  log("Programming voltage dropped, write mode disabled.");
	}
	currentWriteMode = WriteMode.NONE;
	cpu.isFlashBusy = false;
	wait = true;
	mode = 0;
	break;
      }
    }
  };
  
  public Flash(MSP430Core cpu, int[] memory, FlashRange main_range,
      FlashRange info_range, int offset) {
    super("Flash", "Internal Flash", cpu, memory, offset);
    this.main_range = main_range;
    this.info_range = info_range;
    locked = true;

    Arrays.fill(memory, main_range.start, main_range.end, 0xff);
    Arrays.fill(memory, info_range.start, info_range.end, 0xff);

    reset(MSP430.RESET_POR);
  }

  public boolean blocksCPU() {
    return blocked_cpu;
  }
  
  public void interruptServiced(int vector) {
    cpu.flagInterrupt(vector, this, false);
  }
  
  public boolean addressInFlash(int address) {
    if (main_range.isInRange(address)) {
      return true;
    }
    if (info_range.isInRange(address)) {
      return true;
    }
    
    return false;
  }
  
  private int getFlashClockDiv() {
    return (clockcfg & FN_MASK) + 1;
  }
  
  private void waitFlashProcess(int time) {
    int instr_addr = cpu.getPC();
    int freqdiv = getFlashClockDiv();
    int myfreq;
    double finish_msec;
    
    cpu.isFlashBusy = true;
    if (addressInFlash(instr_addr)) {
      blocked_cpu = true;
    }
    
    switch(getClockSource()) {
    case ACLK:
      myfreq = cpu.aclkFrq / freqdiv;
      finish_msec = ((double)time * freqdiv * 1000) / cpu.aclkFrq;
      if (DEBUG)
        log("Using ACLK source with f=" + myfreq + "Hz. Time required=" + finish_msec + " ms");
      cpu.scheduleTimeEventMillis(end_process, finish_msec);
      break;
      
    case SMCLK:
      myfreq = cpu.smclkFrq / freqdiv;
      finish_msec = ((double)time * freqdiv * 1000) / cpu.smclkFrq;
      /* if (DEBUG)
	System.out.println("Flash: Using SMCLK source with f=" + myfreq 
	    + " Hz\nFlash: Time required=" + finish_msec + " ms"); */
      cpu.scheduleTimeEventMillis(end_process, finish_msec);
      break;

      
    case MCLK:
      if (DEBUG)
	log("Using MCLK source with div=" + freqdiv);
      cpu.scheduleCycleEvent(end_process, (long)time * freqdiv);
      break;
    }
  }
  
  public boolean needsTick() {
    return false;
  }
  
  public void flashWrite(int address, int data, AccessMode dataMode) {
    int wait_time = -1;
    
    if (locked) {
      if (DEBUG) {
        log("Write to flash blocked because of LOCK flag.");
      }
      return;
    }
    
    if (lockInfo && info_range.isInRange(address)) {
      if (DEBUG) {
        log("Write to infomem blocked because of LOCKINFO flag.");
      }
    }
    
    if (cpu.isFlashBusy || wait == false) {
      if (!((mode & BLKWRT) != 0 && wait)) {
	triggerAccessViolation("Flash write prohbited while BUSY=1 or WAIT=0");
	return;
      }
    }
    
    switch(currentWriteMode) {
    case ERASE_SEGMENT:
      int a_area_start[] = new int[1];
      int a_area_end[] = new int[1];
      getSegmentRange(address, a_area_start, a_area_end);
      int area_start = a_area_start[0];
      int area_end = a_area_end[0];
      
      if (DEBUG) {
	log("Segment erase @" + Utils.hex(address, 4) + 
	    ": erasing area " + Utils.hex(area_start, 4) + "-" +
	    Utils.hex(area_end, 4));
      }
      for (int i = area_start; i < area_end; i++) {
	memory[i] = 0xff;
      }
      waitFlashProcess(SEGMENT_ERASE_TIME);
      break;
      
    case ERASE_MAIN:
      if (! main_range.isInRange(address)) {
	return;
      }
      for (int i = main_range.start; i < main_range.end; i++) {
	memory[i] = 0xff;
      }
      waitFlashProcess(MASS_ERASE_TIME);
      break;
      
    case ERASE_ALL:
      for (int i = main_range.start; i < main_range.end; i++) {
	memory[i] = 0xff;
      }
      for (int i = info_range.start; i < main_range.end; i++) {
	memory[i] = 0xff;
      }
      waitFlashProcess(MASS_ERASE_TIME);
      break;
    case WRITE_SINGLE:
    case WRITE_BLOCK:
      if (currentWriteMode == WriteMode.WRITE_BLOCK) {
        wait = false;
        // TODO: Register target block and verify all writes stay in the same
        // block. What does the real hardware on random writes?!?
        if (blockwriteCount == 0) {
          wait_time = BLOCKWRITE_FIRST_TIME;
          if (DEBUG) {
            log("Flash write in block mode started @" + Utils.hex(address, 4));
          }
          if (addressInFlash(cpu.getPC())) {
            logw(WarningType.EXECUTION, "Oops. Block write access only allowed when executing from RAM.");
          }
        } else {
          wait_time = BLOCKWRITE_TIME;
        }
      } else {
        wait_time = WRITE_TIME;
      }
      /* Flash memory allows clearing bits only */
      memory[address] &= data & 0xff;
      if (dataMode != AccessMode.BYTE) {
          memory[address + 1] &= (data >> 8) & 0xff;
          if (dataMode == AccessMode.WORD20) {
              /* TODO should the write really write the full word? CHECK THIS */
              memory[address + 2] &= (data >> 16) & 0xff;
              memory[address + 3] &= (data >> 24) & 0xff;
          }
      }
      if (DEBUG) {
        log("Writing $" + Utils.hex20(data) + " to $" + Utils.hex(address, 4) + " (" + dataMode.bytes + " bytes)");
      }
      waitFlashProcess(wait_time);
      break;
    }
  }
  
  public void notifyRead(int address) {
    if (cpu.isFlashBusy) {
      triggerAccessViolation("Flash read not allowed while BUSY flag set");
      return;
    }
    if (DEBUG) {
      if (wait == false && currentWriteMode == WriteMode.WRITE_BLOCK) {
	log("Reading flash prohibited. Would read 0x3fff!!!"); 
	log("CPU PC=$" + Utils.hex(cpu.getPC(), 4) 
	    + " read address $" + Utils.hex(address, 4));
      }
    }
  }
  
  private FlashRange getFlashRange(int address) {
    if (main_range.isInRange(address)) {
      return main_range;
    }
    if (info_range.isInRange(address)) {
      return info_range;
    }
    return null;
  }
  
  private void getSegmentRange(int address, int[] start, int[] end) {
    FlashRange addr_type = getFlashRange(address);
    int segsize, ioffset;
    
    if (addr_type == null) {
      throw new RuntimeException("Address not in flash");
    }
    
    segsize = addr_type.segment_size;
    ioffset = address - addr_type.start;
    
    ioffset /= segsize;
    ioffset *= segsize;
    
    start[0] = addr_type.start + ioffset;
    end[0] = start[0] + segsize;
  }
  
  public int read(int address, boolean word, long cycles) {
    address = address - offset;
      
    if (address == FCTL1) {
      return mode | FRKEY;
    }
    if (address == FCTL2) {
      return clockcfg | FRKEY;
    }
    if (address == FCTL3) {
      int retval = statusreg | FRKEY;
      
      if (cpu.isFlashBusy)
	retval |= BUSY;
      
      if (locked)
	retval |= LOCK;
      
      if (wait)
	retval |= WAIT;
      
      return retval;
    }
    if (address == FCTL4) {
      return infomemcfg | FRKEY;
    }

    return 0;
  }

  private ClockSource getClockSource() {
    switch((clockcfg & FSSEL_MASK) >> FSSEL_SHIFT) {
      case 0:
      return ClockSource.ACLK;
      case 1:
      return ClockSource.MCLK;
      case 2:
      case 3:
      return ClockSource.SMCLK;
    }
    throw new RuntimeException("Bad clock source");
  }

  private boolean checkKey(int value) {
    if ((value & KEYMASK) == FWKEY)
      return true;

    logw(WarningType.EXECUTION, "Bad key accessing flash controller --> reset");
    statusreg |= KEYV;
    cpu.flagInterrupt(RESET_VECTOR, this, true);
    return false;
  }

  private void triggerEmergencyExit() {
    mode = 0;
    cpu.isFlashBusy = false;
    wait = true;
    locked = true;
    currentWriteMode = WriteMode.NONE;   
  }
  
  private WriteMode getEraseMode(int regdata) {
    int idx = (regdata & ERASE_MASK) >> ERASE_SHIFT;
    
    for (WriteMode em : WriteMode.values()) {
      if (em.ordinal() == idx)
	return em;
    }
    throw new IllegalArgumentException("Invalid erase mode: " + regdata);
  }
  
  private void triggerErase(int newmode) {
    currentWriteMode = getEraseMode(newmode);
  }
  
  private void triggerLockFlash() {
    locked = true;
  }
  
  private void triggerUnlockFlash() {
    locked = false;
  }
  
  private void triggerAccessViolation(String reason) {
    logw(WarningType.EXECUTION, "Access violation: " + reason + ". PC=$" + Utils.hex(cpu.getPC(), 4));

    statusreg |= ACCVIFG;
    if (cpu.getSFR().isIEBitsSet(SFR.IE1, ACCVIE)) {
      cpu.flagInterrupt(NMI_VECTOR, this, true);
    }
  }
  
  private void triggerSingleWrite() {
    /*if (DEBUG) {
      System.out.println("Single write triggered");
    }*/
    currentWriteMode = WriteMode.WRITE_SINGLE;
  }
  
  private void triggerBlockWrite() {
    if (DEBUG) {
      log("Block write triggered");
    }
    currentWriteMode = WriteMode.WRITE_BLOCK;
    blockwriteCount = 0;
  }
  
  private void triggerEndBlockWrite() {
    if (DEBUG) {
      log("Got end of flash block write");
    }
    currentWriteMode = WriteMode.WRITE_BLOCK_FINISH;
    waitFlashProcess(BLOCKWRITE_END_TIME);
  }
  
  public void write(int address, int value, boolean word, long cycles) {
    address = address - offset;
    if (!word) {
      logw(WarningType.EXECUTION, "Invalid access type to flash controller");
      return;
    }

    if (!(address == FCTL1 || address == FCTL2 || address == FCTL3 
   		 || address == FCTL4)) {
      return;
    }

    if (!checkKey(value)) {
      return;
    }

    int regdata = value & CMDMASK;
    switch (address) {
    case FCTL1:
      // access violation while erase/write in progress
      // exception: block write mode and WAIT==1
//      if ((mode & ERASE_MASK) != 0 || (mode & WRT) != 0) {
      if (cpu.isFlashBusy && ((mode & BLKWRT) == 0 || wait == false)) {
          //	if (!((mode & BLKWRT) != 0 && wait)) {
        triggerAccessViolation("FCTL1 write not allowed while erase/write active");
        return;
      }

      if ((mode & ERASE_MASK) != (regdata & ERASE_MASK)) {
	if ((mode & ERASE_MASK) == 0) {
	  triggerErase(regdata);
	}
	mode &= ~ERASE_MASK;
	mode |= regdata & ERASE_MASK;
      }

      if ((mode & WRT) != (regdata & WRT)) {
	if ((regdata & WRT) != 0) {
	  if ((regdata & BLKWRT) != 0) {
	    triggerBlockWrite();
	    mode |= BLKWRT;
	  } else {
	    triggerSingleWrite();
	  }
	}
	mode &= ~WRT;
	mode |= regdata & WRT;
      }
      
      if ((mode & BLKWRT) != 0 && (regdata & BLKWRT) == 0) {
	triggerEndBlockWrite();
	mode &= ~BLKWRT;
      }
      break;

    case FCTL2:
      // access violation if BUSY==1
      if (cpu.isFlashBusy) {
	triggerAccessViolation(
	    "Register write to FCTL2 not allowed when busy");
	return;
      }
      clockcfg = regdata;
      break;

    case FCTL3:
      if ((statusreg & EMEX) == 0 && (regdata & EMEX) == 1) {
	triggerEmergencyExit();
      }

      if (locked && (regdata & LOCK) == 0) {
	triggerUnlockFlash();
      } else {
	if (!locked && (regdata & LOCK) != 0) {
	  triggerLockFlash();
	}
      }

      if (((statusreg ^ regdata) & KEYV) != 0) {
	statusreg ^= KEYV;
      }
      if (((statusreg ^ regdata) & ACCVIFG) != 0) {
	statusreg ^= ACCVIFG;
      }

      break;
    case FCTL4:
      lockInfo = (regdata & LOCKINFO) > 0;
      infomemcfg = regdata;
      break;
    }
  }

  public void reset(int type) {
    if (DEBUG) {
      log("Got reset!");
    }

    if (type == MSP430.RESET_POR)
      statusreg = 0;
    
    mode = 0;
    clockcfg = 0x42;
    cpu.isFlashBusy = false;
    wait = true;
    locked = true;
    currentWriteMode = WriteMode.NONE;
  }
}
