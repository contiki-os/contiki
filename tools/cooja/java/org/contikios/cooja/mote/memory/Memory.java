/*
 * Copyright (c) 2014, TU Braunschweig.
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
 */
package org.contikios.cooja.mote.memory;

import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor.EventType;
import org.contikios.cooja.mote.memory.MemoryLayout.DataType;

/**
 * Represents memory that can be accessed with address and size informations.
 *
 * @author Enrico Jorns
 */
public abstract class Memory {

  private final MemoryInterface memIntf;

  /**
   * Creates new memory for given MemoryLayout.
   *
   * @param intf
   */
  public Memory(MemoryInterface intf) {
    memIntf = intf;
  }

  // -- Get fixed size types
  /**
   * Read 8 bit integer from address.
   *
   * @param addr Address to read from
   * @return 8 bit value read from address
   */
  public byte getInt8ValueOf(long addr) {
    return memIntf.getMemorySegment(addr, DataType.INT8.getSize())[0];
  }

  /**
   * Read 16 bit integer from address.
   *
   * @param addr Address to read from
   * @return 16 bit value read from address
   */
  public short getInt16ValueOf(long addr) {
    return MemoryBuffer.wrap(
            memIntf.getLayout(),
            memIntf.getMemorySegment(addr, DataType.INT16.getSize())).getInt16();
  }

  /**
   * Read 32 bit integer from address.
   *
   * @param addr Address to read from
   * @return 32 bit value read from address
   */
  public int getInt32ValueOf(long addr) {
    return MemoryBuffer.wrap(
            memIntf.getLayout(),
            memIntf.getMemorySegment(addr, DataType.INT32.getSize())).getInt32();
  }

  /**
   * Read 64 bit integer from address.
   *
   * @param addr Address to read from
   * @return 64 bit value read from address
   */
  public long getInt64ValueOf(long addr) {
    return MemoryBuffer.wrap(
            memIntf.getLayout(),
            memIntf.getMemorySegment(addr, DataType.INT64.getSize())).getInt64();
  }

  // -- Get compiler-dependent types
  /**
   * Read byte from address.
   *
   * @param addr Address to read from
   * @return byte read from address
   */
  public byte getByteValueOf(long addr) {
    return memIntf.getMemorySegment(addr, DataType.BYTE.getSize())[0];
  }

  /**
   * Read short from address.
   *
   * @param addr Address to read from
   * @return short read from address
   */
  public short getShortValueOf(long addr) {
    return MemoryBuffer.wrap(memIntf.getLayout(), memIntf.getMemorySegment(addr, 2)).getShort();
  }

  /**
   * Read integer from address.
   * <p>
   * Note: Size of integer depends on platform type.
   *
   * @param addr Address to read from
   * @return integer read from address
   */
  public int getIntValueOf(long addr) {
    return MemoryBuffer.wrap(memIntf.getLayout(), memIntf.getMemorySegment(addr, memIntf.getLayout().intSize)).getInt();
  }

  /**
   * Read long from address.
   *
   * @param addr Address to read from
   * @return long read from address
   */
  public long getLongValueOf(long addr) {
    return MemoryBuffer.wrap(memIntf.getLayout(), memIntf.getMemorySegment(addr, 4)).getLong();
  }

  /**
   * Read pointer from address.
   * <p>
   * Note: Size of pointer depends on platform type.
   *
   * @param addr Address to read from
   * @return pointer read from address
   */
  public long getAddrValueOf(long addr) {
    return MemoryBuffer.wrap(memIntf.getLayout(), memIntf.getMemorySegment(addr, memIntf.getLayout().addrSize)).getAddr();
  }

  /**
   * Read byte array starting at given address.
   *
   * @param addr Start address to read from
   * @param length Numbe of bytes to read
   * @return byte array read from location assigned to variable name
   */
  public byte[] getByteArray(long addr, int length)
          throws UnknownVariableException {
    return memIntf.getMemorySegment(addr, length);
  }

  // -- Set fixed size types
  /**
   * Write 8 bit integer to address.
   *
   * @param addr Address to write to
   * @param value 8 bit value to write
   */
  public void setInt8ValueOf(long addr, byte value) {
    memIntf.setMemorySegment(addr, new byte[]{value});
  }

  /**
   * Write 16 bit integer to address.
   *
   * @param addr Address to write to
   * @param value 16 bit value to write
   */
  public void setInt16ValueOf(long addr, short value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(
            memIntf.getLayout(),
            new byte[DataType.INT16.getSize()]).putShort(value).getBytes());
  }

  /**
   * Write 32 bit integer to address.
   *
   * @param addr Address to write to
   * @param value 32 bit value to write
   */
  public void setInt32ValueOf(long addr, int value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(
            memIntf.getLayout(),
            new byte[DataType.INT32.getSize()]).putInt(value).getBytes());
  }

  /**
   * Write 64 bit integer to address.
   *
   * @param addr Address to write to
   * @param value 64 bit value to write
   */
  public void setInt64ValueOf(long addr, long value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(
            memIntf.getLayout(),
            new byte[DataType.INT64.getSize()]).putLong(value).getBytes());
  }

  // -- Set compiler-dependent types
  /**
   * Write byte to address.
   *
   * @param addr Address to write to
   * @param value byte to write
   */
  public void setByteValueOf(long addr, byte value) {
    memIntf.setMemorySegment(addr, new byte[]{value});
  }

  /**
   * Write short to address.
   *
   * @param addr Address to write to
   * @param value short to write
   */
  public void setShortValueOf(long addr, short value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(memIntf.getLayout(), new byte[2]).putShort(value).getBytes());
  }

  /**
   * Write integer to address.
   * <p>
   * Note: Size of integer depends on platform type.
   *
   * @param addr Address to write to
   * @param value integer to write
   */
  public void setIntValueOf(long addr, int value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(memIntf.getLayout(), new byte[memIntf.getLayout().intSize]).putInt(value).getBytes());
  }

  /**
   * Write long to address.
   *
   * @param addr Address to write to
   * @param value long to write
   */
  public void setLongValueOf(long addr, long value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(memIntf.getLayout(), new byte[4]).putLong(value).getBytes());
  }

  /**
   * Write pointer to address.
   * <p>
   * Note: Size of pointer depends on platform type.
   *
   * @param addr Address to write to
   * @param value pointer to write
   */
  public void setAddrValueOf(long addr, long value) {
    memIntf.setMemorySegment(addr, MemoryBuffer.wrap(memIntf.getLayout(), new byte[memIntf.getLayout().addrSize]).putAddr(value).getBytes());
  }

  /**
   * Write byte array starting at given address.
   *
   * @param addr Start address to write to
   * @param data data to write
   */
  public void setByteArray(long addr, byte[] data)
          throws UnknownVariableException {
    memIntf.setMemorySegment(addr, data);
  }

  /**
   * Adds monitor to specified memory region.
   * 
   * @param flag Select memory operation(s) to listen for (read, write, read/write)
   * @param addr Start address of monitored region
   * @param size Size of monitored region
   * @param mm Monitor to add
   * @return if monitor could be added, false if not
   */
  public boolean addMemoryMonitor(EventType flag, long addr, int size, SegmentMonitor mm) {
    return memIntf.addSegmentMonitor(flag, addr, size, mm);
  }

  /**
   * Removes monitor assigned to the specified region.
   *
   * @param addr Start address of monitored region
   * @param size Size of monitored region
   * @param mm Monitor to remove
   */
  public void removeMemoryMonitor(long addr, int size, SegmentMonitor mm) {
    memIntf.removeSegmentMonitor(addr, size, mm);
  }
}
