/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * DbgInstruction
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;

public class DbgInstruction {

  private String asmLine;
  private String regs;
  private String function;
  private int instruction;
  private int size;
  private int pos;

  private boolean extWord = false;
  
  public DbgInstruction() {
  }
  

  public boolean isExtensionWord() {
      return extWord;
  }
  
  public void setPos(int p) {
    pos = p;
  }

  public int getPos() {
    return pos;
  }

  public void setASMLine(String line) {
    asmLine = line;
  }

  public void setRegs(String regs) {
    this.regs = regs;
  }

  public void setInstruction(int instruction, int size) {
    this.instruction = instruction;
    this.size = size;
  }

  public int getSize() {
    return size;
  }

  public int getInstruction() {
    return instruction;
  }

  public String getASMLine(boolean showregs) {
    if (showregs) return getASMLine();
    return asmLine;
  }

  public String getASMLine() {
    return asmLine + "\t" + regs;
  }

  public void setFunction(String fkn) {
    function = fkn;
  }

  public String getFunction() {
    return function;
  }

  public String toString() {
    return getASMLine();
  }


  public void setExtWord(boolean b) {
      extWord = b;
  }

}
