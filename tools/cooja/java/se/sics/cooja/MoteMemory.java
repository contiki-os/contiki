/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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

package se.sics.cooja;


/**
 * This interface represents a mote memory.
 * 
 * Mote memory is represented by byte arrays and this
 * interface provides a few of basic operations.
 * 
 * Note that this memory internally may consist of several
 * different memory sections, not covering the entire range
 * between the start address and the end address of this memory.
 *
 * @see se.sics.cooja.SectionMoteMemory
 * @author Fredrik Osterlind
 */
public interface MoteMemory extends AddressMemory {

  /**
   * Clears the entire memory.
   */
  public void clearMemory();

  /**
   * Returns a memory segment.
   * 
   * @param address Start address of memory segment
   * @param size Size of memory segment
   * @return Memory segment or null if segment not available
   */
  public byte[] getMemorySegment(int address, int size);

  /**
   * Sets a memory segment.
   * 
   * @param address Start address of memory segment
   * @param data Data
   */
  public void setMemorySegment(int address, byte[] data);

  /**
   * Returns the sum of all byte array sizes in this memory.
   * This is not neccessarily the the same as the total memory range,
   * since the entire memory range may not be handled by this memory.
   * 
   * @return Total size
   */
  public int getTotalSize();

  public abstract int parseInt(byte[] memorySegment);

  public enum MemoryEventType { READ, WRITE };

  public interface MemoryMonitor {
    public void memoryChanged(MoteMemory memory, MemoryEventType type, int address);
  }

  public boolean addMemoryMonitor(int address, int size, MemoryMonitor mm);
  public void removeMemoryMonitor(int address, int size, MemoryMonitor mm);
}
