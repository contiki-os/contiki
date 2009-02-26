/*
 * Copyright (c) 2007, Swedish Institute of Computer Science. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: Watchpoint.java,v 1.1 2009/02/26 13:47:38 fros4943 Exp $
 */

package se.sics.chakana.eventpoints;

import java.util.Arrays;
import org.apache.log4j.Logger;

import se.sics.cooja.Mote;

/**
 * A watchpoint watches a memory area, such as a variable, and triggers at changes.
 *
 * @author Fredrik Osterlind
 */
public class Watchpoint implements Eventpoint {
  private static Logger logger = Logger.getLogger(Watchpoint.class);
  private int myID = 0;

  private Mote mote;
  private int address;
  private int size;
  byte[] initialMemory;
  
  private String reason;
  
  public Watchpoint(Mote mote, int address, int size) {
    this.mote = mote;
    this.address = address;
    this.size = size;

    logger.debug("Fetching initial memory");
    initialMemory = mote.getMemory().getMemorySegment(address, size);
    
    // TODO Throw exception if memory area not valid
  }
  
  public boolean evaluate() {
    byte[] currentMemory = mote.getMemory().getMemorySegment(address, size);
    boolean shouldBreak = !Arrays.equals(initialMemory, currentMemory);

    if (shouldBreak) {
      reason = "Memory interval " + "0x" + Integer.toHexString(address) + ":" + size + " changed";
    }
    return shouldBreak;
  }

  public String getMessage() {
    return reason;
  }
  
  public String toString() {
    return "Memory change breakpoint: " + "0x" + Integer.toHexString(address) + ":" + size + " @ " + mote;
  }

  public void setID(int id) {
    myID = id;
  }

  public int getID() {
    return myID;
  }
}
