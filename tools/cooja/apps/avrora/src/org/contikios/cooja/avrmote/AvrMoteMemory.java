/*
 * Copyright (c) 2014, TU Braunschweig. All rights reserved.
 * Copyright (c) 2009, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package org.contikios.cooja.avrmote;


import org.apache.log4j.Logger;

import avrora.arch.avr.AVRProperties;
import avrora.core.SourceMapping;
import avrora.core.SourceMapping.Location;
import avrora.sim.AtmelInterpreter;
import avrora.sim.Simulator.Watch;
import avrora.sim.State;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor.EventType;
import org.contikios.cooja.mote.memory.MemoryLayout;
/**
 * @author Joakim Eriksson, Fredrik Osterlind, David Kopf, Enrico Jorns
 */
public class AvrMoteMemory implements MemoryInterface {
  private static Logger logger = Logger.getLogger(AvrMoteMemory.class);
  private static final boolean DEBUG = logger.isDebugEnabled();

  private final SourceMapping memoryMap;
  private final AVRProperties avrProperties;
  private final AtmelInterpreter interpreter;
  private final ArrayList<AvrByteMonitor> memoryMonitors = new ArrayList<>();
  private final MemoryLayout memLayout = new MemoryLayout(ByteOrder.LITTLE_ENDIAN, MemoryLayout.ARCH_8BIT, 2);

  private boolean coojaIsAccessingMemory;

    public AvrMoteMemory(SourceMapping map, AVRProperties avrProperties, AtmelInterpreter interpreter) {
        memoryMap = map;
        this.interpreter = interpreter;
        this.avrProperties = avrProperties;
    }

  @Override
  public int getTotalSize() {
    return avrProperties.sram_size;
  }

  @Override
  public byte[] getMemory() throws MoteMemoryException {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public byte[] getMemorySegment(long address, int size) throws MoteMemoryException {
    /*logger.info("getMemorySegment(" + String.format("0x%04x", address) +
     ", " + size + ")");*/
    if (!accessInRange(address, size)) {
      throw new MoteMemoryException(
              "Getting memory segment [0x%x,0x%x] failed: Out of range",
              address, address + size - 1);
    }

    /* XXX Unsure whether this is the appropriate method to use, as it
     * triggers memoryRead monitor. Right now I'm using a flag to indicate
     * that Cooja (as opposed to Contiki) read the memory, to avoid infinite
     * recursion. */
    coojaIsAccessingMemory = true;
    byte[] data = new byte[(int) size];
    for (int i = 0; i < size; i++) {
      data[i] = (byte) (interpreter.getDataByte((int) address + i) & 0xff);
    }
    coojaIsAccessingMemory = false;
    return data;
  }

  @Override
  public void setMemorySegment(long address, byte[] data) throws MoteMemoryException {
    if (!accessInRange(address, data.length)) {
      throw new MoteMemoryException(
              "Writing memory segment [0x%x,0x%x] failed: Out of range",
              address, address + data.length - 1);
    }

    /* XXX See comment in getMemorySegment. */
    coojaIsAccessingMemory = true;
    for (int i = 0; i < data.length; i++) {
      interpreter.writeDataByte((int) address + i, data[i]);
    }
    coojaIsAccessingMemory = false;
    if (DEBUG) {
      logger.debug(String.format(
              "Wrote memory segment [0x%x,0x%x]",
              address, address + data.length - 1));
    }
  }

  @Override
  public void clearMemory() {
    setMemorySegment(0L, new byte[avrProperties.sram_size]);
  }
  
  private boolean accessInRange(long address, int size) {
    return (address >= 0) && (address + size <= avrProperties.sram_size);
  }

  @Override
  public long getStartAddr() {
    return 0;// XXX
  }

  @Override
  public Map<String, Symbol> getSymbolMap() {
    // XXX do not fetch in function!
    Map<String, Symbol> symbols = new HashMap<>();
    for (Iterator<Location> iter = memoryMap.getIterator(); iter.hasNext();) {
      Location loc = iter.next();
      if (loc == null || (loc.section.equals(".text"))) {
        continue;
      }
      symbols.put(loc.name, new Symbol(Symbol.Type.VARIABLE, loc.name, loc.section, loc.vma_addr & 0x7fffff, -1));
    }
    return symbols;
  }


  @Override
  public MemoryLayout getLayout() {
    return memLayout;
  }

  class AvrByteMonitor extends Watch.Empty {

    /** start address to monitor */
    final long address;
    /** size to monitor */
    final int size;
    /** Segment monitor to notify */
    final SegmentMonitor mm;
    /** MonitorType we are listening to */
    final EventType flag;

    public AvrByteMonitor(long address, int size, SegmentMonitor mm, EventType flag) {
      this.address = address;
      this.size = size;
      this.mm = mm;
      this.flag = flag;
    }

    @Override
    public void fireAfterRead(State state, int data_addr, byte value) {
      if (flag == EventType.WRITE || coojaIsAccessingMemory) {
        return;
      }
      mm.memoryChanged(AvrMoteMemory.this, EventType.READ, data_addr);
    }

    @Override
    public void fireAfterWrite(State state, int data_addr, byte value) {
      if (flag == EventType.READ || coojaIsAccessingMemory) {
        return;
      }
      mm.memoryChanged(AvrMoteMemory.this, EventType.WRITE, data_addr);
    }
  }

  @Override
  public boolean addSegmentMonitor(EventType flag, long address, int size, SegmentMonitor mm) {
    AvrByteMonitor mon = new AvrByteMonitor(address, size, mm, flag);

    memoryMonitors.add(mon);
    /* logger.debug("Added AvrByteMonitor " + Integer.toString(mon.hashCode()) + " for addr " + mon.address + " size " + mon.size + " with watch" + mon.watch); */

    /* Add byte monitor (watch) for every byte in range */
    for (int idx = 0; idx < mon.size; idx++) {
      interpreter.getSimulator().insertWatch(mon, (int) mon.address + idx);
      /* logger.debug("Inserted watch " + Integer.toString(mon.watch.hashCode()) + " for " + (mon.address + idx)); */
    }
    return true;
  }

  @Override
  public boolean removeSegmentMonitor(long address, int size, SegmentMonitor mm) {
    for (AvrByteMonitor mcm : memoryMonitors) {
      if (mcm.mm != mm || mcm.address != address || mcm.size != size) {
        continue;
      }
      for (int idx = 0; idx < mcm.size; idx++) {
        interpreter.getSimulator().removeWatch(mcm, (int) mcm.address + idx);
        /* logger.debug("Removed watch " + Integer.toString(mcm.watch.hashCode()) + " for " + (mcm.address + idx)); */
      }
      memoryMonitors.remove(mcm);
      return true;
    }
    return false;
  }

}
