/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: CoojaM25P80.java,v 1.1 2008/04/01 08:07:04 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.io.IOException;
import org.apache.log4j.Logger;

import se.sics.mspsim.chip.M25P80;
import se.sics.mspsim.core.*;

public class CoojaM25P80 extends M25P80 {
  private static Logger logger = Logger.getLogger(CoojaM25P80.class);

  public static int SIZE = 1024*1024;
  private byte[] data = new byte[SIZE];
  private long pos;

  public CoojaM25P80(MSP430Core cpu) {
    super(cpu);
    pos = 0;
  }

  public void seek(long pos) throws IOException {
    this.pos = pos;
  }

  public int readFully(byte[] b) throws IOException {
    System.arraycopy(data, (int) pos, b, 0, b.length);
    return b.length;
  }

  public void write(byte[] b) throws IOException {
    System.arraycopy(b, 0, data, (int) pos, b.length);
  }

}
