package se.sics.mspsim.platform.sky;
import se.sics.mspsim.chip.CC2420;
import se.sics.mspsim.chip.DS2411;
import se.sics.mspsim.chip.PacketListener;
import se.sics.mspsim.config.MSP430f1611Config;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.PortListener;
import se.sics.mspsim.core.USART;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;
import se.sics.mspsim.extutil.jfreechart.DataChart;
import se.sics.mspsim.extutil.jfreechart.DataSourceSampler;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.ui.SerialMon;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.NetworkConnection;
import se.sics.mspsim.util.OperatingModeStatistics;

public abstract class CC2420Node extends GenericNode implements PortListener, USARTListener {

    // Port 2.
    public static final int DS2411_DATA_PIN = 4;
    public static final int DS2411_DATA = 1 << DS2411_DATA_PIN;

    /* P1.0 - Input: FIFOP from CC2420 */
    /* P1.3 - Input: FIFO from CC2420 */
    /* P1.4 - Input: CCA from CC2420 */
    public static final int CC2420_FIFOP = 0;
    public static final int CC2420_FIFO = 3;
    public static final int CC2420_CCA = 4;

    /* P4.1 - Input: SFD from CC2420 */
    /* P4.5 - Output: VREG_EN to CC2420 */
    /* P4.2 - Output: SPI Chip Select (CS_N) */
    public static final int CC2420_SFD = 1;
    public static final int CC2420_VREG = (1 << 5);
    public static final int CC2420_CHIP_SELECT = 0x04;

    protected IOPort port1;
    protected IOPort port2;
    protected IOPort port4;
    protected IOPort port5;

    public CC2420 radio;
    public DS2411 ds2411;

    protected String flashFile;

    public CC2420Node(String id) {
        /* this should be a config for the MSP430x1611 */
        super(id, new MSP430f1611Config());
    }

    public void setDebug(boolean debug) {
        cpu.setDebug(debug);
    }

    public boolean getDebug() {
        return cpu.getDebug();
    }

    public ELF getElfInfo() {
        return elf;
    }

    public void setNodeID(int id) {
        ds2411.setMACID(id & 0xff, id & 0xff, id & 0xff, (id >> 8) & 0xff, id & 0xff, id & 0xff);
    }

    public void setupNodePorts() {
        ds2411 = new DS2411(cpu);

        port1 = cpu.getIOUnit(IOPort.class, "P1");
        port1.addPortListener(this);

        port2 = cpu.getIOUnit(IOPort.class, "P2");
        ds2411.setDataPort(port2, DS2411_DATA_PIN);
        port2.addPortListener(this);

        port4 = cpu.getIOUnit(IOPort.class, "P4");
        port4.addPortListener(this);

        port5 = cpu.getIOUnit(IOPort.class, "P5");
        port5.addPortListener(this);

        USART usart0 = cpu.getIOUnit(USART.class, "USART0");
        radio = new CC2420(cpu);
        radio.setCCAPort(port1, CC2420_CCA);
        radio.setFIFOPPort(port1, CC2420_FIFOP);
        radio.setFIFOPort(port1, CC2420_FIFO);

        usart0.addUSARTListener(this);
        radio.setSFDPort(port4, CC2420_SFD);

        USART usart = cpu.getIOUnit(USART.class, "USART1");
        if (usart != null) {
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

        this.flashFile = fileName;

        setupNodePorts();

        if (stats != null) {
            stats.addMonitor(this);
            stats.addMonitor(radio);
            stats.addMonitor(cpu);
        }
        if (!config.getPropertyAsBoolean("nogui", true)) {
            setupGUI();

            // Add some windows for listening to serial output
            USART usart = cpu.getIOUnit(USART.class, "USART1");
            if (usart != null) {
                SerialMon serial = new SerialMon(usart, "USART1 Port Output");
                registry.registerComponent("serialgui", serial);
            }
            if (stats != null) {
                // A HACK for some "graphs"!!!
                DataChart dataChart =  new DataChart(registry, "Duty Cycle", "Duty Cycle");
                registry.registerComponent("dutychart", dataChart);
                DataSourceSampler dss = dataChart.setupChipFrame(cpu);
                dataChart.addDataSource(dss, "LEDS", stats.getDataSource(getID(), 0, OperatingModeStatistics.OP_INVERT));
                dataChart.addDataSource(dss, "Listen", stats.getDataSource(radio.getID(), CC2420.MODE_RX_ON));
                dataChart.addDataSource(dss, "Transmit", stats.getDataSource(radio.getID(), CC2420.MODE_TXRX_ON));
                dataChart.addDataSource(dss, "CPU", stats.getDataSource(cpu.getID(), MSP430.MODE_ACTIVE));
            }
        }

        if (config.getPropertyAsBoolean("enableNetwork", false)) {
            final NetworkConnection network = new NetworkConnection();
            final RadioWrapper radioWrapper = new RadioWrapper(radio);
            radioWrapper.addPacketListener(new PacketListener() {
                public void transmissionStarted() {
                }
                public void transmissionEnded(byte[] receivedData) {
//                    System.out.println("**** Sending data len = " + receivedData.length);
//                    for (int i = 0; i < receivedData.length; i++) {
//                        System.out.println("Byte: " + Utils.hex8(receivedData[i]));
//                    }
                    network.dataSent(receivedData);
                }
            });

            network.addPacketListener(new PacketListener() {
                public void transmissionStarted() {
                }
                public void transmissionEnded(byte[] receivedData) {
//                    System.out.println("**** Receiving data = " + receivedData.length);
                    radioWrapper.packetReceived(receivedData);
                }
            });
        }
    }

    public void setupGUI() {
    }

    public void portWrite(IOPort source, int data) {
        if (source == port4) {
            // Chip select = active low...
            radio.setChipSelect((data & CC2420_CHIP_SELECT) == 0);
            radio.setVRegOn((data & CC2420_VREG) != 0);
            //radio.portWrite(source, data);
            flashWrite(source, data);
        } else if (source == port2) {
            ds2411.dataPin((data & DS2411_DATA) != 0);
        }
    }

    protected abstract void flashWrite(IOPort source, int data);

    public abstract void dataReceived(USARTSource source, int data);

    public void stateChanged(int state) {
        // Ignore UART state changes by default
    }

}
