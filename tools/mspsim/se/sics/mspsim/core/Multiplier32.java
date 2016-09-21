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
 * -----------------------------------------------------------------
 *
 * Multiplier
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.core;
import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.util.Utils;

public class Multiplier32 extends IOUnit {

    public static final int MPY = 0x00;
    public static final int MPYS = 0x02;
    public static final int MAC = 0x04;
    public static final int MACS = 0x06;
    public static final int OP2 = 0x08;
    public static final int RESLO = 0x0a;
    public static final int RESHI = 0x0c;
    public static final int SUMEXT = 0x0e;

    public static final int MPY32L = 0x010;
    public static final int MPY32H = 0x012;
    public static final int MPYS32L = 0x014;
    public static final int MPYS32H = 0x016;
    public static final int MAC32L = 0x018;
    public static final int MAC32H = 0x01a;
    public static final int MACS32L = 0x01c;
    public static final int MACS32H = 0x01e;
    public static final int OP2L = 0x20;
    public static final int OP2H = 0x22;
    public static final int RES0 = 0x24;
    public static final int RES1 = 0x26;
    public static final int RES2 = 0x28;
    public static final int RES3 = 0x2a;
    public static final int MPY32CTL0 = 0x2c;

    private int mpy;
    private int mpys;
    private int op2;

    private int resLo;
    private int resHi;
    private int mac;
    private int macs;
    private int sumext;

    private int op1;

    private int mpy32L;
    private int mpy32H;
    private int mpys32L;
    private int mpys32H;
    private int mac32L;
    private int mac32H;
    private int macs32L;
    private int macs32H;
    private int op2L;
    private int op2H;
    private int res0;
    private int res1;
    private int res2;
    private int res3;
    private int mpy32ctl0;
    private long res64;

    private boolean signed = false;
    private boolean accumulating = false;

    /**
     * Creates a new <code>Multiplier32</code> instance.
     *
     */
    public Multiplier32(MSP430Core cpu, int memory[], int offset) {
        super("Multiplier32", "Hardware Multiplier 32", cpu, memory, offset);
    }

    @Override
    public int read(int address, boolean word, long cycles) {
        address = address - offset;
        switch (address) {
        case MPY:
            return mpy;
        case MPYS:
            return mpys;
        case MAC:
            return mac;
        case MACS:
            return macs;
        case OP2:
            return op2;
        case RESHI:
            if (DEBUG) log("read res hi: " + resHi );
            return resHi;
        case RESLO:
            if (DEBUG) log("read res lo: " + resLo );
            return resLo;
        case SUMEXT:
            if (DEBUG) log("read sumext: " + sumext);
            return sumext;
        case MPY32L:
            return mpy32L;
        case MPY32H:
            return mpy32H;
        case MPYS32L:
            return mpys32L;
        case MPYS32H:
            return mpys32H;
        case MAC32L:
            return mac32L;
        case MAC32H:
            return mac32H;
        case MACS32L:
            return macs32L;
        case MACS32H:
            return macs32H;
        case OP2L:
            return op2L;
        case OP2H:
            return op2H;
        case RES0:
            return res0;
        case RES1:
            return res1;
        case RES2:
            return res2;
        case RES3:
            return res3;
        case MPY32CTL0:
            return mpy32ctl0;
        default:
            logw(WarningType.EMULATION_ERROR, "read unhandled address: 0x" + Utils.hex(address, 4));
            return 0;
        }
    }

    @Override
    public void write(int address, int data, boolean word, long cycles) {
        address = address - offset;
        if (DEBUG) {
            log("write to: $" + Utils.hex(address, 4) + " data=" + data + " word=" + word);
        }
        switch(address) {
        case MPY:
            if (DEBUG) log("Write to MPY: " + data);
            op1 = mpy = data;
            signed = false;
            accumulating = false;
            break;
        case MPYS:
            op1 = mpys = data;
            if (DEBUG) log("Write to MPYS: " + data);
            signed = true;
            accumulating = false;
            break;
        case MAC:
            op1 = mac = data;
            if (DEBUG) log("Write to MAC: " + data);
            signed = false;
            accumulating = true;
            break;
        case MACS:
            op1 = macs = data;
            if (DEBUG) log("Write to MACS: " + data);
            signed = true;
            accumulating = true;
            break;
        case RESLO:
            resLo = data;
            break;
        case RESHI:
            resHi = data;
            break;
        case OP2:
            if (DEBUG) log("Write to OP2: " + data);
            sumext = 0;
            op2 = data;
            // Expand to word
            if (signed) {
                if (!word) {
                    if (op1 > 0x80) op1 = op1 | 0xff00;
                    if (op2 > 0x80) op2 = op2 | 0xff00;
                }
                op1 = op1 > 0x8000 ? op1 - 0x10000 : op1;
                op2 = op2 > 0x8000 ? op2 - 0x10000 : op2;
            }

            long res = (long) op1 * (long) op2;
            if (DEBUG) log("O1:" + op1 + " * " + op2 + " = " + res);

            if (signed) {
                sumext = res < 0 ? 0xffff : 0;
            }

            if (accumulating) {
                res += ((long) resHi << 16) + resLo;
                if (!signed) {
                    sumext = res > 0xffffffffL ? 1 : 0;
                }
            } else if (!signed) {
                sumext = 0;
            }

            resHi = (int) ((res >> 16) & 0xffff);
            resLo = (int) (res & 0xffff);
            if (DEBUG) log(" ===> result = " + res);
            break;
        case MPY32L:
            op1 = mpy32L = data;
            signed = false;
            accumulating = false;
            break;
        case MPY32H:
            mpy32H = data;
            op1 = (op1 & 0xffff) | (data << 16);
            break;
        case MPYS32L:
            if (!word && data >= 0x80) {
                data -= 0x100;
            }
            op1 = mpy32L = data;
            signed = true;
            accumulating = false;
            break;
        case MPYS32H:
            if (!word & data > 0x80) {
                data -= 0x100;
            }
            mpys32H = data;
            op1 = (op1 & 0xffff) | (data << 16);
            break;
        case MAC32L:
            op1 = mac32L = data;
            signed = false;
            accumulating = true;
            break;
        case MAC32H:
            mac32H = data;
            op1 = (op1 & 0xffff) | (data << 16);
            break;
        case MACS32L:
            if (!word & data > 0x80) {
                data -= 0x100;
            }
            op1 = macs32L = data;
            signed = true;
            accumulating = true;
            break;
        case MACS32H:
            if (!word & data > 0x80) {
                data -= 0x100;
            }
            macs32H = data;
            op1 = (op1 & 0xffff) | (data << 16);
            break;
        case OP2L:
            if (signed && !word && data >= 0x80) {
                data -= 0x80;
            }
            op2L = op2 = data;
            break;
        case OP2H: {
            long p;
            if (signed && !word && data >= 0x80) {
                data -= 0x80;
            }
            op2 = (op2 & 0xffff) | (data << 16);

            /* FIXME: Doesn't set SUMEXT and MPYC properly. */
            if (signed) {
                p = (long) op1 * (long) op2;
            }
            else {
                long uop1, uop2;
                uop1 = op1;
                if (uop1 < 0) {
                    uop1 += 0x100000000L;
                }
                uop2 = op2;
                if (uop2 < 0) {
                    uop2 += 0x100000000L;
                }

                p = uop1 * uop2;
            }
            if (accumulating) {
                res64 += p;
            } else {
                res64 = p;
            }
            
            /* FIXME: Ignore accumulate. */
            resLo = res0 = (int) res64 & 0xffff;
            resHi = res1 = (int) (res64 >> 16) & 0xffff;
            res2 = (int) (res64 >> 32) & 0xffff;
            res3 = (int) (res64 >> 48) & 0xffff;

            break;
        }
        default:
            logw(WarningType.EMULATION_ERROR, "**** Not yet implemented multiplier 32 register: 0x" + Utils.hex(address, 4));
            break;
        }
    }

    @Override
    public void interruptServiced(int vector) {
    }
}
