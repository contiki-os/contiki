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

import java.nio.ByteOrder;

/**
 * Holds memory layout informations such as endianess, wordsize, C int size.
 * 
 * @author Enrico Jorns
 */
public class MemoryLayout {

  /** 8 bit memory architecture */
  public static final int ARCH_8BIT = 1;
  /** 16 bit memory architecture */
  public static final int ARCH_16BIT = 2;
  /** 32 bit memory architecture */
  public static final int ARCH_32BIT = 4;
  /** 64 bit memory architecture */
  public static final int ARCH_64BIT = 8;

  /**
   * Size of data types in bytes.
   */
  public enum DataType {

    BYTE(1),
    CHAR(1),
    SHORT(2),
    INT(4),
    LONG(8),
    LONGLONG(8),
    INT8(1),
    INT16(2),
    INT32(4),
    INT64(8),
    FLOAT(4),
    DOUBLE(8),
    POINTER(4);

    private int size;

    DataType(int size) {
      this.size = size;
    }

    public void setSize(int size) {
      this.size = size;
    }

    public int getSize() {
      return this.size;
    }
  }

  public final ByteOrder order;
  public final int addrSize;
  public final int intSize;
  public final int WORD_SIZE;
  private boolean aligned = true;

  /**
   * Creates new MemoryLayout instance.
   *
   * @param order either ByteOrder.BIG_ENDIAN, or ByteOrder.LITTLE_ENDIAN
   * @param wordsize should be one of ARCH_8BIT, ARCH_16BIT, ARCH_32BIT,
   * ARCH_64BIT
   * @param sizeofInt
   */
  public MemoryLayout(ByteOrder order, int wordsize, int sizeofInt) {
    this(order, wordsize, sizeofInt, wordsize);
  }

  /**
   * Creates new MemoryLayout instance.
   *
   * @param order either ByteOrder.BIG_ENDIAN, or ByteOrder.LITTLE_ENDIAN
   * @param wordsize should be one of ARCH_8BIT, ARCH_16BIT, ARCH_32BIT,
   * ARCH_64BIT
   * @param sizeofPointer
   * @param sizeofInt
   */
  public MemoryLayout(ByteOrder order, int wordsize, int sizeofInt, int sizeofPointer) {
    this.order = order;
    this.WORD_SIZE = wordsize;
    this.intSize = sizeofInt;
    this.addrSize = sizeofPointer;
    DataType.INT.setSize(this.intSize);
    DataType.POINTER.setSize(this.addrSize);
  }

  /**
   * Returns the MemoryLayout for the running jvm.
   *
   * @return MemoryLayout for the running jvm.
   */
  public static MemoryLayout getNative() {
    return new MemoryLayout(
            ByteOrder.nativeOrder(),
            Integer.parseInt(System.getProperty("sun.arch.data.model")) / 8,
            Integer.SIZE / 8);
  }

  /**
   * Enable/Disable data alignment.
   *
   * Determines whether data alignemnt is used for packing structs.
   * Default is enabled.
   *
   * @param aligned true to enable data alignment, false to disable
   */
  public void setAligned(boolean aligned) {
    this.aligned = aligned;
  }

  /**
   * Returns true if data is aligned.
   * 
   * @return if aligned
   */
  public boolean isAligned() {
    return aligned;
  }

  /**
   * Returns number of padding bytes between two data types.
   * 
   * @param currType
   * @param nextType
   * @return 
   */
  public int getPaddingBytesFor(DataType currType, DataType nextType) {
    /* No padding bytes for unaligned memory */
    if (!isAligned()) {
      return 0;
    }
    /* get size of next element in structure */
    int nextsize = nextType.getSize();
    /* limit padding to word size */
    nextsize = nextsize > WORD_SIZE ? WORD_SIZE : nextsize;
    /* calc padding */
    int pad = nextsize - currType.getSize();
    return pad;
  }

  /**
   * Returns information string for this MemoryLayout.
   * 
   * @return String that shows Endianess and word size.
   */
  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    return sb.append("MemoryLayout: ")
            .append("Endianess: ").append(order)
            .append(", WORD_SIZE: ").append(WORD_SIZE)
            .toString();
  }
}
