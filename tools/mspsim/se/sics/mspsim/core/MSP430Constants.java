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
 * MSP430Constants
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;

public interface MSP430Constants {

  public static final String VERSION = "0.99";
  
  public static final int MODE_BYTE = 0;
  public static final int MODE_WORD = 1;
  public static final int MODE_WORD20 = 2;
  
  public static final int MODE_BIT_SIZE[] = {8, 16, 20};
  
  public static final int MODE_MASK[] = {0xff, 0xffff, 0xfffff};
  
  /* memory tags - not used yet*/
  public static final int READ_MONITOR = 0x1000;
  public static final int WRITE_MONITOR = 0x2000;
  public static final int EXEC_MONITOR = 0x4000;
  public static final int MEM_FLASH = 0x100;
  public static final int MEM_IO = 0x200;
  
  public static final int RESET_PUC = 0;
  public static final int RESET_POR = 1;
  
  // MODES
  public static final int MODE_ACTIVE = 0;
  public static final int MODE_LPM0 = 1;
  public static final int MODE_LPM1 = 2;
  public static final int MODE_LPM2 = 3;
  public static final int MODE_LPM3 = 4;
  public static final int MODE_LPM4 = 5;
  public static final int MODE_MAX = MODE_LPM4;

  public static String[] MODE_NAMES = {
    "active", "lpm0", "lpm1", "lpm2", "lpm3", "lpm4" 
  };
  
  public static final int CLK_ACLK = 1;
  public static final int CLK_SMCLK = 2;

  // Instructions (full length)
  public static final int RRC = 0x1000;
  public static final int SWPB = 0x1080;
  public static final int RRA = 0x1100;
  public static final int SXT = 0x1180;
  public static final int PUSH = 0x1200;
  public static final int CALL = 0x1280;
  public static final int RETI = 0x1300;

  // Conditional Jumps [
  public static final int JNE = 0x2000;
  public static final int JEQ = 0x2400;
  public static final int JNC = 0x2800;
  public static final int JC = 0x2C00;

  // Conditional Jumps & jumps...
  public static final int JN = 0x3000;
  public static final int JGE = 0x3400;
  public static final int JL = 0x3800;
  public static final int JMP = 0x3C00;

  // Short ones...
  public static final int MOV = 0x4;
  public static final int ADD = 0x5;
  public static final int ADDC = 0x6;
  public static final int SUBC = 0x7;
  public static final int SUB = 0x8;
  public static final int CMP = 0x9;
  public static final int DADD = 0xa;
  public static final int BIT = 0xb;
  public static final int BIC = 0xc;
  public static final int BIS = 0xd;
  public static final int XOR = 0xe;
  public static final int AND = 0xf;

  
  // MSP430X instructions
  public static final int MOVA_IND = 0x0000;
  public static final int MOVA_IND_AUTOINC = 0x0010; /* Indirect with increment */
  public static final int MOVA_ABS2REG = 0x0020;
  public static final int MOVA_INDX2REG = 0x0030;
  public static final int MOVA_REG2ABS = 0x0060;
  public static final int MOVA_REG2INDX = 0x0070;
  public static final int MOVA_IMM2REG = 0x0080;
  public static final int CMPA_IMM = 0x0090;
  public static final int ADDA_IMM = 0x00a0;
  public static final int SUBA_IMM = 0x00b0;
  public static final int MOVA_REG = 0x00c0;
  public static final int CMPA_REG = 0x00d0;
  public static final int ADDA_REG = 0x00e0;
  public static final int SUBA_REG = 0x00f0;
  
  public static final int RRXX_ADDR = 0x0040;
  public static final int RRXX_WORD = 0x0050;
  
  public static final int RRMASK = 0x0300;
  public static final int RRCM = 0x0000; /* rotate right through carry C -> MSB -> MSB-1 ... -> C */
  public static final int RRAM = 0x0100; /* rotate right arithmetically MSB -> MSB -> MSB-1 ...->C*/ 
  public static final int RLAM = 0x0200; /* rotate left arithm. C <- MSB-1 ... <- 0 */
  public static final int RRUM = 0x0300; /* rotate right unsigned 0 -> MSB -> MSB -1, ... */

  
  public static final int CALLA_MASK = 0xfff0;
  public static final int CALLA_REG = 0x1340;
  public static final int CALLA_INDEX = 0x1350;
  public static final int CALLA_IND = 0x1360;
  public static final int CALLA_IND_AUTOINC = 0x1370;
  public static final int CALLA_ABS = 0x1380;
  public static final int CALLA_EDE = 0x1390; /* x(PC) */
  public static final int CALLA_IMM = 0x13b0;
  
  public static final int PUSHM_A = 0x1400;
  public static final int PUSHM_W = 0x1500;
  public static final int POPM_A = 0x1600;
  public static final int POPM_W = 0x1700;
    
  
  public static final int EXTWORD_ZC = 0x100;
  public static final int EXTWORD_REPEAT = 0x80;
  public static final int EXTWORD_AL = 0x40;
  public static final int EXTWORD_SRC = 0x780;
  public static final int EXTWORD_DST = 0x0f;
  
  
  
  
  public static final String[] TWO_OPS = {
    "-","-","-","-","MOV", "ADD", "ADDC", "SUBC", "SUB",
    "CMP", "DADD", "BIT", "BIC", "BIS", "XOR", "AND"
  };

  public static final String[] REGISTER_NAMES = {
    "PC", "SP", "SR", "CG1", "CG2"
  };

  public static final int PC = 0;
  public static final int SP = 1;
  public static final int SR = 2;
  public static final int CG1 = 2;
  public static final int CG2 = 3;

  public static final int[][] CREG_VALUES = new int[][]{
    {0, 0, 4, 8}, {0, 1, 2, 0xffff}
  };

  public static final int CARRY_B = 0;
  public static final int ZERO_B = 1;
  public static final int NEGATIVE_B = 2;
  public static final int OVERFLOW_B = 8;
  public static final int GIE_B = 3;

  public static final int CARRY = 1;
  public static final int ZERO = 2;
  public static final int NEGATIVE = 4;
  public static final int OVERFLOW = 1 << OVERFLOW_B;
  public static final int GIE = 1 << GIE_B;

  /* For the LPM management */
  public static final int CPUOFF = 0x0010;
  public static final int OSCOFF = 0x0020;
  public static final int SCG0 = 0x0040;
  public static final int SCG1 = 0x0080;

// #define C                   0x0001
// #define Z                   0x0002
// #define N                   0x0004
// #define V                   0x0100
// #define GIE                 0x0008
// #define CPUOFF              0x0010
// #define OSCOFF              0x0020
// #define SCG0                0x0040
// #define SCG1                0x0080


  public static final int AM_REG = 0;
  public static final int AM_INDEX = 1;
  public static final int AM_IND_REG = 2;
  public static final int AM_IND_AUTOINC = 3;

  public static final int CLKCAPTURE_NONE = 0;
  public static final int CLKCAPTURE_UP = 1;
  public static final int CLKCAPTURE_DWN = 2;
  public static final int CLKCAPTURE_BOTH = 3;


  public static final int DEBUGGING_LEVEL = 0;
}
