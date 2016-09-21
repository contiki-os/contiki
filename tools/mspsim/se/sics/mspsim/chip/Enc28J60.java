/*
 * Copyright (c) 2013, Thingsquare.
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

/* This is a stub implementation for the Enc28j60 Ethernet chip. This ignores
 * the majority of configuration and setup, and only provides the very minimum
 * functionality to support a simple bit-banged Thingsquare Mist driver. */

package se.sics.mspsim.chip;

import java.util.ArrayList;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOPort.PinState;
import se.sics.mspsim.core.MSP430Core;

public class Enc28J60 extends Chip {
	protected boolean DEBUG = false;

	public static final int EIE = 0x1b;
	public static final int EIR = 0x1c;
	public static final int ESTAT = 0x1d;
	public static final int ECON2 = 0x1e;
	public static final int ECON1 = 0x1f;

	public static final int ESTAT_CLKRDY = 0x01;
	public static final int ESTAT_TXABRT = 0x02;

	public static final int ECON1_RXEN = 0x04;
	public static final int ECON1_TXRTS = 0x08;

	public static final int ECON2_AUTOINC = 0x80;
	public static final int ECON2_PKTDEC = 0x40;

	public static final int EPKTCNT = 0x19;

	private static final int WBM_COMMAND = 0x3a; /* Note: 0x7a with write bit set */
	private static final int RBM_COMMAND = 0x3a; /* Note: 0x7a with write bit set */

	private IOPort myPort;

	private int myClk;
	private int myMosi;
	private int myChipSelect;
	private int myMisoBit;

	public Enc28J60(MSP430Core cpu, IOPort port, int clk, int mosi,
			int miso, int chipSelect) {
		super("Enc28J60", "Ethernet", cpu);

		myPort = port;
		myClk = (1 << clk);
		myMosi = (1 << mosi);
		myChipSelect = (1 << chipSelect);

		myMisoBit = miso;
	}

	public void log(String msg) {
		if (DEBUG) {
			System.out.println(msg);
		}
	}

	private boolean writingToWBM = false;
	private boolean readingFromRBM = false;
	private boolean nextEcon1 = false;
	private boolean nextEcon2 = false;

	private ArrayList<Byte> wbmData = new ArrayList<Byte>();

	private ArrayList<RbmPacket> rbmPackets = new ArrayList<RbmPacket>();
	private static class RbmPacket {
		ArrayList<Byte> data = new ArrayList<Byte>();
		boolean wasRead = false;
	}
	
	public void writePacket(byte[] data) {
		RbmPacket p = new RbmPacket();
		int len = data.length;

		p.data.add(new Byte((byte) 0x00)); /* ignored: next packet pointer */
		p.data.add(new Byte((byte) 0x00)); /* ignored: next packet pointer */

		p.data.add(new Byte((byte) (len & 0xff))); /* length */
		p.data.add(new Byte((byte) ((len >> 8) & 0xff))); /* length */

		p.data.add(new Byte((byte) 0x00)); /* ignored: status */
		p.data.add(new Byte((byte) 0x00)); /* ignored: status */

		for (byte b : data) {
			p.data.add(new Byte(b)); /* data */
		}
		
		rbmPackets.add(p);
		log("Enc28j60: nr pending packets increased to: " + rbmPackets.size());
	}

	private PacketListener listener = null;
	public static interface PacketListener {
		public void packetSent(Byte[] packetData);
	}
	public void setPacketListener(PacketListener l) {
		listener = l;
	}
	
	private int inputByte(int data) {
		int val = 0x00;

		if (writingToWBM) {
			wbmData.add(new Byte((byte) data));
			val = 0x00;
			return val;
		} else if (readingFromRBM) {
			if (rbmPackets.size() > 0) {
				if (rbmPackets.get(0).data.isEmpty()) {
					log("Enc28j60: warning, packet data is already consumed, returning 0");
				} else {
					val = rbmPackets.get(0).data.remove(0);
					rbmPackets.get(0).wasRead = true;
				}
			} else {
				log("Enc28j60: warning, no packet in rbm, returning 0");
				val = 0x00;
			}
			return val;
		}
		

		/* Strip optional write flag */
		boolean writing = (data & 0x40) != 0;
		data = (data & ~0x40);

		if (nextEcon1) {
			if ((data & ECON1_TXRTS) != 0) {
				log("Transmitting enc28j60 packet, size: " + wbmData.size());
				if (listener != null) {
					wbmData.remove(0);
					listener.packetSent(wbmData.toArray(new Byte[0]));
				}
				wbmData.clear();
			}
			nextEcon1 = false;
		} else if (nextEcon2) {
			if (!rbmPackets.isEmpty() && rbmPackets.get(0).wasRead) {
				rbmPackets.remove(0);
				log("Enc28j60: nr pending packets decreased to: " + rbmPackets.size());
			}
			nextEcon2 = false;
		} else if (data == ECON1) {
			/* we are awaiting a ECON1 command */
			nextEcon1 = true;
		} else if (data == ECON2) {
			/* we are awaiting a ECON2 command */
			nextEcon2 = true;
		} else if (data == EPKTCNT) {
			return rbmPackets.size();
		} else if (data == ESTAT) {
			/* chip is always ready */
			val = ESTAT_CLKRDY;
		} else if (writing && data == WBM_COMMAND) {
			writingToWBM = true;
		} else if (data == RBM_COMMAND) {
			if (rbmPackets.size() > 0) {
				if (rbmPackets.get(0).data.isEmpty()) {
					log("Enc28j60: warning, packet data is already consumed, returning 0");
				} else {
					val = rbmPackets.get(0).data.remove(0);
					rbmPackets.get(0).wasRead = true;
				}
			} else {
				log("Enc28j60: warning, no packet in rbm, returning 0");
				val = 0x00;
			}
			readingFromRBM = true;
		}
		return val;
	}


	private int spiOut = 0; /* byte being sent over SPI */
	private int spiOutCount = 0;
	private int spiIn = 0; /* byte being received over SPI */
	private int spiInCount = 0;
	public void write(IOPort port, int data) {
		if (port != myPort) {
			/* ignore  */
			return;
		}

		boolean chipSelect = (data & myChipSelect) == 0;
		boolean clk = (data & myClk) != 0;
		boolean mosi = (data & myMosi) != 0;

		if (!chipSelect) {
			if (readingFromRBM && spiOutCount == 0 && !rbmPackets.isEmpty()) {
				/* XXX Hack: pushing back last byte to rbm */
				rbmPackets.get(0).data.add(0, (byte)spiOut);
			}
			writingToWBM = false;
			readingFromRBM = false;
		}

		if (!chipSelect || !clk) {
			/* ignore */
			//log("write ignored: chipSelect " + chipSelect + ", clk " + clk);
			return;
		}

		/* Prepare next outgoing bit on miso */
		if (spiOutCount < 8) {
			spiOutCount++;

			if ((spiOut & 0x80) != 0) {
				port.setPinState(myMisoBit, PinState.HI);
			} else {
				port.setPinState(myMisoBit, PinState.LOW);
			}

			spiOut = (spiOut << 1);
		}

		/* Read next incoming bit on mosi */
		spiIn = (spiIn << 1);
		if (mosi) {
			spiIn |= 0x01;
		}
		spiInCount++;

		if (spiInCount >= 8) {
			/* We've now read all 8 bits on mosi */
			spiOut = inputByte(spiIn);
			spiOutCount = 0;
			spiIn = 0;
			spiInCount = 0;
		}
	}

	public int getConfiguration(int parameter) {
		return -1;
	}

	public int getModeMax() {
		return -1;
	}
}
