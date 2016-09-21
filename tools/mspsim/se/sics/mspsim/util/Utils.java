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
 * Utils
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */
package se.sics.mspsim.util;

public class Utils {
  private static final String str16 = "0000000000000000";

  public static final int BYTE = 0;
  public static final int UBYTE = 1;
  public static final int WORD = 2;
  public static final int UWORD = 3;
  
  public static final int ASCII = 1;
  public static final int HEX = 2;
  public static final int DEC = 3;
  public static final int ASCII_UNMODIFIED = 4;
  public static final int DIS_ASM = 5;
  
  public static int size(int type) {
      return 1 + (type / 2);
  }

  public static String toString(int data, int type, int mode) {
      int size = 1 + type / 2;
      switch (mode) {
      case    ASCII:
          if (data >= 32 && data <= 127) {
              return "" + ((char) data);
          } else {
              return ".";
          }
      case    ASCII_UNMODIFIED:
          return "" + ((char) data);
      case    HEX: 
          return (size == 2 ? Utils.hex16(data):Utils.hex8(data));
      }
      return "" + data;
  }
  
  public static String binary8(int data) {
    String s = Integer.toString(data, 2);
    if (s.length() < 8) {
      s = str16.substring(0, 8 - s.length()) + s;
    }
    return s;
  }

  public static String binary16(int data) {
    String s = Integer.toString(data, 2);
    if (s.length() < 16) {
      s = str16.substring(0, 16 - s.length()) + s;
    }
    return s;
  }

  public static String hex(int data) {
      return Integer.toString(data, 16);
  }

  public static String hex(int data, int len) {
      String s = Integer.toString(data, 16);
      if (s.length() < len) {
        s = str16.substring(0, len - s.length()) + s;
      }
      return s;
  }

  public static String hex8(int data) {
      return hex(data & 0xff, 2);
  }

  public static String hex16(int data) {
      return hex(data & 0xffff, 4);
  }

  public static String hex20(int data) {
      return hex(data & 0xfffff, 5);
  }

  public static String hex24(int data) {
      return hex(data & 0xffffff, 6);
  }

  public static int decodeInt(String value) throws NumberFormatException {
    int radix = 10;
    int index = 0;
    boolean negative = false;
    if (value.startsWith("-")) {
      index++;
      negative = true;
    }

    if (value.startsWith("$", index) || value.startsWith("#", index)) {
      radix = 16;
      index++;
    } else if (value.startsWith("0x", index) || value.startsWith("0X", index)) {
      radix = 16;
      index += 2;
    } else if (value.startsWith("0", index) && value.length() > index + 1) {
      radix = 8;
      index++;
    } else if (value.startsWith("%", index)) {
      radix = 2;
      index++;
    }
    String intValue = value;
    if (radix != 10) {
      if (value.startsWith("-", index)) {
        throw new NumberFormatException("unexpected negative sign: " + value);
      }
      if (negative) {
        intValue = '-' + value.substring(index);
      } else {
        intValue = value.substring(index);
      }
    }
    return Integer.parseInt(intValue, radix);
  }

  public static long decodeLong(String value) throws NumberFormatException {
    int radix = 10;
    int index = 0;
    boolean negative = false;
    if (value.startsWith("-")) {
      index++;
      negative = true;
    }

    if (value.startsWith("$", index) || value.startsWith("#", index)) {
      radix = 16;
      index++;
    } else if (value.startsWith("0x", index) || value.startsWith("0X", index)) {
      radix = 16;
      index += 2;
    } else if (value.startsWith("0", index) && value.length() > index + 1) {
      radix = 8;
      index++;
    } else if (value.startsWith("%", index)) {
      radix = 2;
      index++;
    }
    String longValue = value;
    if (radix != 10) {
      if (value.startsWith("-", index)) {
        throw new NumberFormatException("unexpected negative sign: " + value);
      }
      if (negative) {
        longValue = '-' + value.substring(index);
      } else {
        longValue = value.substring(index);
      }
    }
    return Long.parseLong(longValue, radix);
  }

  /* converts hexa-decimal data in a string to an array of bytes */
  public static byte[] hexconv(String line) {
    if (line == null) {
      return null;
    }
    byte[] data = new byte[line.length() / 2]; 
    int hpos = 0;
    int totVal = 0;
    int dataPos = 0;
    for (int i = 0, n = line.length(); i < n; i++) {
      int val = line.charAt(i);
      if (val >= '0' && val <= '9') {
        val = val - '0';
      } else if (val >= 'a' && val <= 'f') {
        val = val + 10 - 'a';  
      } else if (val >= 'A' && val <= 'F'){
        val = val + 10 - 'A';  
      } else {
        // Not a hexa-decimal format
        return null;
      }

      if (hpos == 0) {
        totVal = val << 4;
        hpos++;
      } else {
        totVal = totVal + val;
        hpos = 0;
        data[dataPos++] = (byte) (totVal & 0xff);
      }
    }
    return data;
  }

  public static boolean equals(byte[] arr1, byte[] arr2) {
    if (arr1 == null && arr2 == null) return true;
    if (arr1 == null || arr2 == null) return false;
    if (arr1.length != arr2.length) return false;
    for (int i = 0; i < arr1.length; i++) {
      if (arr1[i] != arr2[i]) return false;
    }
    return true;
  }


//  public static void main(String[] args) {
//    System.out.println("Hex 47 = " + hex8(47));
//  }

}
