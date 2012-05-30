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
 */

package se.sics.cooja.mspmote.interfaces;

import java.io.IOException;
import java.util.Arrays;

import se.sics.coffee.CoffeeConfiguration;
import se.sics.coffee.CoffeeImage;
import se.sics.mspsim.chip.M25P80;
import se.sics.mspsim.chip.Storage;
import se.sics.mspsim.core.MSP430Core;

public class CoojaM25P80 extends M25P80 implements CoffeeImage {

  public static int SIZE = 1024*1024;
  private byte[] data = new byte[SIZE];
  private long pos;
  private Storage storage = new Storage() {

    @Override
    public int read(long pos, byte[] buffer) throws IOException {
        System.arraycopy(data, (int) pos, buffer, 0, buffer.length);
        return buffer.length;
    }

    @Override
    public int read(long pos, byte[] buffer, int offset, int len)
            throws IOException {
        System.arraycopy(data, (int) pos, buffer, offset, len);
        return len;
    }

    @Override
    public void write(long pos, byte[] buffer) throws IOException {
        System.arraycopy(buffer, 0, data, (int) pos, buffer.length);
    }

    @Override
    public void write(long pos, byte[] buffer, int offset, int len)
            throws IOException {
        System.arraycopy(buffer, offset, data, (int) pos, len);
    }

    @Override
    public long getMaxSize() {
        return SIZE;
    }

    @Override
    public void setMaxSize(long size) {
        // Ignore
    }

    @Override
    public void close() {
        // Nothing to close
    }

    @Override
    public String info() {
        return CoojaM25P80.class.getName();
    }

  };

  private static CoffeeConfiguration COFFEE_CONF;
  static {
    /* XXX Current implementation only allows for a single coffee configuration */
    try {
      COFFEE_CONF = new CoffeeConfiguration("sky.properties");
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  public CoojaM25P80(MSP430Core cpu) {
    super(cpu);
    pos = 0;
    setStorage(storage);
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

  /**
   * XXX Coffee specific: uses start offset 
   * @see se.sics.coffee.CoffeeImage#erase(int, int)
   */
  public void erase(int size, int offset) throws IOException {
    Arrays.fill(data, COFFEE_CONF.startOffset + offset, size, (byte)0);
  }

  /**
   * XXX Coffee specific: uses start offset 
   * @see se.sics.coffee.CoffeeImage#getConfiguration()
   */
  public CoffeeConfiguration getConfiguration() {
    return COFFEE_CONF;
  }

  /**
   * XXX Coffee specific: uses start offset 
   * @see se.sics.coffee.CoffeeImage#read(byte[], int, int)
   */
  public void read(byte[] bytes, int size, int offset) throws IOException {
    System.arraycopy(data, COFFEE_CONF.startOffset + offset, bytes, 0, size);
  }

  /**
   * XXX Coffee specific: uses start offset 
   * @see se.sics.coffee.CoffeeImage#write(byte[], int, int)
   */
  public void write(byte[] bytes, int size, int offset) throws IOException {
    System.arraycopy(bytes, 0, data, COFFEE_CONF.startOffset + offset, size);
  }

}
