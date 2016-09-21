package se.sics.mspsim.platform.ti;

import se.sics.mspsim.chip.CC1120;
import se.sics.mspsim.chip.Enc28J60;
import se.sics.mspsim.config.MSP430f5437Config;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.PortListener;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;
import se.sics.mspsim.platform.GenericNode;

public class Trxeb1120Node extends GenericNode implements PortListener, USARTListener {

	public static final int CC1120_GDO0 = 7; /* 1.7 */
	public static final int CC1120_GDO2 = 3; /* 1.3 */
	public static final int CC1120_CHIP_SELECT = (1 << 0); // 3.0

	public static final int ENC28J60_CLK = 2; /* 10.2 */
	public static final int ENC28J60_MOSI = 1; /* 10.1 */
	public static final int ENC28J60_MISO = 0; /* 10.0 */
	public static final int ENC28J60_CHIP_SELECT = 3; /* 10.3 */

	IOPort port1;
	IOPort port3;
	IOPort port4;
	IOPort port5;
	IOPort port7;
	IOPort port8;
	IOPort port10;

	public CC1120 radio;
	public Enc28J60 enc = null;

	private boolean withEnc;
	
	public Trxeb1120Node(boolean withEnc) {
		super("Trxeb1120", new MSP430f5437Config());

		this.withEnc = withEnc;
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
			radio.setChipSelect((data & CC1120_CHIP_SELECT) == 0);
		} else if (withEnc && source == port10) {
			enc.write(port10, data);
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
			radio = new CC1120(cpu);
			radio.setGDO0(port1, CC1120_GDO0);
			radio.setGDO2(port1, CC1120_GDO2);
			((USARTSource) usart0).addUSARTListener(this);
		} else {
			throw new EmulationException("Error creating Trxeb1120Node: no USCI B0");
		}

		if (withEnc) {
			port10 = cpu.getIOUnit(IOPort.class, "P10");
			port10.addPortListener(this);
			enc = new Enc28J60(cpu, port10, ENC28J60_CLK, ENC28J60_MOSI, ENC28J60_MISO, ENC28J60_CHIP_SELECT);
		}
		
		IOUnit usart = cpu.getIOUnit("USCI A1");
		if (usart instanceof USARTSource) {
			registry.registerComponent("serialio", usart);
		}
	}

	public void setupNode() {
		setupNodePorts();
	}

	public int getModeMax() {
		return 0;
	}
}
