/*
 * Copyright (c) 2013, Enrico Joerns
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
 */

package org.contikios.cooja.mote.memory;

import java.util.Arrays;
import java.util.Map;

/**
 * A memory that is backed by an array.
 *
 * @author Enrico Joerns
 */
public class ArrayMemory implements MemoryInterface {

  private final byte memory[];
  private final long startAddress;
  private final MemoryLayout layout;
  private final boolean readonly;
  private final Map<String, Symbol> symbols;// XXX Allow to set symbols

  public ArrayMemory(long address, int size, MemoryLayout layout, Map<String, Symbol> symbols) {
    this(address, layout, new byte[size], symbols);
  }

  public ArrayMemory(long address, MemoryLayout layout, byte[] memory, Map<String, Symbol> symbols) {
    this(address, layout, memory, false, symbols);
  }

  public ArrayMemory(long address, MemoryLayout layout, byte[] memory, boolean readonly, Map<String, Symbol> symbols) {
    this.startAddress = address;
    this.layout = layout;
    this.memory = memory;
    this.readonly = readonly;
    this.symbols = symbols;
  }

  @Override
  public byte[] getMemory() {
    return memory;
  }

  /**
   * XXX Should addr be the relative or the absolute address of this section?
   * @param addr
   * @param size
   * @return
   * @throws org.contikios.cooja.mote.memory.MemoryInterface.MoteMemoryException 
   */
  @Override
  public byte[] getMemorySegment(long addr, int size) throws MoteMemoryException {
    byte[] ret = new byte[size];
    System.arraycopy(memory, (int) (addr - startAddress), ret, 0, size);
    return ret;
  }

  @Override
  public void setMemorySegment(long addr, byte[] data) throws MoteMemoryException {
    if (readonly) {
      throw new MoteMemoryException("Invalid write access for readonly memory");
    }
    System.arraycopy(data, 0, memory, (int) (addr - startAddress), data.length);
  }

  @Override
  public void clearMemory() {
    Arrays.fill(memory, (byte) 0x00);
  }

  @Override
  public long getStartAddr() {
    return startAddress;
  }

  @Override
  public int getTotalSize() {
    return memory.length;
  }

  @Override
  public Map<String, Symbol> getSymbolMap() {
    return symbols;
  }

  @Override
  public MemoryLayout getLayout() {
    return layout;
  }

  @Override
  public boolean addSegmentMonitor(SegmentMonitor.EventType flag, long address, int size, SegmentMonitor monitor) {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public boolean removeSegmentMonitor(long address, int size, SegmentMonitor monitor) {
    throw new UnsupportedOperationException("Not supported yet.");
  }
  
}
