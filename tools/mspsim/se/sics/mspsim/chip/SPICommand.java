/**
 * Copyright (c) 2012 Swedish Institute of Computer Science.
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
 */

package se.sics.mspsim.chip;

import java.util.ArrayList;

public class SPICommand {

    private static final boolean DEBUG = false;

/*
 * MEMXCP 0 1 0 1 0 1 0 p c c c c c c c c a a a a e e e e a a a a a a a a e e e e e e e e
 */
    public final static int DYNAMIC_LENGTH = 0xffff;

    public final String name;
    public final int mask;
    public final int value;
    public final int bitCount;
    public final int commandLen;

    private final BitField[] bitFields;

    public static class BitField {
        public final String name;
        public final int startBit;
        public final int endBit;
        public final int firstMask;

        public BitField(String currentName, int start, int c) {
            name = currentName;
            startBit = start;
            endBit = c;
            firstMask = 0xff >> (startBit & 7);
        }

        public int getValue(SPIData spiData) {
            int value;
            int firstByte = startBit / 8;
            int lastByte = endBit / 8;
            int nrBitsRoll = 7 - endBit & 7;
            value = spiData.getSPIData(firstByte) & firstMask;

            for (int i = firstByte + 1; i < lastByte + 1; i++) {
                value = (value << 8) + spiData.getSPIData(i);
            }
            value = value >> nrBitsRoll;

            return value;
        }
    }

    SPICommand(String pattern) {
        String[] subs = pattern.split(" ");
        this.name = subs[0];

        ArrayList<BitField> bitFields = new ArrayList<BitField>();
        int value = 0;
        int mask = 0;
        int bitCount = 0;
        int c = 0;
        int start = 0;
        String currentName = "-";
        for (int i = 1; i < subs.length; i++) {
            /* not more than first byte */
            if (subs[i].equals("1")) {
                if (c < 8) {
                    value = (value << 1) + 1;
                    mask = (mask << 1) | 1;
                    bitCount++;
                }
            } else if (subs[i].equals("0")) {
                if (c < 8) {
                    value = (value << 1);
                    mask = (mask << 1) | 1;
                    bitCount++;
                }
            } else if (subs[i].equals(currentName)) {
                /* do nothing */
            } else {
                if (start != 0) {
                    if (DEBUG) System.out.println("Bitfield: " + currentName + ": [" +
                            start + " - " + (c - 1) + "]");
                    bitFields.add(new BitField(currentName, start, c - 1));
                } else {
                    if (DEBUG) System.out.printf("C: %d value: 0x%02x  mask: 0x%02x\n", c, value, mask);
                    if (c < 8) {
                        value = value << (8 - c);
                        mask = mask << (8 - c);
                    }
                }
                currentName = subs[i];
                start = c;
            }
            c++;
        }
        if (start != 0) {
            if (DEBUG) System.out.print("Bitfield: " + currentName + ": [" +
                    start + " - " + (c - 1) + "]  ");
            bitFields.add(new BitField(currentName, start, c - 1));
        }

        int commandLen = c / 8;
        if ("...".equals(currentName)) {
            commandLen = DYNAMIC_LENGTH;
        }
        if (DEBUG) System.out.printf("value 0x%02x mask 0x%02x len 0x%02x\n", value, mask, commandLen);

        this.value = value;
        this.mask = mask;
        this.bitCount = bitCount;
        this.commandLen = commandLen;
        this.bitFields = bitFields.toArray(new BitField[bitFields.size()]);
    }

    /* return -1 if no match */
    /* or len of the rest of the arguments if any more */
    public int matchSPI(int spiData) {
        if ((spiData & mask) == value) {
            return commandLen;
        }
        return -1;
    }

    /* do nothing here...  - override if needed */
    public boolean dataReceived(int data) {
        return true;
    }

    /* for any command that is executable (finite commands) */
    public void executeSPICommand() {
        System.out.println("SPI Command " + name + " not implemented...");
    }

    public BitField getBitField(String fieldName) {
        for (BitField b : bitFields) {
            if (b.name.equals(fieldName)) {
                return b;
            }
        }
        /* not existing ... */
        throw new IllegalArgumentException("No bitfield with name " + fieldName + " exists for " + name);
    }

    @Override
    public String toString() {
        return "SPICommand[" + name + ']';
    }
}
