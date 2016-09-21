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
 * $Id $
 *
 * -----------------------------------------------------------------
 *
 * MapEntry
 *
 * Author  : Joakim Eriksson
 * Created : Jan 14 2008
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.util;

public class MapEntry {

  public static enum TYPE {function, variable, module}

  private final TYPE type;
  private final int address;
  private final String name;
  private final String file;
  private final boolean isLocal;
  private int size;
  private int dataAddr;
  private int dataSize;
  private int bssAddr;
  private int bssSize;
  
  public MapEntry(TYPE type, int address, int size, String name, String file, boolean isLocal) {
    this.type = type;
    this.address = address;
    this.name = name;
    this.file = file;
    this.isLocal = isLocal;
    this.size = size;
  }

  void setData(int dataAddr, int dataSize) {
    this.dataAddr = dataAddr;
    this.dataSize = dataSize;
  }

  void setBSS(int bssAddr, int bssSize) {
    this.bssAddr = bssAddr;
    this.bssSize = bssSize;
  }

  void setSize(int size) {
    this.size = size;
  }

  public int getSize() {
    return size;
  }

  public int getDataAddress() {
      return dataAddr;
  }

  public int getDataSize() {
    return dataSize;
  }

  public int getBSSAddress() {
      return bssAddr;
  }

  public int getBSSSize() {
    return bssSize;
  }

  public TYPE getType() {
    return type;
  }

  public int getAddress() {
    return address;
  }

  public String getName() {
    return name;
  }

  public String getFile() {
    return file;
  }
  
  public boolean isLocal() {
    return isLocal;
  }
  
  public String getInfo() {
    StringBuilder sb = new StringBuilder();
    sb.append(name);
    if (file != null) {
      sb.append(" (");
      if (isLocal) sb.append("local in ");
      sb.append(file).append(')'); 
    } else if (isLocal) {
      sb.append(" (local)");
    }
    return sb.toString();    
  }
  
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append('$').append(Utils.hex(address, 4)).append(' ').append(type).append(' ').append(name);
    if (file != null) {
      sb.append(" (");
      if (isLocal) sb.append("local in ");
      sb.append(file).append(')'); 
    } else if (isLocal) {
      sb.append(" (local)");
    }
    return sb.toString();
  }
}
