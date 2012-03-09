/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
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
 */

package se.sics.cooja;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

import org.apache.log4j.Logger;

/**
 * Represents a mote memory consisting of non-overlapping memory sections with
 * symbol addresses.
 * <p>
 * When an non-existing memory segment is written, a new section is automatically
 * created for this segment.
 * <p>
 *
 * @author Fredrik Osterlind
 */
public class SectionMoteMemory implements MoteMemory, AddressMemory {
  private static Logger logger = Logger.getLogger(SectionMoteMemory.class);

  private ArrayList<MoteMemorySection> sections = new ArrayList<MoteMemorySection>();
 
  /* readonly memory is never written to Contiki core, and is used to provide 
   * access to, for instance, strings */
  private ArrayList<MoteMemorySection> readonlySections = new ArrayList<MoteMemorySection>();

  private final HashMap<String, Integer> addresses;

  /* used to map Cooja's address space to native (Contiki's) addresses */
  private final int offset;
  
  /**
   * @param addresses Symbol addresses
   * @param offset Offset for internally used addresses
   */
  public SectionMoteMemory(HashMap<String, Integer> addresses, int offset) {
    this.addresses = addresses;
    this.offset = offset;
  }

  public String[] getVariableNames() {
    return addresses.keySet().toArray(new String[0]);
  }

  public int getVariableAddress(String varName) throws UnknownVariableException {
    /* Cooja address space */
    if (!addresses.containsKey(varName)) {
      throw new UnknownVariableException(varName);
    }
    
    return addresses.get(varName).intValue() + offset;
  }

  public int getIntegerLength() {
    return 4;
  }

  public void clearMemory() {
    sections.clear();
  }

  public byte[] getMemorySegment(int address, int size) {
    /* Cooja address space */
    address -= offset;
   
    for (MoteMemorySection section : sections) {
      if (section.includesAddr(address)
          && section.includesAddr(address + size - 1)) {
        return section.getMemorySegment(address, size);
      }
    }
    
    /* Check if in readonly section */
    for (MoteMemorySection section : readonlySections) {
      if (section.includesAddr(address)
          && section.includesAddr(address + size - 1)) {
        return section.getMemorySegment(address, size);
      }
    }
    
    return null;
  }

  public void setMemorySegmentNative(int address, byte[] data) {
    setMemorySegment(address+offset, data);
  }

  public void setMemorySegment(int address, byte[] data) {
    /* Cooja address space */
    address -= offset;

    /* TODO XXX Sections may overlap */
    for (MoteMemorySection section : sections) {
      if (section.includesAddr(address)
          && section.includesAddr(address + data.length - 1)) {
        section.setMemorySegment(address, data);
        return;
      }
    }
    sections.add(new MoteMemorySection(address, data));
  }

  public void setReadonlyMemorySegment(int address, byte[] data) {
    /* Cooja address space */
    address -= offset;

    readonlySections.add(new MoteMemorySection(address, data));
  }

  public int getTotalSize() {
    int totalSize = 0;
    for (MoteMemorySection section : sections) {
      totalSize += section.getSize();
    }
    return totalSize;
  }

  /**
   * Returns the total number of sections in this memory.
   *
   * @return Number of sections
   */
  public int getNumberOfSections() {
    return sections.size();
  }

  /**
   * Get start address of given section in native address space.
   *
   * @param sectionNr Section position
   * @return Start address of section
   */
  public int getSectionNativeAddress(int sectionNr) {
    if (sectionNr >= sections.size()) {
      return -1;
    }
    return sections.get(sectionNr).getStartAddr();
  }

  /**
   * Get size of section at given position.
   *
   * @param sectionNr Section position
   * @return Size of section
   */
  public int getSizeOfSection(int sectionNr) {
    if (sectionNr >= sections.size()) {
      return -1;
    }

    return sections.get(sectionNr).getSize();
  }

  /**
   * Get data of section at given position.
   *
   * @param sectionNr Section position
   * @return Data at section
   */
  public byte[] getDataOfSection(int sectionNr) {
    if (sectionNr >= sections.size()) {
      return null;
    }

    return sections.get(sectionNr).getData();
  }

  public boolean variableExists(String varName) {
    return addresses.containsKey(varName);
  }

  public int getIntValueOf(String varName) throws UnknownVariableException {
    int varAddr = getVariableAddress(varName);
    byte[] varData = getMemorySegment(varAddr, 4);

    if (varData == null) {
      throw new UnknownVariableException(varName);
    }

    return parseInt(varData);
  }

  public void setIntValueOf(String varName, int newVal) throws UnknownVariableException {
    int varAddr = getVariableAddress(varName);

    /* TODO Correct for all platforms? */
    int newValToSet = Integer.reverseBytes(newVal);

    int pos = 0;

    byte[] varData = new byte[4];
    varData[pos++] = (byte) ((newValToSet & 0xFF000000) >> 24);
    varData[pos++] = (byte) ((newValToSet & 0xFF0000) >> 16);
    varData[pos++] = (byte) ((newValToSet & 0xFF00) >> 8);
    varData[pos++] = (byte) ((newValToSet & 0xFF) >> 0);

    setMemorySegment(varAddr, varData);
  }

  public byte getByteValueOf(String varName) throws UnknownVariableException {
    int varAddr = getVariableAddress(varName);
    byte[] varData = getMemorySegment(varAddr, 1);

    if (varData == null) {
      throw new UnknownVariableException(varName);
    }

    return varData[0];
  }

  public void setByteValueOf(String varName, byte newVal) throws UnknownVariableException {
    int varAddr = getVariableAddress(varName);
    byte[] varData = new byte[1];

    varData[0] = newVal;

    setMemorySegment(varAddr, varData);
  }

  public byte[] getByteArray(String varName, int length) throws UnknownVariableException {
    int varAddr = getVariableAddress(varName);
    return getMemorySegment(varAddr, length);
  }

  public void setByteArray(String varName, byte[] data) throws UnknownVariableException {
    int varAddr = getVariableAddress(varName);
    setMemorySegment(varAddr, data);
  }

  /**
   * A memory section contains a byte array and a start address.
   *
   * @author Fredrik Osterlind
   */
  private static class MoteMemorySection {
    private byte[] data = null;
    private final int startAddr;

    /**
     * Create a new memory section.
     *
     * @param startAddr
     *          Start address of section
     * @param data
     *          Data of section
     */
    public MoteMemorySection(int startAddr, byte[] data) {
      this.startAddr = startAddr;
      this.data = data;
    }

    /**
     * Returns start address of this memory section.
     *
     * @return Start address
     */
    public int getStartAddr() {
      return startAddr;
    }

    /**
     * Returns size of this memory section.
     *
     * @return Size
     */
    public int getSize() {
      return data.length;
    }

    /**
     * Returns the entire byte array which defines this section.
     *
     * @return Byte array
     */
    public byte[] getData() {
      return data;
    }

    /**
     * True if given address is part of this memory section.
     *
     * @param addr
     *          Address
     * @return True if given address is part of this memory section, false
     *         otherwise
     */
    public boolean includesAddr(int addr) {
      if (data == null) {
        return false;
      }

      return (addr >= startAddr && addr < (startAddr + data.length));
    }

    /**
     * Returns memory segment.
     *
     * @param addr
     *          Start address of memory segment
     * @param size
     *          Size of memory segment
     * @return Memory segment
     */
    public byte[] getMemorySegment(int addr, int size) {
      byte[] ret = new byte[size];
      System.arraycopy(data, addr - startAddr, ret, 0, size);
      return ret;
    }

    /**
     * Sets a memory segment.
     *
     * @param addr
     *          Start of memory segment
     * @param data
     *          Data of memory segment
     */
    public void setMemorySegment(int addr, byte[] data) {
      System.arraycopy(data, 0, this.data, addr - startAddr, data.length);
    }

    public MoteMemorySection clone() {
      byte[] dataClone = new byte[data.length];
      System.arraycopy(data, 0, dataClone, 0, data.length);

      MoteMemorySection clone = new MoteMemorySection(startAddr, dataClone);
      return clone;
    }
  }

  public SectionMoteMemory clone() {
    ArrayList<MoteMemorySection> sectionsClone = new ArrayList<MoteMemorySection>();
    for (MoteMemorySection section : sections) {
      sectionsClone.add(section.clone());
    }

    SectionMoteMemory clone = new SectionMoteMemory(addresses, offset);
    clone.sections = sectionsClone;
    clone.readonlySections = readonlySections;

    return clone;
  }

  private ArrayList<PolledMemorySegments> polledMemories = new ArrayList<PolledMemorySegments>();
  public void pollForMemoryChanges() {
    for (PolledMemorySegments mem: polledMemories.toArray(new PolledMemorySegments[0])) {
      mem.notifyIfChanged();
    }
  }

  private class PolledMemorySegments {
    public final MemoryMonitor mm;
    public final int address;
    public final int size;
    private byte[] oldMem;

    public PolledMemorySegments(MemoryMonitor mm, int address, int size) {
      this.mm = mm;
      this.address = address;
      this.size = size;
      
      oldMem = getMemorySegment(address, size);
    }

    private void notifyIfChanged() {
      byte[] newMem = getMemorySegment(address, size);
      if (Arrays.equals(oldMem, newMem)) {
        return;
      }
      
      mm.memoryChanged(SectionMoteMemory.this, MemoryEventType.WRITE, address);
      oldMem = newMem;
    }
  }
  
  public boolean addMemoryMonitor(int address, int size, MemoryMonitor mm) {
    PolledMemorySegments t = new PolledMemorySegments(mm, address, size);
    polledMemories.add(t);
    return true;
  }

  public void removeMemoryMonitor(int address, int size, MemoryMonitor mm) {
    for (PolledMemorySegments mcm: polledMemories) {
      if (mcm.mm != mm || mcm.address != address || mcm.size != size) {
        continue;
      }
      polledMemories.remove(mcm);
      break;
    }
  }

  public int parseInt(byte[] memorySegment) {
    int retVal = 0;
    int pos = 0;
    retVal += ((memorySegment[pos++] & 0xFF)) << 24;
    retVal += ((memorySegment[pos++] & 0xFF)) << 16;
    retVal += ((memorySegment[pos++] & 0xFF)) << 8;
    retVal += ((memorySegment[pos++] & 0xFF)) << 0;

    retVal = Integer.reverseBytes(retVal);
    return retVal;
  }
}
