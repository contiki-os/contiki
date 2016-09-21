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
 * Memory
 *
 * Author  : Joakim Eriksson
 * Created : Sept 15 22:00:00 2008
 */
package se.sics.mspsim.core;

public interface Memory {

    public static final int SEGMENT_SIZE = 256;

    public enum AccessType {
        READ,    /* normal read */
        EXECUTE, /* instruction execution read */
        ARG,     /* arguments for execution */
        WRITE    /* write */
    };

    public enum AccessMode {
        BYTE(1, 8, 0xff),
        WORD(2, 16, 0xffff),
        WORD20(4, 20, 0xfffff);

        public final int bytes;
        public final int bitSize;
        public final int mask;
        public final int msb;

        AccessMode(int bytes, int bitSize, int mask) {
            this.bytes = bytes;
            this.bitSize = bitSize;
            this.mask = mask;
            this.msb = 1 << (bitSize - 1);
        }
    };

    public int read(int address, AccessMode mode, AccessType type) throws EmulationException;
    public void write(int dstAddress, int data, AccessMode mode) throws EmulationException;

    public int get(int address, AccessMode mode);
    public void set(int address, int data, AccessMode mode);

}
