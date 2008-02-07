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
 * $Id: MspMoteMemory.java,v 1.1 2008/02/07 14:53:29 fros4943 Exp $
 */

package se.sics.cooja.mspmote;

import java.util.*;
import org.apache.log4j.Logger;

import se.sics.cooja.AddressMemory;
import se.sics.cooja.MoteMemory;
import se.sics.mspsim.core.MSP430;

public class MspMoteMemory implements MoteMemory, AddressMemory {
  private static Logger logger = Logger.getLogger(MspMoteMemory.class);
  private final Properties variableAddresses;

  private int TEMP_MEM_START;
  private MSP430 cpu;

  public MspMoteMemory(Properties variableAddresses, MSP430 cpu) {
    this.variableAddresses = variableAddresses;

    this.cpu = cpu;
    TEMP_MEM_START = 0;
  }

  public String[] getVariableNames() {
    String[] names = new String[variableAddresses.size()];
    Enumeration nameEnum = variableAddresses.keys();
    for (int i = 0; i < variableAddresses.size(); i++) {
      names[i] = (String) nameEnum.nextElement();
    }
    return names;
  }

  public int getVariableAddress(String varName) {
    if (!variableAddresses.containsKey(varName)) {
      return -1;
    }
    return ((Integer) variableAddresses.get(varName)).intValue();
  }

  public void clearMemory() {
    logger.fatal("clearMemory() not implemented");
  }

  public byte[] getMemorySegment(int address, int size) {
    int[] ret = new int[size];

    System.arraycopy(cpu.memory, address - TEMP_MEM_START, ret, 0, size);

    // TODO XXX Slow method
    byte[] ret2 = new byte[size];
    for (int i=0; i < size; i++) {
      ret2[i] = (byte) ret[i];
    }

    return ret2;
  }

  public void setMemorySegment(int address, byte[] data) {
    // TODO XXX Slow method
    int[] intArr = new int[data.length];
    for (int i=0; i < data.length; i++) {
      intArr[i] = data[i];
    }

    System.arraycopy(intArr, 0, cpu.memory, address - TEMP_MEM_START, data.length);
  }

  public int getTotalSize() {
    return cpu.memory.length;
  }

  public boolean variableExists(String varName) {
    return variableAddresses.containsKey(varName);
  }

  public int getIntValueOf(String varName) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName)) {
      return -1;
    }
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    byte[] varData = getMemorySegment(varAddr, 2);

    int retVal = 0;
    int pos = 0;
    retVal += ((varData[pos++] & 0xFF)) << 8;
    retVal += ((varData[pos++] & 0xFF)) << 0;

    return Integer.reverseBytes(retVal) >> 16; // Crop two bytes
  }

  public void setIntValueOf(String varName, int newVal) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName)) {
      return;
    }
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    // TODO Check if small/big-endian when coming from JNI?
    int newValToSet = Integer.reverseBytes(newVal);

    // Create byte array
    int pos = 0;

    byte[] varData = new byte[4];
    varData[pos++] = (byte) ((newValToSet & 0xFF000000) >> 24);
    varData[pos++] = (byte) ((newValToSet & 0xFF0000) >> 16);
    varData[pos++] = (byte) ((newValToSet & 0xFF00) >> 8);
    varData[pos++] = (byte) ((newValToSet & 0xFF) >> 0);

    setMemorySegment(varAddr, varData);
  }

  public byte getByteValueOf(String varName) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName)) {
      return -1;
    }
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    byte[] varData = getMemorySegment(varAddr, 1);

    return varData[0];
  }

  public void setByteValueOf(String varName, byte newVal) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName)) {
      return;
    }
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    byte[] varData = new byte[1];

    varData[0] = newVal;

    setMemorySegment(varAddr, varData);
  }

  public byte[] getByteArray(String varName, int length) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName)) {
      return null;
    }
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    // TODO Check if small/big-endian when coming from JNI?
    return getMemorySegment(varAddr, length);
  }

  public void setByteArray(String varName, byte[] data) {
    // Get start address of variable
    if (!variableAddresses.containsKey(varName)) {
      return;
    }
    int varAddr = ((Integer) variableAddresses.get(varName)).intValue();

    // TODO Check if small/big-endian when coming from JNI?
    setMemorySegment(varAddr, data);
  }

}
