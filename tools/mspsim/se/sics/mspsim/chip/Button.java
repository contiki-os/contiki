/**
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 */
package se.sics.mspsim.chip;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.MSP430Core;

/**
 * @author Niclas Finne
 *
 */
public class Button extends Chip {

    private final IOPort port;
    private final int pin;
    private final boolean polarity;
    private boolean isPressed;

    public Button(String id, MSP430Core cpu, IOPort port, int pin, boolean polarity) {
        super(id, cpu);
        this.port = port;
        this.pin = pin;
        this.polarity = polarity;
    }

    public boolean isPressed() {
        return isPressed;
    }

    public void setPressed(boolean isPressed) {
        if (this.isPressed != isPressed) {
            this.isPressed = isPressed;
            stateChanged(isPressed ? 1 : 0);
            if (DEBUG) log(isPressed ? "pressed" : "released");
            port.setPinState(pin, isPressed == polarity ? IOPort.PinState.HI : IOPort.PinState.LOW);
        }
    }

    @Override
    public int getConfiguration(int parameter) {
        return 0;
    }

    @Override
    public int getModeMax() {
        return 0;
    }

    @Override
    public String info() {
        return " Button is " + (isPressed ? "pressed" : "not pressed");
    }
}
