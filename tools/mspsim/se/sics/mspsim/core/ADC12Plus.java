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
 * ADC12
 *
 * Each time a sample is converted the ADC12 system will check for EOS flag
 * and if not set it just continues with the next conversion (x + 1). 
 * If EOS next conversion is startMem.
 * Interrupt is triggered when the IE flag are set! 
 *
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

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

import java.util.Arrays;

import se.sics.mspsim.core.EmulationLogger.WarningType;


/**
 * ADC12 Plus IO peripheral
 * 
 * This module extends the functionality of the ADC12 by adding one
 * configuration register which controls the power consumption of this
 * peripheral. Additionally, implements several resolutions (8 to 12-bit)
 * 
 * The rest of functionalities are the same as the standard ADC12
 * 
 * @author Joakim Eriksson <joakime@sics.se>
 * @author Víctor Ariño <victor.arino@tado.com>
 */
public class ADC12Plus extends IOUnit {

	/**
	 * Address and size for IO configuration
	 */
	public static int OFFSET = 0x0700;
	public static int SIZE = 0x3e;

	public static final int ADC12CTL0 = 0x00;// Reset with POR
	public static final int ADC12CTL1 = 0x02;// Reset with POR
	public static final int ADC12CTL2 = 0x04;// Reset with POR XXX

	public static final int ADC12IFG = 0x0a; // Reset with POR
	public static final int ADC12IE = 0x0c; // Reset with POR
	public static final int ADC12IV = 0x0e; // Reset with POR

	public static final int ADC12MEM0 = 0x20; // Unchanged
	public static final int ADC12MEM1 = 0x22; // Unchanged
	public static final int ADC12MEM2 = 0x24; // Unchanged
	public static final int ADC12MEM3 = 0x26; // Unchanged
	public static final int ADC12MEM4 = 0x28; // Unchanged
	public static final int ADC12MEM5 = 0x2A; // Unchanged
	public static final int ADC12MEM6 = 0x2C; // Unchanged
	public static final int ADC12MEM7 = 0x2E; // Unchanged
	public static final int ADC12MEM8 = 0x30; // Unchanged
	public static final int ADC12MEM9 = 0x32; // Unchanged
	public static final int ADC12MEM10 = 0x34; // Unchanged
	public static final int ADC12MEM11 = 0x36; // Unchanged
	public static final int ADC12MEM12 = 0x38; // Unchanged
	public static final int ADC12MEM13 = 0x3A; // Unchanged
	public static final int ADC12MEM14 = 0x3C; // Unchanged
	public static final int ADC12MEM15 = 0x3E; // Unchanged

	public static final int ADC12MCTL0 = 0x0010; // Reset with POR
	public static final int ADC12MCTL1 = 0x0011; // Reset with POR
	public static final int ADC12MCTL2 = 0x0012; // Reset with POR
	public static final int ADC12MCTL3 = 0x0013; // Reset with POR
	public static final int ADC12MCTL4 = 0x0014; // Reset with POR
	public static final int ADC12MCTL5 = 0x0015; // Reset with POR
	public static final int ADC12MCTL6 = 0x0016; // Reset with POR
	public static final int ADC12MCTL7 = 0x0017; // Reset with POR
	public static final int ADC12MCTL8 = 0x0018; // Reset with POR
	public static final int ADC12MCTL9 = 0x0019; // Reset with POR
	public static final int ADC12MCTL10 = 0x001A; // Reset with POR
	public static final int ADC12MCTL11 = 0x001B; // Reset with POR
	public static final int ADC12MCTL12 = 0x001C; // Reset with POR
	public static final int ADC12MCTL13 = 0x001D; // Reset with POR
	public static final int ADC12MCTL14 = 0x001E; // Reset with POR
	public static final int ADC12MCTL15 = 0x001F; // Reset with POR

	public static final int[] SHTBITS = new int[] { 4, 8, 16, 32, 64, 96, 128,
			192, 256, 384, 512, 768, 1024, 1024, 1024, 1024 };

	public static final int BUSY_MASK = 0x01;
	public static final int EOS_MASK = 0x80;

	public static final int CONSEQ_SINGLE = 0x00;
	public static final int CONSEQ_SEQUENCE = 0x01;
	public static final int CONSEQ_REPEAT_SINGLE = 0x02;
	public static final int CONSEQ_REPEAT_SEQUENCE = 0x03;
	public static final int CONSEQ_SEQUENCE_MASK = 0x01;

	private int adc12ctl0 = 0;
	private int adc12ctl1 = 0;
	private int adc12ctl2 = 0;
	private int[] adc12mctl = new int[16];
	private int[] adc12mem = new int[16];
	private int adc12Pos = 0;

	private int shTime0 = 4;
	private int shTime1 = 4;
	private boolean adc12On = false;
	private boolean enableConversion;
	private boolean startConversion;
	private boolean isConverting;

	private int shSource = 0;
	private int startMem = 0;
	private int adcDiv = 1;

	private ADCInput adcInput[] = new ADCInput[16];

	private int conSeq;
	private int adc12ie;
	private int adc12ifg;
	private int adc12iv;

	private int adcSSel;
	private int adc12Vector = 0x38;

	private TimeEvent adcTrigger = new TimeEvent(0) {
		public void execute(long t) {
			// System.out.println(getName() + " **** executing update timers at " +
			// t + " cycles=" + cpu.cycles);
			convert();
		}
	};

	/* These are CTL2 variables */
	private int bitsResolution = 12;
	private boolean formatSigned = false;
	private int clockPredivider = 1;

	/* Reference voltage 2.5V or 1.5V */
	private boolean ref25V = false;

	public ADC12Plus(MSP430Core cpu, int offset, int intVector) {
		super("ADC12Plus", cpu, cpu.memory, offset);
		adc12Vector = intVector;
	}

	public void reset(int type) {
		enableConversion = false;
		startConversion = false;
		isConverting = false;
		adc12ctl0 = 0;
		adc12ctl1 = 0;
		adc12ctl2 = 0;
		shTime0 = shTime1 = 4;
		adc12On = false;
		shSource = 0;
		startMem = adc12Pos = 0;
		adcDiv = 1;

		conSeq = 0;
		adc12ie = 0;
		adc12ifg = 0;
		adc12iv = 0;
		adcSSel = 0;

		Arrays.fill(adc12mctl, 0);

		clockPredivider = 1;
		formatSigned = false;
		bitsResolution = 12;
		ref25V = false;
	}

	public void setADCInput(int adindex, ADCInput input) {
		adcInput[adindex] = input;
	}

	/**
	 * Get the maximum input voltage set by the configuration of the registers
	 * (in mV)
	 * 
	 * @return
	 */
	public int getMaxInVoltage() {
		if (ref25V) {
			return 2500;
		} else {
			return 1500;
		}
	}

	/**
	 * Get the minimum in voltage set by the registers (in mV)
	 * 
	 * @return
	 */
	public int getMinInVoltage() {
		// TODO
		return 0;
	}

	// write a value to the IO unit
	public void write(int address, int value, boolean word, long cycles) {
		address -= offset;
		switch (address) {
		case ADC12CTL0:
			if (enableConversion) {
				// Ongoing conversion: only some parts may be changed
				adc12ctl0 = (adc12ctl0 & 0xfff0) + (value & 0xf);
			} else {
				adc12ctl0 = value;
				shTime0 = SHTBITS[(value >> 8) & 0x0f];
				shTime1 = SHTBITS[(value >> 12) & 0x0f];
				adc12On = (value & 0x10) > 0;
			}
			enableConversion = (value & 0x02) > 0;
			startConversion = (value & 0x01) > 0;
			ref25V = (value & 0x20) > 0;

			if (DEBUG)
				log("Set SHTime0: " + shTime0 + " SHTime1: " + shTime1 + " ENC:"
						+ enableConversion + " Start: " + startConversion
						+ " ADC12ON: " + adc12On);
			if (adc12On && enableConversion && startConversion && !isConverting) {
				// Set the start time to be now!
				isConverting = true;
				adc12Pos = startMem;
				int delay = clockPredivider * adcDiv
						* ((adc12Pos < 8 ? shTime0 : shTime1) + 13);
				cpu.scheduleTimeEvent(adcTrigger, cpu.getTime() + delay);
			}
			break;
		case ADC12CTL1:
			if (enableConversion) {
				// Ongoing conversion: only some parts may be changed
				adc12ctl1 = (adc12ctl1 & 0xfff8) + (value & 0x6);
			} else {
				adc12ctl1 = value & 0xfffe;
				startMem = (value >> 12) & 0xf;
				shSource = (value >> 10) & 0x3;
				adcDiv = ((value >> 5) & 0x7) + 1;
				adcSSel = (value >> 3) & 0x03;
			}
			conSeq = (value >> 1) & 0x03;
			if (DEBUG)
				log("Set startMem: " + startMem + " SHSource: " + shSource
						+ " ConSeq-mode:" + conSeq + " Div: " + adcDiv + " ADCSSEL: "
						+ adcSSel);
			break;

		case ADC12CTL2: /* Low Power Specs */
			if (enableConversion) {
				/*
				 * Clock pre divider can't be modified when conversion is already
				 * enabled
				 */
				value &= 0xfeff;
				value |= (adc12ctl2 & 0x100);
			}
			clockPredivider = ((value & 0x100) > 0) ? 4 : 1;
			/* bit resolution 8, 10, 12 */
			int tmp = (value & 0x30) >> 4;
			tmp = (tmp <= 2) ? tmp * 2 : 4;
			bitsResolution = tmp + 8;
			formatSigned = ((value & 0x08) > 0);
			if (formatSigned) {
				logw(WarningType.EMULATION_ERROR, "signed format not implemented");
			}
			adc12ctl2 = value;
			break;
		case ADC12IE:
			adc12ie = value;
			break;
		case ADC12IFG:
			adc12ifg = value;
			break;
		default:
			if (address >= ADC12MCTL0 && address <= ADC12MCTL15) {
				if (enableConversion) {
					/* Ongoing conversion: not possible to modify */
				} else {
					adc12mctl[address - ADC12MCTL0] = value & 0xff;
					if (DEBUG)
						log("ADC12MCTL" + (address - ADC12MCTL0) + " source = "
								+ (value & 0xf)
								+ (((value & EOS_MASK) != 0) ? " EOS bit set" : ""));
				}
			}
		}
	}

	// read a value from the IO unit
	public int read(int address, boolean word, long cycles) {
		address -= offset;
		switch (address) {
		case ADC12CTL0:
			return adc12ctl0;
		case ADC12CTL1:
			return isConverting ? (adc12ctl1 | BUSY_MASK) : adc12ctl1;
		case ADC12IE:
			return adc12ie;
		case ADC12IFG:
			return adc12ifg;
		default:
			if (address >= ADC12MCTL0 && address <= ADC12MCTL15) {
				return adc12mctl[address - ADC12MCTL0];
			} else if (address >= ADC12MEM0 && address <= ADC12MEM15) {
				int reg = (address - ADC12MEM0) / 2;
				// Clear ifg!
				adc12ifg &= ~(1 << reg);
				// System.out.println("Read ADCMEM" + (reg / 2));
				if (adc12iv == reg * 2 + 6) {
					cpu.flagInterrupt(adc12Vector, this, false);
					adc12iv = 0;
					// System.out.println("** de-Trigger ADC12 IRQ for ADCMEM" +
					// adc12Pos);
				}
				return adc12mem[reg];
			}
		}
		return 0;
	}

	int smp = 0;

	private void convert() {
		// If off then just return...
		if (!adc12On) {
			isConverting = false;
			return;
		}
		boolean runAgain = enableConversion && conSeq != CONSEQ_SINGLE;
		// Some noise...
		ADCInput input = adcInput[adc12mctl[adc12Pos] & 0xf];
		int reading = input != null ? input.nextData() : 2048 + 100 - smp & 255;
		/* Adapt resolution to 8, 10, 12 bits */
		float percent = (reading - getMinInVoltage()) * 1f
				/ ((getMaxInVoltage() - getMinInVoltage()) * 1f);
		reading = (int) (((1L << bitsResolution) - 1) * percent);
		adc12mem[adc12Pos] = reading;
		smp += 7;
		adc12ifg |= (1 << adc12Pos);
		if ((adc12ie & (1 << adc12Pos)) > 0) {
			// This should check if there already is an higher iv!
			adc12iv = adc12Pos * 2 + 6;
			// System.out.println("** Trigger ADC12 IRQ for ADCMEM" + adc12Pos);
			cpu.flagInterrupt(adc12Vector, this, true);
		}
		if ((conSeq & CONSEQ_SEQUENCE_MASK) != 0) {
			// Increase
			if ((adc12mctl[adc12Pos] & EOS_MASK) == EOS_MASK) {
				adc12Pos = startMem;
				if (conSeq == CONSEQ_SEQUENCE) {
					// Single sequence only
					runAgain = false;
				}
			} else {
				adc12Pos = (adc12Pos + 1) & 0x0f;
			}
		}
		if (!runAgain) {
			isConverting = false;
		} else {
			int delay = clockPredivider * adcDiv
					* ((adc12Pos < 8 ? shTime0 : shTime1) + 13);
			cpu.scheduleTimeEvent(adcTrigger, adcTrigger.getTime() + delay);
		}
	}

	public void interruptServiced(int vector) {
	}

	/**
	 * Get the reference voltage if it is 1.5 volts or 2.5V
	 * 
	 * @return
	 */
	public boolean isRef25V() {
		return ref25V;
	}

}
