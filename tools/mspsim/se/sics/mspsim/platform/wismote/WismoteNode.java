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
 * -----------------------------------------------------------------
 *
 * WismoteNode
 *
 * Author  : Niclas Finne
 */

package se.sics.mspsim.platform.wismote;
import java.io.IOException;
import se.sics.mspsim.chip.Button;
import se.sics.mspsim.chip.CC2520;
import se.sics.mspsim.chip.DS2411;
import se.sics.mspsim.chip.Leds;
import se.sics.mspsim.config.MSP430f5437Config;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.PortListener;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.ui.SerialMon;
import se.sics.mspsim.util.ArgumentManager;

public class WismoteNode extends GenericNode implements PortListener, USARTListener {

    // Port 1.
    public static final int DS2411_DATA_PIN = 1;
    public static final int DS2411_DATA = 1 << DS2411_DATA_PIN;

    /* P1.6 - Input: FIFOP from CC2520 */
    /* P1.5 - Input: FIFO from CC2520 */
    /* P1.7 - Input: CCA from CC2520 */
    public static final int CC2520_FIFOP = 6;
    public static final int CC2520_FIFO = 5;
    public static final int CC2520_CCA = 7;
    /* P2.0 - Input: SFD from CC2520 */
    public static final int CC2520_SFD = 0;
    /* P3.0 - Output: SPI Chip Select (CS_N) */
    public static final int CC2520_CHIP_SELECT = 0x01;
    /* P4.3 - Output: VREG_EN to CC2520 */
    public static final int CC2520_VREG = 1 << 3;
    /* P4.4 - Output: RESET_N to CC2520 */
    public static final int CC2520_RESET = 1 << 4;

    /* P1.4 - Button */
    public static final int BUTTON_PIN = 4;

    /* P8.6 - Red (left) led */
    private static final int LEDS_CONF_RED1   = 1 << 6;
    private static final int LEDS_RED1        = 1 << 0;
    /* P5.2 - Green (middle) led */
    private static final int LEDS_CONF_GREEN  = 1 << 4;
    private static final int LEDS_GREEN       = 1 << 1;
    /* P2.4 - Red (right) led */
    private static final int LEDS_CONF_RED2   = 1 << 2;
    private static final int LEDS_RED2        = 1 << 2;

    private static final int[] LEDS = { 0xff2020, 0x20ff20, 0xff2020 };

    //private M25P80 flash;
    //private String flashFile;
    private CC2520 radio;
    private Leds leds;
    private Button button;
    private WismoteGui gui;
    private DS2411 ds2411;

    public WismoteNode() {
        super("Wismote", new MSP430f5437Config());
    }

    public Leds getLeds() {
        return leds;
    }

    public Button getButton() {
        return button;
    }

//    public M25P80 getFlash() {
//        return flash;
//    }

//    public void setFlash(M25P80 flash) {
//        this.flash = flash;
//        registry.registerComponent("xmem", flash);
//    }

    public void dataReceived(USARTSource source, int data) {
        radio.dataReceived(source, data);
        //flash.dataReceived(source, data);
        /* if nothing selected, just write back a random byte to these devs */
        if (!radio.getChipSelect() /*&& !flash.getChipSelect()*/) {
            source.byteReceived(0);
        }
    }

    public void portWrite(IOPort source, int data) {
        switch (source.getPort()) {
        case 1:
            ds2411.dataPin((data & DS2411_DATA) != 0);
            break;
        case 2:
            leds.setLeds(LEDS_GREEN, (data & LEDS_CONF_GREEN) == 0 && (source.getDirection() & LEDS_CONF_GREEN) != 0);
            break;
        case 3:
            // Chip select = active low...
            radio.setChipSelect((data & CC2520_CHIP_SELECT) == 0);
            break;
        case 4:
            //radio.portWrite(source, data);
            //flash.portWrite(source, data);
            radio.setVRegOn((data & CC2520_VREG) != 0);
            break;
        case 5:
            leds.setLeds(LEDS_RED2, (data & LEDS_CONF_RED2) == 0 && (source.getDirection() & LEDS_CONF_RED2) != 0);
            break;
        case 8:
            leds.setLeds(LEDS_RED1, (data & LEDS_CONF_RED1) == 0 && (source.getDirection() & LEDS_CONF_RED1) != 0);
            break;
        }
    }

    private void setupNodePorts() {
//        if (flashFile != null) {
//            setFlash(new FileM25P80(cpu, flashFile));
//        }
        ds2411 = new DS2411(cpu);

        IOPort port1 = cpu.getIOUnit(IOPort.class, "P1");
        port1.addPortListener(this);
        ds2411.setDataPort(port1, DS2411_DATA_PIN);

        IOPort port2 = cpu.getIOUnit(IOPort.class, "P2");
        port2.addPortListener(this);
        cpu.getIOUnit(IOPort.class, "P3").addPortListener(this);
        cpu.getIOUnit(IOPort.class, "P4").addPortListener(this);
        cpu.getIOUnit(IOPort.class, "P5").addPortListener(this);
        cpu.getIOUnit(IOPort.class, "P8").addPortListener(this);

        IOUnit usart0 = cpu.getIOUnit("USCI B0");
        if (usart0 instanceof USARTSource) {

            radio = new CC2520(cpu);
            radio.setGPIO(1, port1, CC2520_FIFO);
            radio.setGPIO(3, port1, CC2520_CCA);
            radio.setGPIO(2, port1, CC2520_FIFOP);
            radio.setGPIO(4, port2, CC2520_SFD);

            ((USARTSource) usart0).addUSARTListener(this);
        } else {
            throw new EmulationException("Could not setup wismote mote - missing USCI B0");
        }
        leds = new Leds(cpu, LEDS);
        button = new Button("Button", cpu, port1, BUTTON_PIN, true);

        IOUnit usart = cpu.getIOUnit("USCI A1");
        if (usart instanceof USARTSource) {
            registry.registerComponent("serialio", usart);
        }
    }

    public void setupNode() {
        // create a filename for the flash file
        // This should be possible to take from a config file later!
        String fileName = config.getProperty("flashfile");
        if (fileName == null) {
            fileName = firmwareFile;
            if (fileName != null) {
                int ix = fileName.lastIndexOf('.');
                if (ix > 0) {
                    fileName = fileName.substring(0, ix);
                }
                fileName = fileName + ".flash";
            }
        }
        if (DEBUG) System.out.println("Using flash file: " + (fileName == null ? "no file" : fileName));

        //this.flashFile = fileName;

        setupNodePorts();

        if (!config.getPropertyAsBoolean("nogui", true)) {
            setupGUI();

            // Add some windows for listening to serial output
            IOUnit usart = cpu.getIOUnit("USCI A1");
            if (usart instanceof USARTSource) {
                SerialMon serial = new SerialMon((USARTSource)usart, "USCI A1 Port Output");
                registry.registerComponent("serialgui", serial);
            }
        }
    }

    public void setupGUI() {
        if (gui == null) {
            gui = new WismoteGui(this);
            registry.registerComponent("nodegui", gui);
        }
    }

    public int getModeMax() {
        return 0;
    }

    public static void main(String[] args) throws IOException {
        WismoteNode node = new WismoteNode();
        ArgumentManager config = new ArgumentManager();
        config.handleArguments(args);
        node.setupArgs(config);
    }
}
