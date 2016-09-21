package se.sics.mspsim.platform.ti;

import java.io.IOException;

import se.sics.mspsim.config.CC430f5137Config;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.PortListener;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.ui.SerialMon;
import se.sics.mspsim.util.ArgumentManager;

public class CC430Node extends GenericNode implements PortListener, USARTListener {

    IOPort port1;
    IOPort port3;
    IOPort port4;
    IOPort port5;
    IOPort port7;
    IOPort port8;

    public CC430Node() {
    	/* TODO XXX MSP430F5438 */
        super("CC430", new CC430f5137Config());
    }

    public void dataReceived(USARTSource source, int data) {
    }

    public void portWrite(IOPort source, int data) {
        
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
            registry.registerComponent("serialio0", usart0);
        }
        
        IOUnit usart = cpu.getIOUnit("USCI A0");
        if (usart instanceof USARTSource) {
            registry.registerComponent("serialio", usart);
        }
    }

    public void setupNode() {
        setupNodePorts();

        if (!config.getPropertyAsBoolean("nogui", true)) {
            // Add some windows for listening to serial output
            IOUnit usart = cpu.getIOUnit("USCI A0");
            if (usart instanceof USARTSource) {
            	SerialMon serial = new SerialMon((USARTSource)usart, "USCI A0 Port Output");
            	registry.registerComponent("serialgui", serial);
            }
        }
    }

    public int getModeMax() {
        return 0;
    }

    public static void main(String[] args) throws IOException {
        CC430Node node = new CC430Node();
        ArgumentManager config = new ArgumentManager();
        config.handleArguments(args);
        node.setupArgs(config);
    }
}