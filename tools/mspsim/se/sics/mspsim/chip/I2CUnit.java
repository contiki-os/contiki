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

package se.sics.mspsim.chip;

import se.sics.mspsim.core.GenericUSCI;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.TimeEvent;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;

/**
 * Simple I2C unit
 * 
 * Any class extending this unit can use the i2c protocol by just implementing
 * two methods for reading and writing registers.
 * 
 * @author Víctor Ariño <victor.arino@tado.com>
 */
public abstract class I2CUnit implements USARTListener {

	/**
	 * Auxiliar class for emulating the i2c signaling. This is necessary as
	 * there's no line messages in the emulator.
	 * 
	 * @author Víctor Ariño <victor.arino@tado.com>
	 */
	public class I2CData {

		/**
		 * Several mask for messages. This may not implement a real i2c but is
		 * useful for message passing
		 */
		public final static int START = 0x100;
		public final static int STOP = 0x200;
		public final static int ACK = 0x400;
		public final static int NACK = 0x800;

		private int data;

		public I2CData(int data) {
			this.data = data;
		}

		protected int getData() {
			return data & 0xff;
		}

		protected boolean isStart() {
			return (data & START) == START;
		}

		protected boolean isStop() {
			return (data & STOP) == STOP;
		}

		protected boolean isAck() {
			return data == ACK;
		}

		protected int getAddress() {
			if (isStart()) {
				return (data & 0xfe) >> 1;
			}
			return 0;
		}

		protected boolean isRead() {
			if (isStart()) {
				return (data & 0x01) != 0x01;
			}
			return false;
		}
	}

	/**
	 * Address of the I2C peripheral
	 */
	protected int busAddress = 0x00;

	/**
	 * String name for logging
	 */
	protected String name = "Unknown";

	/**
	 * Enable debug
	 */
	protected boolean DEBUG = false;

	/**
	 * Is a read message or a write
	 */
	private boolean isRead = false;

	/**
	 * Memory address of the peripheral to read/write
	 */
	private int registerAddress = 0x00;

	/**
	 * Length in bytes of the bus address
	 */
	protected int regAddressLen = 1;

	/**
	 * Length in bytes of the buffer value
	 */
	protected int numBytesTotal = 2;

	/**
	 * Number of received value bytes
	 */
	private int numBytesRxTx = 0;

	/**
	 * Received bytes of the address
	 */
	private int regAddressBytesRx = 0;

	/**
	 * Value to read/write
	 */
	private int value;

	/**
	 * Is the peripheral ready to receive / send data?
	 */
	private boolean ready = false;

	private MSP430Core cpu;
	private USARTSource source;
	private boolean txScheduled = false;

	/**
	 * Tx Event Handler
	 */
	private TimeEvent txTrigger = new TimeEvent(0) {
		public void execute(long t) {
			/* Transmit the next pending byte to the uC */
			if (numBytesTotal > numBytesRxTx) {
				int tmp = (value >> ((numBytesTotal - numBytesRxTx - 1) * 8)) & 0xff;
				log("<sent> " + tmp);
				source.byteReceived(tmp);
				numBytesRxTx++;
			}
			txScheduled = false;
		}
	};

	/**
	 * Class constructor
	 * 
	 * @param name
	 * @param address
	 * @param src
	 * @param cpu
	 */
	public I2CUnit(String name, int address, USARTSource src, MSP430Core cpu) {
		this.name = name;
		busAddress = address;
		if (src != null) {
			src.addUSARTListener((USARTListener) this);
		}
		this.cpu = cpu;
		source = src;
	}

	/**
	 * The microcontroller requested to write a register of the i2c peripheral
	 * 
	 * @param address
	 *           address of the register
	 * @param value
	 *           value to write
	 */
	protected abstract void registerWrite(int address, int value);

	/**
	 * Read a register of the peripheral
	 * 
	 * @param address
	 *           address of the register to read
	 * @return write this value
	 */
	protected abstract int registerRead(int address);

	@Override
	public void dataReceived(USARTSource source, int data) {

		I2CData d = new I2CData(data);

		if (d.isStart()) {
			if (d.getAddress() != busAddress) {
				// Message not for us!
				return;
			}

			isRead = d.isRead();
			numBytesRxTx = 0;
			ready = true;

			log("<start> read?" + isRead);

			source.byteReceived(I2CData.ACK);

			/*
			 * Read the register if necessary. When a write reset the memory
			 * address to avoid strange situations
			 */
			if (isRead) {
				value = registerRead(registerAddress);
				scheduleTransmission();
			} else {
				registerAddress = 0x00;
				value = 0;
				regAddressBytesRx = 0;
			}
			return;
		}

		if (ready) {
			/*
			 * In case of stop condition write the register if necessary and set
			 * the end of the message acceptance
			 */
			if (d.isStop()) {
				log("<stop>");
				if (!isRead && numBytesRxTx > 1) {
					registerWrite(registerAddress, value);
				}

				ready = false;
				return;
			}

			/* If the master transmits and ACK we can transmit the next byte */
			if (d.isAck() && isRead) {
				log("<ack>");
				if (numBytesRxTx < numBytesTotal) {
					scheduleTransmission();
				}
			}

			/*
			 * Write messages are for the address and for the value. First the
			 * address of the register is received. Afterwards the value itself
			 */
			if (!isRead) {
				if (regAddressBytesRx < regAddressLen) {
					log("<addr>");
					registerAddress = d.getData();
					regAddressBytesRx++;
				} else if (numBytesRxTx < numBytesTotal) {
					value <<= 8;
					value |= d.getData();
					numBytesRxTx++;
					log("<data>" + value);
				} else {
					logw("Received more bytes than expected!");
				}
				source.byteReceived(I2CData.ACK);
			}
		}
	}

	/**
	 * Schedule a transmission with the correct baud rate
	 */
	private void scheduleTransmission() {
		int baudRate = defaultBaudRate;

		if (source instanceof GenericUSCI) {
			baudRate = ((GenericUSCI) source).getBaudRate();
		}

		if (!txScheduled && numBytesRxTx < numBytesTotal) {
			cpu.scheduleCycleEvent(txTrigger, cpu.cpuCycles + 1);
			txScheduled = true;
		}
	}

	/**
	 * Log a message when the DEBUG flag is set
	 * 
	 * @param msg
	 */
	protected void log(String msg) {
		if (DEBUG) {
			System.out.println("(i2c) " + name + ": " + msg);
		}
	}

	/**
	 * Log a warning message
	 * 
	 * @param msg
	 */
	protected void logw(String msg) {
		System.err.println("(i2c) " + name + ": WARNING " + msg);
	}

	/**
	 * Default baud rate for the communications
	 */
	private int defaultBaudRate = 100;

	protected void setDefaultBaudRate(int br) {
		defaultBaudRate = br;
	}

}
