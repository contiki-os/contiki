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
 * OperatingModeStatistics
 *
 * Author  : Joakim Eriksson
 * Created : 17 jan 2008
 * Updated : $Date$
 *           $Revision$
 */
package se.sics.mspsim.util;

import java.io.PrintStream;
import java.util.ArrayList;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.OperatingModeListener;

/**
 * @author Joakim
 *
 */
public class OperatingModeStatistics {

  public static final int OP_NORMAL = 0;
  public static final int OP_INVERT = 1;

  private final MSP430Core cpu;
  private ArrayList<StatEntry> statistics = new ArrayList<StatEntry>();

  public OperatingModeStatistics(MSP430Core cpu) {
    this.cpu = cpu;
  }

  private synchronized StatEntry getStatEntry(String name) {
      for (StatEntry entry : statistics) {
          if (name.equalsIgnoreCase(entry.chip.getID()) ||
                  name.equalsIgnoreCase(entry.chip.getName())) {
              return entry;
          }
      }
      return null;
  }

  public Chip getChip(String chipName) {
    StatEntry entry = getStatEntry(chipName);
    return entry == null ? null : entry.chip;
  }

  public synchronized Chip[] getChips() {
    Chip[] chips = new Chip[statistics.size()];
    int index = 0;
    for (StatEntry entry : statistics) {
      chips[index++] = entry.chip;
    }
    return chips;
  }

  public void addMonitor(Chip chip) {
    StatEntry entry = new StatEntry(chip);
    synchronized (this) {
        statistics.add(entry);
    }
  }

  public synchronized void printStat() {
    for (StatEntry entry : statistics) {
      entry.printStat(System.out);
    }
  }

  public DataSource getDataSource(String chip, int mode) {
    return getDataSource(chip, mode, OP_NORMAL);
  }

  public DataSource getDataSource(String chip, String modeStr) {
    StatEntry se = getStatEntry(chip);
    if (se != null) {
      int mode = se.chip.getModeByName(modeStr);
      if (mode != -1) { 
        return new StatDataSource(se, mode, OP_NORMAL);
      }
    }
    return null;
  }

  
  public DataSource getDataSource(String chip, int mode, int operation) {
    StatEntry se = getStatEntry(chip);
    if (se != null) {
      return new StatDataSource(se, mode, operation);
    }
    return null;
  }

  public MultiDataSource getMultiDataSource(String chip) {
    StatEntry se = getStatEntry(chip);
    if (se != null) {
      return new StatMultiDataSource(se);
    }
    return null;  
  }

  private class StatDataSource implements DataSource {

    private final StatEntry entry;
    private final int mode;
    private final int operation;
    private long lastCycles;
    private long lastValue;
    
    public StatDataSource(StatEntry entry, int mode, int operation) {
      this.entry = entry;
      this.mode = mode;
      this.operation = operation;
      this.lastCycles = cpu.cycles;
      this.lastValue = entry.getValue(mode, this.lastCycles);
    }

    // returns percentage since last call...
    public double getDoubleValue() {
      long diff = cpu.cycles - lastCycles;
      if (diff == 0) return 0;
      long val = entry.getValue(mode, cpu.cycles);
      long valDiff = val - lastValue;
      lastValue = val;
      lastCycles = cpu.cycles;
      if (operation == OP_INVERT) {
        return (100.0 - (100.0 * valDiff) / diff); 
      }
      return (100.0 * valDiff) / diff;
    }

    public int getValue() {
      return (int) getDoubleValue();
    }
  }

  private class StatMultiDataSource implements MultiDataSource {

    private final StatEntry entry;
    private long[] lastValue;
    private long[] lastCycles;
    
    public StatMultiDataSource(StatEntry entry) {
      this.entry = entry;
      this.lastValue = new long[entry.elapsed.length];
      this.lastCycles = new long[entry.elapsed.length];
      long cycles = cpu.cycles;
      for (int i = 0, n = this.lastValue.length; i < n; i++) {
        this.lastValue[i] = entry.getValue(i, cycles);
        this.lastCycles[i] = cycles;
      }
    }
    
    public int getModeMax() {
      return entry.chip.getModeMax();
    }

    // returns percentage since last call...
    public int getValue(int mode) {
      return (int) getDoubleValue(mode);
    }
    
    public double getDoubleValue(int mode) {
      long diff = cpu.cycles - lastCycles[mode];
      if (diff == 0) return 0;

      long val = entry.getValue(mode, cpu.cycles);
      long valDiff = (val - lastValue[mode]);
      lastValue[mode] = val;
      lastCycles[mode] = cpu.cycles;
      return (100.0 * valDiff) / diff;
    }

  }


  private class StatEntry implements OperatingModeListener {
    final Chip chip;
    long startTime;
    int mode;
    long[] elapsed;

    StatEntry(Chip chip) {
      this.chip = chip;
      this.elapsed = new long[chip.getModeMax() + 1];
      this.mode = chip.getMode();
      this.startTime = cpu.cycles;
      chip.addOperatingModeListener(this);
    }

    long getValue(int mode, long cycles) {
      if (mode == this.mode) {
        return elapsed[mode] + (cycles - startTime);
      }
      return elapsed[mode];
    }

    public void modeChanged(Chip source, int mode) {
      this.elapsed[this.mode] += cpu.cycles - startTime;
      this.mode = mode;
      this.startTime = cpu.cycles;
    }

    void printStat(PrintStream out) {
      out.println("Stat for: " + chip.getID());
      for (int i = 0; i < elapsed.length; i++) {
        out.println("" + (i + 1) + " = " + elapsed[i]);
      }
    }
  }

}
