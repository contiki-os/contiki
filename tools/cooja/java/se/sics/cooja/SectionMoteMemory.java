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
 * $Id: SectionMoteMemory.java,v 1.1 2006/08/21 12:12:56 fros4943 Exp $
 */

package se.sics.cooja;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.*;
import org.apache.log4j.Logger;

import se.sics.cooja.MoteMemory;

/**
 * Represents a mote memory consisting of non-overlapping
 * memory sections. This memory also contains information
 * about variable starts addresses.
 * <p>
 * When an unhandled memory segment is set a new section is
 * automatically created for this segment.
 * <p>
 * @author Fredrik Osterlind
 */
public class SectionMoteMemory implements MoteMemory {
  private static Logger logger = Logger.getLogger(SectionMoteMemory.class);

  private Vector<MoteMemorySection> sections = new Vector<MoteMemorySection>();
  private final Properties variableAddresses;
  
  /**
   * Create a new mote memory with information about which
   * variables exist and their relative memory addresses.
   * 
   * @param variableAddresses Variable addresses
   */
  public SectionMoteMemory(Properties variableAddresses) {
    this.variableAddresses = variableAddresses;
  }

  /**
   * @return All variable names known and residing in this memory
   */
  public String[] getVariableNames() {
    String[] names = new String[variableAddresses.size()];
    Enumeration nameEnum = variableAddresses.keys();
    for (int i=0; i < variableAddresses.size(); i++) {
      names[i] = (String) nameEnum.nextElement();
    }
    return names;
  }
  
  public void clearMemory() {
    sections.clear();
  }

  public byte[] getMemorySegment(int address, int size) {
    for (int i=0; i < sections.size(); i++) {
      if (sections.elementAt(i).includesAddr(address) && sections.elementAt(i).includesAddr(address + size - 1)) {
        return sections.elementAt(i).getMemorySegment(address, size);
      }
    }
    return null;
  }

  public void setMemorySegment(int address, byte[] data) {
    // TODO Creating overlapping memory sections possible
    for (int i=0; i < sections.size(); i++) {
      if (sections.elementAt(i).includesAddr(address) && sections.elementAt(i).includesAddr(address + data.length - 1)) {
        sections.elementAt(i).setMemorySegment(address, data);
        return;
      }
    }
    sections.add(new MoteMemorySection(address, data));
  }

  public int getTotalSize() {
    int totalSize = 0;
    for (MoteMemorySection section: sections)
      totalSize += section.getSize();
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
   * Removes a memory segment from this memory.
   * The section containing the segment may be split into two sections.
   *
   * @param startAddr Start address
   * @param size Length
   */
  public void removeSegmentFromMemory(int startAddr, int size) {
    for (MoteMemorySection section: sections)
      // Find section containing segment to remove
      if (section.includesAddr(startAddr) && 
          section.includesAddr(startAddr + size - 1)) {
        MoteMemorySection oldSection = section;
        
        byte[] dataFirstPart = oldSection.getMemorySegment(
            oldSection.startAddr,
            (int) (startAddr - oldSection.startAddr)
        );
        byte[] dataSecondPart = oldSection.getMemorySegment(
            startAddr + size,
            (int) (oldSection.startAddr + oldSection.getSize() - (startAddr + size)));
        
        MoteMemorySection newSectionFirstPart = new MoteMemorySection(oldSection.startAddr, dataFirstPart);
        MoteMemorySection newSectionSecondPart = new MoteMemorySection(startAddr + size, dataSecondPart);

        // Remove old section, add new sections
        sections.remove(oldSection);
        if (newSectionFirstPart.getSize() > 0)
          sections.add(newSectionFirstPart);
        if (newSectionSecondPart.getSize() > 0)
          sections.add(newSectionSecondPart);
      }
  }
  

  /**
   * Get start address of section at given position.
   * 
   * @param sectionNr Section position
   * @return Start address of section
   */
  public int getStartAddrOfSection(int sectionNr) {
    if (sectionNr >= sections.size())
      return 0;

    return sections.elementAt(sectionNr).getStartAddr();
  }

  /**
   * Get size of section at given position.
   * 
   * @param sectionNr Section position
   * @return Size of section
   */
  public int getSizeOfSection(int sectionNr) {
    if (sectionNr >= sections.size())
      return 0;

    return sections.elementAt(sectionNr).getSize();
  }

  /**
   * Get data of section at given position.
   * 
   * @param sectionNr Section position
   * @return Data at section
   */
  public byte[] getDataOfSection(int sectionNr) {
    if (sectionNr >= sections.size())
      return null;

    return sections.elementAt(sectionNr).getData();
  }

  /**
   * Returns a value of the integer variable with the given name.
   * 
   * @param varName Name of integer variable
   * @return Value of integer variable
   */
  public int getIntValueOf(String varName) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName))
      return -1;
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    byte[] varData = getMemorySegment(varAddr, 4);

    int retVal = 0;
    int pos = 0;
    retVal += ((int) (varData[pos++] & 0xFF)) << 24;
    retVal += ((int) (varData[pos++] & 0xFF)) << 16;
    retVal += ((int) (varData[pos++] & 0xFF)) << 8;
    retVal += ((int) (varData[pos++] & 0xFF)) << 0;

    // TODO Check if small/big-endian when coming from JNI?
    retVal = Integer.reverseBytes(retVal);

    return retVal;
  }

  /**
   * Set integer value of variable with given name.
   * 
   * @param varName Name of integer variable
   * @param newVal New value to set
   */
  public void setIntValueOf(String varName, int newVal) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName))
      return;
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();
    
    // TODO Check if small/big-endian when coming from JNI?
    int newValToSet = Integer.reverseBytes(newVal);
    
    // Create byte array
    int pos = 0;
    
    byte[] varData = new byte[4];
    varData[pos++]  = (byte) ((newValToSet & 0xFF000000) >> 24);
    varData[pos++]  = (byte) ((newValToSet & 0xFF0000) >> 16);
    varData[pos++]  = (byte) ((newValToSet & 0xFF00) >> 8);
    varData[pos++]  = (byte) ((newValToSet & 0xFF) >> 0);
    
    setMemorySegment(varAddr, varData);
  }

  /**
   * Returns a value of the byte variable with the given name.
   * 
   * @param varName Name of byte variable
   * @return Value of byte variable
   */
  public byte getByteValueOf(String varName) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName))
      return -1;
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    byte[] varData = getMemorySegment(varAddr, 1);

    return varData[0];
  }

  /**
   * Set byte value of variable with given name.
   * 
   * @param varName Name of byte variable
   * @param newVal New value to set
   */
  public void setByteValueOf(String varName, byte newVal) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName))
      return;
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    byte[] varData = new byte[1];

    varData[0] = newVal;

    setMemorySegment(varAddr, varData);
  }

  /**
   * Returns byte array of given length and with the given name.
   * 
   * @param varName Name of array
   * @param length Length of array
   * @return Data of array
   */
  public byte[] getByteArray(String varName, int length) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName))
      return null;
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    // TODO Check if small/big-endian when coming from JNI?
    return getMemorySegment(varAddr, length);
  }

  /**
   * Set byte array of the variable with the given name.
   * 
   * @param varName Name of array
   * @param data New data of array
   */
  public void setByteArray(String varName, byte[] data) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName))
      return;
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    // TODO Check if small/big-endian when coming from JNI?
    setMemorySegment(varAddr, data);
  }
  
  /**
   * A memory section represented of a byte array and a start address.
   * 
   * @author Fredrik Osterlind
   */
  private class MoteMemorySection {
    private byte[] data = null;
    private int startAddr;

    /**
     * Create a new memory section.
     * 
     * @param startAddr Start address of section
     * @param data Data of section
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
     * @param addr Address
     * @return True if given address is part of this memory section, false otherwise
     */
    public boolean includesAddr(int addr) {
      if (data == null)
        return false;
      
      return (addr >= startAddr && addr < (startAddr + data.length));
    }
    
    /**
     * Returns memory segment.
     * 
     * @param addr Start address of memory segment
     * @param size Size of memory segment
     * @return Memory segment
     */
    public byte[] getMemorySegment(int addr, int size) {
      byte[] ret = new byte[size];
      for (int i = 0; i < size; i++)
        ret[i] = data[(int) (addr - startAddr + i)];
      return ret;
    }

    /**
     * Sets a memory segment.
     * @param addr Start of memory segment
     * @param data Data of memory segment
     */
    public void setMemorySegment(int addr, byte[] data) {
      int nonnull=0;
      for (int i = 0; i < data.length; i++) {
        if (data[i] != 0) nonnull++;
        this.data[(int) (addr - startAddr + i)] = data[i];
      }
    }
    
    public MoteMemorySection clone() {
      byte[] dataClone = new byte[data.length];
      for (int i=0; i < data.length; i++)
        dataClone[i] = data[i];
      
      MoteMemorySection clone = new MoteMemorySection(startAddr, dataClone);
      return clone;
    }
    
  }

  
  // EXPERIMENTAL AND DEBUG METHODS
  public SectionMoteMemory clone() {
    Vector<MoteMemorySection> clonedSections = new Vector<MoteMemorySection>();
    for (int i=0; i < sections.size(); i++) {
      clonedSections.add((MoteMemorySection) sections.elementAt(i).clone());
    }

    SectionMoteMemory clone = new SectionMoteMemory(variableAddresses);
    clone.sections = clonedSections;

    return clone;
  }

  protected byte[] getChecksum() {
    MessageDigest messageDigest;
    try {
      messageDigest = MessageDigest.getInstance("MD5");
      
      for (int i=0; i < sections.size(); i++) {
        messageDigest.update(sections.get(i).data, 0, sections.get(i).getSize());
      }
    } catch (NoSuchAlgorithmException e) {
      return null;
    }
    return messageDigest.digest();
  }

  protected Vector<Integer> getDifferenceAddressesOf(SectionMoteMemory anotherMem) {
    Vector<Integer> differences = new Vector<Integer>();
    
    if (this.getNumberOfSections() != anotherMem.getNumberOfSections()) {
      logger.fatal("Number of section do not match!");
      return null;
    }
    
    for (int i=0; i < sections.size(); i++) {
      if (this.getSizeOfSection(i) != anotherMem.getSizeOfSection(i)) {
        logger.fatal("Section " + i + " sizes do not match!");
        return null;
      }
      if (this.getStartAddrOfSection(i) != anotherMem.getStartAddrOfSection(i)) {
        logger.fatal("Section " + i + " start addresses do not match!");
        return null;
      }
      
      for (int j=0; j < sections.get(i).getSize(); j++)
        if (this.sections.get(i).data[j] != anotherMem.getDataOfSection(i)[j])
          differences.add(new Integer(sections.get(i).startAddr + j));
      System.err.println();
    }
    return differences;
  }

  protected void printMemory() {
    for (int i=0; i < sections.size(); i++) {
      System.err.print("Section[" + i + "]: ");
      for (int j=0; j < sections.get(i).getSize(); j++)
        System.err.print(sections.get(i).getData()[j] + ",");
      System.err.println();
    }
  }
  
  protected void printDifferences(SectionMoteMemory anotherMem) {
    if (this.getNumberOfSections() != anotherMem.getNumberOfSections()) {
      logger.fatal("Number of section do not match!");
      return;
    }
    
    for (int i=0; i < sections.size(); i++) {
      if (this.getSizeOfSection(i) != anotherMem.getSizeOfSection(i)) {
        logger.fatal("Section " + i + " sizes do not match!");
        return;
      }
      if (this.getStartAddrOfSection(i) != anotherMem.getStartAddrOfSection(i)) {
        logger.fatal("Section " + i + " start addresses do not match!");
        return;
      }
      
      System.err.print("Section[" + i + "]: ");
      for (int j=0; j < sections.get(i).getSize(); j++)
        if (this.sections.get(i).data[j] != anotherMem.getDataOfSection(i)[j])
          System.err.print(j + ",");
      System.err.println();
    }
  }
  
  protected void printChecksum() {
    byte[] checksum = this.getChecksum();
    System.err.print("Checksum: ");
    for (int i=0; i < checksum.length; i++) {
      System.err.print(checksum[i] + ",");
    }
    System.err.println();
  }
  

  
}
