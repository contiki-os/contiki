/*
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

import java.util.ArrayList;
import java.util.Map;

import org.apache.log4j.Logger;

import org.contikios.cooja.Mote;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryLayout;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.util.MapEntry;

public class MspMoteMemory implements MemoryInterface {
  private static Logger logger = Logger.getLogger(MspMoteMemory.class);
  private final ArrayList<MapEntry> mapEntries;

  private final MSP430 cpu;

  public MspMoteMemory(Mote mote, MapEntry[] allEntries, MSP430 cpu) {
    this.mapEntries = new ArrayList<MapEntry>();

    for (MapEntry entry: allEntries) {
      if (entry.getType() == MapEntry.TYPE.variable) {
        mapEntries.add(entry);
      }
    }

    this.cpu = cpu;
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
  public byte[] getMemorySegment(long addr, int size) throws MoteMemoryException {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public void setMemorySegment(long addr, byte[] data) throws MoteMemoryException {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public void clearMemory() {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public long getStartAddr() {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public Map<String, Symbol> getSymbolMap() {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public MemoryLayout getLayout() {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public boolean addSegmentMonitor(SegmentMonitor.EventType flag, long address, int size, SegmentMonitor monitor) {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  @Override
  public boolean removeSegmentMonitor(long address, int size, SegmentMonitor monitor) {
    throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
  }

  public int parseInt(byte[] memorySegment) {
    if (memorySegment.length < 2) {
      return -1;
    }
    
    int retVal = 0;
    int pos = 0;
    retVal += ((memorySegment[pos++] & 0xFF)) << 8;
    retVal += ((memorySegment[pos++] & 0xFF)) << 0;

    return Integer.reverseBytes(retVal) >> 16;
  }
}
