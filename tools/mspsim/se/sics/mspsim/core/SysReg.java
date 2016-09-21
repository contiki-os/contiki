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

public class SysReg extends IOUnit {
	public static int ADDRESS = 0x180;
	public static int SIZE = 0x1f;

	/*
	 * This is only a stub to avoid immediate Mspsim exceptions.
	 * TODO XXX: Implement me!
	 * 
System control SYSCTL 00h
Bootstrap loader configuration area SYSBSLC 02h
JTAG mailbox control SYSJMBC 06h
JTAG mailbox input 0 SYSJMBI0 08h
JTAG mailbox input 1 SYSJMBI1 0Ah
JTAG mailbox output 0 SYSJMBO0 0Ch
JTAG mailbox output 1 SYSJMBO1 0Eh
Bus Error vector generator SYSBERRIV 18h
User NMI vector generator SYSUNIV 1Ah
System NMI vector generator SYSSNIV 1Ch
Reset vector generator SYSRSTIV 1Eh
	 */

	public SysReg(MSP430Core cpu, int[] memory) {
		super("SysReg", "System registers", cpu, memory, SysReg.ADDRESS);
		reset(0);
	}

	public void reset(int type) {
	}

	public void write(int address, int value, boolean word, long cycles) {
		if (DEBUG)
			log(this.getName() + ": Write to: "
					+ String.format("0x%05x = 0x%05x", address, value));
		memory[address] = value;
	}

	public int read(int address, boolean word, long cycles) {
		if (DEBUG)
			log(this.getName()
					+ ": Read from: "
					+ String.format("0x%05x = 0x%05x", address, memory[address]));
		return memory[address];
	}

	public void interruptServiced(int vector) {
		if (DEBUG)
			log(this.getName() + ": Interrupt services vector: " + vector);
	}
}
