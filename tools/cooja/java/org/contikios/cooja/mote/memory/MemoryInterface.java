/*
 * Copyright (c) 2014, TU Braunschweig
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

import java.util.Map;

/**
 * A minimal interface to a motes memory.
 * 
 * Allows reading and writing memory, obtaining symbol information
 * and provides some basic inforation about size, layout, etc.
 *
 * @author Enrico Jorns
 */
public interface MemoryInterface {

  /**
   * Represents a symbol in memory (variable / function)
   */
  public static class Symbol {

    public final Type type;
    public final String name;
    public final String section;
    public final long addr;
    public final int size;

    public enum Type {

      VARIABLE,
      FUNCTION
    }

    public Symbol(Type type, String name, String section, long addr, int size) {
      this.type = type;
      this.name = name;
      this.section = section;
      this.addr = addr;
      this.size = size;
    }

    public Symbol(Type type, String name, long addr, int size) {
      this(type, name, null, addr, size);
    }

    @Override
    public String toString() {
      return new StringBuilder("Symbol(").append(type == null ? "N/A" : type.toString())
              .append(") '").append(name)
              .append("' in '").append(section == null ? "N/A" : section)
              .append("' at 0x").append(addr == -1 ? "N/A" : Long.toHexString(addr))
              .append(" size ").append(size == -1 ? "N/A" : String.valueOf(size)).toString();
    }
  }

  /**
   * Class represents memory access exceptions.
   */
  public class MoteMemoryException extends RuntimeException {

    public MoteMemoryException(String message, Object... args) {
      super(String.format(message, args));
    }
  }

  /**
   * Returns entire mote memory
   * @return Memory byte array
   * @throws org.contikios.cooja.mote.memory.MemoryInterface.MoteMemoryException 
   */
  public byte[] getMemory() throws MoteMemoryException;

  /**
   * Reads a segment from memory.
   *
   * @param addr Start address to read from
   * @param size Size to read [bytes]
   * @return Byte array
   */
  public byte[] getMemorySegment(long addr, int size) throws MoteMemoryException;

  /**
   * Sets a segment of memory.
   *
   * @param addr Start address to write to
   * @param data Size to write [bytes]
   */
  void setMemorySegment(long addr, byte[] data) throws MoteMemoryException;

  /**
   * Clears the memory.
   */
  void clearMemory();

  /**
   * 
   * @return 
   */
  long getStartAddr();

  /**
   * Returns total size of memory.
   *
   * @return Size [bytes]
   */
  int getTotalSize();// XXX getSize();

  /**
   * Returns Map of all variables in memory.
   * Map must be of typ name / symbol.
   *
   * @return Variables
   */
  Map<String, Symbol> getSymbolMap();

  /**
   * Returns the MemoryLayout for this memory.
   * 
   * @return Memory layout for this memory
   */
  MemoryLayout getLayout();

  /**
   * Monitor to listen for memory updates.
   */
  interface SegmentMonitor {

    public static enum EventType {

      READ,
      WRITE,
      READWRITE
    }

    /**
     * Invoked if the monitored segment changed
     * 
     * @param memory Reference to the memory
     * @param type XXX ???
     * @param address Address in segment where modification occured
     */
    void memoryChanged(MemoryInterface memory, EventType type, long address);
  }

  /**
   * Adds a SegmentMonitor for the specified address region.
   *
   * @param flag Select memory operation(s) to listen for
   * (read, write, read/write)
   * @param address Start address of monitored data region
   * @param size Size of monitored data region
   * @param monitor SegmentMonitor to add
   * @return true if monitor could be added, false if not
   */
  boolean addSegmentMonitor(SegmentMonitor.EventType flag, long address, int size, SegmentMonitor monitor);

  /**
   * Removes SegmentMonitor assigned to the specified region.
   *
   * @param address Start address of Monitor data region
   * @param size Size of Monitor data region
   * @param monitor SegmentMonitor to remove
   * @return true if monitor was removed, false if not
   */
  boolean removeSegmentMonitor(long address, int size, SegmentMonitor monitor);
}
