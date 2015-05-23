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

import java.nio.ByteBuffer;
import org.contikios.cooja.mote.memory.MemoryLayout.DataType;

/**
 * Basic routines for memory access with multi-arch support.
 *
 * Handles endianess, integer size and address size.
 *
 * Supports padding/aligning.
 *
 * @author Enrico Jorns
 */
public class MemoryBuffer {

  private final MemoryLayout memLayout;
  private final ByteBuffer bbuf;
  private final DataType[] structure;
  private int structIndex;

  private MemoryBuffer(MemoryLayout memLayout, ByteBuffer buffer, DataType[] structure) {
    this.memLayout = memLayout;
    this.bbuf = buffer;
    this.structure = structure;
    this.structIndex = 0;
  }

  /**
   * Wraps a byte array into an unstructered MemoryBuffer with given MemoryLayout.
   *<p>
   * Note that modifications to the buffer are applied to the backed array and vice versa.
   *
   * @param layout MemoryLayout for memory to access array from
   * @param array Byte array that will back this buffer
   * @return the new MemroyBuffer
   */
  public static MemoryBuffer wrap(MemoryLayout layout, byte[] array) {
    return wrap(layout, array, null);
  }

  /**
   * Wraps a byte array into a structured MemoryBuffer with given MemoryLayout.
   *<p>
   * Note that modifications to the buffer are applied to the backed array and vice versa.
   *<p>
   * A structured MemoryBuffer can be used to simplify reading c struct data
   * from the memory as it provides support for data aligning.
   *<p>
   * The structure array elements should be set to exactly the same types and order
   * that the corresponding c struct has.
   * 
   * @param layout MemoryLayout for memory to access array from
   * @param structure Array of data types representing the structure to read
   * @param array Byte array that will back this buffer
   * @return the new MemroyBuffer
   */
  public static MemoryBuffer wrap(MemoryLayout layout, byte[] array, DataType[] structure) {
    ByteBuffer b = ByteBuffer.wrap(array);
    b.order(layout.order); // preset endianess
    return new MemoryBuffer(layout, b, structure);
  }

  /**
   * Returns the byte array that backs this buffer
   *
   * @return  byte array that backs this buffer
   */
  public byte[] getBytes() {
    if (bbuf.hasArray()) {
      return bbuf.array();
    }
    else {
      return null;
    }
  }

  /**
   * Calculates the padding bytes to be added/skipped between current and next
   * element.
   *
   * @param type Current data type
   */
  private void skipPaddingBytesFor(DataType type) {
    /* Check if we have a structure and not yet reached the last element */
    if (structure != null && structure[structIndex + 1] != null) {
      /* get size of next element in structure */
      int nextsize = structure[structIndex + 1].getSize();
      /* limit padding to word size */
      nextsize = nextsize > memLayout.WORD_SIZE ? memLayout.WORD_SIZE : nextsize;
      /* calc padding */
      int pad = nextsize - type.getSize();
      /* Skip padding bytes */
      if (pad > 0) {
        bbuf.position(bbuf.position() + pad);
      }
    }
    structIndex++;
  }

  /**
   * Returns current element type.
   *
   * @return current element type or null if no struct is used
   */
  public DataType getType() {
    if (structure == null) {
      return null;
    }
    return structure[structIndex];
  }
  
  // --- fixed size types

  /**
   * Reads 8 bit integer from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return 8 bit integer at the buffer's current position
   */
  public byte getInt8() {
    byte value = bbuf.get();
    skipPaddingBytesFor(DataType.INT8);
    return value;
  }

  /**
   * Reads 16 bit integer from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return 16 bit integer at the buffer's current position
   */
  public short getInt16() {
    short value = bbuf.getShort();
    skipPaddingBytesFor(DataType.INT16);
    return value;
  }

  /**
   * Reads 32 bit integer from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return 32 bit integer at the buffer's current position
   */
  public int getInt32() {
    int value = bbuf.getInt();
    skipPaddingBytesFor(DataType.INT32);
    return value;
  }

  /**
   * Reads 64 bit integer from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return 64 bit integer at the buffer's current position
   */
  public long getInt64() {
    long value = bbuf.getLong();
    skipPaddingBytesFor(DataType.INT64);
    return value;
  }

  // --- platform-dependent types

  /**
   * Reads byte from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return byte at the buffer's current position
   */
  public byte getByte() {
    byte value = bbuf.get();
    skipPaddingBytesFor(DataType.BYTE);
    return value;
  }

  /**
   * Reads short from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return short at the buffer's current position
   */
  public short getShort() {
    short value = bbuf.getShort();
    skipPaddingBytesFor(DataType.SHORT);
    return value;
  }

  /**
   * Reads integer from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return integer at the buffer's current position
   */
  public int getInt() {
    int value;
    switch (memLayout.intSize) {
      case 2:
        value = bbuf.getShort();
        break;
      case 4:
      default:
        value = bbuf.getInt();
        break;
    }
    skipPaddingBytesFor(DataType.INT);
    return value;
  }

  /**
   * Reads long from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return long at the buffer's current position
   */
  public long getLong() {
    long value = bbuf.getLong();
    skipPaddingBytesFor(DataType.LONG);
    return value;
  }

  /**
   * Reads pointer from current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured reading is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @return pointer at the buffer's current position
   */
  public long getAddr() {
    long value;
    switch (memLayout.addrSize) {
      case 2:
        value = bbuf.getShort();
        break;
      case 4:
        value = bbuf.getInt();
        break;
      case 8:
        value = bbuf.getLong();
        break;
      default:
        value = bbuf.getInt();
        break;
    }
    skipPaddingBytesFor(DataType.POINTER);
    return value;
  }

  // --- fixed size types
  
  /**
   * Writes 8 bit integer to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value 8 bit integer to write
   * @return A reference to this object
   */
  public MemoryBuffer putInt8(byte value) {
    bbuf.put(value);
    skipPaddingBytesFor(DataType.INT8);
    return this;
  }

  /**
   * Writes 16 bit integer to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value 16 bit integer to write
   * @return A reference to this object
   */
  public MemoryBuffer putInt16(short value) {
    bbuf.putShort(value);
    skipPaddingBytesFor(DataType.INT16);
    return this;
  }

  /**
   * Writes 32 bit integer to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value 32 bit integer to write
   * @return A reference to this object
   */
  public MemoryBuffer putInt32(int value) {
    bbuf.putInt(value);
    skipPaddingBytesFor(DataType.INT32);
    return this;
  }

  /**
   * Writes 64 bit integer to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value 64 bit integer to write
   * @return A reference to this object
   */
  public MemoryBuffer putInt64(long value) {
    bbuf.putLong(value);
    skipPaddingBytesFor(DataType.INT64);
    return this;
  }

  // --- platform-dependent types
  
  /**
   * Writes byte to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value byte to write
   * @return A reference to this object
   */
  public MemoryBuffer putByte(byte value) {
    bbuf.put(value);
    skipPaddingBytesFor(DataType.BYTE);
    return this;
  }

  /**
   * Writes short to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value short to write
   * @return A reference to this object
   */
  public MemoryBuffer putShort(short value) {
    bbuf.putShort(value);
    skipPaddingBytesFor(DataType.SHORT);
    return this;
  }

  /**
   * Writes integer to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   * <p>
   * Note: Size of integer depends on platform type
   *
   * @param value integer to write
   * @return A reference to this object
   */
  public MemoryBuffer putInt(int value) {
    switch (memLayout.intSize) {
      case 2:
        /**
         * @TODO: check for size?
         */
        bbuf.putShort((short) value);
        break;
      case 4:
      default:
        bbuf.putInt(value);
        break;
    }
    skipPaddingBytesFor(DataType.INT);
    return this;
  }

  /**
   * Writes long to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   *
   * @param value long to write
   * @return A reference to this object
   */
  public MemoryBuffer putLong(long value) {
    bbuf.putLong(value);
    skipPaddingBytesFor(DataType.LONG);
    return this;
  }

  /**
   * Writes pointer to current buffer position,
   * and then increments position.
   * <p>
   * Note: If structured writing is enabled,
   * additional padding bytes may be skipped automatically.
   * <p>
   * Note: Size of pointer depends on platform type
   *
   * @param value pointer to write
   * @return A reference to this object
   */
  public MemoryBuffer putAddr(long value) {
    /**
     * @TODO: check for size?
     */
    switch (memLayout.addrSize) {
      case 2:
        bbuf.putShort((short) value);
        break;
      case 4:
        bbuf.putInt((int) value);
        break;
      case 8:
        bbuf.putLong(value);
        break;
      default:
        bbuf.putInt((int) value);
        break;
    }
    skipPaddingBytesFor(DataType.POINTER);
    return this;
  }
}
