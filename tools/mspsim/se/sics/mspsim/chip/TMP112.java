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

import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.USARTSource;

/**
 * Temperature chipset TI tmp112 emulation
 * 
 * @author Víctor Ariño <victor.arino@tado.com>
 */
public class TMP112 extends I2CUnit implements TemperatureChip {

	private int temperature = 0; // in degrees
	private int config = 0x60a0; // default config

	private boolean extendedMode = false;
	private boolean polarity = false;
	private boolean alarm = false;
	private boolean shutdown = false;
	private boolean thermostat = false;
	private int resolution = 0x03;
	private boolean oneShot = false;

	private float[] resFactors = { 50f, 25f, 12.5f, 6.25f };

	public static final int TEMP_REG = 0;
	public static final int CONFIG_REG = 1;
	public static final int TEMP_L_REG = 2;
	public static final int TEMP_H_REG = 3;

	public TMP112(USARTSource src, MSP430Core cpu) {
		super("tmp112", 0x48, src, cpu);
	}

	@Override
	protected int registerRead(int address) {
		switch (address) {
		case TEMP_REG:
			return getRawTemperature() & 0xffff;
		case CONFIG_REG:
			return config;
		case TEMP_L_REG:
		case TEMP_H_REG:
			logw("not implemented");
			break;
		}
		return 0;
	}

	@Override
	protected void registerWrite(int address, int value) {
		switch (address) {
		case CONFIG_REG:
			config = value;
			/* Parse some of the configurations */
			polarity = ((value & 0x400) > 0);
			extendedMode = ((value & 0x10) > 0);
			alarm = ((value & 0x020) > 0);
			shutdown = ((value & 0x100) > 0);
			thermostat = ((value & 0x200) > 0);
			resolution = ((value & 0x6000) >> 13);
			oneShot = ((value & 0x8000) > 0);
			break;
		default:
			logw("not implemented");
			break;
		}
	}

	/**
	 * Get the raw temperature
	 * 
	 * The temperature in the class is stored in a user-friendly way, however the
	 * chipset must transmit it in a specific way without resolution conversions.
	 * This is done by this function: convert a temperature into the format that
	 * the tmp112 would send it to via i2c.
	 * 
	 * @return
	 */
	private synchronized int getRawTemperature() {
		int tmp = (int) (Math.abs(temperature) / resFactors[resolution]);
		if (temperature < 0) {
			/* Do the two-complements value and mask it */
			int nbits = 9 + resolution + (extendedMode ? 1 : 0);
			int mask = (int) ((1L << nbits) - 1);
			tmp = ~tmp + 1;
			tmp &= mask;
		}
		return tmp << 4;
	}

	@Override
	public synchronized int getTemperature() {
		return temperature;
	}

	@Override
	public synchronized void setTemperature(int temp) {
		if (temp <= getMaxTemperature() && temp >= getMinTemperature()) {
			temperature = temp;
		}
	}

	@Override
	public int getMaxTemperature() {
		int nbits = 9 + resolution + (extendedMode ? 1 : 0);
		nbits -= 1;
		return (int) (((1L << nbits) - 1) * resFactors[resolution]);
	}

	@Override
	public int getMinTemperature() {
		return -4000;
	}

	public int getConfig() {
		return config;
	}

	public boolean isExtendedMode() {
		return extendedMode;
	}

	public boolean isPolarity() {
		return polarity;
	}

	public boolean isAlarm() {
		return alarm;
	}

	public boolean isShutdown() {
		return shutdown;
	}

	public boolean isThermostat() {
		return thermostat;
	}

	public int getResolution() {
		return resolution;
	}

	public boolean isOneShot() {
		return oneShot;
	}

}
