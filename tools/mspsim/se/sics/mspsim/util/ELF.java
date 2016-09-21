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
 * ELF
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.util;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import se.sics.mspsim.debug.DwarfReader;
import se.sics.mspsim.debug.StabDebug;

public class ELF {

  // private static final int EI_NIDENT = 16;
  private static final int EI_ENCODING = 5;
  private static final int[] MAGIC = new int[] {0x7f, 'E', 'L', 'F'};
  
  public static final boolean DEBUG = false;
  
  
  boolean encMSB = true;
  int type;
  int machine;
  int version;
  int entry;
  int phoff;
  int shoff;
  int flags;
  int ehsize;
  int phentsize;
  int phnum;
  int shentsize;
  int shnum;
  int shstrndx;

  byte[] elfData;
  private int pos = 0;

  private ELFSection sections[];
  private ELFProgram programs[];
  private ArrayList<FileInfo> files = new ArrayList<FileInfo>();

  ELFSection strTable;
  ELFSection symTable;
  ELFSection dbgStab;
  public ELFSection dbgStabStr;

  ELFDebug debug;

  public ELF(byte[] data) {
    elfData = data;
    setPos(0);
  }

  /* check if the file exists and is an ELF file */
  public static boolean isELF(File file) {
    try {
      InputStream input = new BufferedInputStream(new FileInputStream(file));
      for (int i = 0; i < MAGIC.length; i++) {
        if (MAGIC[i] != input.read()) {
          input.close();
          return false;
        }
      }
      input.close();
      return true;
    } catch(IOException ioe) {
      // ignore and return false - this is not an elf.
      return false;
    }
  }

  private void readHeader() throws ELFException {
    for (int i = 0; i < MAGIC.length; i++) {
      if (elfData[i] != (byte) (MAGIC[i] & 0xff)) {
        throw new ELFException("Not an elf file");
      }
    }
    
    if (elfData[EI_ENCODING] == 2) {
      encMSB = true;
    } else if (elfData[EI_ENCODING] == 1) {
      encMSB = false;
    } else {
      throw new ELFException("Illegal encoding: " + elfData[EI_ENCODING]);
    }
    
    setPos(getPos() + 16);
    type = readElf16();
    machine = readElf16();
    version = readElf32();
    entry = readElf32();
    phoff = readElf32();
    shoff = readElf32();
    flags = readElf32();
    ehsize = readElf16();
    phentsize = readElf16();
    phnum = readElf16();
    shentsize = readElf16();
    shnum = readElf16();
    shstrndx = readElf16();

    if (DEBUG) {
      System.out.println("-- ELF Header --");
      System.out.println("type: " + Integer.toString(type, 16));
      System.out.println("machine: " + Integer.toString(machine, 16));
      System.out.println("version: " + Integer.toString(version, 16));
      System.out.println("entry: " + Integer.toString(entry, 16));
      System.out.println("phoff: " + Integer.toString(phoff, 16));
      System.out.println("shoff: " + Integer.toString(shoff, 16));
      System.out.println("flags: " + Integer.toString(flags, 16));
      System.out.println("ehsize: " + Integer.toString(ehsize, 16));
      System.out.println("phentsize: " + Integer.toString(phentsize, 16));
      System.out.println("phentnum: " + Integer.toString(phnum, 16));
      System.out.println("shentsize: " + Integer.toString(shentsize, 16));
      System.out.println("shentnum: " + Integer.toString(shnum, 16));
      System.out.println("shstrndx: " + Integer.toString(shstrndx, 16));
    }
  }

  private ELFSection readSectionHeader() {
    ELFSection sec = new ELFSection();
    sec.name = readElf32();
    sec.type = readElf32();
    sec.flags = readElf32();
    sec.addr = readElf32();
    sec.setOffset(readElf32());
    sec.size = readElf32();
    sec.link = readElf32();
    sec.info = readElf32();
    sec.addralign = readElf32();
    sec.setEntrySize(readElf32());
    sec.elf = this;
    return sec;
  }

  private ELFProgram readProgramHeader() {
    ELFProgram pHeader = new ELFProgram();
    pHeader.type = readElf32();
    pHeader.offset = readElf32();
    pHeader.vaddr = readElf32();
    pHeader.paddr = readElf32();
    pHeader.fileSize = readElf32();
    pHeader.memSize = readElf32();
    pHeader.flags = readElf32();
    pHeader.align = readElf32();
    // 8 * 4 = 32
    if (phentsize > 32) {
      System.out.println("Program Header Entry SIZE differs from specs?!?!??!?!?***");
    }
    return pHeader;
  }

  public int getSectionCount() {
      return shnum;
  }

  public ELFSection getSection(int index) {
      return sections[index];
  }

  public int readElf32() {
      int val = readElf32(getPos());
      setPos(getPos() + 4);
      return val;
  }

  public int readElf16() {
      int val = readElf16(getPos());
      setPos(getPos() + 2);
      return val;
  }
  
  public int readElf8() {
      int val = readElf16(getPos());
      setPos(getPos() + 1);
      return val;
  }
  
  int readElf32(int pos) {
    int b = 0;
    if (encMSB) {
      b = (elfData[pos++] & 0xff) << 24 |
	((elfData[pos++] & 0xff) << 16) |
	((elfData[pos++] & 0xff) << 8) |
	(elfData[pos++] & 0xff);
    } else {
      b = (elfData[pos++] & 0xff) |
	((elfData[pos++] & 0xff) << 8) |
	((elfData[pos++] & 0xff) << 16) |
	((elfData[pos++] & 0xff) << 24);
    }
    return b;
  }

  int readElf16(int pos) {
    int b = 0;
    if (encMSB) {
      b = ((elfData[pos++] & 0xff) << 8) |
	(elfData[pos++] & 0xff);
    } else {
      b = (elfData[pos++] & 0xff) |
	((elfData[pos++] & 0xff) << 8);
    }
    return b;
  }

  int readElf8(int pos) {
    return elfData[pos++] & 0xff;
  }

  public static void printBytes(String name, byte[] data) {
    System.out.print(name + " ");
    for (byte element : data) {
      System.out.print("" + (char) element);
    }
    System.out.println("");
  }

  private void readSections() {
    setPos(shoff);

    sections = new ELFSection[shnum];
    for (int i = 0, n = shnum; i < n; i++) {
      sections[i] = readSectionHeader();
      if (sections[i].type == ELFSection.TYPE_SYMTAB) {
	symTable = sections[i];
      }
      if (i == shstrndx) {
	strTable = sections[i];
      }
    }

    boolean readDwarf = false;
    /* Find sections */
    for (int i = 0, n = shnum; i < n; i++) {
        String name = sections[i].getSectionName();
        if (DEBUG) {
          System.out.println("ELF-Section: " + name);
        }
      if (".stabstr".equals(name)) {
	dbgStabStr = sections[i];
      }
      if (".stab".equals(name)) {
	dbgStab = sections[i];
      }
      if (".debug_aranges".equals(name) || 
          ".debug_line".equals(name)) {
          readDwarf = true;
      }
    }
    if (readDwarf) {
        DwarfReader dwarf = new DwarfReader(this);
        dwarf.read();
        debug = dwarf;
    }

  }

  private void readPrograms() {
    setPos(phoff);
    programs = new ELFProgram[phnum];
    for (int i = 0, n = phnum; i < n; i++) {
      programs[i] = readProgramHeader();
      if (DEBUG) {
	System.out.println("-- Program header --\n" + programs[i].toString());
      }
    }
  }

  public void readAll() throws ELFException {
    readHeader();
    readPrograms();
    readSections();
    if (dbgStab != null) {
      debug = new StabDebug(this, dbgStab, dbgStabStr);
    }
  }

  public void loadPrograms(int[] memory) {
    for (int i = 0, n = phnum; i < n; i++) {
      // paddr or vaddr???
      loadBytes(memory, programs[i].offset, programs[i].paddr,
		programs[i].fileSize, programs[i].memSize);
    }
  }

  private void loadBytes(int[] memory, int offset, int addr, int len,
			 int fill) {
    if (DEBUG) {
      System.out.println("Loading " + len + " bytes into " +
             Integer.toString(addr, 16) + " fill " + fill);
    }
    for (int i = 0, n = len; i < n; i++) {
      memory[addr++] = elfData[offset++] & 0xff;
    }
    if (fill > len) {
      int n = fill - len;
      if (n + addr > memory.length) {
	n = memory.length - addr;
      }
      for (int i = 0; i < n; i++) {
	memory[addr++] = 0;
      }
    }
  }

  public ELFDebug getDebug() {
    return debug;
  }

  public DebugInfo getDebugInfo(int adr) {
      if (debug != null) {
          return debug.getDebugInfo(adr);
      }
      return null;
  }

  public String lookupFile(int address) {
    if (debug != null) {
        DebugInfo di = debug.getDebugInfo(address);
        if (di != null) {
            return di.getFile();
        }
    }
    for (int i = 0; i < files.size(); i++) {
      FileInfo fi = files.get(i);
      if (address >= fi.start && address <= fi.end) {
        return fi.name;
      }
    }
    return null;
  }

  public MapTable getMap() {
    MapTable map = new MapTable();
    int sAddrHighest = -1;
    boolean foundEnd = false;

    ELFSection name = sections[symTable.link];
    int len = symTable.size;
    int count = len / symTable.getEntrySize();
    int addr = symTable.getOffset();
    String currentFile = null;
    if (DEBUG) {
      System.out.println("Number of symbols:" + count);
    }
    int currentAddress = 0;
    for (int i = 0, n = count; i < n; i++) {
      setPos(addr);
      int nI = readElf32();
      String sn = name.getName(nI);
      int sAddr = readElf32();
      int size = readElf32();
      int info = readElf8();
      int bind = info >> 4;
      int type = info & 0xf;

      if (type == ELFSection.SYMTYPE_NONE && sn != null){
        if ("Letext".equals(sn)) {
          if (currentFile != null) {
            files.add(new FileInfo(currentFile, currentAddress, sAddr));
            currentAddress = sAddr;
          }
        } else if (!sn.startsWith("_")) {
            map.setEntry(new MapEntry(MapEntry.TYPE.variable, sAddr, 0, sn, currentFile,
                    false));
        }
      }
      if (type == ELFSection.SYMTYPE_FILE) {
	currentFile = sn;
      }

      if (DEBUG) {
        System.out.println("Found symbol: " + sn + " at " +
        		   Integer.toString(sAddr, 16) + " bind: " + bind +
        		   " type: " + type + " size: " + size);
      }

      if (sAddr > 0 && sAddr < 0x100000) {
	String symbolName = sn;
	
	if (sAddr < 0x5c00 && sAddr > sAddrHighest && !sn.equals("__stack")) {
	  sAddrHighest = sAddr;
	}
//	if (bind == ELFSection.SYMBIND_LOCAL) {
//	  symbolName += " (" + currentFile + ')';
//	}
	if ("_end".equals(symbolName)) {
      foundEnd = true;
	  map.setHeapStart(sAddr);
	} else if ("__stack".equals(symbolName)){
	  map.setStackStart(sAddr);
	}


	if (type == ELFSection.SYMTYPE_FUNCTION) {
          String file = lookupFile(sAddr);
          if (file == null) {
            file = currentFile;
          }
	  map.setEntry(new MapEntry(MapEntry.TYPE.function, sAddr, 0, symbolName, file,
	      bind == ELFSection.SYMBIND_LOCAL));
	} else if (type == ELFSection.SYMTYPE_OBJECT) {
          String file = lookupFile(sAddr);
          if (file == null) {
            file = currentFile;
          }
	  map.setEntry(new MapEntry(MapEntry.TYPE.variable, sAddr, size, symbolName, file,
	      bind == ELFSection.SYMBIND_LOCAL));
	} else {
	  if (DEBUG) {
	    System.out.println("Skipping entry: '" + symbolName + "' @ 0x" + Integer.toString(sAddr, 16) + " (" + currentFile + ")");
	  }
	}

      }
      addr += symTable.getEntrySize();
    }

  if (!foundEnd && sAddrHighest > 0) {
    System.out.printf("Warning: Unable to parse _end symbol. I'm guessing that heap starts at 0x%05x\n", sAddrHighest);
    map.setHeapStart(sAddrHighest);
  }

    return map;
  }

  public static ELF readELF(String file) throws IOException {
    DataInputStream input = new DataInputStream(new FileInputStream(file));
    ByteArrayOutputStream baous = new ByteArrayOutputStream();
    byte[] buf = new byte[2048];
    for(int read; (read = input.read(buf)) != -1; baous.write(buf, 0, read)) {
      ;
    }
    input.close();
    buf = null;
    byte[] data = baous.toByteArray();
    if (DEBUG) {
      System.out.println("Length of data: " + data.length);
    }

    ELF elf = new ELF(data);
    elf.readAll();

    return elf;
  }

  public static void main(String[] args) throws Exception {
    ELF elf = readELF(args[0]);

    if (args.length < 2) {
      for (int i = 0, n = elf.shnum; i < n; i++) {
        if (DEBUG) {
          System.out.println("-- Section header " + i + " --\n" + elf.sections[i]);
        }
        if (".stab".equals(elf.sections[i].getSectionName()) ||
            ".stabstr".equals(elf.sections[i].getSectionName())) {
          int adr = elf.sections[i].getOffset();
          if (DEBUG) {
            System.out.println(" == Section data ==");
          }
          for (int j = 0, m = 2000; j < m; j++) {
            if (DEBUG) {
              System.out.print((char) elf.elfData[adr++]);
              if (i % 20 == 19) {
                System.out.println();
              }
            }
          }
        }
        System.out.println();
      }
    }
    elf.getMap();
    if (args.length > 1) {
      DebugInfo dbg = elf.getDebugInfo(Integer.parseInt(args[1]));
      if (dbg != null) {
	System.out.println("File: " + dbg.getFile());
	System.out.println("Function: " + dbg.getFunction());
	System.out.println("LineNo: " + dbg.getLine());
      }
    }
  }

  public void setPos(int pos) {
    this.pos = pos;
  }

  public int getPos() {
    return pos;
  }

  private static class FileInfo {
      public final String name;
      public final int start;
      public final int end;

      FileInfo(String name, int start, int end) {
          this.name = name;
          this.start = start;
          this.end = end;
      }

  }

} // ELF
