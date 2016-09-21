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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * Leds
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 17 jul 2010
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.chip;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.util.Utils;

public class Leds extends Chip {

    private final int[] ledColors;

    private int leds;

    public Leds(MSP430Core cpu, int[] ledColors) {
        super("Leds", cpu);
        if (ledColors == null) {
            throw new NullPointerException("ledColors");
        }
        this.ledColors = ledColors;
    }

    public int getLeds() {
        return leds;
    }

    public void setLeds(int leds) {
        if (this.leds != leds) {
            this.leds = leds;
            stateChanged(leds);
            if (DEBUG) log(ledColors.length <= 8 ? Utils.binary8(leds) : Utils.binary16(leds));
        }
    }

    public void setLeds(int leds, boolean on) {
        if (on) {
            setLeds(this.leds | leds);
        } else {
            setLeds(this.leds & ~leds);
        }
    }

    public boolean isLedOn(int led) {
        return (leds & (1 << led)) != 0;
    }


    public int getLedsColor(int led) {
        return ledColors[led];
    }

    public int getLedsCount() {
        return ledColors.length;
    }

    @Override public int getModeMax() {
        return 0;
    }

    @Override public String info() {
        return "Leds: " + (ledColors.length <= 8 ? Utils.binary8(leds) : Utils.binary16(leds));
    }

    @Override public int getConfiguration(int parameter) {
        return 0;
    }

}
