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

import java.util.Collection;
import java.util.Set;

import org.contikios.cooja.mote.memory.MemoryInterface.Symbol;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor.EventType;

/**
 * Represents memory that can be accessed with names of variables.
 *
 * @author Enrico Jorns
 */
public class VarMemory extends Memory {

  private MemoryInterface memIntf;

  /**
   * Creates new VarMemory.
   *
   * @param intf
   */
  public VarMemory(MemoryInterface intf) {
    super(intf);
    memIntf = intf;
  }

  /**
   * Allows to change the MemoryInterface associated with this access class.
   *
   * @param intf Interface to associate with
   */
  public void associateMemory(MemoryInterface intf) {
    memIntf = intf;
  }

  /**
   * Generates and returns an array of all variables in this memory
   *
   * @return All variables located in this memory
   */
  public Collection<Symbol> getVariables() {
    return memIntf.getSymbolMap().values();
  }

  /**
   * Generates an array of all variable names in this memory.
   *
   * @return All variable names located in this memory
   */
  public Set<String> getVariableNames() {
    return memIntf.getSymbolMap().keySet();
  }

  /**
   * Checks if given variable exists in memory.
   *
   * @param varName Variable name
   * @return True if variable exists, false otherwise
   */
  public boolean variableExists(String varName) {
    return memIntf.getSymbolMap().containsKey(varName);
  }

  /**
   * Returns address of variable with given name.
   *
   * @param varName Variable name
   * @return Variable address
   */
  public Symbol getVariable(String varName) throws UnknownVariableException {
    Symbol sym = memIntf.getSymbolMap().get(varName);
    if (sym == null) {
      throw new UnknownVariableException(varName);
    }
    return sym;
  }

  /**
   * Returns address of variable with given name.
   *
   * @param varName Variable name
   * @return Address of variable
   * @throws UnknownVariableException If variable not found
   */
  public long getVariableAddress(String varName) throws UnknownVariableException {
    return getVariable(varName).addr;
  }

  /**
   * Return size of variable with given name.
   *
   * @param varName Variable name
   * @return Size of variable, -1 if unknown size
   * @throws UnknownVariableException If variable not found
   */
  public int getVariableSize(String varName) throws UnknownVariableException {
    return getVariable(varName).size;
  }

  /**
   * Read 8 bit integer from location associated with this variable name.
   *
   * @param varName Variable name
   * @return 8 bit integer value read from location assigned to variable name
   */
  public byte getInt8ValueOf(String varName)
          throws UnknownVariableException {
    return getInt8ValueOf(getVariable(varName).addr);
  }

  /**
   * Read 16 bit integer from location associated with this variable name.
   *
   * @param varName Variable name
   * @return 16 bit integer value read from location assigned to variable name
   */
  public short getInt16ValueOf(String varName)
          throws UnknownVariableException {
    return getInt16ValueOf(getVariable(varName).addr);
  }

  /**
   * Read 32 bit integer from location associated with this variable name.
   *
   * @param varName Variable name
   * @return 32 bit integer value read from location assigned to variable name
   */
  public int getInt32ValueOf(String varName)
          throws UnknownVariableException {
    return getInt32ValueOf(getVariable(varName).addr);
  }

  /**
   * Read 64 bit integer from location associated with this variable name.
   *
   * @param varName Variable name
   * @return 64 bit integer value read from location assigned to variable name
   */
  public long getInt64ValueOf(String varName)
          throws UnknownVariableException {
    return getInt64ValueOf(getVariable(varName).addr);
  }

  /**
   * Read byte from location associated with this variable name.
   *
   * @param varName Variable name
   * @return byte value read from location assigned to variable name
   */
  public byte getByteValueOf(String varName)
          throws UnknownVariableException {
    return getByteValueOf(getVariable(varName).addr);
  }

  /**
   * Read short from location associated with this variable name.
   *
   * @param varName Variable name
   * @return short value read from location assigned to variable name
   */
  public short getShortValueOf(String varName)
          throws UnknownVariableException {
    short val = getShortValueOf(getVariable(varName).addr);
    return val;
  }

  /**
   * Read integer from location associated with this variable name.
   *
   * @param varName Variable name
   * @return integer value read from location assigned to variable name
   */
  public int getIntValueOf(String varName)
          throws UnknownVariableException {
    int val = getIntValueOf(getVariable(varName).addr);
    return val;
  }

  /**
   * Read long from location associated with this variable name.
   *
   * @param varName Variable name
   * @return long value read from location assigned to variable name
   */
  public long getLongValueOf(String varName)
          throws UnknownVariableException {
    long val = getLongValueOf(getVariable(varName).addr);
    return val;
  }

  /**
   * Read pointer from location associated with this variable name.
   *
   * The number of bytes actually read depends on the pointer size
   * defined in memory layout.
   *
   * @param varName Variable name
   * @return pointer value read from location assigned to variable name
   */
  public long getAddrValueOf(String varName)
          throws UnknownVariableException {
    long val = getAddrValueOf(getVariable(varName).addr);
    return val;
  }

  /**
   * Read byte array starting at location associated with this variable name.
   *
   * @param varName Variable name
   * @param length Numbe of bytes to read
   * @return byte array read from location assigned to variable name
   */
  public byte[] getByteArray(String varName, int length)
          throws UnknownVariableException {
    return getByteArray(getVariable(varName).addr, length);
  }

  /**
   * Write 8 bit integer value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value 8 bit integer value to write
   */
  public void setInt8ValueOf(String varName, byte value)
          throws UnknownVariableException {
    setInt8ValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write 16 bit integer value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value 16 bit integer value to write
   */
  public void setInt16ValueOf(String varName, short value)
          throws UnknownVariableException {
    setInt16ValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write 32 bit integer value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value 32 bit integer value to write
   */
  public void setInt32ValueOf(String varName, int value)
          throws UnknownVariableException {
    setInt32ValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write 64 bit integer value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value 64 bit integer value to write
   */
  public void setInt64ValueOf(String varName, long value)
          throws UnknownVariableException {
    setInt64ValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write byte value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value byte value to write
   */
  public void setByteValueOf(String varName, byte value)
          throws UnknownVariableException {
    setByteValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write short value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value short value to write
   */
  public void setShortValueOf(String varName, short value)
          throws UnknownVariableException {
    setShortValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write int value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value int value to write
   */
  public void setIntValueOf(String varName, int value)
          throws UnknownVariableException {
    setIntValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write long value to location associated with this variable name.
   *
   * @param varName Variable name
   * @param value long value to write
   */
  public void setLongValueOf(String varName, long value)
          throws UnknownVariableException {
    setLongValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write pointer value to location associated with this variable name.
   *
   * The number of bytes actually written depends on the pointer size
   * defined in memory layout.
   *
   * @param varName Variable name
   * @param value Value to write
   */
  public void setAddrValueOf(String varName, long value)
          throws UnknownVariableException {
    setAddrValueOf(getVariable(varName).addr, value);
  }

  /**
   * Write byte array starting at location associated with this variable name.
   *
   * @param varName Variable name
   * @param data data to write
   */
  public void setByteArray(String varName, byte[] data)
          throws UnknownVariableException {
    setByteArray(getVariable(varName).addr, data);
  }

  /**
   * Adds a monitor for the specified address region.
   *
   * @param flag Select memory operation(s) to listen for (read, write, read/write)
   * @param varName Name of variable to monitor
   * @param mm Monitor to add
   * @return if monitor could be added, false if not
   */
  public boolean addVarMonitor(EventType flag, final String varName, final SegmentMonitor mm) {
    return memIntf.addSegmentMonitor(
            flag,
            getVariable(varName).addr,
            getVariable(varName).size,
            mm);
  }

  /**
   * Removes monitor assigned to the specified region.
   *
   * @param varName Name of monitored variable
   * @param mm Monitor to remove
   */
  public void removeVarMonitor(String varName, SegmentMonitor mm) {
    memIntf.removeSegmentMonitor(getVariable(varName).addr, getVariable(varName).size, mm);
  }
}
