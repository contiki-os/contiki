/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * -----------------------------------------------------------------
 *
 * MMA7260QT
 *
 * Authors : Niclas Finne
 * Created : 7 jul 2010
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.chip;
import se.sics.mspsim.core.MSP430Core;

/**
 * MMA7260QT - 1.5g-6g Three Axis Low-g Micromachined Accelerometer
 */
public class MMA7260QT extends Accelerometer {

    public static final int MODE_SLEEP = 0x00;
    public static final int MODE_ACTIVE = 0x01;
    private static final String[] MODE_NAMES = new String[] {
      "sleep", "active"
    };
    private static final float[] GSELECT = new float[] {
        1.5f, 2, 4, 6
    };

    private int gSelect = 0;

    public MMA7260QT(MSP430Core cpu) {
        super("MMA7260QT", cpu);
        setModeNames(MODE_NAMES);
        setMode(MODE_SLEEP);

        // Set initial state: x=4094, y=2715, z=2715
        setPosition(1.0, -0.0156, -0.0156);
    }

    private int convertToADC(double x) {
        if (x > 1.0) {
            x = 1.0;
        } else if (x < -1.0) {
            x = -1.0;
        }
        return 2047 + (int) (x * 2047);
    }

    public int getADCX() {
        return convertToADC(getX());
    }

    public int getADCY() {
        return convertToADC(getY());
    }

    public int getADCZ() {
        return convertToADC(getZ());
    }

    public int getSensitivity() {
        return gSelect;
    }

    public String getSensitivityAsString() {
        return GSELECT[gSelect] + "g (" + (int)(1200/GSELECT[gSelect]) + "mV/g)";
    }

    public void setSensitivity(int gSelect) {
        this.gSelect = gSelect & 0x03;
    }

    @Override
    public void setMode(int mode) {
        super.setMode(mode);
    }

    @Override
    public int getModeMax() {
        return MODE_NAMES.length;
    }

    @Override
    public String info() {
        return "Mode: " + getModeName(getMode())
        + " Sensitivity: " + getSensitivityAsString()
        + String.format(" [x=%.2f (%d),y=%.2f (%d),z=%.2f (%d)]",
                getX(), getADCX(), getY(), getADCY(), getZ(), getADCZ());
    }

    /* currently just return the gSelect as configuration */
    @Override
    public int getConfiguration(int parameter) {
        return gSelect;
    }

}
