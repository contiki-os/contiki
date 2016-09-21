/**
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 * MemoryMonitor
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.core;
import se.sics.mspsim.util.ProxySupport;

public interface MemoryMonitor {

  public void notifyReadBefore(int addr, Memory.AccessMode mode, Memory.AccessType type);
  public void notifyReadAfter(int addr, Memory.AccessMode mode, Memory.AccessType type);

  public void notifyWriteBefore(int dstAddress, int data, Memory.AccessMode mode);
  public void notifyWriteAfter(int dstAddress, int data, Memory.AccessMode mode);

  public static class Adapter implements MemoryMonitor {

    @Override
    public void notifyReadBefore(int addr, Memory.AccessMode mode, Memory.AccessType type) {
    }

    @Override
    public void notifyReadAfter(int addr, Memory.AccessMode mode, Memory.AccessType type) {
    }

    @Override
    public void notifyWriteBefore(int dstAddress, int data, Memory.AccessMode mode) {
    }

    @Override
    public void notifyWriteAfter(int dstAddress, int data, Memory.AccessMode mode) {
    }

  }

  public static class Proxy extends ProxySupport<MemoryMonitor> implements MemoryMonitor {
      public static final Proxy INSTANCE = new Proxy();

      @Override
      public void notifyReadBefore(int address, Memory.AccessMode mode, Memory.AccessType type) {
          MemoryMonitor[] listeners = this.listeners;
          for(MemoryMonitor listener : listeners) {
              listener.notifyReadBefore(address, mode, type);
          }
      }

      @Override
      public void notifyReadAfter(int address, Memory.AccessMode mode, Memory.AccessType type) {
          MemoryMonitor[] listeners = this.listeners;
          for(MemoryMonitor listener : listeners) {
              listener.notifyReadAfter(address, mode, type);
          }
      }

      @Override
      public void notifyWriteBefore(int dstAddress, int data, Memory.AccessMode mode) {
          MemoryMonitor[] listeners = this.listeners;
          for(MemoryMonitor listener : listeners) {
              listener.notifyWriteBefore(dstAddress, data, mode);
          }
      }

      @Override
      public void notifyWriteAfter(int dstAddress, int data, Memory.AccessMode mode) {
          MemoryMonitor[] listeners = this.listeners;
          for(MemoryMonitor listener : listeners) {
              listener.notifyWriteAfter(dstAddress, data, mode);
          }
      }

  }

}
