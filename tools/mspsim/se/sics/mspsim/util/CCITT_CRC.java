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
 * -----------------------------------------------------------------
 *
 * Author  : Joakim Eriksson
 */

package se.sics.mspsim.util;

/* basic CRC-CCITT code */
public class CCITT_CRC {
  int crc;

  public int getCRC() {
    return crc;
  }

  /* this will only work with zero... */
  public void setCRC(int val) {
    crc = val;
  }
  
  public void clr() {
    crc = 0xffff;
  }

  public void addBitrev(int data) {
      add(bitrev(data));
  }
  
  public int getCRCLow() {
      return bitrev(crc & 0xff);
  }

  public int getCRCHi() {
      return bitrev(crc >> 8);
  }

  
  public int add(int data) {
    int newCrc = ((crc >> 8) & 0xff) | (crc << 8) & 0xffff;
    newCrc ^= (data & 0xff);
    newCrc ^= (newCrc & 0xff) >> 4;
    newCrc ^= (newCrc << 12) & 0xffff;
    newCrc ^= (newCrc & 0xff) << 5;
    crc = newCrc & 0xffff;
    return crc;
  }

  public int getCRCBitrev() {
      return getCRCLow() + (getCRCHi() << 8);
  }
  
  public static final String hex = "0123456789abcdef";

  private static int bitrev(int data) {
      return ((data << 7) & 0x80) | ((data << 5) & 0x40) |
      (data << 3) & 0x20 | (data << 1) & 0x10 |
      (data >> 7) & 0x01 | (data >> 5) & 0x02 |
      (data >> 3) & 0x04 | (data >> 1) & 0x08;
  }
  
  public static void main(String[] args) {
      CCITT_CRC crc = new CCITT_CRC();
      crc.setCRC(0x0);
      
      String packet = "41c8a7cdabffff777777feff7777027b493a0201ff777777870015d300000000fe80000000000000007777fffe777777";
      packet = "41cc39adba020202000274120007070700077412007b333a8700939800000000fe80000000000000021274020002020201020012740700070707000000000000";
      packet = "41c816adbaffff09090900097412007a3b3a029b02d1f5e0000011aaaa00000000000002127401000101010500020000040005080a011004";
      for (int i = 0; i < packet.length(); i += 2) {
          int data = (hex.indexOf(packet.charAt(i)) << 4) + hex.indexOf(packet.charAt(i + 1)); 
          crc.addBitrev(data);
      }
      
      //0x901d
      System.out.println("CRC: " + Utils.hex8(crc.getCRCHi()) + Utils.hex8(crc.getCRCLow()));
  }
  
  
}
