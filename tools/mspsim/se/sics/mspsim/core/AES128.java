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

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

import se.sics.mspsim.core.EmulationLogger.WarningType;


/**
 * AES128 msp430 peripheral emulation
 * 
 * TODO: 
 * 	advanced cipher modes
 * 	longer key support 
 * 	interrupts 
 * 	timing concerns 
 * 	first round key operations
 * 
 * @author Víctor Ariño <victor.arino@tado.com>
 */
public class AES128 extends IOUnit {

	/**
	 * Address and size for IO configuration
	 */
	public static int OFFSET = 0x09C0;
	public static int SIZE = 12;

	/**
	 * Enable/Disable debug output of the module
	 */
	private static boolean DEBUG = false;

	/**
	 * Register definition as in the documentation Offset from 0xff80
	 */
	public static final int AES_VECTOR = 0x005A;

	/* Main registers */
	public static final int AESACTL0 = 0x00;
	public static final int AESACTL1 = 0x02;
	public static final int AESASTAT = 0x04;
	public static final int AESAKEY = 0x06;
	public static final int AESADIN = 0x08;
	public static final int AESADOUT = 0x0a;

	/* AESACTL0 Control Bits */
	public static final int AESSWRST = 0x0080;
	public static final int AESRDYIFG = 0x0100;
	public static final int AESERRFG = 0x0800;
	public static final int AESRDYIE = 0x1000;

	public static final int AESOP_0 = 0x0000;
	public static final int AESOP_1 = 0x0001;
	public static final int AESOP_2 = 0x0002;
	public static final int AESOP_3 = 0x0003;

	/* AESASTAT Control Bits */
	public static final int AESBUSY = 0x0001;
	public static final int AESKEYWR = 0x0002;
	public static final int AESDINWR = 0x0004;
	public static final int AESDOUTRD = 0x0008;
	public static final int AESCMEN = 0x8000;

	/* Cipher modes */
	private static final int MODE_ECB = 0;
	private static final int MODE_CBC = 1;
	private static final int MODE_OFB = 2;
	private static final int MODE_CFB = 3;

	/* Key lengths */
	private static final int KEY_128 = 0;
	private static final int KEY_192 = 1;
	private static final int KEY_256 = 2;

	/**
	 * Emulate peripheral using the default offset
	 * 
	 * @param cpu
	 */
	public AES128(MSP430Core cpu) {
		this(cpu, OFFSET);
	}

	/**
	 * Emulate peripheral using custom offset
	 * 
	 * @param cpu
	 * @param offset
	 */
	public AES128(MSP430Core cpu, int offset) {
		super("CRC", cpu, cpu.memory, offset);
	}

	/**
	 * Clear everything when reset. The cleared fields are specified in the
	 * manual
	 */
	public void reset(int type) {
		/*
		 * AES software reset. Immediately resets the complete AES accelerator
		 * module even when busy except for the AESRDYIE, the AESKLx and the
		 * AESOPx bits. It also clears the (internal) state memory.
		 */
		key.clear();
		inData.clear();
		outData.clear();
		isBusy = false;
		advancedCipherMode = false;
		interruptEnable = false;
		errorFlag = false;
		resetFlag = false;
		cipherMode = MODE_ECB;
		cipherBlockCounter = 0;
	}

	/**
	 * Variable holders for the different registers needed by this peripheral
	 */

	/* avoid using NIO resources */
	private static class ByteBuffer {
	    byte[] buffer;
	    int pos;

	    ByteBuffer(int size) {
	        buffer = new byte[size]; 
	        pos = 0;
	    }

	    public int position() {
	        return pos;
	    }

	    public void position(int p) {
	        pos = p;
	    }

	    public boolean hasRemaining() {
	        return pos < buffer.length;
	    }

	    public void clear() {
	        pos = 0;
	    }

	    public void resetPos() {
	        pos = 0;
	    }

	    public byte[] array() {
	        return buffer;
	    }

	    public void put(byte[] bytes) {
	        for (int i = 0; i < bytes.length; i++) {
                    put(bytes[i]);
                }
	    }

	    public void put(byte data) {
	        buffer[pos++] = data;
	    }

	    /* assume that calling code is ok... */
	    public byte get() {
	        return buffer[pos++];
	    }
	    
	    public int limit() {
	        return buffer.length;
	    }
	}

	private ByteBuffer key = new ByteBuffer(16);
	private ByteBuffer inData = new ByteBuffer(16);
	private ByteBuffer outData = new ByteBuffer(16);

	/**
	 * Syntax sugar
	 */
	private boolean isBusy = false;

	/**
	 * Nice names for buffer operations :-)
	 */
	private int bytesReadOut() {
		return outData.position();
	}

	private int bytesWrittenIn() {
		return inData.position();
	}

	private int bytesKeyWritten() {
		return key.position();
	}

	private boolean allBytesReadOut() {
		return !outData.hasRemaining();
	}

	private boolean isKeyReady() {
		return !key.hasRemaining();
	}

	private boolean isDataReady() {
		return !inData.hasRemaining();
	}

	private int getStatReg() {
		int stat = 0;
		stat |= (bytesReadOut() & 0x0f) << 12;
		stat |= (bytesWrittenIn() & 0x0f) << 8;
		stat |= (bytesKeyWritten() & 0x0f) << 4;
		stat |= (allBytesReadOut() ? 1 : 0) << 3;
		stat |= ((isDataReady() ? 1 : 0) & 0x01) << 2;
		stat |= ((isKeyReady() ? 1 : 0) & 0x01) << 1;
		stat |= ((isBusy ? 1 : 0) & 0x01);
		return stat & 0xffff;
	}

	/* AESACTL0 register */
	private boolean advancedCipherMode = false;
	private boolean interruptEnable = false;
	private boolean errorFlag = false;
	private boolean readyInterruptFlag = false;
	private boolean resetFlag = false;
	private int cipherMode = MODE_ECB;
	private int keyLength = KEY_128;
	private int operation = AESOP_0;
	private int cipherBlockCounter = 0;

	/**
	 * CTL0 Register built upon variables
	 * 
	 * @return uint16_t register
	 */
	private int getCTL0Reg() {
		int ctl0 = 0;
		ctl0 |= ((advancedCipherMode ? 1 : 0) & 0x01) << 15;
		ctl0 |= ((interruptEnable ? 1 : 0) & 0x01) << 12;
		ctl0 |= ((errorFlag ? 1 : 0) & 0x01) << 11;
		ctl0 |= ((readyInterruptFlag ? 1 : 0) & 0x01) << 8;
		ctl0 |= ((resetFlag ? 1 : 0) & 0x01) << 7;
		ctl0 |= (cipherMode & 0x03) << 5;
		ctl0 |= (keyLength & 0x03) << 2;
		ctl0 |= (operation & 0x03);
		return ctl0 & 0xffff;
	}

	private int getCTL1Reg() {
		int ctl1 = cipherBlockCounter;
		return ctl1 & 0x000f;
	}

	/**
	 * Java implementation of the AES encryption algorithm
	 * 
	 * This method encrypts whatever is in inData using key and sets it into
	 * outData
	 */
	private void aesEncrypt() {
		log("encrypt");
		Cipher cipher;
		SecretKeySpec spec = new SecretKeySpec(key.array(), "AES");
		try {
			cipher = Cipher.getInstance("AES/ECB/NoPadding");
			cipher.init(Cipher.ENCRYPT_MODE, spec);
			byte[] bytes = cipher.doFinal(inData.array());
			outData.clear();
			outData.put(bytes);
			outData.resetPos();
		} catch (Exception e) {
			log(e.getStackTrace().toString());
		}
	}

	/**
	 * Java implementation of the AES decryption algorithm
	 * 
	 * This method decrypts whatever is in inData using key and sets it into
	 * outData
	 */
	private void aesDecrypt() {
		log("decrypt");
		Cipher cipher;
		SecretKeySpec spec = new SecretKeySpec(key.array(), "AES");
		try {
			cipher = Cipher.getInstance("AES/ECB/NoPadding");
			cipher.init(Cipher.DECRYPT_MODE, spec);
			byte[] bytes = cipher.doFinal(inData.array());
			outData.clear();
			outData.put(bytes);
			outData.resetPos();
		} catch (Exception e) {
			log(e.getStackTrace().toString());
		}
	}

	/*
	 * The inherited log function is not working for whatever reason. A quick
	 * redefinition helps a lot while developing the module
	 */
	@Override
	protected void log(String msg) {
		if (DEBUG) {
			System.out.println(msg);
		}
	}

	/**
	 * Log using printf format
	 * 
	 * @param format
	 * @param arguments
	 */
	protected void log(final String format, final Object... arguments) {
		if (DEBUG) {
			System.out.printf(format, arguments);
		}
	}

	/**
	 * The registers are written
	 */
	public void write(int address, int value, boolean word, long cycles) {
		log("write @ %x <-- %x (word=%b)\n", address, value, word);
		int lo = (value) & 0xff; // low byte
		int hi = (value >> 8) & 0xff; // high byte

		switch (address - offset) {
		case AESACTL0:
			if ((value & AESSWRST) == AESSWRST) {
				reset(0);
			}

			if (!(advancedCipherMode && cipherBlockCounter > 0)) {
				advancedCipherMode = ((value & AESCMEN) == AESCMEN);
			}

			interruptEnable = ((value & AESRDYIE) == AESRDYIE);
			errorFlag = ((value & AESERRFG) == AESERRFG);
			readyInterruptFlag = ((value & AESRDYIFG) == AESRDYIFG);

			if (!(advancedCipherMode && cipherBlockCounter > 0)
					&& !advancedCipherMode) {
				cipherMode = ((value & 0x60) >> 5);
			}

			keyLength = ((value & 0xC) >> 2);

			if (!(advancedCipherMode && cipherBlockCounter > 0)) {
				operation = value & 0x03;
			}
			break;

		case AESACTL1:
			/* Only lower byte allowed */
			if (!(advancedCipherMode && cipherBlockCounter > 0)) {
				value &= 0x000f;
				cipherBlockCounter = value;
			}

			break;

		case AESASTAT:
			/* Only two fields can be written in this register */
			value &= (AESDINWR | AESKEYWR);

			if ((value & AESKEYWR) == 0) {
				/*
				 * This flag can only be cleared if the advanceModeSupport is not
				 * enabled (ref. User Guide)
				 */
				if (!advancedCipherMode) {
					value |= AESKEYWR;
				} else {
					key.resetPos();
				}
			} else {
				isBusy = true;
				key.position(key.limit());
				switch (operation) {
				case AESOP_0: // encrypt
					aesEncrypt();
					break;
				case AESOP_1: // decrypt
					aesDecrypt();
					break;
				case AESOP_2: // gen 1st round key
					logw(WarningType.ILLEGAL_IO_WRITE, "to implement");
					break;
				case AESOP_3: // decrypt 1st round key
					logw(WarningType.ILLEGAL_IO_WRITE, "to implement");
					break;
				}
				isBusy = false;
				readyInterruptFlag = true;
			}

			if ((value & AESDINWR) == 0) {
				if (!advancedCipherMode) {
					value |= AESKEYWR;
				} else {
					inData.resetPos();
				}
			} else {
				inData.position(inData.limit());
			}

			break;

		case AESAKEY:
			if (key.hasRemaining()) {
				/* Clear the ready interrupt flag */
				readyInterruptFlag = false;
				key.put((byte) lo);
				if (word && key.hasRemaining()) {
					key.put((byte) hi);
				}
			}
			break;

		case AESADIN:
			if (inData.hasRemaining()) {
				/* Clear the ready interrupt flag */
				readyInterruptFlag = false;
				inData.put((byte) lo);
				if (word && inData.hasRemaining()) {
					inData.put((byte) hi);
				}
			}
			break;
		}

		log("ctl0: %04x\nstat: %04x\n", getCTL0Reg(), getStatReg());
	}

	/**
	 * Registers are read
	 */
	public int read(int address, boolean word, long cycles) {
		log("read %x (word?%b)\n", address, word);
		switch (address - offset) {
		case AESACTL0:
			return getCTL0Reg();
		case AESACTL1:
			if (advancedCipherMode) {
				return cipherBlockCounter;
			}
		case AESASTAT:
			return getStatReg();
		case AESADOUT:
			if (isDataReady()) {
				/* Clear the ready interrupt flag */
				readyInterruptFlag = false;
				if (outData.hasRemaining()) {
					int temp = outData.get() & 0xff;
					if (word && outData.hasRemaining()) {
						temp |= (outData.get() << 8) & 0xff00;
					}
					return temp & 0xffff;
				}
			}
		}
		return 0;
	}

	public void interruptServiced(int vector) {
		if (vector == AES_VECTOR) {
			readyInterruptFlag = false;
		}
	}
}
