package se.sics.mspsim.platform.ti;

import java.io.IOException;
import se.sics.mspsim.chip.CC2420;
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

public class Exp5438Node extends GenericNode implements PortListener, USARTListener {

    public static final int CC2420_FIFOP = 6; // 1.6
    public static final int CC2420_FIFO = 5; // 1.5
    public static final int CC2420_CCA = 7; // 1.7
    public static final int CC2420_SFD = 3; // 1.3

    /* Output: VREG_EN to CC2420 */
    public static final int CC2420_VREG = (1 << 4); // 1.4
    /* Output: SPI Chip Select (CS_N) */
    public static final int CC2420_CHIP_SELECT = (1 << 0); // 3.0
    public static final int CC2420_RESET = (1 << 2); // 1.2

    IOPort port1;
    IOPort port3;
    IOPort port4;
    IOPort port5;
    IOPort port7;
    IOPort port8;

    public static final int LEDS_CONF_RED    = (1 << 0); // 1.0
    public static final int LEDS_CONF_YELLOW = (1 << 1); // 1.1

    public CC2420 radio;

    public Exp5438Node() {
    	/* TODO XXX MSP430F5438 */
        super("Exp5438", new MSP430f5437Config());
    }

    public void dataReceived(USARTSource source, int data) {
        radio.dataReceived(source, data);

        /* if nothing selected, just write back a random byte to these devs */
        if (!radio.getChipSelect()) {
            source.byteReceived(0);
        }
    }

    public void portWrite(IOPort source, int data) {
        if (source == port3) {
            // Chip select = active low...
            radio.setChipSelect((data & CC2420_CHIP_SELECT) == 0);
        }
        if (source == port1) {
            radio.setVRegOn((data & CC2420_VREG) != 0);
            //System.out.println("Red led: " + ((data & LEDS_CONF_RED) > 0));
            //System.out.println("Yellow led: " + ((data & LEDS_CONF_YELLOW) > 0));
        }
    }

    private void setupNodePorts() {
        port1 = cpu.getIOUnit(IOPort.class, "P1");
        port1.addPortListener(this);
        port3 = cpu.getIOUnit(IOPort.class, "P3");
        port3.addPortListener(this);
        port4 = cpu.getIOUnit(IOPort.class, "P4");
        port4.addPortListener(this);
        port5 = cpu.getIOUnit(IOPort.class, "P5");
        port5.addPortListener(this);
        port7 = cpu.getIOUnit(IOPort.class, "P7");
        port7.addPortListener(this);
        port8 = cpu.getIOUnit(IOPort.class, "P8");
        port8.addPortListener(this);

        IOUnit usart0 = cpu.getIOUnit("USCI B0"); 
        if (usart0 instanceof USARTSource) {
            radio = new CC2420(cpu);
            radio.setCCAPort(port1, CC2420_CCA);
            radio.setFIFOPPort(port1, CC2420_FIFOP);
            radio.setFIFOPort(port1, CC2420_FIFO);

            ((USARTSource) usart0).addUSARTListener(this);
            radio.setSFDPort(port1, CC2420_SFD);
        } else {
            throw new EmulationException("Could not setup exp5438 mote - missing USCI B0");
        }

        IOUnit usart = cpu.getIOUnit("USCI A1");
        if (usart instanceof USARTSource) {
            registry.registerComponent("serialio", usart);
        }
    }

    public void setupNode() {
        setupNodePorts();

        if (!config.getPropertyAsBoolean("nogui", true)) {
            // Add some windows for listening to serial output
            IOUnit usart = cpu.getIOUnit("USCI A1");
            if (usart instanceof USARTSource) {
            	SerialMon serial = new SerialMon((USARTSource)usart, "USCI A1 Port Output");
            	registry.registerComponent("serialgui", serial);
            }
        }
    }

    public int getModeMax() {
        return 0;
    }

    public static void main(String[] args) throws IOException {
        Exp5438Node node = new Exp5438Node();
        ArgumentManager config = new ArgumentManager();
        config.handleArguments(args);
        node.setupArgs(config);
    }
}