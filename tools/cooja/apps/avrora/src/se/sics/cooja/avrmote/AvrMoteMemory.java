/*
 * Copyright (c) 2009, Swedish Institute of Computer Science. All rights
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
 */

package se.sics.cooja.avrmote;

import java.util.ArrayList;
import java.util.Iterator;

import org.apache.log4j.Logger;

import se.sics.cooja.AddressMemory;
import se.sics.cooja.MoteMemory;
import avrora.arch.avr.AVRProperties;
import avrora.core.SourceMapping;
import avrora.core.SourceMapping.Location;
import avrora.sim.AtmelInterpreter;
import avrora.sim.Simulator.Watch;
/**
 * @author Joakim Eriksson
 */
public class AvrMoteMemory implements MoteMemory, AddressMemory {
    private static Logger logger = Logger.getLogger(AvrMoteMemory.class);

    private SourceMapping memoryMap;
    private AtmelInterpreter interpreter;
    private AVRProperties avrProperties;
    
    public AvrMoteMemory(SourceMapping map, AVRProperties avrProperties, AtmelInterpreter interpreter) {
        memoryMap = map;
        this.interpreter = interpreter;
        this.avrProperties = avrProperties;
    }

    public void insertWatch(Watch w, int address) {
        interpreter.getSimulator().insertWatch(w, address);
    }
    
    public void clearMemory() {
        logger.fatal("not implemented");
    }

    public byte[] getMemorySegment(int address, int size) {
        logger.fatal("getMemorySegment is not implemented");
        return null;
    }

    public int getTotalSize() {
        return 0;
    }

    public void setMemorySegment(int address, byte[] data) {
        logger.fatal("setMemorySegment is not implemented");
    }

    public byte[] getByteArray(String varName, int length)
            throws UnknownVariableException {
        return null;
    }

    public byte getByteValueOf(String varName) throws UnknownVariableException {
        return (byte) getValueOf(varName, 1);
    }
    
    private int getValueOf(String varName, int len) throws UnknownVariableException {
        Location mem = memoryMap.getLocation(varName);
        if (mem == null) throw new UnknownVariableException("Variable does not exist: " + varName);

        System.out.println("Variable:" + varName + " in section: " + mem.section);
        System.out.println("LMA: " + Integer.toHexString(mem.lma_addr));
        System.out.println("VMA: " + Integer.toHexString(mem.vma_addr));

        System.out.println("Data: " + interpreter.getDataByte(mem.lma_addr & 0xfffff));
        System.out.println("Flash: " + interpreter.getFlashByte(mem.lma_addr & 0xfffff));
        int data = 0;
        if (mem.vma_addr > 0xfffff) {
            for (int i = 0; i < len; i++) {
                data = (data << 8) + (interpreter.getDataByte((mem.vma_addr & 0xfffff) + len - i - 1) & 0xff);
                System.out.println("Read byte: " + interpreter.getDataByte((mem.vma_addr & 0xfffff) + i) +
                        " => " + data);
            }
        } else {
            for (int i = 0; i < len; i++) {
                data = (data << 8) + interpreter.getFlashByte(mem.vma_addr + len - i - 1) & 0xff;
            }
        }
        return data;
    }

    private void setValue(String varName, int val, int len) throws UnknownVariableException {
        Location mem = memoryMap.getLocation(varName);
        if (mem == null) throw new UnknownVariableException("Variable does not exist: " + varName);

        int data = val;
        if (mem.vma_addr > 0xfffff) {       
            // write LSB first.
            for (int i = 0; i < len; i++) {
                interpreter.writeDataByte((mem.vma_addr & 0xfffff) + i, (byte) (data & 0xff));
                System.out.println("Wrote byte: " + (data & 0xff));
                data = data >> 8;
            }
        } else {
            for (int i = 0; i < len; i++) {
                interpreter.writeFlashByte(mem.vma_addr + i, (byte) (data & 0xff));
                data = data >> 8;
            }
        }
    }
    
    public int getIntValueOf(String varName) throws UnknownVariableException {
        return getValueOf(varName, 2);
    }

    public int getIntegerLength() {
        return 2;
    }

    public int getVariableAddress(String varName)
            throws UnknownVariableException {
        return 0;
    }

    public String[] getVariableNames() {
        ArrayList<String> symbols = new ArrayList<String>();
        for (Iterator i = memoryMap.getIterator(); i.hasNext();) {
            symbols.add(((Location) i.next()).name);
        }
        return symbols.toArray(new String[0]);
    }

    public void setByteArray(String varName, byte[] data)
            throws UnknownVariableException {
    }

    public void setByteValueOf(String varName, byte newVal)
            throws UnknownVariableException {
        setValue(varName, newVal, 1);
    }

    public void setIntValueOf(String varName, int newVal)
            throws UnknownVariableException {
        setValue(varName, newVal, 2);
    }

    public boolean variableExists(String varName) {
        return memoryMap.getLocation(varName) != null;
    }

    public boolean addMemoryMonitor(int address, int size, MemoryMonitor mm) {
      logger.warn("Not implemented");
      return false;
    }

    public void removeMemoryMonitor(int address, int size, MemoryMonitor mm) {
    }

    public int parseInt(byte[] memorySegment) {
      logger.warn("Not implemented");
      return 0;
    }
}
