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
 * -----------------------------------------------------------------
 *
 * StabDebug
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date: 2010-07-09 23:22:13 +0200 (Fri, 09 Jul 2010) $
 *           $Revision: 717 $
 */

package se.sics.mspsim.debug;
import java.util.ArrayList;

import se.sics.mspsim.util.DebugInfo;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.ELFDebug;
import se.sics.mspsim.util.ELFSection;
import se.sics.mspsim.util.Utils;

public class StabDebug implements ELFDebug {

  private Stab[] stabs;

  public static final int N_FUN = 0x24;
  public static final int N_STSYM = 0x26; // Data segment file-scope variable; 
  public static final int N_LCSYM = 0x28; // BSS segment file-scope variable;
  public static final int N_REG_PARAM= 0x40;
  public static final int N_VAR_PARAM= 0xa0;
  public static final int N_SLINE = 0x44;
  public static final int N_SO = 0x64; // filename and path
  public static final int N_LSYM = 0x80; // stack var, typdef or struct

  public static final boolean DEBUG = false;

  ELFSection dbgStab;
  ELFSection dbgStabStr;

  public StabDebug(ELF elf, ELFSection stab, ELFSection stabstr) {
    dbgStab = stab;
    dbgStabStr = stabstr;

    int len = dbgStab.getSize();
    int count = len / dbgStab.getEntrySize();
    int addr = dbgStab.getOffset();

    if (DEBUG) {
      System.out.println("Number of stabs:" + count);
    }
    stabs = new Stab[count];
    for (int i = 0, n = count; i < n; i++) {
      elf.setPos(addr);
      int nI = elf.readElf32();
      String stabData = elf.dbgStabStr.getName(nI);
      int type = elf.readElf8();
      int other = elf.readElf8();
      int desc = elf.readElf16();
      int value = elf.readElf32();
      stabs[i] = new Stab(stabData, type, other, desc, value);

      if (DEBUG) {
	System.out.println("Stab: " + Utils.hex8(type) +
			   " '" + stabData + "' o:" + other
			   + " d:" + desc + " v:" + value);
      }
      addr += dbgStab.getEntrySize();
    }
    // getStabFiles();
  }

  public StabFile[] getStabFiles() {
      ArrayList<StabFile> files = new ArrayList<StabFile>();
      StabFile currentFile = null;
      for (int i = 0, n = stabs.length; i < n; i++) {
          Stab stab = stabs[i];
          switch(stab.type) {
          case N_SO:
              if (currentFile == null || currentFile.startAddress != stab.value) {
                  /* end of file ? */
                  currentFile = new StabFile();
                  files.add(currentFile);
                  currentFile.startAddress = stab.value;
                  currentFile.stabIndex = i;
                  currentFile.handleStabs(stabs);
                  System.out.println("Found: " + currentFile);
              }
              break;
          }
      }
      return files.toArray(new StabFile[files.size()]);
  }
  
  
  /* Just pick up file + some other things */
  public DebugInfo getDebugInfo(int address) {
    String currentPath = null;
    String currentFile = null;
    String currentFunction = null;
    int lastAddress = 0;
    int currentLine = 0;
    int currentLineAdr = 0;
    for (int i = 0, n = stabs.length; i < n; i++) {
      Stab stab = stabs[i];
      switch(stab.type) {
      case N_SO:
	if (stab.value < address) {
	  if (stab.data != null && stab.data.endsWith("/")) {
	    currentPath = stab.data;
	    lastAddress = stab.value;
	    currentFunction = null;
	  } else {
	    currentFile = stab.data;
	    lastAddress = stab.value;
	    currentFunction = null;
	  }
	} else {
	  /* requires sorted order of all file entries in stab section */
	  if (DEBUG) {
	    System.out.println("FILE: Already passed address..." +
			       currentPath + " " +
			       currentFile + " " + currentFunction);
	  }
	  return null;
	}
	break;
      case N_SLINE:
	if (currentPath != null) { /* only files with path... */
	  if (currentLineAdr < address) {
	    currentLine = stab.desc;
	    currentLineAdr = lastAddress + stab.value;
	    if (currentLineAdr >= address) {
	      // Finished!!!
	      if (DEBUG) {
		System.out.println("File: " + currentPath + " " + currentFile);
		System.out.println("Function: " + currentFunction);
		System.out.println("Line No: " + currentLine);
	      }
	      return new DebugInfo(currentLine, currentPath, currentFile,
				   currentFunction);
	    }
	  }
	}
	break;
      case N_FUN:
	if (stab.value < address) {
	  currentFunction = stab.data;
	  lastAddress = stab.value;
	} else {
	  if (DEBUG) {
	    System.out.println("FUN: Already passed address...");
	  }
	  return null;
	}
	break;
      }
    }
    return null;
  }

  public ArrayList<Integer> getExecutableAddresses() {
    ArrayList<Integer> allAddresses = new ArrayList<Integer>();

    int address = Integer.MAX_VALUE;

    String currentPath = null;
    String currentFile = null;
    String currentFunction = null;
    int lastAddress = 0;
//    int currentLine = 0;
    int currentLineAdr = 0;
    for (Stab stab : stabs) {
      switch(stab.type) {
      case N_SO:
        if (stab.value < address) {
          if (stab.data != null && stab.data.endsWith("/")) {
            currentPath = stab.data;
            lastAddress = stab.value;
            allAddresses.add(new Integer(lastAddress));
            currentFunction = null;
          } else {
            currentFile = stab.data;
            lastAddress = stab.value;
            allAddresses.add(new Integer(lastAddress));
            currentFunction = null;
          }
        } else {
          /* requires sorted order of all file entries in stab section */
          if (DEBUG) {
            System.out.println("FILE: Already passed address..." +
                currentPath + " " +
                currentFile + " " + currentFunction);
          }
          return allAddresses;
        }
        break;
      case N_SLINE:
        if (currentPath != null) { /* only files with path... */
          if (currentLineAdr < address) {
//            currentLine = stab.desc;
            currentLineAdr = lastAddress + stab.value;
            allAddresses.add(new Integer(currentLineAdr));
            /*if (currentLineAdr >= address) {
              // Finished!!!
              if (DEBUG) {
                System.out.println("File: " + currentPath + " " + currentFile);
                System.out.println("Function: " + currentFunction);
                System.out.println("Line No: " + currentLine);
              }
              return new DebugInfo(currentLine, currentPath, currentFile,
                  currentFunction);
            }*/
          }
        }
        break;
      case N_FUN:
        if (stab.value < address) {
          currentFunction = stab.data;
          lastAddress = stab.value;
          allAddresses.add(new Integer(lastAddress));
        } else {
          if (DEBUG) {
            System.out.println("FUN: Already passed address...");
          }
          return allAddresses;
        }
        break;
      }
    }
    return allAddresses;
}

  public String[] getSourceFiles() {
    String currentPath = null;
    String currentFile = null;
    ArrayList<String> sourceFiles = new ArrayList<String>();

    for (Stab stab : stabs) {
      if (stab.type == N_SO) {
        if (stab.data != null && stab.data.endsWith("/")) {
          currentPath = stab.data;
        } else {
          currentFile = stab.data;

          if (currentFile != null && currentFile.length() > 0) {
            if (currentPath == null) {
              sourceFiles.add(currentFile);
            } else {
              sourceFiles.add(currentPath + currentFile);
            }
          }
        }
      }
    }

    String[] sourceFilesArray = new String[sourceFiles.size()];
    for (int i=0; i < sourceFilesArray.length; i++) {
      sourceFilesArray[i] = sourceFiles.get(i);
    }

    return sourceFilesArray;
  }

  public static class Stab {

    public String data;
    public int type;
    public int other;
    public int desc;
    public int value;

    Stab(String data, int type, int other, int desc, int value) {
      this.data = data;
      this.type = type;
      this.other = other;
      this.desc = desc;
      this.value = value;
    }
    
   public String toString() {
        return "" + Integer.toHexString(type) + " " + data +
            "   [" + other + "," + desc + "," + value + "]";
    }
  }

} // StabDebug
