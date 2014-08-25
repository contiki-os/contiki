/*
 * Copyright (c) 2014, TU Braunschweig. All rights reserved.
 * Copyright (c) 2007, Swedish Institute of Computer Science. All rights
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

package org.contikios.cooja.mspmote;

import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import org.contikios.cooja.Mote;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor.EventType;
import org.contikios.cooja.mote.memory.MemoryLayout;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.Memory.AccessType;
import se.sics.mspsim.util.MapEntry;

public class MspMoteMemory implements MemoryInterface {
  private static Logger logger = Logger.getLogger(MspMoteMemory.class);
  private final ArrayList<MapEntry> mapEntries;
  private final MemoryLayout memLayout;

  private final MSP430 cpu;

  public MspMoteMemory(Mote mote, MapEntry[] allEntries, MSP430 cpu) {
    this.mapEntries = new ArrayList<MapEntry>();

    for (MapEntry entry: allEntries) {
      if (entry.getType() == MapEntry.TYPE.variable) {
        mapEntries.add(entry);
      }
    }

    this.cpu = cpu;
    memLayout = new MemoryLayout(ByteOrder.LITTLE_ENDIAN, MemoryLayout.ARCH_16BIT, 2);
  }

  @Override
  public int getTotalSize() {
    return cpu.memory.length;
  }

  @Override
  public byte[] getMemory() throws MoteMemoryException {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public byte[] getMemorySegment(long address, int size) {
    int[] memInts = new int[size];

    System.arraycopy(cpu.memory, (int) address, memInts, 0, size);

    /* Convert to byte array */
    byte[] memBytes = new byte[size];
    for (int i = 0; i < size; i++) {
      memBytes[i] = (byte) memInts[i];
    }

    return memBytes;
  }

  @Override
  public void setMemorySegment(long address, byte[] data) {
    /* Convert to int array */
    int[] memInts = new int[data.length];
    for (int i = 0; i < data.length; i++) {
      memInts[i] = data[i];
    }

    System.arraycopy(memInts, 0, cpu.memory, (int) address, data.length);
  }

  @Override
  public void clearMemory() {
    Arrays.fill(cpu.memory, 0);
  }

  @Override
  public long getStartAddr() {
    return 0;// XXXX
  }

  @Override
  public Map<String, Symbol> getSymbolMap() {
    Map<String, Symbol> vars = new HashMap<>();
    for (MapEntry entry : mapEntries) {
      if (entry.getType() != MapEntry.TYPE.variable) {
        continue;
      }
      vars.put(entry.getName(), new Symbol(
              Symbol.Type.VARIABLE,
              entry.getName(), 
              entry.getAddress(), 
              entry.getSize()));
    }
    return vars;
  }

  @Override
  public MemoryLayout getLayout() {
    return memLayout;
  }

  private final ArrayList<MemoryCPUMonitor> cpuMonitorArray = new ArrayList<>();

  class MemoryCPUMonitor extends se.sics.mspsim.core.MemoryMonitor.Adapter {

    public final SegmentMonitor mm;
    public final int address;
    public final int size;

    public MemoryCPUMonitor(SegmentMonitor mm, int address, int size) {
      this.mm = mm;
      this.address = address;
      this.size = size;
    }

    @Override
    public void notifyReadAfter(int address, AccessMode mode, AccessType type) {
      mm.memoryChanged(MspMoteMemory.this, EventType.READ, address);
    }

    @Override
    public void notifyWriteAfter(int dstAddress, int data, AccessMode mode) {
      mm.memoryChanged(MspMoteMemory.this, EventType.WRITE, dstAddress);
    }
  }

  @Override
  public boolean addSegmentMonitor(EventType type, long address, int size, SegmentMonitor mm) {
    MemoryCPUMonitor t = new MemoryCPUMonitor(mm, (int) address, size);
    cpuMonitorArray.add(t);

    for (int a = (int) address; a < address + size; a++) {
      cpu.addWatchPoint(a, t);
    }

    return true;
  }

  @Override
  public boolean removeSegmentMonitor(long address, int size, SegmentMonitor mm) {
    for (MemoryCPUMonitor mcm : cpuMonitorArray) {
      if (mcm.mm != mm || mcm.address != address || mcm.size != size) {
        continue;
      }
      for (int a = (int) address; a < (int) address + size; a++) {
        cpu.removeWatchPoint(a, mcm);
      }
      cpuMonitorArray.remove(mcm);
      return true;
    }
    return false;
  }
}
