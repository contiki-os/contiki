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
 * DisAsm
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.MapTable;
import se.sics.mspsim.util.Utils;

public class DisAsm implements MSP430Constants {

  private boolean step = true; //false;

  private MapTable map;

  // Idiots solution to single stepping...
  private BufferedReader input =
    new BufferedReader(new InputStreamReader(System.in));

  
  public void setMap(MapTable m) {
    map = m;
  }

  public MapTable getMap() {
    return map;
  }

  public DbgInstruction disassemble(int pc, int[] memory, int[] reg) {
    return disassemble(pc, memory, reg, 0);
  }

  public DbgInstruction disassemble(int pc, int[] memory, int[] reg,
				 int interrupt) {
    DbgInstruction dbg = disassemble(pc, memory, reg, new DbgInstruction(),
				     interrupt);
    String fkn;
    if ((fkn = dbg.getFunction()) != null) {
      System.out.println("//// " + fkn);
    }
    System.out.println(dbg.getASMLine());

    /* Hack for printing the instruction after the ext word... */
    if (dbg.isExtensionWord()) {
        pc = pc + 2;
        dbg = disassemble(pc, memory, reg, new DbgInstruction(),
                interrupt);
        System.out.println(dbg.getASMLine());
    }
    
    return dbg;
  }

  public DbgInstruction getDbgInstruction(int pc, MSP430 cpu) {
    return disassemble(pc, cpu.memory, cpu.reg, new DbgInstruction(),
		       cpu.servicedInterrupt);
  }

  public DbgInstruction disassemble(int pc, int[] memory, int[] reg,
					   DbgInstruction dbg, int interrupt) {
    int startPC = pc;
    int size = 0;
    int instruction = memory[pc] + (memory[pc + 1] << 8);
    int op = instruction >> 12;
    boolean word = (instruction & 0x40) == 0;

    String output = "    ";
    if (interrupt > 0) {
      output = "I:" + Integer.toString(interrupt) + ' ';
    }

    String regs = "";


    if (pc < 0x0010) {
      output += "000" + Integer.toString(pc, 16);
    } else if (pc < 0x0100) {
      output += "00" + Integer.toString(pc, 16);
    } else if (pc < 0x1000) {
      output += "0" + Integer.toString(pc, 16);
    } else {
      output += Integer.toString(pc, 16);
    }


    output += ":\t";

    pc += 2;
    size += 2;

    switch (op) {
    case 0: // MSP430X instructions
    {
        // MSP430X - additional instructions
        String opstr = "";
        op = instruction & 0xf0f0;
        int srcdata = (instruction & 0x0f00) >> 8;
        int dst = instruction & 0x000f;
        int nextData = memory[pc] + (memory[pc + 1] << 8);
        boolean rrword = true;

        switch(op) {
        case MOVA_IND:
            opstr = "MOVA @R" + srcdata +  ",R" + dst;
            break;
        case MOVA_IND_AUTOINC:
            opstr = "MOVA @R" + srcdata +  "+,R" + dst;
            break;
        case MOVA_ABS2REG:
            opstr = "MOVA &$" + Utils.hex20(((srcdata << 16) | nextData)) +  ",R" + dst;
            size += 2;
            break;
        case MOVA_INDX2REG:
            opstr = "MOVA $" + Utils.hex16(nextData) +  "(R" + srcdata + "),R" + dst;
            size += 2;
            break;
        case MOVA_REG2ABS:
            opstr = "MOVA R" + srcdata + ",&$" + Utils.hex20(((dst << 16) | nextData));
            size += 2;
            break;
        case MOVA_REG2INDX:
            opstr = "MOVA R" + srcdata + ",$" + Utils.hex16(nextData) + "(R" + dst + ")";
            size += 2;
            break;
        case MOVA_IMM2REG:
            opstr = "MOVA #$" + Utils.hex20(((srcdata << 16) | nextData)) +  ",R" + dst;
            size += 2;
            break;
        case CMPA_IMM:
            opstr = "CMPA #$" + Utils.hex20(((srcdata << 16) | nextData)) +  ",R" + dst;
            size += 2;
            break;
        case ADDA_IMM:
            opstr = "ADDA #$" + Utils.hex20(((srcdata << 16) | nextData)) +  ",R" + dst;
            size += 2;
            break;
        case SUBA_IMM:
            opstr = "SUBA #$" + Utils.hex20(((srcdata << 16) | nextData)) +  ",R" + dst;
            size += 2;
            break;
        case MOVA_REG:
            opstr = "MOVA R" + srcdata +  ",R" + dst;
            break;
        case CMPA_REG:
            opstr = "CMPA R" + srcdata +  ",R" + dst;
            break;
        case ADDA_REG:
            opstr = "ADDA R" + srcdata +  ",R" + dst;
            break;
        case SUBA_REG:
            opstr = "SUBA R" + srcdata +  ",R" + dst;
            break;
        case RRXX_ADDR:
            rrword = false;
        case RRXX_WORD:
            String rrwordStr = rrword ? ".W" : ".A";
            int count = ((instruction >> 10) & 0x03) + 1; // shift amount
            switch (instruction & RRMASK) {
                case RRCM:
                    opstr = "RRCM" + rrwordStr + " #" + count + ",R" + dst;
                    break;
                case RRAM:
                    opstr = "RRAM" + rrwordStr + " #" + count + ",R" + dst;
                    break;
                case RLAM:
                    opstr = "RLAM" + rrwordStr + " #" + count + ",R" + dst;
                    break;
                case RRUM:
                    opstr = "RRUM" + rrwordStr + " #" + count + ",R" + dst;
                    break;
            }
            break;
        }
        
        output += dumpMem(startPC, size, memory);
        output += opstr + " ";
        regs = "R" + srcdata + "=" + Utils.hex16(reg[srcdata]);
        regs += " SP=" + Utils.hex16(reg[SP]);
    }
    break;
    case 1: // Single operand instructions
    {
        /* check CALLA first */
        int dst = instruction & 0x000f;
        int nextData = memory[pc] + (memory[pc + 1] << 8);
        String opstr = null;
        switch(instruction & 0xfff0) {
        case CALLA_REG:
            opstr = "CALLA R" + dst;
            break;
        case CALLA_IND:
            opstr = "CALLA @R" + dst;
            break;
        case CALLA_IND_AUTOINC:
            opstr = "CALLA @R" + dst + "+";
            break;
        case CALLA_ABS:
            opstr = "CALLA &" + Utils.hex20(((dst << 16) | nextData));
            size += 2;
            break;
        case CALLA_EDE:
            opstr = "CALLA " + Utils.hex20(((dst << 16) | nextData)) + "(PC)";
            size += 2;
            break;
        case CALLA_IMM:
            opstr = "CALLA #" + Utils.hex20(((dst << 16) | nextData));
            size += 2;
            break;
        default:
            switch (instruction & 0xff00) {
            case PUSHM_A:
                opstr = "PUSHM.A #" + (1 + ((instruction >> 4) & 0x0f)) + ", R" + (instruction & 0x0f);
                break;
            case PUSHM_W:
                opstr = "PUSHM.W #" + (1 + ((instruction >> 4) & 0x0f)) + ", R" + (instruction & 0x0f);
                break;
            case POPM_A:
                opstr = "POPM.A #" + (1 + ((instruction >> 4) & 0x0f)) + ", R" + (instruction & 0x0f);
                break;
            case POPM_W:
                opstr = "POPM.W #" + (1 + ((instruction >> 4) & 0x0f)) + ", R" + (instruction & 0x0f);
                break;                
            }
        }
        if (opstr != null) {        
            output += dumpMem(startPC, size, memory);
            output += opstr + " ";
            regs = "R" + dst + "=" + Utils.hex16(reg[dst]);
            regs += " SP=" + Utils.hex16(reg[SP]);
        } else {
            // Register
            int register = instruction & 0xf;
            // Adress mode of destination...
            int ad = (instruction >> 4) & 3;
            // Pick up the destination address based on ad more and regs...
            int dstAddress = 0;
            String adr = "";
            switch(ad) {
            // Operand in register!
            case AM_REG:
                adr = "R" + register;
                break;
            case AM_INDEX:
                dstAddress = memory[pc] + (memory[pc + 1] << 8);
                adr = "R" + register + "(" + dstAddress + ")";
                dstAddress = (register == CG1 ? 0 : reg[register]) + dstAddress;
                pc += 2;
                size += 2;
                break;
                // Indirect register
            case AM_IND_REG:
                adr = "@(R" + register + ")";
                dstAddress = reg[register];
                break;
            case AM_IND_AUTOINC:
                if (register == 0) {
                    // Can this be PC and be incremented only one byte?
                    int tmp = memory[pc] + (memory[pc + 1] << 8);
                    MapEntry me;
                    if (map != null && (me = map.getEntry(tmp)) != null) {
                        adr = me.getName(); // + " = $" + Utils.hex16(tmp);
                    } else {
                        adr = "#$" + Utils.hex16(tmp);
                    }
                    size += 2;
                } else {
                    adr = "@(R" + register + "+)";
                    dstAddress = reg[register];
                }
                break;
            }

            switch(instruction & 0xff80) {
            case RRC:
                opstr = "RRC" + (word ? ".W" : ".B");
                break;
            case SWPB:
                opstr = "SWPB" + (word ? ".W" : ".B");
                break;
            case RRA:
                opstr = "RRA" + (word ? ".W" : ".B");
                break;
            case SXT:
                opstr = "RRA" + (word ? ".W" : ".B");
                break;
            case PUSH:
                opstr = "PUSH" + (word ? ".W" : ".B");
                break;
            case CALL:
                opstr = "CALL";
                break;
            case RETI:
                opstr = "RETI";
                break;
            default:
                if ((instruction & 0xf800) == 0x1800) {
                    int zc = (instruction & EXTWORD_ZC) > 0 ? 1 : 0;
                    int al = (instruction & EXTWORD_AL) > 0 ? 1 : 0;
                    int rp = (instruction & EXTWORD_REPEAT) > 0 ? 1 : 0;
                    int shi = (instruction & EXTWORD_SRC) >> 7;
                    int dhi = (instruction & EXTWORD_DST);
                    opstr = "ExtWord " + Utils.hex16(instruction) + ":ZC:" + zc + " #:" + rp +
                    " A/L:" + al + " src:" + shi + " dst:" + dhi;
                    dbg.setExtWord(true);
                } else {
                    System.out.println("Not implemented instruction: $" + Utils.hex16(instruction) +
                            " at " + Utils.hex16(startPC));
                    opstr = "<Unkown>";
                }
            }
            output += dumpMem(startPC, size, memory);
            output += opstr + " " + adr;
            regs = "R" + register + "=" + Utils.hex16(reg[register]);
            regs += " SP=" + Utils.hex16(reg[SP]);
        }
    }
    break;
    // Jump instructions
    case 2:
    case 3:
      // 10 bits for address for these => 0x00fc => remove 2 bits
      int jmpOffset = instruction & 0x3ff;
      jmpOffset = (jmpOffset & 0x200) == 0 ?
	2 * jmpOffset : -(2 * (0x200 - (jmpOffset & 0x1ff)));
      boolean jump = false;
      String opstr = "";
      switch(instruction & 0xfc00) {
      case JNE:
	opstr = "JNE";
	break;
      case JEQ:
	opstr = "JEQ";
	break;
      case JNC:
	opstr = "JNC";
	break;
      case JC:
	opstr = "JC";
	break;
      case JN:
	opstr = "JN";
	break;
      case JGE:
	opstr = "JGE";
	break;
      case JL:
	opstr = "JL";
	break;
      case JMP:
	opstr = "JMP";
	break;
      default:
	System.out.println("Not implemented instruction: " +
			   Utils.binary16(instruction));
      }
      output += dumpMem(startPC, size, memory);
      output += opstr + " $" + Utils.hex16(jmpOffset);
      regs = "\tSR=" + dumpSR(reg[SR]);
      break;
    default:
      // ---------------------------------------------------------------
      // Double operand instructions!
      // ---------------------------------------------------------------
      int dstRegister = (instruction & 0xf);
      int srcRegister = (instruction >> 8) & 0xf;
      int as = (instruction >> 4) & 3;

      // AD: 0 => register direct, 1 => register index, e.g. X(Rn)
      boolean dstRegMode = ((instruction >> 7) & 1) == 0;
      int dstAddress = 0;
      int srcAddress = 0;
      int src = 0;
      int dst = 0;
      boolean write = false;
      boolean updateStatus = true;
      String srcadr = "";
      String dstadr = "";
      switch(as) {
	// Operand in register!
      case AM_REG:
	if (srcRegister == CG2) {
	  srcadr = "#0";
	} else if (srcRegister == CG1) {
	  srcadr = "#0";
	} else {
	  srcadr = getRegName(srcRegister);
	}
	break;
      case AM_INDEX:
	// Indexed if reg != PC & CG1/CG2 - will PC be incremented?
	if (srcRegister == CG1) {
	  srcAddress = memory[pc] + (memory[pc + 1] << 8);

	  MapEntry me;
	  if (map != null && (me = map.getEntry(srcAddress)) != null) {
	    srcadr = "&" + me.getName(); // + " = $" + Utils.hex16(srcAddress);
	  } else {
	    srcadr = "&$" + Utils.hex16(srcAddress);
	  }
	  size += 2;
	} else if (srcRegister == CG2) {
	  srcadr = "#1";
	} else {
	  srcAddress = reg[srcRegister] + memory[pc] + (memory[pc + 1] << 8);
	  srcadr = "$" + Utils.hex16(memory[pc] + (memory[pc + 1] << 8)) + "(R" + srcRegister + ")";
	  size += 2;
	}
	pc += 2;
	break;
	// Indirect register
      case AM_IND_REG:
	if (srcRegister == CG2) {
	  srcadr = "#2";
	} else if (srcRegister == CG1) {
	  srcadr = "#4";
	} else {
	  srcadr = "@" + getRegName(srcRegister);
	}
	break;
      case AM_IND_AUTOINC:
	if (srcRegister == CG2) {
	  srcadr = "#$ffff";
	} else if (srcRegister == CG1) {
	  srcadr = "#8";
	} else if (srcRegister == PC) {
	  srcadr = "#$" + Utils.hex16(memory[pc] + (memory[pc + 1] << 8));
	  pc += 2;
	  size += 2;
	} else if (srcRegister == CG2) {
	  srcadr = "#$ffff";
	} else {
	  srcadr = "@" + getRegName(srcRegister) + "+";
	  srcAddress = reg[srcRegister];
	}
	break;
      }

      if (dstRegMode) {
	dstadr = getRegName(dstRegister);
      } else {
	dstAddress = memory[pc] + (memory[pc + 1] << 8);
        MapEntry me = map != null ? map.getEntry(dstAddress) : null;
	if (dstRegister == 2) {
	  if (me != null) {
            dstadr = "&" + me.getName(); // + " = $" + Utils.hex16(srcAddress);
	  } else {
	    dstadr = "&$" + Utils.hex16(dstAddress);
	  }
	} else {
          if (me != null) {
            dstadr = me.getName() + "(R" + dstRegister + ")";
          } else {
            dstadr = "$" + Utils.hex16(dstAddress) + "(R" + dstRegister + ")";
          }
	}
	pc += 2;
	size += 2;
      }

      // If byte mode the source will not contain the full word...
      if (!word) {
	src = src & 0xff;
	dst = dst & 0xff;
      }
      opstr = "";
      switch (op) {
      case MOV: // MOV
	if (instruction == 0x3041) {
	  opstr = "RET /emulated: MOV.W ";
	} else {
	  opstr = "MOV" + (word ? ".W" : ".B");
	}
	break;
      case ADD: // ADD
	opstr = "ADD" + (word ? ".W" : ".B");
	break;
      case ADDC: // ADDC
	opstr = "ADDC" + (word ? ".W" : ".B");
	break;
      case SUBC: // SUBC
	opstr = "SUBC" + (word ? ".W" : ".B");
	break;
      case SUB: // SUB
	opstr = "SUB" + (word ? ".W" : ".B");
	break;
      case CMP: // CMP
	opstr = "CMP" + (word ? ".W" : ".B");
	break;
      case DADD: // DADD
	opstr = "DADD" + (word ? ".W" : ".B");
	break;
      case BIT: // BIT
	opstr = "BIT" + (word ? ".W" : ".B");
	break;
      case BIC: // BIC
	opstr = "BIC" + (word ? ".W" : ".B");
	break;
      case BIS: // BIS
	opstr = "BIS" + (word ? ".W" : ".B");
	break;
      case XOR: // XOR
	opstr = "XOR" + (word ? ".W" : ".B");
	break;
      case AND: // AND
	opstr = "AND" + (word ? ".W" : ".B");
	break;
      default:
        if (startPC > 0x200)
          System.out.println(output + " DoubleOperand not implemented: " +
              op + " instruction: " +
              Utils.binary16(instruction) + " = " +
              Utils.hex16(instruction));
      }


      output += dumpMem(startPC, size, memory);
      output += opstr + " " + srcadr + ", " + dstadr;

      regs = "R" + dstRegister + "=" + Utils.hex16(reg[dstRegister]) +
	" R" + srcRegister + "=" + Utils.hex16(reg[srcRegister]);
      regs += " SR=" + dumpSR(reg[SR]);
      regs += " SP=" + Utils.hex16(reg[SP]);
      regs += "; as = " + as;
      srcAddress &= 0xffff;
      if (srcAddress != -1) {
	srcAddress &= 0xffff;
	regs += " sMem:" + Utils.hex16(memory[srcAddress] +
				       (memory[(srcAddress + 1) % 0xffff]
					<< 8));
      }
    }

    dbg.setASMLine(output);
    dbg.setRegs(regs);
    dbg.setInstruction(instruction, size);
    if (map != null) {
      dbg.setFunction(map.getFunctionName(startPC));
    }

    if (!step) {
      String line = "";
      try {line = input.readLine();}catch(Exception e){}
      if (line != null && line.length() > 0 && line.charAt(0) == 'r') {
	System.out.println("Registers:");
	for (int i = 0, n = 16; i < n; i++) {
	  System.out.print("R" + i + "=" + Utils.hex16(reg[i]) + "  ");
	  if (i % 7 == 0 && i != 0) System.out.println();
	}
	System.out.println();
      }
    }
    return dbg;
  }

  private String getRegName(int index) {
    if (index == 0) return "PC";
    if (index == 1) return "SP";
    if (index == 2) return "SR";
    return "R" + index;
  }

  public static String getSingleOPStr(int instruction) {
    boolean word = (instruction & 0x40) == 0;
    switch(instruction & 0xff80) {
    case RRC:
      return "RRC" + (word ? ".W" : ".B");
    case SWPB:
      return "SWPB" + (word ? ".W" : ".B");
    case RRA:
      return "RRA" + (word ? ".W" : ".B");
    case SXT:
      return "RRA" + (word ? ".W" : ".B");
    case PUSH:
      return "PUSH" + (word ? ".W" : ".B");
    case CALL:
      return "CALL";
    case RETI:
      return "RETI";
    default:
      return "-";
    }
  }

  private static String dumpSR(int sr) {
    return "" +
      (((sr & OVERFLOW) != 0) ? 'V' : '-') +
      (((sr & NEGATIVE) != 0) ? 'N' : '-') +
      (((sr & ZERO) != 0) ? 'Z' : '-') +
      (((sr & CARRY) != 0) ? 'C' : '-');
  }

  private static String dumpMem(int pc, int size, int[] memory) {
    String output = "";
    for (int i = 0, n = size; i < n; i++) {
      if (size > i) {
	output += Utils.hex8(memory[pc + i]) + " ";
      } else {
	output += "   ";
      }
    }
    return output;
  }
}
