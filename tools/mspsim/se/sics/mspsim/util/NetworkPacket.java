/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: $
 *
 * -----------------------------------------------------------------
 *
 * NetworkPacket
 *
 * Author  : Joakim Eriksson
 * Created : 31 mar 2008
 * Updated : $Date:$
 *           $Revision:$
 */

package se.sics.mspsim.util;
import java.util.Enumeration;
import java.util.Hashtable;

public class NetworkPacket {
  byte[] data;
  String description;
  public static final String IPv6 = 
    "version:4=6|trafficClass:8|flowLabel:20" +
    "|payloadLength:16|nextHeader:8|hopLimit:8" +
    "|sourceAddress:128|destinationAddress:128";

  Hashtable<String,Field> fields = new Hashtable<String,Field>();

  private static class Field {
    String name;
    int pos;
    int size;
    int mask = 0;
    int value = 0;

    Field(String name, int pos, int size) {
      this.name = name;
      this.pos = pos;
      this.size = size;
    }

    public void setMatchMask(int mask, int value) {
      this.mask = mask;
      this.value = value;
    }
    
    public String toString() {
      return name + ":" + pos + "-" + (pos + size - 1) + 
      (mask != 0 ? "=" + value : "");
    }
  }


  public NetworkPacket(String pattern) {
      description = pattern;      
      String[] parts = description.split("\\|");
      int pos = 0;
      for (int i = 0; i < parts.length; i++) {
        String field[] = parts[i].split(":");
        String val = field[1];
        String matchVal = null;
        if (val.indexOf('=') > 0) {
          String[] match = val.split("=");
          val = match[0];
          matchVal = match[1];
        }
        int size = Integer.parseInt(val);
        Field f = new Field(field[0], pos, size);
        if (matchVal != null) {
          int mask = Integer.parseInt(matchVal);
          f.setMatchMask(mask, mask);
        }
        pos += size;
        System.out.println("Adding field: " + f);
        fields.put(f.name, f);
      }
  }


  NetworkPacket(String pattern, Hashtable<String, Field> fields) {
    this.description = pattern;
    this.fields = fields;
  }
  
  public boolean matches(byte[] data) {
    for (Enumeration<Field> iterator = fields.elements(); iterator.hasMoreElements();) {
      Field f = iterator.nextElement();
      if (f.mask != 0) {
        int val = getIntBits(data, f.pos, f.pos + f.size - 1);
        if ((val & f.mask) != f.value) {
          return false;
        } else {
          System.out.println("Field: " + f.name + " matches");
        }
      }
    }
    return true;
  }
  
  /* create a network packet based on a packet with description */
  public NetworkPacket parseData(byte[] data) {
    if (matches(data)) {
      NetworkPacket np = new NetworkPacket(description, fields);
      np.data = data;
      return np;
    }
    return null;
  }

  public int getLength() {
    return data.length;
  }

  public static int getIntBits(byte[] data, int startBit, int endBit) {
    int startByte = startBit >> 8;
    int endByte = endBit >> 8;
    startBit = startBit & 7;
    endBit = endBit & 7;

    int result = 0;
    for (int i = startByte; i < endByte + 1; i++) {
      int bt = data[i] & 0xff;

      if (i == startByte) {
        bt = bt >> startBit;
      }
      if (i == endByte) {
        // Rotate with 0 -- 7
        bt = bt >> (7 - endBit);
      }
      result = (result << 8) | bt;
    }
    return result;
  }

  public int getInt(String field) {
    Field f = fields.get(field);
    return getIntBits(data, f.pos, f.pos + f.size - 1);
  }

  public static void main(String[] args) {
    byte data[] = new byte[] {
        0x61, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    NetworkPacket np = new NetworkPacket(IPv6);
    np = np.parseData(data);
    System.out.println("Version: " + NetworkPacket.getIntBits(np.data, 0, 3));
    System.out.println("Version: " + np.getInt("version"));
  }

}
