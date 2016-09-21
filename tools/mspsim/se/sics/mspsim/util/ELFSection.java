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
 * ELFSection
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.util;

import se.sics.mspsim.debug.DwarfReader;

public class ELFSection {

  public static final int TYPE_NULL       =   0;
  public static final int TYPE_PROGBITS   =   1;
  public static final int TYPE_SYMTAB     =   2;
  public static final int TYPE_STRTAB	  =   3;
  public static final int TYPE_RELA	  =   4;
  public static final int TYPE_HASH	  =   5;
  public static final int TYPE_DYNAMIC    =   6;
  public static final int TYPE_NOTE	  =   7;
  public static final int TYPE_NOBITS	  =   8;
  public static final int TYPE_REL	  =   9;
  public static final int TYPE_SHLIB      =  10;
  public static final int TYPE_DYNSYM     =  11;

  public static final int SYMBIND_LOCAL = 0;
  public static final int SYMBIND_GLOBAL = 1;
  public static final int SYMBIND_WEAK = 2;

  public static final int SYMTYPE_NONE = 0;
  public static final int SYMTYPE_OBJECT = 1;
  public static final int SYMTYPE_FUNCTION = 2;
  public static final int SYMTYPE_SECTION = 3;
  public static final int SYMTYPE_FILE = 4;

  public static final String[] typeNames = new String[] {
    "null", "progbits", "symtab", "strtab", "rela",
    "hash", "dynamic", "note", "nobits", "rel", "shlib", "dynsym"
  };

  public static final String[] symTypeNames = new String[] {
    "none", "object", "function", "section", "file"
  };

  int name;
  int type;
  int flags;
  int addr;
  private int offset;
  int size;
  int link;
  int info;
  int addralign;
  private int entrySize;

  ELF elf;

  int pos = 0;
  
  public String getSectionName() {
    if (elf.strTable != null) {
      return elf.strTable.getName(name);
    } else {
      return Integer.toString(name, 16);
    }
  }

  public void reset() {
      pos = 0;
  }

  public int getPosition() {
      return pos;
  }
  
  public int getSize() {
      return size;
  }
  
  public String getName(int i) {
    int pos = getOffset() + i;
    StringBuilder sb = new StringBuilder();
    char c;
    int elfSize = elf.elfData.length;
    while (pos < elfSize && (c = (char) elf.elfData[pos++]) != 0) {
      sb.append(c);
    }
    return sb.toString();
  }

  public String readString() {
      StringBuilder sb = new StringBuilder();
      int c;
      while ((c = readElf8()) != 0) {
          sb.append((char)c);
      }
      return sb.toString();
  }

  public int readElf8() {
      return readElf8(pos++);
  }
  
  public int readElf16() {
      int val = readElf16(pos);
      pos += 2;
      return val;
  }
  
  public int readElf32() {
      int val = readElf32(pos);
      pos += 4;
      return val;
  }

  public long readLEB128() {
      long val = 0;
      /* LSB first always? */
      int b;
      int bitPos = 0;
      do {
          b = readElf8(pos++);
          val = val + ((b & 127) << bitPos);
          bitPos += 7;
      } while ((b & 128) != 0);
      return val;
  }

  public long readLEB128S() {
      long val = 0;
      /* LSB first always? */
      int b;
      int bitPos = 0;
      do {
          b = readElf8(pos++);
          val = val + ((b & 127) << bitPos);
          bitPos += 7;
      } while ((b & 128) != 0);
      long negval = 0x1 << bitPos;
      if (b < 0x40) {
          return val;
      }
      if (ELF.DEBUG || DwarfReader.DEBUG)
          System.out.println("Line: read negative : " + val + " negval: " + negval);
      return -(negval - val);
  }

  public int readElf8(int pos) {
      return elf.readElf8(pos + getOffset());
  }

  public int readElf16(int pos) {
      return elf.readElf16(pos + getOffset());
  }

  public int readElf32(int pos) {
      return elf.readElf32(pos + getOffset());
  }
  
  public int LEB128Size(long val) {
      /* at least one byte, but possibly more */
      return (int) (1 + (val / 128));
  }

  public long readLEB128(int pos) {
      long val = 0;
      /* LSB first always? */
      int b;
      int bitPos = 0;
      do {
          b = readElf8(pos++);
          val = val + ((b & 127) << bitPos);
          bitPos += 7;
      } while ((b & 128) != 0);
      return val;
  }
  
  
  public String toString() {
    String nameStr = getSectionName();
    return "name: " + nameStr +
      "\ntype: " + Integer.toString(type, 16) +
      ((type <= TYPE_DYNSYM) ? " " + typeNames[type] : "") +
      "\nflags: " + Integer.toString(flags, 16) +
      "\naddr: " + Integer.toString(addr, 16) +
      "\noffset: " + Integer.toString(getOffset(), 16) +
      "\nsize: " + Integer.toString(size, 16) +
      "\nlink: " + Integer.toString(link, 16) +
      "\ninfo: " + Integer.toString(info, 16);
  }

public void setEntrySize(int entrySize) {
    this.entrySize = entrySize;
}

public int getEntrySize() {
    return entrySize;
}

public void setOffset(int offset) {
    this.offset = offset;
}

public int getOffset() {
    return offset;
}

} // ELFSection
