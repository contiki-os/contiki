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
 * RAMOffsetMemory
 *
 * Author  : Niclas Finne
 * Created : Oct 24 21:47:42 2012
 */

package se.sics.mspsim.core;

import se.sics.mspsim.core.EmulationLogger.WarningType;

class RAMOffsetSegment implements Memory {

    private final MSP430Core core;
    private final int memory[];
    private final int offset;

    RAMOffsetSegment(MSP430Core core, int offset) {
        this.core = core;
        this.memory = core.memory;
        this.offset = offset;
    }

    @Override public int read(int address, AccessMode mode, AccessType type)
            throws EmulationException {
        address += offset;
        int val = memory[address] & 0xff;
        if (mode != AccessMode.BYTE) {
            val |= (memory[address + 1] << 8);
            if ((address & 1) != 0) {
                core.printWarning(WarningType.MISALIGNED_READ, address);
            }
            if (mode == AccessMode.WORD20) {
                val |= (memory[address + 2] << 16) | (memory[address + 3] << 24);
            }
            val &= mode.mask;
        }
        return val;
    }

    @Override public void write(int dstAddress, int dst, AccessMode mode)
            throws EmulationException {
        dstAddress += offset;
        memory[dstAddress] = dst & 0xff;
        if (mode != AccessMode.BYTE) {
            memory[dstAddress + 1] = (dst >> 8) & 0xff;
            if ((dstAddress & 1) != 0) {
                core.printWarning(WarningType.MISALIGNED_WRITE, dstAddress);
            }
            if (mode != AccessMode.WORD) {
                memory[dstAddress + 2] = (dst >> 16) & 0xff;
                memory[dstAddress + 3] = (dst >> 24) & 0xff;
            }
        }
    }

    @Override public int get(int address, AccessMode mode) {
        return read(address, mode, AccessType.READ);
    }

    @Override public void set(int address, int data, AccessMode mode) {
        write(address, data, mode);
    }

}
