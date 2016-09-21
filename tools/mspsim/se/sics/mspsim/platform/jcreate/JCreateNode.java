/**
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * JCreateNode
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : 7 jul 2010
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.platform.jcreate;
import java.io.IOException;
import se.sics.mspsim.chip.FileStorage;
import se.sics.mspsim.chip.Leds;
import se.sics.mspsim.chip.M25P80;
import se.sics.mspsim.chip.MMA7260QT;
import se.sics.mspsim.core.ADC12;
import se.sics.mspsim.core.ADCInput;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.USARTSource;
import se.sics.mspsim.platform.sky.CC2420Node;
import se.sics.mspsim.util.ArgumentManager;

/**
 * Emulation of Sentilla JCreate Mote
 */
public class JCreateNode extends CC2420Node {

    public static final int MODE_LEDS_OFF = 0;
    public static final int MODE_MAX = 9;

    private static final int[] LEDS = {
        0xff2020, 0xff2020, 0xff2020, 0xff2020,
        0xff2020, 0xff2020, 0xff2020, 0xff2020
    };

    private Leds leds;
    private MMA7260QT accelerometer;
    private M25P80 flash;

    private JCreateGui gui;

    public JCreateNode() {
        super("Sentilla JCreate");
        setMode(MODE_LEDS_OFF);
    }

    public Leds getLeds() {
        return leds;
    }

    public MMA7260QT getAccelerometer() {
        return accelerometer;
    }

    public M25P80 getFlash() {
        return flash;
    }

    public void setFlash(M25P80 flash) {
        this.flash = flash;
        registry.registerComponent("xmem", flash);
    }

    // USART Listener
    @Override
    public void dataReceived(USARTSource source, int data) {
        radio.dataReceived(source, data);
        flash.dataReceived(source, data);
        /* if nothing selected, just write back a random byte to this device */
        if (!radio.getChipSelect() && !flash.getChipSelect()) {
            source.byteReceived(0);
        }
    }

    @Override
    protected void flashWrite(IOPort source, int data) {
        flash.portWrite(source, data);
    }

    @Override
    public void setupNodePorts() {
        super.setupNodePorts();
        leds = new Leds(cpu, LEDS);
        accelerometer = new MMA7260QT(cpu);
        ADC12 adc = cpu.getIOUnit(ADC12.class, "ADC12");
        adc.setADCInput(4, new ADCInput() {
            public int nextData() {
                return accelerometer.getADCX();
            }
        });
        adc.setADCInput(5, new ADCInput() {
            public int nextData() {
                return accelerometer.getADCY();
            }
        });
        adc.setADCInput(6, new ADCInput() {
            public int nextData() {
                return accelerometer.getADCZ();
            }
        });

        if (getFlash() == null) {
            setFlash(new M25P80(cpu));
        }
        if (flashFile != null) {
            getFlash().setStorage(new FileStorage(flashFile));
        }
    }

    public void setupGUI() {
        if (gui == null) {
            gui = new JCreateGui(this);
            registry.registerComponent("nodegui", gui);
        }
    }

    @Override
    public void portWrite(IOPort source, int data) {
        super.portWrite(source, data);

        if (source == port5) {
            if (leds.getLeds() != (~data & 0xff)) {
                leds.setLeds(~data & 0xff);

                int newMode = 0;
                for (int i = 0; i < 8; i++) {
                    if ((data & (1 << i)) != 0) {
                        newMode++;
                    }
                }
                setMode(newMode);
            }
        } else if (source == port2) {
            int out = source.getOut() & source.getDirection();
            if ((out & 0x08) == 0x08) {
                accelerometer.setMode(MMA7260QT.MODE_ACTIVE);
            } else {
                accelerometer.setMode(MMA7260QT.MODE_SLEEP);
            }
            accelerometer.setSensitivity(out & 0x03);
        }
      }

    @Override
    public int getModeMax() {
        return MODE_MAX;
    }

    public static void main(String[] args) throws IOException {
        JCreateNode node = new JCreateNode();
        ArgumentManager config = new ArgumentManager();
        config.handleArguments(args);
        node.setupArgs(config);
    }

}
