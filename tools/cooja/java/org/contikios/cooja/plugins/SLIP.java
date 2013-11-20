/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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

package org.contikios.cooja.plugins;

import java.util.ArrayList;

import org.apache.log4j.Logger;

public class SLIP {
  private static Logger logger = Logger.getLogger(SLIP.class);

  private final static byte SLIP_END = (byte)0300;
  private final static byte SLIP_ESC = (byte)0333;
  private final static byte SLIP_ESC_END = (byte)0334;
  private final static byte SLIP_ESC_ESC = (byte)0335;

  /**
   * Wraps packet as SLIP.
   *
   * @param packet Packet data
   */
  public static byte[] asSlip(byte[] packet) {
    ArrayList<Byte> bytes = new ArrayList<Byte>();

    bytes.add(SLIP_END);

    for (byte b: packet) {
      if (b == SLIP_END) {
        bytes.add(SLIP_ESC);
        bytes.add(SLIP_ESC_END);
      } else if (b == SLIP_ESC) {
        bytes.add(SLIP_ESC);
        bytes.add(SLIP_ESC_ESC);
      } else {
        bytes.add(b);
      }
    }

    bytes.add(SLIP_END);

    byte[] arr = new byte[bytes.size()];
    for (int i=0; i < bytes.size(); i++) {
      arr[i] = (bytes.get(i)).byteValue();
    }
    return arr;
  }
}
