/*
 * Copyright (c) 2012, Thingsquare.
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
 */

package se.sics.mspsim.core;

public class PMM extends IOUnit {
    public static int SIZE = 32;

    /*
     * This is only a stub to avoid immediate Mspsim exceptions. TODO XXX:
     * Implement me!
     */

    private int baseAddress;

    public PMM(MSP430Core cpu, int[] memory, int address) {
        super("PMM", "PMM", cpu, memory, address);
        baseAddress = address;
        reset(0);
    }

    public void reset(int type) {
        /* Initial values */
        memory[baseAddress + 0x0] = 0x9600; /* PMMCTL0 */
        memory[baseAddress + 0x2] = 0x0000; /* PMMCTL1 */
        memory[baseAddress + 0x4] = 0x4400; /* SVSMHCTL */
        memory[baseAddress + 0x6] = 0x4400; /* SVSMLCTL */
        memory[baseAddress + 0x8] = 0x0020; /* SVSMIO */
        memory[baseAddress + 0xc] = 0x0000; /* PMMIFG */
        memory[baseAddress + 0xe] = 0x1100; /* PMMRIE */
    }

    public void write(int address, int value, boolean word, long cycles) {
        if (DEBUG)
            log(this.getName() + ": Write to: "
                    + String.format("0x%05x = 0x%05x", address, value));

        memory[address] = value;
    }

    public int read(int address, boolean word, long cycles) {
        if (address == baseAddress + 0xc) {
            /* PMMIFG always settled */
            int SVSMLDLYIFG = 0x01;
            int SVSMHDLYIFG = 0x10;
            int SVMHVLRIFG = 0x40;
            memory[address] |= SVSMLDLYIFG | SVSMHDLYIFG | SVMHVLRIFG;
        }

        if (DEBUG) {
            log(this.getName()
                    + ": Read from: "
                    + String.format("0x%05x = 0x%05x", address, memory[address]));
        }

        return memory[address];
    }

    public void interruptServiced(int vector) {
        if (DEBUG) {
            log(this.getName() + ": Interrupt services vector: " + vector);
        }
    }
}
