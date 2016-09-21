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
 * IHexReader
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.util;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import se.sics.mspsim.config.MSP430f1611Config;
import se.sics.mspsim.core.MSP430;

public class IHexReader {

  // 64k tmp ram!
  private int tmpMemory[] = new int[64 * 1024];

  /**
   * Creates a new <code>IHexReader</code> instance.
   *
   */
  public IHexReader() {
  }

  public boolean readFile(int memory[], String file) {
    for (int i = 0, n = tmpMemory.length; i < n; i++) {
      tmpMemory[i] = -1;
    }
    try {
      BufferedReader bInput = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
      String line;
      boolean terminate = false;
      while ((line = bInput.readLine()) != null && !terminate) {
	if (line.charAt(0) != ':') {
	  System.out.println("Not an IHex file?! " + line.charAt(0));
	  bInput.close();
	  return false;
	}
	int size = hexToInt(line.charAt(1)) * 0x10 + hexToInt(line.charAt(2));
	int adr =  hexToInt(line.charAt(3)) * 0x1000 +
	  hexToInt(line.charAt(4)) * 0x100 + hexToInt(line.charAt(5)) * 0x10 +
	  hexToInt(line.charAt(6));
	int type = hexToInt(line.charAt(7)) * 0x10 + hexToInt(line.charAt(8));
// 	System.out.println("Block:  size: " + size + "=>  address: " +
// 			   hex(adr) +   "  type: " + type);

	// Termination !!!
	if (type == 0x01) {
	  System.out.println("IHEX file ended (termination)");
	  terminate = true;
	} else {
	  int index = 9;
	  for (int i = 0, n = size; i < n; i++) {
	    tmpMemory[adr + i] = (hexToInt(line.charAt(index++)) * 0x10 +
				  hexToInt(line.charAt(index++)));
	  }

//	  int checksum = (byte) hexToInt(line.charAt(index++)) * 0x10
//	    + hexToInt(line.charAt(index++));
	  //	System.out.println("Checksum: " + checksum);
	}
      }
      bInput.close();

      // Write all data that we got in to the real memory!!!
      System.out.println("Writing to memory!");
      for (int i = 0, n = tmpMemory.length; i < n; i++) {
	if (tmpMemory[i] != -1) {
	  memory[i] = tmpMemory[i];
	}
      }

      return true;
    } catch (IOException ioe) {
      ioe.printStackTrace();
    }
    return false;
  }

  private static String hex(int data) {
    return Integer.toString(data, 16);
  }

  private static int hexToInt(char c) {
    if (c >= '0' && c <= '9') {
      return c - '0';
    } else {
      return c - 'A' + 10;
    }
  }

  public static void main(String[] args) throws Exception {
    IHexReader reader = new IHexReader();
    int data = 0x84;
    System.out.println("RRA: " + hex((data & 0x80) + (data >> 1)));

//     for (int i = 0, n = 0x3ff; i < n; i++) {
//       int val = (i & 0x200) == 0 ? i : -(0x200 - (i & 0x1ff));
//     System.out.println("Value: " + val);
//     }
//     System.out.println("T ^ T =>  " + (true ^ true) +
// 		       " T ^ F => " + (false ^ true));

    MSP430 cpu = new MSP430(0, new ComponentRegistry(), new MSP430f1611Config());
    int[] memory = cpu.memory;
    reader.readFile(memory, args[0]);
    cpu.reset();
    cpu.cpuloop();
  }


}
