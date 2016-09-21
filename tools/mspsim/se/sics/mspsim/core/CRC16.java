/* Copyright (c) 2013, tado° GmbH. Munich, Germany.
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
 * This file is part of MSPSim.
 * 
 * Author: Víctor Ariño <victor.arino@tado.com>
 * 
 */
package se.sics.mspsim.core;


/**
 * CRC16 module for the MSP430
 * 
 * @author Victor Ariño <victor.arino@tado.com>
 */
public class CRC16 extends IOUnit {

	/**
	 * Address and size for IO configuration
	 */
	public static final int OFFSET = 0x0150;
	public static final int SIZE = 8;

	/**
	 * Register offsets definition as in the documentation.
	 */
	public static final int CRCDI = 0;
	public static final int CRCDIRB = 0x2;
	public static final int CRCINIRES = 0x4;
	public static final int CRCRESR = 0x6;

	/**
	 * Initial seed as recommended by CCITT
	 */
	private static final int CCITTSeed = 0xFFFF;

	/**
	 * CRC16 peripheral for the MSP430 using the default memory offset
	 * 
	 * @param cpu
	 */
	public CRC16(MSP430Core cpu) {
		this(cpu, OFFSET);
	}

	/**
	 * CRC16 peripheral for the MSP430
	 * 
	 * @param cpu
	 *           CPU core
	 * @param offset
	 *           Address offset, by default is 0x0150
	 */
	public CRC16(MSP430Core cpu, int offset) {
		super("CRC16", cpu, cpu.memory, offset);
		setLogLevel(Loggable.DEBUG);
	}

	/**
	 * CRC16 CCITT Java implementation
	 * 
	 * @author Víctor Ariño <victor.arino@tado.com>
	 */
	private class CRC16Java {
		private int crc = CCITTSeed;
		private int polynomial = 0x1021; // 0001 0000 0010 0001 (0, 5, 12)

		/**
		 * Adds one byte to the CRC computation
		 * 
		 * @param b
		 *           byte to add
		 */
		protected void process(byte b) {
			for (int i = 0; i < 8; i++) {
				boolean bit = ((b >> (7 - i) & 1) == 1);
				boolean c15 = ((crc >> 15 & 1) == 1);
				crc <<= 1;
				if (c15 ^ bit)
					crc ^= polynomial;
			}
		}

		/**
		 * Add one byte in reverse way
		 * 
		 * @param b
		 *           byte to add
		 */
		protected void processRb(byte b) {
			process(reflectByte(b));
		}

		/**
		 * Set the seed of the CRC
		 * 
		 * @param seed
		 */
		protected void reset(int seed) {
			crc = seed;
		}

		/**
		 * Get computed CRC
		 * 
		 * @return
		 */
		protected int getCRC() {
			return crc;
		}

		/**
		 * Reflect a byte
		 * 
		 * @author Valentin Sawadski <valentin@tado.com>
		 * @param b
		 *           byte to reflect
		 * @return reversed byte
		 */
		private byte reflectByte(byte b) {
			byte ret = 0;
			for (int i = 0; i < 8; i++) {
				if ((b & (1 << i)) == (1 << i)) {
					ret += (1 << (7 - i));
				}
			}
			return ret;
		}

		/**
		 * Reflect all bytes of the CRC
		 * 
		 * @author Valentin Sawadski <valentin@tado.com>
		 * @param crc
		 * @return crc reversed
		 */
		private int reflectCrcBytewise(int crc) {
			int lowByte = reflectByte((byte) (crc & 0xFF)) & 0xFF;
			int highByte = reflectByte((byte) ((crc & 0xFF00) >> 8)) & 0xFF;
			return ((highByte << 8) + lowByte) & 0xFFFF;
		}

		/**
		 * Swap two bytes
		 * 
		 * @author Valentin Sawadski <valentin@tado.com>
		 * @param crc
		 * @return swapped bytes
		 */
		private int swapBytes(int crc) {
			int lowByte = crc & 0xFF;
			int highByte = crc & 0xFF00;
			return ((lowByte << 8) + (highByte >> 8)) & 0xFFFF;
		}

		/**
		 * Get the CRC reversed
		 * 
		 * @return reversed crc
		 */
		protected int getCrcRb() {
			return swapBytes(reflectCrcBytewise(crc));
		}
	}

	private CRC16Java crc = new CRC16Java();

	/**
	 * Clear everything when reset
	 */
	public void reset(int type) {
		crc.reset(CCITTSeed);
	}

	/**
	 * The registers are written
	 */
	public void write(int address, int value, boolean word, long cycles) {
		/*
		 * The offset variable is used in case the peripheral changes the base
		 * address in some microcontroller model
		 */
		switch (address - offset) {
		case CRC16.CRCDI:
			if (word) {
				int hi = (value >> 8) & 0x00ff;
				int lo = (value) & 0x00ff;
				crc.processRb((byte) lo);
				crc.processRb((byte) hi);
			} else {
				crc.processRb((byte) value);
			}
			break;
		case CRC16.CRCDIRB:
			if (word) {
				int hi = (value >> 8) & 0x00ff;
				int lo = (value) & 0x00ff;
				crc.process((byte) hi);
				crc.process((byte) lo);
			} else {
				crc.process((byte) value);
			}
			break;
		case CRC16.CRCINIRES:
			crc.reset(value & 0xffff);
			break;
		}
	}

	/**
	 * Registers are read
	 */
	public int read(int address, boolean word, long cycles) {
		switch (address - offset) {
		case CRC16.CRCINIRES:
			return crc.getCRC();
		case CRC16.CRCRESR:
			return crc.getCrcRb();
		}
		return 0;
	}

	public void interruptServiced(int vector) {
	}
}
