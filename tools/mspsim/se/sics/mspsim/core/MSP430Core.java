/**
 * Copyright (c) 2007, 2008, 2009, Swedish Institute of Computer Science.
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
 * -----------------------------------------------------------------
 *
 * MSP430Core
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.core;
import java.io.PrintStream;
import java.util.ArrayList;

import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.Memory.AccessType;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.DefaultEmulationLogger;
import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.MapTable;
import se.sics.mspsim.util.Utils;

/**
 * The CPU of the MSP430
 */
public class MSP430Core extends Chip implements MSP430Constants {

  public static final int RETURN = 0x4130;

  public static final boolean debugInterrupts = false;

  public static final boolean EXCEPTION_ON_BAD_OPERATION = true;

  // Try it out with 64 k memory
  public final int MAX_MEM;
  public final int MAX_MEM_IO;
  
  // 16 registers of which some are "special" - PC, SP, etc.
  public final int[] reg = new int[16];

  private final RegisterMonitor[] regWriteMonitors = new RegisterMonitor[16];
  private final RegisterMonitor[] regReadMonitors = new RegisterMonitor[16];

  // true => breakpoints can occur!
  boolean breakpointActive = true;

  public final int memory[];
  private final Flash flash;
  boolean isFlashBusy;
  boolean isStopping = false;

  private final Memory memorySegments[];
  Memory currentSegment;

  public long cycles = 0;
  public long cpuCycles = 0;
  MapTable map;
  public final boolean MSP430XArch;
  public final MSP430Config config;

  private final ArrayList<IOUnit> ioUnits;
  private final SFR sfr;
  private final Watchdog watchdog;
  private final ClockSystem bcs;

  // From the possible interrupt sources - to be able to indicate is serviced.
  // NOTE: 64 since more modern MSP430's have more than 16 vectors (5xxx has 64).
  private InterruptHandler interruptSource[] = new InterruptHandler[64];
  public int MAX_INTERRUPT;
  
  protected int interruptMax = -1;
  // Op/instruction represents the last executed OP / instruction
  private int op;
  public int instruction;
  private int extWord;
  int servicedInterrupt = -1;
  InterruptHandler servicedInterruptUnit = null;

  protected boolean interruptsEnabled = false;
  protected boolean cpuOff = false;

  // Not private since they are needed (for fast access...)
  public int dcoFrq = 2500000;
  int aclkFrq = 32768;
  public int smclkFrq = dcoFrq;

  long lastCyclesTime = 0;
  long lastVTime = 0;
  long currentTime = 0;
  long lastMicrosDelta;
  double currentDCOFactor = 1.0;
  
  // Clk A can be "captured" by timers - needs to be handled close to CPU...?
//  private int clkACaptureMode = CLKCAPTURE_NONE;
  // Other clocks too...
  long nextEventCycles;
  private EventQueue vTimeEventQueue = new EventQueue();
  private long nextVTimeEventCycles;

  private EventQueue cycleEventQueue = new EventQueue();
  private long nextCycleEventCycles;
  
  private ArrayList<Chip> chips = new ArrayList<Chip>();

  final ComponentRegistry registry;
  Profiler profiler;

  public MSP430Core(int type, ComponentRegistry registry, MSP430Config config) {
    super("MSP430", "MSP430 Core", null);

    logger = registry.getComponent(EmulationLogger.class);
    if (logger == null) {
        logger = new DefaultEmulationLogger(this, System.out);
        registry.registerComponent("logger", logger);
    }
    
    MAX_INTERRUPT = config.maxInterruptVector;
    MAX_MEM_IO = config.maxMemIO;
    MAX_MEM = config.maxMem;
    MSP430XArch = config.MSP430XArch;

    memory = new int[MAX_MEM];
    memorySegments = new Memory[MAX_MEM >> 8];

    flash = new Flash(this, memory,
            new FlashRange(config.mainFlashStart, config.mainFlashStart + config.mainFlashSize, 512, 64),
            new FlashRange(config.infoMemStart, config.infoMemStart + config.infoMemSize, 128, 64),
            config.flashControllerOffset);

    currentSegment = new Memory() {
        @Override
        public int read(int address, AccessMode mode, AccessType type) throws EmulationException {
            if (address >= MAX_MEM) {
            	throw new EmulationException("Reading outside memory: 0x" + Utils.hex(address, 4));
            }
            return memorySegments[address >> 8].read(address, mode, type);
        }
        @Override
        public void write(int address, int data, AccessMode mode) throws EmulationException {
            if (address >= MAX_MEM) {
                throw new EmulationException("Writing outside memory: 0x" + Utils.hex(address, 4));
            }
            memorySegments[address >> 8].write(address, data, mode);
        }
        @Override
        public int get(int address, AccessMode mode) {
            if (address >= MAX_MEM) {
                throw new EmulationException("Reading outside memory: 0x" + Utils.hex(address, 4));
            }
            return memorySegments[address >> 8].get(address, mode);
        }
        @Override
        public void set(int address, int data, AccessMode mode) {
            if (address >= MAX_MEM) {
                throw new EmulationException("Writing outside memory: 0x" + Utils.hex(address, 4));
            }
            memorySegments[address >> 8].set(address, data, mode);
        }
    };

//    System.out.println("Set up MSP430 Core with " + MAX_MEM + " bytes memory");

    /* this is for detecting writes/read to/from non-existing IO */
    IOUnit voidIO = new IOUnit("void", this, memory, 0) {
        public void interruptServiced(int vector) {
        }
        public void write(int address, int value, boolean word, long cycles) {
            cpu.logw(WarningType.VOID_IO_WRITE, "*** IOUnit write to non-existent IO at $" + Utils.hex(address, 4));
        }
        public int read(int address, boolean word, long cycles) {
            cpu.logw(WarningType.VOID_IO_READ, "*** IOUnit read from non-existent IO at $" + Utils.hex(address, 4));
            return 0;
        }
    };

    /* setup memory segments */
    int maxSeg = MAX_MEM >> 8;
    Memory ramSegment = new RAMSegment(this);
    RAMOffsetSegment ramMirrorSegment = null;
    Memory flashSegment = new FlashSegment(this, flash);
    IOSegment ioSegment = new IOSegment(this, MAX_MEM_IO, voidIO);
    Memory noMemorySegment = new NoMemSegment(this);
    for (int i = 0; i < maxSeg; i++) {
        if (config.isRAM(i << 8)) {
//            System.out.println("Setting RAM segment at: " + Utils.hex16(i << 8));
            memorySegments[i] = ramSegment;
        } else if (config.isRAMMirror(i << 8)) {
            if (ramMirrorSegment == null) {
                ramMirrorSegment = new RAMOffsetSegment(this, config.ramMirrorAddress - config.ramMirrorStart);
            }
//            System.out.println("Setting RAM mirror segment at: " + Utils.hex(i << 8, 4)
//                    + " => " + Utils.hex((i << 8) + ramMirrorSegment.getOffset()));
            memorySegments[i] = ramMirrorSegment;
        } else if (config.isFlash(i << 8) || config.isInfoMem(i << 8)) {
//            System.out.println("Setting Flash segment at: " + Utils.hex16(i << 8));
            memorySegments[i] = flashSegment;
        } else if (config.isIO(i << 8)) {
//            System.out.println("Setting IO segment at: " + Utils.hex16(i << 8));
            memorySegments[i] = ioSegment;
        } else {
//            System.out.println("Setting NoMem segment at: " + Utils.hex16(i << 8));
            memorySegments[i] = noMemorySegment;
        }
    }

    this.registry = registry;
    this.config = config;
    // The CPU need to register itself as chip
    addChip(this);

    // Ignore type for now...
    setModeNames(MODE_NAMES);
    // IOUnits should likely be placed in a hashtable?
    // Maybe for debugging purposes...
    ioUnits = new ArrayList<IOUnit>();

    ioSegment.setIORange(config.flashControllerOffset, Flash.SIZE, flash);
 
    /* Setup special function registers */
    sfr = new SFR(this, memory);
    ioSegment.setIORange(config.sfrOffset, 0x10, sfr);

    // first step towards making core configurable
    Timer[] timers = new Timer[config.timerConfig.length];
    for (int i = 0; i < config.timerConfig.length; i++) {
        Timer t = new Timer(this, memory, config.timerConfig[i]);
        ioSegment.setIORange(config.timerConfig[i].offset, 0x20, t);
        ioSegment.setIORange(config.timerConfig[i].timerIVAddr, 1, t);
        timers[i] = t;
    }

    bcs = config.createClockSystem(this, memory, timers);
    ioSegment.setIORange(bcs.getAddressRangeMin(), bcs.getAddressRangeMax() - bcs.getAddressRangeMin() + 1, bcs);

    // SFR and Basic clock system.
    ioUnits.add(sfr);
    ioUnits.add(bcs);

    config.setup(this, ioUnits);

    /* timers after ports ? */
    for (int i = 0; i < timers.length; i++) {
        ioUnits.add(timers[i]);
    }

    watchdog = new Watchdog(this, config.watchdogOffset);
    ioSegment.setIORange(config.watchdogOffset, 1, watchdog);

    ioUnits.add(watchdog);

    bcs.reset(0);
  }

  public void setIORange(int address, int range, IOUnit io) {
      if (address + range > MAX_MEM_IO) {
          throw new IllegalStateException("Outside IO memory: 0x" + Utils.hex(address, 4));
      }
      IOSegment ioSegment = (IOSegment) memorySegments[address >> 8];
      ioSegment.setIORange(address, range, io);
  }

  public Profiler getProfiler() {
    return profiler;
  }

  public void setProfiler(Profiler prof) {
    registry.registerComponent("profiler", prof);
    profiler = prof;
    profiler.setCPU(this);
  }

  public synchronized void addGlobalMonitor(MemoryMonitor mon) {
      GlobalWatchedMemory gwm;
      if (currentSegment instanceof GlobalWatchedMemory) {
          gwm = (GlobalWatchedMemory)currentSegment;
      } else {
          currentSegment = gwm = new GlobalWatchedMemory(currentSegment);
      }
      gwm.addGlobalMonitor(mon);
  }

  public synchronized void removeGlobalMonitor(MemoryMonitor mon) {
      if (currentSegment instanceof GlobalWatchedMemory) {
          GlobalWatchedMemory gwm = (GlobalWatchedMemory)currentSegment;
          gwm.removeGlobalMonitor(mon);
          if (!gwm.hasGlobalMonitor()) {
              // No more monitors - switch back to normal memory
              currentSegment = gwm.getWatchedMemory();
          }
      }
  }

  public ComponentRegistry getRegistry() {
    return registry;
  }

  public SFR getSFR() {
    return sfr;
  }

  public void addChip(Chip chip) {
    chips.add(chip);
  }

  public Chip getChip(String name) {
    for(Chip chip : chips) {
      if (name.equalsIgnoreCase(chip.getID()) || name.equalsIgnoreCase(chip.getName())) {
        return chip;
      }
    }
    return null;
  }

  public <T extends Chip> T getChip(Class<T> type) {
    for(Chip chip : chips) {
      if (type.isInstance(chip)) {
        return type.cast(chip);
      }
    }
    return null;
  }

  public <T extends Chip> T getChip(Class<T> type, String name) {
      for(Chip chip : chips) {
        if (type.isInstance(chip) &&
                (name.equalsIgnoreCase(chip.getID()) || name.equalsIgnoreCase(chip.getName()))) {
          return type.cast(chip);
        }
      }
      return null;
  }

  public Chip[] getChips() {
      return chips.toArray(new Chip[chips.size()]);
  }

  public <T extends Chip> T[] getChips(Class<T> type) {
      ArrayList<T> list = new ArrayList<T>();
      for(Chip chip : chips) {
          if (type.isInstance(chip)) {
              list.add(type.cast(chip));
          }
      }
      @SuppressWarnings("unchecked")
      T[] tmp = (T[]) java.lang.reflect.Array.newInstance(type, list.size());
      return list.toArray(tmp);
  }

  public Loggable[] getLoggables() {
      Loggable[] ls = new Loggable[ioUnits.size() + chips.size()];
      for (int i = 0; i < ioUnits.size(); i++) {
          ls[i] = ioUnits.get(i);
      }
      for (int i = 0; i < chips.size(); i++) {
          ls[i + ioUnits.size()] = chips.get(i);
      }
      return ls;
  }

  public Loggable getLoggable(String name) {
      Loggable l = getChip(name);
      if (l == null) {
          l = getIOUnit(name);
      }
      return l;
  }

  public boolean hasWatchPoint(int address) {
      Memory mem = memorySegments[address >> 8];
      if (mem instanceof WatchedMemory) {
          return ((WatchedMemory)mem).hasWatchPoint(address);
      }
      return false;
  }

  public synchronized void addWatchPoint(int address, MemoryMonitor mon) {
      int seg = address >> 8;
      WatchedMemory wm;
      if (memorySegments[seg] instanceof WatchedMemory) {
          wm = (WatchedMemory) memorySegments[seg];
      } else {
          wm = new WatchedMemory(address & 0xfff00, memorySegments[seg]);
          memorySegments[seg] = wm;
      }
      wm.addWatchPoint(address, mon);
  }

  public synchronized void removeWatchPoint(int address, MemoryMonitor mon) {
      if (memorySegments[address >> 8] instanceof WatchedMemory) {
          WatchedMemory wm = (WatchedMemory) memorySegments[address >> 8];
          wm.removeWatchPoint(address, mon);
      }
  }

  public synchronized void addRegisterMonitor(int r, RegisterMonitor mon) {
      addRegisterWriteMonitor(r, mon);
      addRegisterReadMonitor(r, mon);
  }

  public synchronized void removeRegisterMonitor(int r, RegisterMonitor mon) {
      removeRegisterWriteMonitor(r, mon);
      removeRegisterReadMonitor(r, mon);
  }

  public synchronized void addRegisterWriteMonitor(int r, RegisterMonitor mon) {
      regWriteMonitors[r] = RegisterMonitor.Proxy.INSTANCE.add(regWriteMonitors[r], mon);
  }

  public synchronized void removeRegisterWriteMonitor(int r, RegisterMonitor mon) {
      regWriteMonitors[r] = RegisterMonitor.Proxy.INSTANCE.remove(regWriteMonitors[r], mon);
  }

  public synchronized void addRegisterReadMonitor(int r, RegisterMonitor mon) {
      regReadMonitors[r] = RegisterMonitor.Proxy.INSTANCE.add(regReadMonitors[r], mon);
  }

  public synchronized void removeRegisterReadMonitor(int r, RegisterMonitor mon) {
      regReadMonitors[r] = RegisterMonitor.Proxy.INSTANCE.remove(regReadMonitors[r], mon);
  }

  public void writeRegister(int r, int value) {
      value &= 0xfffff;
    
      /* PC must never be odd */
      if (r == PC && (value % 2) != 0) {
          /* With a solid compiler, this should never happen.
           * Yet, this has been observed at least once with msp430-gcc 4.6.3. */
          System.out.println("Warning: tried to write odd PC, not allowed! PC=0x" + Integer.toHexString(value));
          value -= 1;
      }

      /* SR can never take values above 0xfff */
      if (r == SR) {
          value &= 0xfff;
      }

    // Before the write!
//    if (value >= MAX_MEM) {
//        System.out.println("Writing larger than MAX_MEM to " + r + " value:" + value);
//        new Throwable().printStackTrace();
//    }
      
      RegisterMonitor rwm = regWriteMonitors[r];
    if (rwm != null) {
        // TODO Add register access mode
        rwm.notifyWriteBefore(r, value, AccessMode.WORD);
        reg[r] = value;
        rwm.notifyWriteAfter(r, value, AccessMode.WORD);
    } else {
        reg[r] = value;
    }
    if (r == SR) {
      boolean oldCpuOff = cpuOff;
      if (debugInterrupts) {
          if (((value & GIE) == GIE) != interruptsEnabled) {
              System.out.println("InterruptEnabled changed: " + !interruptsEnabled);
          }
      }
      boolean oldIE = interruptsEnabled;
      interruptsEnabled = ((value & GIE) == GIE);

//      if (debugInterrupts) System.out.println("Wrote to InterruptEnabled: " + interruptsEnabled + " was: " + oldIE);
      
      if (oldIE == false && interruptsEnabled && servicedInterrupt >= 0) {
//          System.out.println("*** Interrupts enabled while in interrupt : " +
//                  servicedInterrupt + " PC: $" + getAddressAsString(reg[PC]));
          /* must handle pending immediately */
          handlePendingInterrupts();
      }
      
      cpuOff = ((value & CPUOFF) == CPUOFF);
      if (cpuOff != oldCpuOff) {
// 	System.out.println("LPM CPUOff: " + cpuOff + " cycles: " + cycles);
      }
      if (cpuOff) {
        boolean scg0 = (value & SCG0) == SCG0;
        boolean scg1 = (value & SCG1) == SCG1;
        boolean oscoff = (value & OSCOFF) == OSCOFF;
        if (oscoff && scg1 && scg0) {
          setMode(MODE_LPM4);
        } else if (scg1 && scg0){
          setMode(MODE_LPM3);
        } else if (scg1) {
          setMode(MODE_LPM2);
        } else if (scg0) {
          setMode(MODE_LPM1);
        } else {
          setMode(MODE_LPM0); 
        }
      } else {
        setMode(MODE_ACTIVE);
      }
    }
  }

  public int readRegister(int r) {
    int value;
    RegisterMonitor rrm = regReadMonitors[r];
    if (rrm != null) {
        // TODO Register access mode
        rrm.notifyReadBefore(r, AccessMode.WORD);
        value = reg[r];
        rrm.notifyReadAfter(r, AccessMode.WORD);
    } else {
        value = reg[r];
    }
    return value;
  }

  public int readRegisterCG(int r, int m) {
      // CG1 + m == 0 => SR!
    if ((r == CG1 && m != 0) || r == CG2) {
      // No monitoring here... just return the CG values
      return CREG_VALUES[r - 2][m];
    }
    int value;
    RegisterMonitor rrm = regReadMonitors[r];
    if (rrm != null) {
        // TODO Register access mode
        rrm.notifyReadBefore(r, AccessMode.WORD);
        value = reg[r];
        rrm.notifyReadAfter(r, AccessMode.WORD);
    } else {
        value = reg[r];
    }
    return value;
  }

  public int incRegister(int r, int value) {
    int registerValue;
    RegisterMonitor rm = regReadMonitors[r];
    if (rm != null) {
        rm.notifyReadBefore(r, AccessMode.WORD);
        registerValue = reg[r];
        rm.notifyReadAfter(r, AccessMode.WORD);
    } else {
        registerValue = reg[r];
    }
    rm = regWriteMonitors[r];
    registerValue += value;
    if (rm != null) {
      rm.notifyWriteBefore(r, registerValue, AccessMode.WORD);
      reg[r] = registerValue;
      rm.notifyWriteAfter(r, registerValue, AccessMode.WORD);
    } else {
      reg[r] = registerValue;
    }
    return reg[r];
  }

  public void setACLKFrq(int frequency) {
    aclkFrq = frequency;
  }

  public void setDCOFrq(int frequency, int smclkFrq) {
    dcoFrq = frequency;
    this.smclkFrq = smclkFrq;
    // update last virtual time before updating DCOfactor
    lastVTime = getTime();
    lastCyclesTime = cycles;
    lastMicrosDelta = 0;

    currentDCOFactor = 1.0 * bcs.getMaxDCOFrequency() / frequency;

    /*    System.out.println("*** DCO: MAX:" + bcs.getMaxDCOFrequency() +
	  " current: " + frequency + " DCO_FAC = " + currentDCOFactor);*/
    if (DEBUG)
      log("Set smclkFrq: " + smclkFrq);
    dcoReset();
  }

  /* called after dcoReset */
  protected void dcoReset() {
  }
  
  // returns global time counted in max speed of DCOs (~5Mhz)
  public long getTime() {
    long diff = cycles - lastCyclesTime;
    return lastVTime + (long) (diff * currentDCOFactor);
  }

  // Converts a virtual time to a cycles time according to the current
  // cycle speed
  private long convertVTime(long vTime) {
    long tmpTime = lastCyclesTime + (long) ((vTime - lastVTime) / currentDCOFactor);
//    System.out.println("ConvertVTime: vTime=" + vTime + " => " + tmpTime);
    return tmpTime;
  }
  
  // get elapsed time in seconds
  public double getTimeMillis() {
      return 1000.0 * getTime() / bcs.getMaxDCOFrequency();
  }
  
  private void executeEvents() {
    if (cycles >= nextVTimeEventCycles) {
      if (vTimeEventQueue.eventCount == 0) {
        nextVTimeEventCycles = cycles + 10000;
      } else {
        TimeEvent te = vTimeEventQueue.popFirst();
        long now = getTime();
//        if (now > te.time) {
//          System.out.println("VTimeEvent got delayed by: " + (now - te.time) + " at " +
//              cycles + " target Time: " + te.time + " class: " + te.getClass().getName());
//        }
        te.execute(now);
        if (vTimeEventQueue.eventCount > 0) {
          nextVTimeEventCycles = convertVTime(vTimeEventQueue.nextTime);
        } else {
          nextVTimeEventCycles = cycles + 10000;          
        }
      }
    }
    
    if (cycles >= nextCycleEventCycles) {
      if (cycleEventQueue.eventCount == 0) {
        nextCycleEventCycles = cycles + 10000;
      } else {
        TimeEvent te = cycleEventQueue.popFirst();
        te.execute(cycles);
        if (cycleEventQueue.eventCount > 0) {
          nextCycleEventCycles = cycleEventQueue.nextTime;
        } else {
          nextCycleEventCycles = cycles + 10000;          
        }
      }
    }
    
    // Pick the one with shortest time in the future.
    nextEventCycles = nextCycleEventCycles < nextVTimeEventCycles ? 
        nextCycleEventCycles : nextVTimeEventCycles;
  }
  
  /**
   * Schedules a new Time event using the cycles counter
   * @param event
   * @param time
   */
  public void scheduleCycleEvent(TimeEvent event, long cycles) {
    long currentNext = cycleEventQueue.nextTime;
    cycleEventQueue.addEvent(event, cycles);
    if (currentNext != cycleEventQueue.nextTime) {
      nextCycleEventCycles = cycleEventQueue.nextTime;
      if (nextEventCycles > nextCycleEventCycles) {
        nextEventCycles = nextCycleEventCycles;
      }
    }
  }

  
  /**
   * Schedules a new Time event using the virtual time clock
   * @param event
   * @param time
   */
  public void scheduleTimeEvent(TimeEvent event, long time) {
    long currentNext = vTimeEventQueue.nextTime;
    vTimeEventQueue.addEvent(event, time);
    if (currentNext != vTimeEventQueue.nextTime) {
      // This is only valid when not having a cycle event queue also...
      // if we have it needs to be checked also!
      nextVTimeEventCycles = convertVTime(vTimeEventQueue.nextTime);
      if (nextEventCycles > nextVTimeEventCycles) {
        nextEventCycles = nextVTimeEventCycles;
      }
      /* Warn if someone schedules a time backwards in time... */
      if (cycles > nextVTimeEventCycles) {
        logger.logw(this, WarningType.EMULATION_ERROR, "Scheduling time event backwards in time!!!");
        throw new IllegalStateException("Cycles are passed desired future time...");
      }
    }
  }
  
  
  /**
   * Schedules a new Time event msec milliseconds in the future
   * @param event
   * @param time
   */
  public long scheduleTimeEventMillis(TimeEvent event, double msec) {
      /*    System.out.println("MAX_DCO " + bcs.getMaxDCOFrequency());*/
    long time = (long) (getTime() + msec / 1000 * bcs.getMaxDCOFrequency());
//    System.out.println("Scheduling at: " + time + " (" + msec + ") getTime: " + getTime());
    scheduleTimeEvent(event, time);
    return time;
  }

  public void printEventQueues(PrintStream out) {
      out.println("Current cycles: " + cycles + "  virtual time:" + getTime());
      out.println("Cycle event queue: (next time: " + nextCycleEventCycles + ")");
      cycleEventQueue.print(out);
      out.println("Virtual time event queue: (next time: " + nextVTimeEventCycles + ")");
      vTimeEventQueue.print(out);
  }
 
  // Should also return active units...
  public IOUnit getIOUnit(String name) {
    for (IOUnit ioUnit : ioUnits) {
      if (name.equalsIgnoreCase(ioUnit.getID()) ||
          name.equalsIgnoreCase(ioUnit.getName())) {
	return ioUnit;
      }
    }
    return null;
  }

  public <T> T getIOUnit(Class<T> type) {
      for (IOUnit ioUnit : ioUnits) {
          if (type.isInstance(ioUnit)) {
              return type.cast(ioUnit);
          }
      }
      return null;
  }

  public <T> T getIOUnit(Class<T> type, String name) {
      for (IOUnit ioUnit : ioUnits) {
          if (type.isInstance(ioUnit)
                  && (name.equalsIgnoreCase(ioUnit.getID())
                          || name.equalsIgnoreCase(ioUnit.getName()))) {
              return type.cast(ioUnit);
          }
      }
      return null;
  }

  private void resetIOUnits() {
      for (IOUnit ioUnit : ioUnits) {
          ioUnit.reset(RESET_POR);
      }
  }

  private void internalReset() {
    for (int i = 0, n = interruptSource.length; i < n; i++) {
      interruptSource[i] = null;
    }
    servicedInterruptUnit = null;
    servicedInterrupt = -1;
    interruptMax = -1;
    writeRegister(SR, 0);
   
    cycleEventQueue.removeAll();
    vTimeEventQueue.removeAll();

    for (Chip chip : chips) {
      chip.notifyReset();
    }
    // Needs to be last since these can add events...
    resetIOUnits();
  
    if (profiler != null) {
        profiler.resetProfile();
    }
  }

  public EmulationLogger getLogger() {
      return logger;
  }
  
  public void reset() {
    flagInterrupt(MAX_INTERRUPT, null, true);
  }

  // Indicate that we have an interrupt now!
  // We should only get same IOUnit for same interrupt level
  public void flagInterrupt(int interrupt, InterruptHandler source,
      boolean triggerIR) {
    if (triggerIR) {
      interruptSource[interrupt] = source;

      if (debugInterrupts) {
        if (source != null) {
          System.out.println("### Interrupt " + interrupt  + " flagged ON by " + source.getName() + " prio: " + interrupt);
        } else {
          System.out.println("### Interrupt " + interrupt + " flagged ON by <null>");
        }
      }

      // MAX priority is executed first - update max if this is higher!
      if (interrupt > interruptMax) {
        interruptMax = interrupt;
      }
      if (interruptMax == MAX_INTERRUPT) {
        // This can not be masked at all!
        interruptsEnabled = true;
        servicedInterrupt = -1; /* Make sure this interrupt will be executed immediately */
      }
    } else {
      if (interruptSource[interrupt] == source) {
        if (debugInterrupts) {
          System.out.println("### Interrupt flagged OFF by " + source.getName() + " prio: " + interrupt);
        }
        interruptSource[interrupt] = null;
        reevaluateInterrupts();
      }
    }
  }

  private void reevaluateInterrupts() {
    interruptMax = -1;
    for (int i = 0; i < interruptSource.length; i++) {
      if (interruptSource[i] != null)
        interruptMax = i;
    }
  }

  // returns the currently serviced interrupt (vector ID)
  public int getServicedInterrupt() {
    return servicedInterrupt;
  }

  // This will be called after an interrupt have been handled
  // In the main-CPU loop
  public void handlePendingInterrupts() {
    // By default no int. left to process...
    
    reevaluateInterrupts();
    
    servicedInterrupt = -1;
    servicedInterruptUnit = null;
  }  

  void profileCall(int dst, int pc) {
      MapEntry function = map.getEntry(dst);
      if (function == null) {
          function = getFunction(map, dst);
      }
      profiler.profileCall(function, cpuCycles, pc);
  }
  
  void printWarning(EmulationLogger.WarningType type, int address) throws EmulationException {
      String message;
      switch(type) {
      case MISALIGNED_READ:
          message = "**** Illegal read - misaligned word from $" +
                  getAddressAsString(address) + " at $" + getAddressAsString(reg[PC]);
          break;
      case MISALIGNED_WRITE:
          message = "**** Illegal write - misaligned word to $" +
                  getAddressAsString(address) + " at $" + getAddressAsString(reg[PC]);
          break;
      case ADDRESS_OUT_OF_BOUNDS_READ:
          message = "**** Illegal read - out of bounds from $" +
                  getAddressAsString(address) + " at $" + getAddressAsString(reg[PC]);
          break;
      case ADDRESS_OUT_OF_BOUNDS_WRITE:
          message = "**** Illegal write -  out of bounds from $" +
                  getAddressAsString(address) + " at $" + getAddressAsString(reg[PC]);
          break;
      default:
          message = "**** " + type + " address $" + getAddressAsString(address) +
          " at $" + getAddressAsString(reg[PC]);
          break;
      }
      logger.logw(this, type, message);
  }

  public void generateTrace(PrintStream out) {
    /* Override if a stack trace or other additional warning info should
     * be printed */ 
  }

  private int serviceInterrupt(int pc) {
    int pcBefore = pc;
    int spBefore = readRegister(SP);
    int sp = spBefore;
    int sr = readRegister(SR);
    
    if (profiler != null) {
      profiler.profileInterrupt(interruptMax, cycles);
    }
        
    if (flash.blocksCPU()) {
      /* TODO: how should this error/warning be handled ?? */
      throw new IllegalStateException(
          "Got interrupt while flash controller blocks CPU. CPU CRASHED.");
    }
    
    // Only store stuff on irq except reset... - not sure if this is correct...
    // TODO: Check what to do if reset is called!
    if (interruptMax < MAX_INTERRUPT) {
      // Push PC and SR to stack
      // store on stack - always move 2 steps (W) even if B.
      writeRegister(SP, sp = spBefore - 2);
      currentSegment.write(sp, pc, AccessMode.WORD);

      writeRegister(SP, sp = sp - 2);
      currentSegment.write(sp, (sr & 0x0fff) | ((pc & 0xf0000) >> 4), AccessMode.WORD);
    }
    // Clear SR
    writeRegister(SR, 0); // sr & ~CPUOFF & ~SCG1 & ~OSCOFF);

    // Jump to the address specified in the interrupt vector
    pc = currentSegment.read(0xfffe - (MAX_INTERRUPT - interruptMax) * 2, AccessMode.WORD, AccessType.READ);
    writeRegister(PC, pc);

    servicedInterrupt = interruptMax;
    servicedInterruptUnit = interruptSource[servicedInterrupt];

    // Flag off this interrupt - for now - as soon as RETI is
    // executed things might change!
    reevaluateInterrupts();
    
    if (servicedInterrupt == MAX_INTERRUPT) {
        if (debugInterrupts) System.out.println("**** Servicing RESET! => $" + getAddressAsString(pc));
        internalReset();
    }
    
    
    // Interrupts take 6 cycles!
    cycles += 6;

    if (debugInterrupts) {
      System.out.println("### Executing interrupt: " +
			 servicedInterrupt + " at "
			 + pcBefore + " to " + pc +
			 " SP before: " + spBefore +
			 " Vector: " + Utils.hex16(0xfffe - (MAX_INTERRUPT - servicedInterrupt) * 2));
    }
    
    // And call the serviced routine (which can cause another interrupt)
    if (servicedInterruptUnit != null) {
      if (debugInterrupts) {
        System.out.println("### Calling serviced interrupt on: " +
                           servicedInterruptUnit.getName());
      }
      servicedInterruptUnit.interruptServiced(servicedInterrupt);
    }
    return pc;
  }

  /* returns true if any instruction was emulated - false if CpuOff */
  public int emulateOP(long maxCycles) throws EmulationException {
    //System.out.println("CYCLES BEFORE: " + cycles);
    int pc = readRegister(PC);
    long startCycles = cycles;
    
    // -------------------------------------------------------------------
    // Interrupt processing [after the last instruction was executed]
    // -------------------------------------------------------------------
    if (interruptsEnabled && servicedInterrupt == -1 && interruptMax >= 0) {
      pc = serviceInterrupt(pc);
    }

    /* Did not execute any instructions */
    if (cpuOff || flash.blocksCPU()) {
      //       System.out.println("Jumping: " + (nextIOTickCycles - cycles));
      // nextEventCycles must exist, otherwise CPU can not wake up!?

      // If CPU is not active we must run the events here!!!
      // this can trigger interrupts that wake the CPU
      // -------------------------------------------------------------------
      // Event processing - note: This can trigger IRQs!
      // -------------------------------------------------------------------
      /* This can flag an interrupt! */
      while (cycles >= nextEventCycles) {
        executeEvents();
      }

      if (interruptsEnabled && interruptMax > 0) {
          /* can not allow for jumping to nextEventCycles since that would jump too far */
          return -1;
      }

      if (maxCycles >= 0 && maxCycles < nextEventCycles) {
        // Should it just freeze or take on extra cycle step if cycles > max?
        cycles = cycles < maxCycles ? maxCycles : cycles;
      } else {
        cycles = nextEventCycles;
      }
      return -1;
    }

    int pcBefore = pc;
    instruction = currentSegment.read(pc, AccessMode.WORD, AccessType.EXECUTE);
    if (isStopping) {
        // Signaled to stop the execution before performing the instruction
        return -2;
    }
    int ext3_0 = 0;
    int ext10_7 = 0;
    int extSrc = 0;
    int extDst = 0;
    boolean repeatsInDstReg = false;
    boolean wordx20 = false;

    /* check for extension words */
    if ((instruction & 0xf800) == 0x1800) {
        extWord = instruction;
        ext3_0 = instruction & 0xf; /* bit 3 - 0 - either repeat count or dest 19-16 */
        ext10_7 = (instruction >> 7) & 0xf; /* bit 10 - 7 - src 19-16 */
        extSrc = ext10_7 << 16;
        extDst = ext3_0 << 16;
        pc += 2;

	// Bit 7 in the extension word indicates that the number of
	// repeats is found in the register pointed to by ext3_0. If
	// the bit is 0, ext3_0 contains the number of repeats. If the
	// bit is 1, ext3_0 contains the register number that holds
	// the number of repeats.
	if ((instruction & 0x80) == 0x80) {
	    repeatsInDstReg = true;
	}
	// Bit 6 indicates whether or not the data length mode should
	// be 20 bits. A one means traditional MSP430 mode; a zero
	// indicates 20 bit mode. (XXX: there is a reserved data
	// length mode if this bit is zero and the MSP430 instruction
	// that follows the extension word also has a zero bit data
	// length mode.)
	wordx20 = (instruction & 0x40) == 0;

	instruction = currentSegment.read(pc, AccessMode.WORD, AccessType.EXECUTE);
        /*System.out.println("*** Extension word!!! " + Utils.hex16(extWord) +
                "  read the instruction too: " + Utils.hex16(instruction) + " at " + Utils.hex16(pc - 2));*/
    } else {
        extWord = 0;
    }
    
    op = instruction >> 12;
    int sp = 0;
    int sr = 0;
    int rval = 0; /* register value */
    int repeats = 1; /* msp430X can repeat some instructions in some cases */
    boolean zeroCarry = false; /* msp430X can zero carry in repeats */
    boolean word = (instruction & 0x40) == 0;

    /* NOTE: there is a mode when wordx20 = true & word = true that is resereved */
    AccessMode mode = wordx20 ? AccessMode.WORD20 : (word ? AccessMode.WORD : AccessMode.BYTE);
    
    //if (mode == AccessMode.WORD20) System.out.println("WORD20 not really supported...");

    // Destination vars
    int dstRegister = 0;
    int dstAddress = -1;
    boolean dstRegMode = false;
    int dst = -1;

    boolean write = false;
    boolean updateStatus = true;

    // When is PC increased  probably immediately (e.g. here)?
    pc += 2;

    writeRegister(PC, pc);

    switch (op) {
    case 0:
        // MSP430X - additional instructions
        op = instruction & 0xf0f0;
        if (!MSP430XArch) 
            throw new EmulationException("Executing MSP430X instruction but MCU is not a MSP430X");
//        System.out.println("Executing MSP430X instruction op:" + Utils.hex16(op) +
//                " ins:" + Utils.hex16(instruction) + " PC = $" + getAddressAsString(pc - 2));
        int src = 0;
        /* data is either bit 19-16 or src register */
        int srcData = (instruction & 0x0f00) >> 8;
        int dstData = (instruction & 0x000f);
        boolean rrword = true;
        mode = AccessMode.WORD20;

        switch(op) {
        // 20 bit register write
        case MOVA_IND:
        	/* Read from address in src register (20-bit?), move to destination register (=20 bit). */
        	writeRegister(dstData, currentSegment.read(readRegister(srcData), mode, AccessType.READ));
        	updateStatus = false;
        	cycles += 3;
            break;
        case MOVA_IND_AUTOINC:
            if (profiler != null && instruction == 0x0110) {
                profiler.profileReturn(cpuCycles);
            }
            writeRegister(PC, pc);
            /* read from address in register */
            src = readRegister(srcData);
//            System.out.println("Reading $" + getAddressAsString(src) +
//                    " from register: " + srcData);
            dst = currentSegment.read(src, mode, AccessType.READ);
//            System.out.println("Reading from mem: $" + getAddressAsString(dst));
            writeRegister(srcData, src + 4);
//            System.out.println("*** Writing $" + getAddressAsString(dst) + " to reg: " + dstData);
            writeRegister(dstData, dst);
            updateStatus = false;
	    cycles += 3;
            break;
        case MOVA_ABS2REG:
            src = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
            writeRegister(PC, pc += 2);
            dst = src + (srcData << 16);
            //System.out.println(Utils.hex20(pc) + " MOVA &ABS Reading from $" + getAddressAsString(dst) + " to reg: " + dstData);
            dst = currentSegment.read(dst, mode,  AccessType.READ);
            //System.out.println("   => $" + getAddressAsString(dst));
            writeRegister(dstData, dst);
            updateStatus = false;
	    cycles += 4;
            break;
	case MOVA_INDX2REG:
		/* Read data from address in memory, indexed by source
		 * register, and place into destination register. */
		int index = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
		int indexModifier = readRegister(srcData);

		index = convertTwoComplement16(index);
		indexModifier = convertTwoComplement20(indexModifier);

		writeRegister(dstData, currentSegment.read(indexModifier + index, mode, AccessType.READ));
		writeRegister(PC, pc += 2);
		updateStatus = false;
		cycles += 4;
		break;

	case MOVA_REG2ABS:
            dst = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
            writeRegister(PC, pc += 2);
	    currentSegment.write(dst + (dstData << 16), readRegister(srcData), mode);
            updateStatus = false;
	    cycles += 4;
            break;

	case MOVA_REG2INDX:
		/* Read data from register, write to address in memory,
		 * indexed by source register. */
		index = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
		indexModifier = readRegister(dstData);

		index = convertTwoComplement16(index);
		indexModifier = convertTwoComplement20(indexModifier);

		currentSegment.write(indexModifier + index, readRegister(srcData), mode);
		writeRegister(PC, pc += 2);
		updateStatus = false;
		cycles += 4;
		break;

        case MOVA_IMM2REG:
            src = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
            writeRegister(PC, pc += 2);
            dst = src + (srcData << 16);
//            System.out.println("*** Writing $" + getAddressAsString(dst) + " to reg: " + dstData);
            dst &= 0xfffff;
            writeRegister(dstData, dst);
            updateStatus = false;
	    cycles += 2;
            break;


        case ADDA_IMM:
        	// For all immediate instructions, the data low 16 bits of
        	// the data is stored in the following word (PC + 2) and
        	// the high 4 bits in the instruction word, which we have
        	// masked out as srcData.
        	int immData = currentSegment.read(pc, AccessMode.WORD, AccessType.READ) + (srcData << 16);
        	writeRegister(PC, pc += 2);
        	int dstArg = readRegister(dstData);
        	dst = dstArg + immData;

        	sr = StatusRegister.updateSR(readRegister(SR), immData, dstArg, dst);
    		writeRegister(SR, sr);
    		updateStatus = false;

        	dst &= 0xfffff;
        	writeRegister(dstData, dst);
        	cycles += 3;
        	break;

	case CMPA_IMM: {
		/* Status Bits N: Set if result is negative (src > dst), reset if positive (src ≤ dst)
	       Z: Set if result is zero (src = dst), reset otherwise (src ≠ dst)
	       C: Set if there is a carry from the MSB, reset otherwise
	       V: Set if the subtraction of a negative source operand from a positive destination
	       operand delivers a negative result, or if the subtraction of a positive source
	       operand from a negative destination operand delivers a positive result, reset
	       otherwise (no overflow) */
		immData = currentSegment.read(pc, AccessMode.WORD, AccessType.READ) + (srcData << 16);
		writeRegister(PC, pc += 2);
		sr = readRegister(SR);

		int destRegValue = readRegister(dstData);
		sr &= ~(NEGATIVE | ZERO | CARRY | OVERFLOW);
		if (destRegValue >= immData) {
			sr |= CARRY;
		}
		if (destRegValue < immData) {
			sr |= NEGATIVE;
		}
		if (destRegValue == immData) {
			sr |= ZERO;
		}

		int cmpTmp = destRegValue - immData;
		int b = 0x80000; // CMPA always use 20 bit data length

		if (((destRegValue ^ cmpTmp) & b) == 0 &&
				(((destRegValue ^ immData) & b) != 0)) {
			sr |= OVERFLOW;
		}

		writeRegister(SR, sr);
		updateStatus = false;
		cycles += 3;
		break;
	}
        case SUBA_IMM:
            immData = currentSegment.read(pc, AccessMode.WORD, AccessType.READ) + (srcData << 16);
            writeRegister(PC, pc += 2);
	    dst = readRegister(dstData) - immData;
	    writeRegister(dstData, dst);
	    cycles += 3;
	    break;

        case MOVA_REG:
	    cycles += 1;
	    /* as = 0 since register mode */
	    writeRegister(dstData, readRegisterCG(srcData, 0));
		updateStatus = false;
	    break;

        case CMPA_REG: {
        	sr = readRegister(SR);
        	sr &= ~(NEGATIVE | ZERO | CARRY | OVERFLOW);
        	int destRegValue = readRegister(dstData);
        	int srcRegValue = readRegisterCG(srcData, 0);
        	if (destRegValue >= srcRegValue) {
        		sr |= CARRY;
        	}
        	if (destRegValue < srcRegValue) {
        		sr |= NEGATIVE;
        	}
        	if (destRegValue == srcRegValue) {
        		sr |= ZERO;
        	}

        	int cmpTmp = destRegValue - srcRegValue;
        	int b = 0x80000; // CMPA always use 20 bit data length

        	if (((destRegValue ^ cmpTmp) & b) == 0 &&
        			(((destRegValue ^ srcRegValue) & b) != 0)) {
        		sr |= OVERFLOW;
        	}

        	writeRegister(SR, sr);
        	updateStatus = false;
        	cycles += 1;
        	break;
        }

	case ADDA_REG:
	    // Assume AS = 2
	    dst = readRegister(dstData) + readRegisterCG(srcData, 2);
	    writeRegister(dstData, dst);
    	sr = StatusRegister.updateSR(readRegister(SR), readRegisterCG(srcData, 2), dstData, dst);
		writeRegister(SR, sr);
		updateStatus = false;
	    cycles += 1;
	    break;
        case SUBA_REG:
            // Assume AS = 2
	    dst = readRegister(dstData) - readRegisterCG(srcData, 2);
	    writeRegister(dstData, dst);
	    cycles += 1;
	    break;

        case RRXX_ADDR:
            rrword = false;
        case RRXX_WORD:
            int count = ((instruction >> 10) & 0x03) + 1;
            dst = readRegister(dstData);
            sr = readRegister(SR);
            int nxtCarry = 0;
	    int carry = (sr & CARRY) > 0? 1: 0;
            if (rrword) {
                mode = AccessMode.WORD;
                dst = dst & 0xffff;
            }
	    cycles += 1 + count;
            switch(instruction & RRMASK) {
            /* if word zero anything above */
            case RRCM:
                /* if (rrword): Rotate right through carry the 16-bit CPU register content
		   if (!rrword): Rotate right through carry the 20-bit CPU register content */

		/* Pull the (count) lowest bits from dst - those will
		 * be placed in the (count) high bits of dst after the
		 * instruction is complete. */
		int dst_low = dst & ((1 << count) - 1);

		/* Grab the bit that will be in the carry flag when instruction completes. */
		nxtCarry = (dst & (1 << (count + 1))) > 0? CARRY: 0;

		/* Rotate dst. */
		dst = dst >> (count);

		/* Rotate the high bits, insert into dst. */
		if (rrword) {
		    dst |= (dst_low << (17 - count)) | (carry << (16 - count));
		} else {
		    dst |= (dst_low << (21 - count)) | (carry << (20 - count));
		}
                break;
            case RRAM:
//                System.out.println("RRAM executing");
                /* roll in MSB from above */
                /* 1 11 111 1111 needs to get in if MSB is 1 */
                if ((dst & (rrword ? 0x8000 : 0x80000)) > 0) {
                    /* add some 1 bits above MSB if MSB is 1 */
                    dst = dst | (rrword ? 0xf8000 : 0xf80000);
                }
                dst = dst >> (count - 1);
                nxtCarry = (dst & 1) > 0 ? CARRY : 0;
                dst = dst >> 1;
                break;
            case RLAM:
		//                System.out.println("RLAM executing at " + pc);
                /* just roll in "zeroes" from left */
                dst = dst << (count - 1);
                nxtCarry = (dst & (rrword ? 0x8000 : 0x80000)) > 0 ? CARRY : 0;
                dst = dst << 1;
                break;
            case RRUM:
                //System.out.println("RRUM executing");
                /* just roll in "zeroes" from right */
                dst = dst >> (count - 1);
                nxtCarry = (dst & 1) > 0 ? CARRY : 0;
                dst = dst >> 1;
                break;
            }
            /* clear overflow - set carry according to above OP */
            writeRegister(SR, (sr & ~(CARRY | OVERFLOW)) | nxtCarry);
            dst = dst & (rrword ? 0xffff : 0xfffff);
            writeRegister(dstData, dst);
            break;
        default:
            System.out.println("MSP430X instruction not yet supported: " +
			       Utils.hex16(instruction) +
			       " op " + Utils.hex16(op));
            throw new EmulationException("Found unsupported MSP430X instruction " +
					 Utils.hex16(instruction) +
					 " op " + Utils.hex16(op));
        }
        break;
    case 1:
    {
      // -------------------------------------------------------------------
      //  Single operand instructions
      // -------------------------------------------------------------------

      // Register
      dstRegister = instruction & 0xf;
      
      /* check if this is a MSP430X CALLA instruction */
      if ((op = instruction & CALLA_MASK) > RETI) {
          pc = readRegister(PC);

          dst = -1; /* will be -1 if not a call! */
          /* do not update status after these instructions!!! */
          updateStatus = false;
          switch(op) {
          case CALLA_REG:
              // The CALLA operations increase the SP before 
              // address resolution!
              // store on stack - always move 2 steps before resolution
              sp = readRegister(SP) - 2;
              writeRegister(SP, sp);

              dst = readRegister(dstRegister);
              /*System.out.println("CALLA REG => " + Utils.hex20(dst));*/
              cycles += 5;
              break;
          case CALLA_INDEX:
              /* CALLA X(REG) => REG + X is the address*/
              sp = readRegister(SP) - 2;
              writeRegister(SP, sp);

//              System.out.println("CALLA INDX: R" + dstRegister);
              dst = readRegister(dstRegister);

              /* what happens if wrapping here??? */
              /* read the index which is from -15 bit - +15 bit. - so extend sign to 20-bit */
              int index = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
              index = convertTwoComplement16(index);

//              System.out.println("CALLA INDX: Reg = " + Utils.hex20(dst) + " INDX: " +  index);

              dst += index;
              dst &= 0xfffff;

//              System.out.println("CALLA INDX => " + Utils.hex20(dst));
              dst = currentSegment.read(dst, AccessMode.WORD20, AccessType.READ);
//              System.out.println("CALLA Read from INDX => " + Utils.hex20(dst));
              cycles += 5;
              pc += 2;
//              System.exit(0);
              break;
          case CALLA_IMM:
              sp = readRegister(SP) - 2;
              writeRegister(SP, sp);

              dst = (dstRegister << 16) | currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
              pc += 2;
              cycles += 5;
              break;
          case CALLA_IND:
              sp = readRegister(SP) - 2;
              writeRegister(SP, sp);

              dstAddress = readRegister(dstRegister);
              
              dst = currentSegment.read(dstAddress, AccessMode.WORD20, AccessType.READ);
              cycles += 5;
              break;
          case CALLA_ABS:
              sp = readRegister(SP) - 2;
              writeRegister(SP, sp);

              /* read the address of where the address to call is */
              dst = (dstRegister << 16) | currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
              dst = currentSegment.read(dst, AccessMode.WORD20, AccessType.READ);
              pc += 2;
              cycles += 7;
              break;
          default:
        	  AccessMode type = AccessMode.WORD;
        	  int size = 2;
        	  sp = readRegister(SP);
        	  /* check for PUSHM... POPM... */
        	  switch(op & 0x1f00) {
        	  case PUSHM_A:
        		  type = AccessMode.WORD20;
        		  size = 4;
        		  cycles += 2;
        	  case PUSHM_W:
        		  int n = 1 + ((instruction >> 4) & 0x0f);
        		  int regNo = instruction & 0x0f;

        		  //                  System.out.println("PUSHM " + (type == AccessMode.WORD20 ? "A" : "W") +
        		  //                          " n: " + n + " " + regNo + " at " + Utils.hex16(pcBefore));

        		  /* decrease stack pointer and write n times */
        		  for(int i = 0; i < n; i++) {
        			  sp -= size;
        			  cycles += 2;
        			  currentSegment.write(sp, this.reg[regNo], type);
        			  /*System.out.println("Saved reg: " + (regNo) + " was " + reg[regNo]);*/
        			  regNo--;

        			  /* what happens if regNo is wrapped ??? */
        			  if (regNo < 0) regNo = 15;
        		  }
        		  writeRegister(SP, sp);
        		  break;
        	  case POPM_A:
        		  type = AccessMode.WORD20;
        		  size = 4;
        		  cycles += 2;
        	  case POPM_W:
        		  n = 1 + ((instruction >> 4) & 0x0f);
        		  regNo = instruction & 0x0f;
        		  /* read and increase stack pointer n times */

        		  for(int i = 0; i < n; i++) {
        			  cycles += 2;
        			  this.reg[regNo] = currentSegment.read(sp, type, AccessType.READ);
        			  regNo++;
        			  //                      System.out.println("Restored reg: " + (regNo - 1) + " to " + reg[regNo - 1]);
        			  sp += size;
        			  /* what happens if regNo is wrapped ??? */
        			  if (regNo > 15) regNo = 0;
        		  }

        		  writeRegister(SP, sp);
        		  break;
        	  default:
            	  System.out.println("CALLA/PUSH/POP: mode not implemented");
            	  throw new EmulationException("CALLA: mode not implemented "
            			  + Utils.hex16(instruction) + " => " + Utils.hex16(op));
              }
          }
          // store current PC on stack. (current PC points to next instr.)
          /* store 20 bits on stack (costs two words) */
          if (dst != -1) {
              currentSegment.write(sp, (pc >> 16) & 0xf, AccessMode.WORD);
              sp = sp - 2;
              currentSegment.write(sp, pc & 0xffff, AccessMode.WORD);
              writeRegister(SP, sp);
              writeRegister(PC, dst);
              
              if (profiler != null) {
                  profileCall(dst, pc);
              }
          }
      } else {
          // Address mode of destination...
          int ad = (instruction >> 4) & 3;
          int nxtCarry = 0;
          op = instruction & 0xff80;
          if (op == PUSH || op == CALL) {
              // The PUSH and CALL operations increase the SP before 
              // address resolution!
              // store on stack - always move 2 steps (W) even if B./
              sp = readRegister(SP) - 2;
              writeRegister(SP, sp);
          }

          if ((dstRegister == CG1 && ad > AM_INDEX) || dstRegister == CG2) {
              dstRegMode = true;
              cycles++;
          } else {
              switch(ad) {
              // Operand in register!
              case AM_REG:
                  dstRegMode = true;
                  cycles++;
                  break;
              case AM_INDEX:
                  // TODO: needs to handle if SR is used!
                  rval = readRegisterCG(dstRegister, ad);

                  if (extWord != 0) {
                    /* MSP430X Instruction With Indexed Mode */
                    /*
                     * TODO XXX There are MSP430X instructions without extension
                     * words!
                     */
                    /*
                     * "The operand address is the sum of the 20-bit CPU register
                     * content and the 20-bit index."
                     */
                    dstAddress = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
                    dstAddress += extDst;
                    dstAddress += rval;
                    dstAddress &= 0xfffff;
                  } else {
                    if (rval <= 0xffff) {
                      /* Indexed Mode in Lower 64-KB Memory */
                      /*
                       * ".. the calculated memory address bits 19:16 are cleared
                       * after the addition of the CPU register Rn and the signed
                       * 16-bit index."
                       */
                      dstAddress = convertTwoComplement16(currentSegment.read(pc, AccessMode.WORD, AccessType.READ));
                      dstAddress += rval;
                      dstAddress &= 0xffff;
                    } else {
                      /* MSP430 Instruction With Indexed Mode in Upper Memory */
                      /*
                       * "The operand may be located in memory in the range Rn +-32
                       * KB, because the index, X, is a signed 16-bit value"
                       */
                      dstAddress = convertTwoComplement16(currentSegment.read(pc, AccessMode.WORD, AccessType.READ));
                      dstAddress += rval;
                      dstAddress &= 0xfffff;
                    }
                  }

                  // When is PC incremented - assuming immediately after "read"?
                  pc += 2;
                  writeRegister(PC, pc);
                  cycles += 4;
                  break;
                  // Indirect register
              case AM_IND_REG:
                  dstAddress = readRegister(dstRegister)  + extDst;

                  cycles += 3;
                  break;
                  // Bugfix suggested by Matt Thompson
              case AM_IND_AUTOINC:
                  if (dstRegister == PC) {
                      dstAddress = pc;

                      dst = currentSegment.read(dstAddress, mode != AccessMode.BYTE ? AccessMode.WORD : AccessMode.BYTE, AccessType.READ);
                      
                      dst += extDst;
                      pc += 2;
                      writeRegister(PC, pc);
                  } else {
                      dstAddress = readRegister(dstRegister) + extDst;
                      writeRegister(dstRegister, dstAddress + mode.bytes); // XXX (word ? 2 : 1));
                  }
                  cycles += 3;
                  break;
              }
          }

          // Perform the read
          if (dstRegMode) {
              dst = readRegisterCG(dstRegister, ad);

	      dst &= mode.mask;
	      
              /* set the repeat here! */
	      if (repeatsInDstReg) {
		  repeats = 1 + readRegister(ext3_0);
	      } else {
		  repeats = 1 + ext3_0;
	      }
              zeroCarry = (extWord & EXTWORD_ZC) > 0;

//              if (repeats > 1) {
//                  System.out.println("*** Repeat " + repeats + " ZeroCarry: " + zeroCarry);
//              }
          } else if (dst == -1) {
              dst = currentSegment.read(dstAddress, mode, AccessType.READ);
          }
          
          /* TODO: test add the loop here! */
          while(repeats-- > 0) {
              sr = readRegister(SR);
              /* always clear carry before repeat */
              if (repeats >= 0) {
                  if (zeroCarry) {
                      sr = sr & ~CARRY;
                      //System.out.println("ZC => Cleared carry...");
                  }
                  //System.out.println("*** Repeat: " + repeats);
              }
              switch(op) {
              case RRC:
                  nxtCarry = (dst & 1) > 0 ? CARRY : 0;
                  dst = dst >> 1;
                  dst |= (sr & CARRY) > 0 ? mode.msb : 0;

                  // Indicate write to memory!!
                  write = true;
                  // Set the next carry!
                  writeRegister(SR, (sr & ~(CARRY | OVERFLOW)) | nxtCarry);
                  break;
              case SWPB:
                  int tmp = dst;
                  dst = ((tmp >> 8) & 0xff) + ((tmp << 8) & 0xff00);
                  write = true;
                  updateStatus = false;
                  break;
              case RRA:
                  nxtCarry = (dst & 1) > 0 ? CARRY : 0;
                  dst = (dst & mode.msb) | dst >> 1;
                  write = true;
                  writeRegister(SR, (sr & ~(CARRY | OVERFLOW)) | nxtCarry);
                  break;
              case SXT:
                  // Extend Sign (bit 8-15 => same as bit 7)
                  dst = (dst & 0x80) > 0 ? dst | 0xfff00 : dst & 0x7f;
                  write = true;
                  sr = sr & ~(CARRY | OVERFLOW);
                  if (dst != 0) {
                      sr |= CARRY;
                  }
                  writeRegister(SR, sr);
                  break;
              case PUSH:
                  if (mode == AccessMode.WORD20) {
                      sp = readRegister(SP) - 2;
                      writeRegister(SP, sp);
                  }
                  currentSegment.write(sp, dst, mode);

                  /* if REG or INDIRECT AUTOINC then add 2 cycles, otherwise 1 */
                  cycles += (ad == AM_REG || ad == AM_IND_AUTOINC) ? 2 : 1;
                  write = false;
                  updateStatus = false;
                  break;
              case CALL:
                  // store current PC on stack. (current PC points to next instr.)
                  pc = readRegister(PC);

                  //	memory[sp] = pc & 0xff;
                  //	memory[sp + 1] = pc >> 8;
                  currentSegment.write(sp, pc, AccessMode.WORD);
                  writeRegister(PC, dst);

                  /* Additional cycles: REG => 3, AM_IND_AUTO => 2, other => 1 */
                  cycles += (ad == AM_REG) ? 3 : (ad == AM_IND_AUTOINC) ? 2 : 1;

                  /* profiler will be called during calls */
                  if (profiler != null) {
                      profileCall(dst, pc);
                  }

                  write = false;
                  updateStatus = false;
                  break;
              case RETI:
                  // Put Top of stack to Status DstRegister (TOS -> SR)
                  servicedInterrupt = -1; /* needed before write to SR!!! */
                  sp = readRegister(SP);
                  sr = currentSegment.read(sp, AccessMode.WORD, AccessType.READ);
                  writeRegister(SR, sr & 0x0fff);
                  sp = sp + 2;
                  //	writeRegister(SR, memory[sp++] + (memory[sp++] << 8));
                  // TOS -> PC
                  //	writeRegister(PC, memory[sp++] + (memory[sp++] << 8));
                  writeRegister(PC, currentSegment.read(sp, AccessMode.WORD, AccessType.READ) | (sr & 0xf000) << 4);
                  sp = sp + 2;
                  writeRegister(SP, sp);
                  write = false;
                  updateStatus = false;

                  cycles += 4;

                  if (debugInterrupts) {
                      System.out.println("### RETI at " + pc + " => " + reg[PC] +
                              " SP after: " + reg[SP]);
                  }        
                  if (profiler != null) {
                      profiler.profileRETI(cycles);
                  }

                  // This assumes that all interrupts will get back using RETI!
                  handlePendingInterrupts();

                  break;
              default:
                  System.out.println("Error: Not implemented instruction:" +
                          Utils.hex16(instruction));
              }
              if (repeats > 0) {
                  dst &= mode.mask;
              }
          }
      }
    }
    break;
    // Jump instructions
    case 2:
    case 3:
      // 10 bits for address for these => 0x00fc => remove 2 bits
      int jmpOffset = instruction & 0x3ff;
      jmpOffset = (jmpOffset & 0x200) == 0 ?
	2 * jmpOffset : -(2 * (0x200 - (jmpOffset & 0x1ff)));
      boolean jump = false;

      // All jump takes two cycles
      cycles += 2;
      sr = readRegister(SR);
      switch(instruction & 0xfc00) {
      case JNE:
	jump = (sr & ZERO) == 0;
	break;
      case JEQ:
	jump = (sr & ZERO) > 0;
	break;
      case JNC:
	jump = (sr & CARRY) == 0;
	break;
      case JC:
	jump = (sr & CARRY) > 0;
	break;
      case JN:
	jump = (sr & NEGATIVE) > 0;
	break;
      case JGE:
	jump = (sr & NEGATIVE) > 0 == (sr & OVERFLOW) > 0;
	break;
      case JL:
	jump = (sr & NEGATIVE) > 0 != (sr & OVERFLOW) > 0;
	break;
      case JMP:
	jump = true;
	break;
      default:
        logw(WarningType.EMULATION_ERROR, "Not implemented instruction: #" + Utils.binary16(instruction));
      }
      // Perform the Jump
      if (jump) {
        writeRegister(PC, pc + jmpOffset);
      }
      updateStatus = false;
      break;
    default:
      // ---------------------------------------------------------------
      // Double operand instructions!
      // ---------------------------------------------------------------
      dstRegister = instruction & 0xf;
      int srcRegister = (instruction >> 8) & 0xf;
      int as = (instruction >> 4) & 3;

      // AD: 0 => register direct, 1 => register index, e.g. X(Rn)
      dstRegMode = ((instruction >> 7) & 1) == 0;
      dstAddress = -1;
      int srcAddress = -1;
      src = 0;

      // Some CGs should be handled as registry reads only...
      if ((srcRegister == CG1 && as > AM_INDEX) || srcRegister == CG2) {
        src = CREG_VALUES[srcRegister - 2][as];
        src &= mode.mask;
        cycles += dstRegMode ? 1 : 4;
      } else {
	switch(as) {
	  // Operand in register!
	case AM_REG:
	  // CG handled above!
	  src = readRegister(srcRegister);
	  src &= mode.mask;
	  cycles += dstRegMode ? 1 : 4;
	  /* add cycle if destination register = PC */
          if (dstRegister == PC) cycles++;
          
          if (dstRegMode) {
              /* possible to have repeat, etc... */
              /* TODO: decode the # also */
	      if (repeatsInDstReg) {
		  repeats = 1 + readRegister(ext3_0);
	      } else {
		  repeats = 1 + ext3_0;
	      }

              zeroCarry = (extWord & EXTWORD_ZC) > 0;
          }
          
	  break;
	case AM_INDEX: {
	  // Indexed if reg != PC & CG1/CG2 - will PC be incremented?
	  int sval = readRegisterCG(srcRegister, as);

	  if (extWord != 0) {
	    /* MSP430X Instruction With Indexed Mode */
	    /*
	     * TODO XXX There are MSP430X instructions without extension
	     * words!
	     */
	    /*
	     * "The operand address is the sum of the 20-bit CPU register
	     * content and the 20-bit index."
	     */
	    srcAddress = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
	    srcAddress += extSrc;
	    srcAddress += sval;
	    srcAddress &= 0xfffff;
	  } else {
	    if (sval <= 0xffff) {
	      /* Indexed Mode in Lower 64-KB Memory */
	      /*
	       * ".. the calculated memory address bits 19:16 are cleared
	       * after the addition of the CPU register Rn and the signed
	       * 16-bit index."
	       */
	      srcAddress = convertTwoComplement16(currentSegment.read(pc, AccessMode.WORD, AccessType.READ));
	      srcAddress += sval;
	      srcAddress &= 0xffff;
	    } else {
	      /* MSP430 Instruction With Indexed Mode in Upper Memory */
	      /*
	       * "The operand may be located in memory in the range Rn +-32
	       * KB, because the index, X, is a signed 16-bit value"
	       */
	      srcAddress = convertTwoComplement16(currentSegment.read(pc, AccessMode.WORD, AccessType.READ));
	      srcAddress += sval;
	      srcAddress &= 0xfffff;
	    }
	  }

	  pc += 2;
	  writeRegister(PC, pc);
	  cycles += dstRegMode ? 3 : 6;
	  break;
	}
	  // Indirect register
	case AM_IND_REG:
	  srcAddress = readRegister(srcRegister);
	  cycles += dstRegMode ? 2 : 5;
	  break;
	case AM_IND_AUTOINC:
		if (srcRegister == PC) {
			/* PC is always handled as word */
			if (mode == AccessMode.BYTE) {
				src = currentSegment.read(pc, AccessMode.BYTE,
						AccessType.READ);
			} else {
				src = currentSegment.read(pc, AccessMode.WORD,
						AccessType.READ);
			}
			src += extSrc;

			pc += 2;
			writeRegister(PC, pc);
			cycles += dstRegMode ? 2 : 5;
		} else {
			srcAddress = readRegister(srcRegister);
			incRegister(srcRegister, mode.bytes);
			cycles += dstRegMode ? 2 : 5;
		}

		/* If destination register is PC another cycle is consumed */
		if (dstRegister == PC) {
			cycles++;
		}
		break;
	}
      }

      // Perform the read of destination!
      if (dstRegMode) {
        if (op != MOV) {
          dst = readRegister(dstRegister);
          dst &= mode.mask;
        }
      } else {
        // PC Could have changed above!
        pc = readRegister(PC);

        if (dstRegister == 2) {
          /* absolute mode */
          dstAddress = currentSegment.read(pc, AccessMode.WORD, AccessType.READ); //memory[pc] + (memory[pc + 1] << 8);
          dstAddress += extDst;
        } else {
          // CG here - probably not!???
          rval = readRegister(dstRegister);

          if (extWord != 0) {
            /* MSP430X Instruction With Indexed Mode */
            /*
             * TODO XXX There are MSP430X instructions without extension
             * words!
             */
            /*
             * "The operand address is the sum of the 20-bit CPU register
             * content and the 20-bit index."
             */
            dstAddress = currentSegment.read(pc, AccessMode.WORD, AccessType.READ);
            dstAddress += extDst;
            dstAddress += rval;
            dstAddress &= 0xfffff;
          } else {
            if (rval <= 0xffff) {
              /* Indexed Mode in Lower 64-KB Memory */
              /*
               * ".. the calculated memory address bits 19:16 are cleared
               * after the addition of the CPU register Rn and the signed
               * 16-bit index."
               */
              dstAddress = convertTwoComplement16(currentSegment.read(pc, AccessMode.WORD, AccessType.READ));
              dstAddress += rval;
              dstAddress &= 0xffff;
            } else {
              /* MSP430 Instruction With Indexed Mode in Upper Memory */
              /*
               * "The operand may be located in memory in the range Rn +-32
               * KB, because the index, X, is a signed 16-bit value"
               */
              dstAddress = convertTwoComplement16(currentSegment.read(pc, AccessMode.WORD, AccessType.READ));
              dstAddress += rval;
              dstAddress &= 0xfffff;
            }
          }
        }

        if (op != MOV) {
        	dst = currentSegment.read(dstAddress, mode, AccessType.READ);
        }
        pc += 2;
        incRegister(PC, 2);
      }

      // **** Perform the read...
      if (srcAddress != -1) {

//        if (srcAddress  > 0xffff) {
//            System.out.println("SrcAddress is: " + Utils.hex20(srcAddress));
//        }
//	srcAddress = srcAddress & 0xffff;
        src = currentSegment.read(srcAddress, mode, AccessType.READ);

//	src = currentSegment.read(srcAddress, word ? AccessMode.WORD : AccessMode.BYTE, AccessType.READ);

	// 	  if (debug) {
	// 	    System.out.println("Reading from " + getAddressAsString(srcAddress) +
	// 			       " => " + src);
	// 	  }
      }

      /* TODO: test add the loop here! */
      while(repeats-- > 0) {
          sr = readRegister(SR);
          if (repeats >= 0) {
              if (zeroCarry) {
                  sr = sr & ~CARRY;
                  //System.out.println("ZC => Cleared carry...");
              }
              //System.out.println("*** Repeat: " + repeats);
          }

          int tmp = 0;
          int tmpAdd = 0;
          switch (op) {
          case MOV: // MOV
              dst = src;
              write = true;
              updateStatus = false;

              if (instruction == RETURN && profiler != null) {
                  profiler.profileReturn(cpuCycles);
              }

              break;
              // FIX THIS!!! - make SUB a separate operation so that
              // it is clear that overflow flag is correct...
          case SUB:
              // Carry always 1 with SUB
              tmpAdd = 1;
          case SUBC:
              // Both sub and subc does one complement (not) + 1 (or carry)
              src = (src ^ 0xffff) & 0xffff;
          case ADDC: // ADDC
              if (op == ADDC || op == SUBC)
                  tmpAdd = ((sr & CARRY) > 0) ? 1 : 0;
          case ADD: // ADD
              // Tmp gives zero if same sign! if sign is different after -> overf.
              sr &= ~(OVERFLOW | CARRY);
	      int b = word ? 0x8000 : (wordx20 ? 0x80000 : 0x80);
              tmp = (src ^ dst) & b;
              // Includes carry if carry should be added...

              dst = dst + src + tmpAdd;
	      int b2 = word ? 0xffff : (wordx20 ? 0xfffff : 0xff);
              if (dst > b2) {
                  sr |= CARRY;
              }
              // If tmp == 0 and currenly not the same sign for src & dst
              if (tmp == 0 && ((src ^ dst) & b) != 0) {
                  sr |= OVERFLOW;
                  // 	    System.out.println("OVERFLOW - ADD/SUB " + Utils.hex16(src)
                  // 			       + " + " + Utils.hex16(tmpDst));
              }

              // 	  System.out.println(Utils.hex16(dst) + " [SR=" +
              // 			     Utils.hex16(reg[SR]) + "]");
              writeRegister(SR, sr);
              write = true;
              break;
          case CMP: // CMP
              // Set CARRY if A >= B, and it's clear if A < B
	      b = mode.msb;
              sr = (sr & ~(CARRY | OVERFLOW)) | (dst >= src ? CARRY : 0);

              tmp = (dst - src);

              if (((src ^ tmp) & b) == 0 && (((src ^ dst) & b) != 0)) {
                  sr |= OVERFLOW;
              }
              writeRegister(SR, sr);
              // Must set dst to the result to set the rest of the status register
              dst = tmp;
              break;
          case DADD: // DADD
              if (DEBUG)
                  log("DADD: Decimal add executed - result error!!!");
              // Decimal add... this is wrong... each nibble is 0-9...
              // So this has to be reimplemented...
              dst = dst + src + ((sr & CARRY) > 0 ? 1 : 0);
              write = true;
              break;
          case BIT: // BIT
              dst = src & dst;
              // Clear overflow and carry!
              sr = sr & ~(CARRY | OVERFLOW);
              // Set carry if result is non-zero!
              if (dst != 0) {
                  sr |= CARRY;
              }
              writeRegister(SR, sr);
              break;
          case BIC: // BIC
              // No status reg change
              // 	  System.out.println("BIC: =>" + Utils.hex16(dstAddress) + " => "
              // 			     + Utils.hex16(dst) + " AS: " + as +
              // 			     " sReg: " + srcRegister + " => " + src +
              // 			     " dReg: " + dstRegister + " => " + dst);
              dst = (~src) & dst;

              write = true;
              updateStatus = false;
              break;
          case BIS: // BIS
              dst = src | dst;
              write = true;
              updateStatus = false;
              break;
          case XOR: // XOR
              sr = sr & ~(CARRY | OVERFLOW);
	      b = mode.msb; //word ? 0x8000 : (wordx20 ? 0x80000 : 0x80);
              if ((src & b) != 0 && (dst & b) != 0) {
                  sr |= OVERFLOW;
              }
              dst = src ^ dst;
              if (dst != 0) {
                  sr |= CARRY;
              }
              write = true;
              writeRegister(SR, sr);
              break;
          case AND: // AND
              sr = sr & ~(CARRY | OVERFLOW);
              dst = src & dst;
              if (dst != 0) {
                  sr |= CARRY;
              }
              write = true;
              writeRegister(SR, sr);
              break;
          default:
              String address = getAddressAsString(pc);
              logw(WarningType.EMULATION_ERROR, 
                      "DoubleOperand not implemented: op = " + Integer.toHexString(op) + " at " + address);
              if (EXCEPTION_ON_BAD_OPERATION) {
                  EmulationException ex = new EmulationException("Bad operation: $" + Integer.toHexString(op) + " at $" + address);
                  ex.initCause(new Throwable("" + pc));
                  throw ex;
              }
          } /* after switch(op) */
          /* If we have the same register as dst and src then copy here to get input
           * in next loop
           */
          if (repeats > 0 && srcRegister == dstRegister) {
              src = dst;
              src &= mode.mask;
              dst &= mode.mask;
          }
      }
    }
    
    /* Processing after each instruction */
    dst &= mode.mask;
    if (write) {
      if (dstRegMode) {
	writeRegister(dstRegister, dst);
      } else {
        currentSegment.write(dstAddress, dst, mode);
      }
    }
    if (updateStatus) {
      // Update the Zero and Negative status!
      // Carry and overflow must be set separately!
      sr = readRegister(SR);
      sr = (sr & ~(ZERO | NEGATIVE)) |
	((dst == 0) ? ZERO : 0) | ((dst & mode.msb) > 0 ? NEGATIVE : 0);
      writeRegister(SR, sr);
    }

    //System.out.println("CYCLES AFTER: " + cycles);

    // -------------------------------------------------------------------
    // Event processing (when CPU is awake)
    // -------------------------------------------------------------------
    while (cycles >= nextEventCycles) {
      executeEvents();
    }
    
    cpuCycles += cycles - startCycles;
    
    /* return the address that was executed */
    return pcBefore;
  }
  
  private static int convertTwoComplement16(int index) {
	  if(index > 0x8000) {
		  return -(0x10000 - index);
	  }
	  return index;
  }
  private static int convertTwoComplement20(int index) {
	  if(index > 0x80000) {
		  return -(0x100000 - index);
	  }
	  return index;
  }

  public int getModeMax() {
    return MODE_MAX;
  }

  MapEntry getFunction(MapTable map, int address) {
    MapEntry function = new MapEntry(MapEntry.TYPE.function, address, 0,
        "fkn at $" + getAddressAsString(address), null, true);
    map.setEntry(function);
    return function;
  }

  public Memory getMemory() {
      return currentSegment;
  }

  public int getPC() {
    return reg[PC];
  }

  public int getSR() {
      return reg[SR];
  }

  public int getSP() {
      return reg[SP];
  }

  public int getRegister(int register) {
      return reg[register];
  }

  public String getAddressAsString(int addr) {
      return config.getAddressAsString(addr);
  }

  public int getConfiguration(int parameter) {
      return 0;
  }

  public String info() {
      StringBuilder buf = new StringBuilder();
      buf.append(" Mode: " + getModeName(getMode())
              + "  ACLK: " + aclkFrq + " Hz  SMCLK: " + smclkFrq + " Hz\n"
              + " Cycles: " + cycles + "  CPU Cycles: " + cpuCycles
              + "  Time: " + (long)getTimeMillis() + " msec\n");
      buf.append(" Interrupt enabled: " + interruptsEnabled +  " HighestInterrupt: " + interruptMax);
      for (int i = 0; i < MAX_INTERRUPT; i++) {
          int value = currentSegment.get(0xfffe - i * 2, AccessMode.WORD); 
          if (value != 0xffff) {
              buf.append(" Vector " + (MAX_INTERRUPT - i) + " at $"
                      + Utils.hex16(0xfffe - i * 2) + " -> $"
                      + Utils.hex16(value) + "\n");
          }
      }
      return buf.toString();
  }

  public int getAclkFrq() {
    return aclkFrq;
  }

  public int getSmclkFrq() {
    return smclkFrq;
  }

}
