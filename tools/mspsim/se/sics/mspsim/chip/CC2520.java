/**
 * Copyright (c) 2012 Swedish Institute of Computer Science.
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
 * CC2520
 *
 * Author  : Joakim Eriksson, Niclas Finne
 */

package se.sics.mspsim.chip;
import se.sics.mspsim.core.*;
import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.util.ArrayFIFO;
import se.sics.mspsim.util.CCITT_CRC;
import se.sics.mspsim.util.Utils;

public class CC2520 extends Radio802154 implements USARTListener, SPIData {

    public static class GPIO {
        private IOPort port;
        private int pin;

        boolean polarity = true;
        boolean isActive;

        public void setConfig(IOPort port, int pin) {
            this.port = port;
            this.pin = pin;
            port.setPinState(pin, isActive == polarity ? IOPort.PinState.HI : IOPort.PinState.LOW);
        }

        public boolean isActive() {
            return isActive;
        }

        public void setActive(boolean isActive) {
            if (this.isActive != isActive) {
                this.isActive = isActive;
                if (port != null) {
                    port.setPinState(pin, isActive == polarity ? IOPort.PinState.HI : IOPort.PinState.LOW);
                }
            }
        }

        public void setPolarity(boolean polarity) {
            if (this.polarity != polarity) {
                this.polarity = polarity;
                if (port != null) {
                    port.setPinState(pin, isActive == polarity ? IOPort.PinState.HI : IOPort.PinState.LOW);
                }
            }
        }
    }

    // FREG definitions (BSET/BCLR supported)
    public final static int REG_FRMFILT0            = 0x000;
    public final static int REG_FRMFILT1            = 0x001;
    public final static int REG_SRCMATCH            = 0x002;
    public final static int REG_SRCSHORTEN0         = 0x004;
    public final static int REG_SRCSHORTEN1         = 0x005;
    public final static int REG_SRCSHORTEN2         = 0x006;
    public final static int REG_SRCEXTEN0           = 0x008;
    public final static int REG_SRCEXTEN1           = 0x009;
    public final static int REG_SRCEXTEN2           = 0x00A;
    public final static int REG_FRMCTRL0            = 0x00C;
    public final static int REG_FRMCTRL1            = 0x00D;
    public final static int REG_RXENABLE0           = 0x00E;
    public final static int REG_RXENABLE1           = 0x00F;
    public final static int REG_EXCFLAG0            = 0x010;
    public final static int REG_EXCFLAG1            = 0x011;
    public final static int REG_EXCFLAG2            = 0x012;
    public final static int REG_EXCMASKA0           = 0x014;
    public final static int REG_EXCMASKA1           = 0x015;
    public final static int REG_EXCMASKA2           = 0x016;
    public final static int REG_EXCMASKB0           = 0x018;
    public final static int REG_EXCMASKB1           = 0x019;
    public final static int REG_EXCMASKB2           = 0x01A;
    public final static int REG_EXCBINDX0           = 0x01C;
    public final static int REG_EXCBINDX1           = 0x01D;
    public final static int REG_EXCBINDY0           = 0x01E;
    public final static int REG_EXCBINDY1           = 0x01F;
    public final static int REG_GPIOCTRL0           = 0x020;
    public final static int REG_GPIOCTRL1           = 0x021;
    public final static int REG_GPIOCTRL2           = 0x022;
    public final static int REG_GPIOCTRL3           = 0x023;
    public final static int REG_GPIOCTRL4           = 0x024;
    public final static int REG_GPIOCTRL5           = 0x025;
    public final static int REG_GPIOPOLARITY        = 0x026;
    public final static int REG_GPIOCTRL            = 0x028;
    public final static int REG_DPUCON              = 0x02A;
    public final static int REG_DPUSTAT             = 0x02C;
    public final static int REG_FREQCTRL            = 0x02E;
    public final static int REG_FREQTUNE            = 0x02F;
    public final static int REG_TXPOWER             = 0x030;
    public final static int REG_TXCTRL              = 0x031;
    public final static int REG_FSMSTAT0            = 0x032;
    public final static int REG_FSMSTAT1            = 0x033;
    public final static int REG_FIFOPCTRL           = 0x034;
    public final static int REG_FSMCTRL             = 0x035;
    public final static int REG_CCACTRL0            = 0x036;
    public final static int REG_CCACTRL1            = 0x037;
    public final static int REG_RSSI                = 0x038;
    public final static int REG_RSSISTAT            = 0x039;
    public final static int REG_TXFIFO_BUF          = 0x03A;
    public final static int REG_RXFIRST             = 0x03C;
    public final static int REG_RXFIFOCNT           = 0x03E;
    public final static int REG_TXFIFOCNT           = 0x03F;

    // SREG definitions (BSET/BCLR unsupported)
    public final static int REG_CHIPID              = 0x040;
    public final static int REG_VERSION             = 0x042;
    public final static int REG_EXTCLOCK            = 0x044;
    public final static int REG_MDMCTRL0            = 0x046;
    public final static int REG_MDMCTRL1            = 0x047;
    public final static int REG_FREQEST             = 0x048;
    public final static int REG_RXCTRL              = 0x04A;
    public final static int REG_FSCTRL              = 0x04C;
    public final static int REG_FSCAL0              = 0x04E;
    public final static int REG_FSCAL1              = 0x04F;
    public final static int REG_FSCAL2              = 0x050;
    public final static int REG_FSCAL3              = 0x051;
    public final static int REG_AGCCTRL0            = 0x052;
    public final static int REG_AGCCTRL1            = 0x053;
    public final static int REG_AGCCTRL2            = 0x054;
    public final static int REG_AGCCTRL3            = 0x055;
    public final static int REG_ADCTEST0            = 0x056;
    public final static int REG_ADCTEST1            = 0x057;
    public final static int REG_ADCTEST2            = 0x058;
    public final static int REG_MDMTEST0            = 0x05A;
    public final static int REG_MDMTEST1            = 0x05B;
    public final static int REG_DACTEST0            = 0x05C;
    public final static int REG_DACTEST1            = 0x05D;
    public final static int REG_ATEST               = 0x05E;
    public final static int REG_DACTEST2            = 0x05F;
    public final static int REG_PTEST0              = 0x060;
    public final static int REG_PTEST1              = 0x061;
    public final static int REG_RESERVED            = 0x062;
    public final static int REG_DPUBIST             = 0x07A;
    public final static int REG_ACTBIST             = 0x07C;
    public final static int REG_RAMBIST             = 0x07E;

    // Instructions
    public final static int INS_SNOP                = 0x00;
    public final static int INS_IBUFLD              = 0x02;
    public final static int INS_SIBUFEX             = 0x03;
    public final static int INS_SSAMPLECCA          = 0x04;
    public final static int INS_SRES                = 0x0F;
    public final static int INS_MEMRD               = 0x10;
    public final static int INS_MEMWR               = 0x20;
    public final static int INS_RXBUF               = 0x30;
    public final static int INS_RXBUFCP             = 0x38;
    public final static int INS_RXBUFMOV            = 0x32;
    public final static int INS_TXBUF               = 0x3A;
    public final static int INS_TXBUFCP             = 0x3E;
    public final static int INS_RANDOM              = 0x3C;
    public final static int INS_SXOSCON             = 0x40;
    public final static int INS_STXCAL              = 0x41;
    public final static int INS_SRXON               = 0x42;
    public final static int INS_STXON               = 0x43;
    public final static int INS_STXONCCA            = 0x44;
    public final static int INS_SRFOFF              = 0x45;
    public final static int INS_SXOSCOFF            = 0x46;
    public final static int INS_SFLUSHRX            = 0x47;
    public final static int INS_SFLUSHTX            = 0x48;
    public final static int INS_SACK                = 0x49;
    public final static int INS_SACKPEND            = 0x4A;
    public final static int INS_SNACK               = 0x4B;
    public final static int INS_SRXMASKBITSET       = 0x4C;
    public final static int INS_SRXMASKBITCLR       = 0x4D;
    public final static int INS_RXMASKAND           = 0x4E;
    public final static int INS_RXMASKOR            = 0x4F;
    public final static int INS_MEMCP               = 0x50;
    public final static int INS_MEMCPR              = 0x52;
    public final static int INS_MEMXCP              = 0x54;
    public final static int INS_MEMXWR              = 0x56;
    public final static int INS_BCLR                = 0x58;
    public final static int INS_BSET                = 0x59;
    public final static int INS_CTR                 = 0x60;
    public final static int INS_CBCMAC              = 0x64;
    public final static int INS_UCBCMAC             = 0x66;
    public final static int INS_CCM                 = 0x68;
    public final static int INS_UCCM                = 0x6A;
    public final static int INS_ECB                 = 0x70;
    public final static int INS_ECBO                = 0x72;
    public final static int INS_ECBX                = 0x74;
    public final static int INS_ECBXO               = 0x76;
    public final static int INS_INC                 = 0x78;
    public final static int INS_ABORT               = 0x7F;
    public final static int INS_REGRD               = 0x80;
    public final static int INS_REGWR               = 0xC0;

    // Status register flags
    public static final int STATUS_XOSC16M_STABLE   = 1 << 7;
    public static final int STATUS_RSSI_VALID       = 1 << 6;
    public static final int STATUS_EXCEPTION_CHA    = 1 << 5;
    public static final int STATUS_EXCEPTION_CHB    = 1 << 4;
    public static final int STATUS_DPU_H            = 1 << 3;
    public static final int STATUS_DPU_L            = 1 << 2;
    public static final int STATUS_TX_ACTIVE        = 1 << 1;
    public static final int STATUS_RX_ACTIVE        = 1 << 0;

    // Exceptions (bits in the EXCFLAGx memory)
    public final static int EXC_RF_IDLE             = 1 << 0;
    public final static int EXC_TX_FRM_DONE         = 1 << 1;
    public final static int EXC_RX_FRM_ABORTED      = 0x20;
    public final static int EXC_RX_FRM_UNDERFLOW    = 0x20;

    // RAM Addresses
    public static final int RAM_TXFIFO              = 0x100;
    public static final int RAM_RXFIFO              = 0x180;
    public static final int RAM_IEEEADDR            = 0x3EA;
    public static final int RAM_PANID               = 0x3F2;
    public static final int RAM_SHORTADDR           = 0x3F4;
//    public static final int RAM_KEY0                = 0x100;
//    public static final int RAM_RXNONCE             = 0x110;
//    public static final int RAM_SABUF               = 0x120;
//    public static final int RAM_KEY1                = 0x130;
//    public static final int RAM_TXNONCE             = 0x140;
//    public static final int RAM_CBCSTATE            = 0x150;

    /* one single byte instruction can be stored in the IBUF */
    int instructionBuffer = 0;

    // IOCFG0 memory Bit masks
    public static final int BCN_ACCEPT = (1<<11);
    public static final int FIFOP_THR = 0x7F;

    // IOCFG1 memory Bit Masks
//    public static final int SFDMUX = 0x3E0;
//    public static final int CCAMUX = 0x1F;

    // CCAMUX values
//    public static final int CCAMUX_CCA = 0;
//    public static final int CCAMUX_XOSC16M_STABLE = 24;

    // FRMFILT0/FRMCTRL0 values
    public static final int FRAME_FILTER = (1 << 0);
    public static final int AUTOCRC      = (1 << 6);
    public static final int AUTOACK      = (1 << 5);

    // FRMFILT1
    public static final int ACCEPT_FT_4TO7_RESERVED = (1 << 7);
    public static final int ACCEPT_FT_3_MAC_CMD     = (1 << 6);
    public static final int ACCEPT_FT_2_ACK         = (1 << 5);
    public static final int ACCEPT_FT_1_DATA        = (1 << 4);
    public static final int ACCEPT_FT_0_BEACON      = (1 << 3);

    public static final int SHORT_ADDRESS = 2;
    public static final int LONG_ADDRESS = 3;


    // The Operation modes of the CC2520
    public static final int MODE_TXRX_OFF = 0x00;
    public static final int MODE_RX_ON = 0x01;
    public static final int MODE_TXRX_ON = 0x02;
    public static final int MODE_POWER_OFF = 0x03;
    public static final int MODE_MAX = MODE_POWER_OFF;
    private static final String[] MODE_NAMES = new String[] {
        "off", "listen", "transmit", "power_off"
    };

    // State Machine - Datasheet Figure 30 page 85
    public enum RadioState {
        VREG_OFF(-2),
        POWER_DOWN(-1),
        IDLE(0),
        RX_CALIBRATE(2),
        RX_SFD_SEARCH(3),
        RX_WAIT(14),
        RX_FRAME(15),
        RX_OVERFLOW(17),
        TX_CALIBRATE(32),
        TX_PREAMBLE(34),
        TX_FRAME(37),
        TX_ACK_CALIBRATE(48),
        TX_ACK_PREAMBLE(49),
        TX_ACK(52),
        TX_UNDERFLOW(56);

        private final int state;
        RadioState(int stateNo) {
            state = stateNo;
        }

        public int getFSMState() {
            return state;
        }
    };

    // FCF High
    public static final int FRAME_TYPE        = 0x07;
    public static final int SECURITY_ENABLED  = (1 << 3);
    public static final int FRAME_PENDING     = (1 << 4);
    public static final int ACK_REQUEST       = (1 << 5);
    public static final int INTRA_PAN         = (1 << 6);

    public static final int TYPE_BEACON_FRAME = 0x00;
    public static final int TYPE_DATA_FRAME   = 0x01;
    public static final int TYPE_ACK_FRAME    = 0x02;

    // FCF Low
    public static final int DESTINATION_ADDRESS_MODE = 0x30;
    public static final int SOURCE_ADDRESS_MODE = 0x3;

    // Position of SEQ-NO in ACK packet...
    public static final int ACK_SEQPOS = 3;

    private RadioState stateMachine = RadioState.VREG_OFF;

    // 802.15.4 symbol period in ms
    public static final double SYMBOL_PERIOD = 0.016; // 16 us

    private static final int[] BC_ADDRESS = new int[] {0xff, 0xff};

    private int shrPos;
    private int txfifoPos;
    private boolean txfifoFlush;  // TXFIFO is automatically flushed on next write
    private int rxfifoReadLeft;   // number of bytes left to read from current packet
    private int rxlen;
    private int rxread;
    private int zeroSymbols;

    /* RSSI is an externally set value of the RSSI for this CC2520 */
    /* low RSSI => CCA = true in normal mode */

    private int rssi = -100;
    private static int RSSI_OFFSET = -45; /* cc2520 datasheet */

    /* This is the magical LQI */
    private int corrval = 37;

    /* FIFOP Threshold */
    private int fifopThr = 0x40;

    /* Configuration for frame filtering and auto acknowledgments */
    private boolean frameFilter = false;
    private boolean autoAck = false;
    private boolean shouldAck = false;
    private boolean ackRequest = false;
    private boolean autoCRC = false;

    // Data from last received packet
    private int dsn = 0;
    private int fcf0 = 0;
    private int fcf1 = 0;
    private int frameType = 0;
    private boolean crcOk = false;

    private int activeFrequency = 0;
    private int activeChannel = 0;

    //private int status = STATUS_XOSC16M_STABLE | STATUS_RSSI_VALID;
    private int status = 0;

    private final int[] memory = new int[0x400]; /* total memory */

    private CC2520SPI cc2520SPI = new CC2520SPI(this);
    private SPICommand command;
    private int[] spiData = new int[20]; /* SPI data buffer */
    private int spiLen;

    // Buffer to hold 5 byte Synchronization header, as it is not written to the TXFIFO
    private final byte[] SHR = new byte[5];

    /* the data that should be SPI response */
    private int outputSPI;

    private boolean chipSelect;
    private final GPIO[] gpio = new GPIO[6];
    private GPIO ccaGPIO;
    private GPIO fifopGPIO;
    private GPIO fifoGPIO;
    private GPIO sfdGPIO;
    private boolean currentFIFO;
    private boolean currentFIFOP;

    /* current CCA value */
    private boolean currentCCA = false;

    private int txCursor;
    private boolean isRadioOn;

    private TimeEvent oscillatorEvent = new TimeEvent(0, "CC2520 OSC") {
        public void execute(long t) {
            status |= STATUS_XOSC16M_STABLE;
            if(DEBUG) log("Oscillator Stable Event.");
            setState(RadioState.IDLE);
            updateCCA();
        }
    };

    private TimeEvent vregEvent = new TimeEvent(0, "CC2520 VREG") {
        public void execute(long t) {
            if(DEBUG) log("VREG Started at: " + t + " cyc: " +
                    cpu.cycles + " " + getTime());
            isRadioOn = true;
            setState(RadioState.POWER_DOWN);
            updateCCA();
        }
    };

    private TimeEvent sendEvent = new TimeEvent(0, "CC2520 Send") {
        public void execute(long t) {
            txNext();
        }
    };

    private TimeEvent ackEvent = new TimeEvent(0, "CC2520 Ack") {
        public void execute(long t) {
            ackNext();
        }
    };

    private TimeEvent shrEvent = new TimeEvent(0, "CC2520 SHR") {
        public void execute(long t) {
            shrNext();
        }
    };

    private TimeEvent symbolEvent = new TimeEvent(0, "CC2520 Symbol") {
        public void execute(long t) {
            switch(stateMachine) {
            case RX_CALIBRATE:
                setState(RadioState.RX_SFD_SEARCH);
                break;
                /* this will be called 8 symbols after first SFD_SEARCH */
            case RX_SFD_SEARCH:
                status |= STATUS_RSSI_VALID;
                memory[REG_RSSISTAT] = 1;
                updateCCA();
                break;

            case TX_CALIBRATE:
                setState(RadioState.TX_PREAMBLE);
                break;

            case RX_WAIT:
                setState(RadioState.RX_SFD_SEARCH);
                break;

            case TX_ACK_CALIBRATE:
                setState(RadioState.TX_ACK_PREAMBLE);
                break;

            default:
                // Ignore other states
                break;
            }
        }
    };
    private boolean overflow = false;
    private boolean frameRejected = false;

    private int ackPos;
    /* type = 2 (ACK), third byte needs to be sequence number... */
    private int[] ackBuf = {0x05, 0x02, 0x00, 0x00, 0x00, 0x00};
    private boolean ackFramePending = false;
    private CCITT_CRC rxCrc = new CCITT_CRC();
    private CCITT_CRC txCrc = new CCITT_CRC();

    private final ArrayFIFO rxFIFO = new ArrayFIFO("RXFIFO", memory, 128, 128);

    public CC2520(MSP430Core cpu) {
        super("CC2520", "Radio", cpu);

        for (int i = 0; i < gpio.length; i++) {
            gpio[i] = new GPIO();
        }

        setModeNames(MODE_NAMES);
        setMode(MODE_POWER_OFF);
        rxFIFO.reset();
        overflow = false;
        reset();
    }

    public RadioState getState() {
        return stateMachine;
    }

    private int getFCFReservedMask() {
        return (memory[REG_FRMFILT0] >> 4) & 7;
    }

    private int getFCFMaxFrameVersion() {
        return (memory[REG_FRMFILT0] >> 2) & 3;
    }

    private void updateGPIOConfig() {
        int bit = 1;
        for (GPIO io : gpio) {
            io.setPolarity((memory[REG_GPIOPOLARITY] & bit) > 0);
            bit = bit << 1;
        }
    }

    public int getFooterLength() {
    	if (autoCRC) {
    		return 2;
    	}
    	return 0;
    }
    
    private void reset() {
        // FCF max fram version = 3 and frame filtering enabled
        memory[REG_FRMFILT0] = 0x0d;
        frameFilter = true;
        memory[REG_FRMFILT1] = 0x78;
        // autocrc enabled, autoack disabled
        memory[REG_FRMCTRL0] = 0x40;
        autoCRC = true;
        autoAck = false;

        memory[REG_MDMCTRL0] = 0x45;
        memory[REG_MDMCTRL1] = 0x3e;
        memory[REG_FSMSTAT0] = 0;
        memory[REG_FSMSTAT1] = 0;
        memory[REG_RSSISTAT] = 0;
        memory[REG_TXPOWER] = 0x06;
        memory[REG_FIFOPCTRL] = fifopThr = 0x40;
        memory[REG_FREQCTRL] = 0x0b;

        /* back to default configuration of GPIOs */
        memory[REG_GPIOPOLARITY] = 0x3f;
        updateGPIOConfig();

        fifoGPIO = gpio[1];
        fifopGPIO = gpio[2];
        ccaGPIO = gpio[3];
        sfdGPIO = gpio[4];

        setFIFO(false);
        setFIFOP(false);
        setSFD(false);
        updateCCA();
    }

    private boolean setState(RadioState state) {
        if(DEBUG) log("State transition from " + stateMachine + " to " + state);
        stateMachine = state;
        /* write to FSM state register */
        memory[REG_FSMSTAT0] = (memory[REG_FSMSTAT0] & 0x3f);//state.getFSMState();

        switch(stateMachine) {

        case VREG_OFF:
            if (DEBUG) log("VREG Off.");
            flushRX();
            flushTX();
            status &= ~(STATUS_RSSI_VALID | STATUS_XOSC16M_STABLE);
            memory[REG_RSSISTAT] = 0;
            crcOk = false;
            reset();
            setMode(MODE_POWER_OFF);
            updateCCA();
            break;

        case POWER_DOWN:
            rxFIFO.reset();
            status &= ~(STATUS_RSSI_VALID | STATUS_XOSC16M_STABLE);
            memory[REG_RSSISTAT] = 0;
            crcOk = false;
            reset();
            setMode(MODE_POWER_OFF);
            updateCCA();
            break;

        case RX_CALIBRATE:
            /* should be 12 according to specification */
            setSymbolEvent(12);
            setMode(MODE_RX_ON);
            break;
        case RX_SFD_SEARCH:
            zeroSymbols = 0;
            /* eight symbols after first SFD search RSSI will be valid */
            if ((status & STATUS_RSSI_VALID) == 0) {
                setSymbolEvent(8);
            }
            //      status |= STATUS_RSSI_VALID;
            updateCCA();
            setMode(MODE_RX_ON);
            break;

        case TX_CALIBRATE:
            /* 12 symbols calibration, and one byte's wait since we deliver immediately
             * to listener when after calibration?
             */
            setSymbolEvent(12 + 2);
            setMode(MODE_TXRX_ON);
            break;

        case TX_PREAMBLE:
            shrPos = 0;
            SHR[0] = 0;
            SHR[1] = 0;
            SHR[2] = 0;
            SHR[3] = 0;
            SHR[4] = 0x7A;
            shrNext();
            break;

        case TX_FRAME:
            txfifoPos = 0;
            // Reset CRC ok flag to disable software acknowledgments until next received packet
            crcOk = false;
            txNext();
            break;

        case RX_WAIT:
            setSymbolEvent(8);
            setMode(MODE_RX_ON);
            break;

        case IDLE:
            status &= ~STATUS_RSSI_VALID;
            memory[REG_RSSISTAT] = 0;
            setMode(MODE_TXRX_OFF);
            updateCCA();
            break;

        case TX_ACK_CALIBRATE:
            /* TX active during ACK + NOTE: we ignore the SFD when receiving full packets so
             * we need to add another extra 2 symbols here to get a correct timing */
            status |= STATUS_TX_ACTIVE;
            memory[REG_FSMSTAT1] |= (1 << 1);
            setSymbolEvent(12 + 2 + 2);
            setMode(MODE_TXRX_ON);
            break;
        case TX_ACK_PREAMBLE:
            /* same as normal preamble ?? */
            shrPos = 0;
            SHR[0] = 0;
            SHR[1] = 0;
            SHR[2] = 0;
            SHR[3] = 0;
            SHR[4] = 0x7A;
            shrNext();
            break;
        case TX_ACK:
            ackPos = 0;
            // Reset CRC ok flag to disable software acknowledgments until next received packet
            crcOk = false;
            ackNext();
            break;
        case RX_FRAME:
            /* mark position of frame start - for rejecting when address is wrong */
            rxFIFO.mark();
            rxread = 0;
            frameRejected = false;
            shouldAck = false;
            crcOk = false;
            break;

        case RX_OVERFLOW:
            break;

        case TX_UNDERFLOW:
            // TODO handle TX underflow
            break;
        }

        /* Notify state listener */
        stateChanged(stateMachine.state);

        return true;
    }

    private void rejectFrame() {
        // Immediately jump to SFD Search again... something more???
        /* reset state */
        rxFIFO.restore();
        setSFD(false);
        setFIFO(rxFIFO.length() > 0);
        frameRejected = true;
    }

    /* variables for the address recognition */
    int destinationAddressMode = 0;
    boolean decodeAddress = false;
    /* Receive a byte from the radio medium
     * @see se.sics.mspsim.chip.RFListener#receivedByte(byte)
     */
    public void receivedByte(byte data) {
        // Received a byte from the "air"

        if (DEBUG)
            log("RF Byte received: " + Utils.hex8(data) + " state: " + stateMachine + " noZeroes: " + zeroSymbols +
                    ((stateMachine == RadioState.RX_SFD_SEARCH || stateMachine == RadioState.RX_FRAME) ? "" : " *** Ignored"));

        if(stateMachine == RadioState.RX_SFD_SEARCH) {
            // Look for the preamble (4 zero bytes) followed by the SFD byte 0x7A
            if(data == 0) {
                // Count zero bytes
                zeroSymbols++;
            } else if(zeroSymbols >= 4 && data == 0x7A) {
                // If the received byte is !zero, we have counted 4 zero bytes prior to this one,
                // and the current received byte == 0x7A (SFD), we're in sync.
                // In RX mode, SFD goes high when the SFD is received
                setSFD(true);
                if (DEBUG) log("RX: Preamble/SFD Synchronized.");
                setState(RadioState.RX_FRAME);
            } else {
                /* if not four zeros and 0x7A then no zeroes... */
                zeroSymbols = 0;
            }

        } else if(stateMachine == RadioState.RX_FRAME) {
            if (overflow) {
                /* if the CC2520 RX FIFO is in overflow - it needs a flush before receiving again */
            } else if(rxFIFO.isFull()) {
                setRxOverflow();
            } else {
                if (!frameRejected) {
                    rxFIFO.write(data);
                    if (rxread == 0) {
                        rxCrc.setCRC(0);
                        rxlen = data & 0xff;
                        //System.out.println("Starting to get packet at: " + rxfifoWritePos + " len = " + rxlen);
                        decodeAddress = frameFilter;
                        if (DEBUG) log("RX: Start frame length " + rxlen);
                        // FIFO pin goes high after length byte is written to RXFIFO
                        setFIFO(true);
                    } else if (rxread < rxlen - 1) {
                        /* As long as we are not in the length or FCF (CRC) we count CRC */
                        rxCrc.addBitrev(data & 0xff);
                        if (rxread == 1) {
                            fcf0 = data & 0xff;
                            frameType = fcf0 & FRAME_TYPE;
                        } else if (rxread == 2) {
                            fcf1 = data & 0xff;

                            if (frameFilter
                                    && (((getFCFReservedMask() & (((fcf0 & 3) << 1) | (fcf1 & 1))) != 0)
                                            || (getFCFMaxFrameVersion() < ((fcf0 >> 2) & 3)))) {
                                // Illegal frame version or reserved bits set
                                rejectFrame();
                            } else if (frameType == TYPE_DATA_FRAME) {
                                ackRequest = (fcf0 & ACK_REQUEST) > 0;
                                destinationAddressMode = (fcf1 >> 2) & 3;
                                /* check this !!! */
                                if (frameFilter) {
                                    if ((destinationAddressMode != LONG_ADDRESS
                                            && destinationAddressMode != SHORT_ADDRESS)
                                            || (memory[REG_FRMFILT1] & ACCEPT_FT_1_DATA) == 0) {
                                        rejectFrame();
                                    }
                                }
                            } else if (frameType == TYPE_ACK_FRAME) {
                                decodeAddress = false;
                                ackRequest = false;
                                if (frameFilter) {
                                    if (rxlen != 5
                                            || (memory[REG_FRMFILT1] & ACCEPT_FT_2_ACK) == 0) {
                                        rejectFrame();
                                    }
                                }
                            } else if (frameType == TYPE_BEACON_FRAME) {
                                decodeAddress = false;
                                ackRequest = false;
                                destinationAddressMode = (fcf1 >> 2) & 3;
                                if (frameFilter) {
                                    if (rxlen < 9
                                            || (memory[REG_FRMFILT1] & ACCEPT_FT_0_BEACON) == 0
                                            || destinationAddressMode != 0) {
                                        rejectFrame();
                                    }
                                }
                            } else if (frameFilter) {
                                /* illegal frame when decoding address... */
                                rejectFrame();
                            }
                        } else if (rxread == 3) {
                            // save data sequence number
                            dsn = data & 0xff;
                        } else if (decodeAddress) {
                            boolean flushPacket = false;
                            /* here we decode the address !!! */
                            if (destinationAddressMode == LONG_ADDRESS && rxread == 8 + 5) {
                                /* here we need to check that this address is correct compared to the stored address */
                                flushPacket = !rxFIFO.tailEquals(memory, RAM_IEEEADDR, 8);
                                flushPacket |= !rxFIFO.tailEquals(memory, RAM_PANID, 2, 8)
                                        && !rxFIFO.tailEquals(BC_ADDRESS, 0, 2, 8);
                                decodeAddress = false;
                            } else if (destinationAddressMode == SHORT_ADDRESS && rxread == 2 + 5){
                                /* should check short address */
                                flushPacket = !rxFIFO.tailEquals(BC_ADDRESS, 0, 2)
                                        && !rxFIFO.tailEquals(memory, RAM_SHORTADDR, 2);
                                flushPacket |= !rxFIFO.tailEquals(memory, RAM_PANID, 2, 2)
                                        && !rxFIFO.tailEquals(BC_ADDRESS, 0, 2, 2);
                                decodeAddress = false;
                            }
                            if (flushPacket) {
                                rejectFrame();
                            }
                        }
                    }

                    /* In RX mode, FIFOP goes high when the size of the first enqueued packet exceeds
                     * the programmable threshold and address recognition isn't ongoing */
                    if (currentFIFOP == false
                            && rxFIFO.length() <= rxlen + 1
                            && !decodeAddress && !frameRejected
                            && rxFIFO.length() > fifopThr) {
                        setFIFOP(true);
                        if (DEBUG) log("RX: FIFOP Threshold reached - setting FIFOP");
                    }
                }

                if (rxread++ == rxlen) {
                    if (frameRejected) {
                        if (DEBUG) log("Frame rejected - setting SFD to false and RXWAIT\n");
                        setSFD(false);
                        setState(RadioState.RX_WAIT);
                        return;
                    }
                    // In RX mode, FIFOP goes high, if threshold is higher than frame length....

                    // Here we check the CRC of the packet!
                    //System.out.println("Reading from " + ((rxfifoWritePos + 128 - 2) & 127));
                    if (autoCRC) {
                        int crc = rxFIFO.get(-2) << 8;
                        crc += rxFIFO.get(-1); //memory[RAM_RXFIFO + ((rxfifoWritePos + 128 - 1) & 127)];

                        crcOk = crc == rxCrc.getCRCBitrev();
                        if (DEBUG && !crcOk) {
                            log("CRC not OK: recv:" + Utils.hex16(crc) + " calc: " + Utils.hex16(rxCrc.getCRCBitrev()));
                        }
                        // Should take a RSSI value as input or use a set-RSSI value...
                        rxFIFO.set(-2, memory[REG_RSSI] & 0xff);
                        rxFIFO.set(-1, (corrval & 0x7F) | (crcOk ? 0x80 : 0));
                        //          memory[RAM_RXFIFO + ((rxfifoWritePos + 128 - 2) & 127)] = ;
                        //          // Set CRC ok and add a correlation - TODO: fix better correlation value!!!
                        //          memory[RAM_RXFIFO + ((rxfifoWritePos + 128 - 1) & 127)] = 37 |
                        //              (crcOk ? 0x80 : 0);
                    } else {
                    	crcOk = true;
                    }

                    /* set FIFOP only if this is the first received packet - e.g. if rxfifoLen is at most rxlen + 1
                     * TODO: check what happens when rxfifoLen < rxlen - e.g we have been reading before FIFOP */
                    if (rxFIFO.length() <= rxlen + 1) {
                        setFIFOP(true);
                    } else {
                        if (DEBUG) log("Did not set FIFOP rxfifoLen: " + rxFIFO.length() + " rxlen: " + rxlen);
                    }
                    setSFD(false);
                    if (DEBUG) log("RX: Complete: packetStart: " + rxFIFO.stateToString());

                    /* if either manual ack request (shouldAck) or autoack + ACK_REQ on package do ack! */
                    /* Autoack-mode + good CRC => autoack */
                    if (((autoAck && ackRequest) || shouldAck) && crcOk) {
                        setState(RadioState.TX_ACK_CALIBRATE);
                    } else {
                        setState(RadioState.RX_WAIT);
                    }
                }
            }
        }
    }

    /* API used in CC2520 SPI for both memory and registers */
    void writeMemory(int address, int data) {
//        System.out.printf("CC2520: writing to %x => %x\n", address, data);
        int oldValue = memory[address];
        memory[address] = data;
        switch(address) {
        case REG_FRMFILT0:
            frameFilter = (data & FRAME_FILTER) != 0;
            break;
        case REG_FRMCTRL0:
            autoCRC = (data & AUTOCRC) != 0;
            autoAck = (data & AUTOACK) != 0;
            break;
        case REG_TXPOWER:
            if (!isDefinedTxPower(data)) {
                logw(WarningType.EXECUTION, "*** Warning - writing an undefined TXPOWER value (0x"
                        + Utils.hex8(data) + ") to CC2520!!!");
            }
            break;
        case REG_FIFOPCTRL:
            fifopThr = data & FIFOP_THR;
            if (DEBUG) log("FIFOPCTRL: 0x" + Utils.hex16(oldValue) + " => 0x" + Utils.hex16(data));
            break;
        case REG_GPIOPOLARITY:
            if (DEBUG) log("GIOPOLARITY: 0x" + Utils.hex16(oldValue) + " => 0x" + Utils.hex16(data));
            if (oldValue != data) {
                updateGPIOConfig();
            }
            break;
            //        case REG_IOCFG1:
            //            if (DEBUG)
            //                log("IOCFG1: SFDMUX "
            //                        + ((memory[address] & SFDMUX) >> SFDMUX)
            //                        + " CCAMUX: " + (memory[address] & CCAMUX));
            //            updateCCA();
            //            break;
        case REG_GPIOCTRL0:
			/*
			 * XXX TODO Implement support for GPIO control. Below example code
			 * demonstrates how GPIO0 is set to fifop functionality (0x28).
			 */
			if (data == 0x28) {
				fifopGPIO = gpio[0];
			}
        	break;
        case REG_FSCTRL: {
            ChannelListener listener = this.channelListener;
            if (listener != null) {
                int oldChannel = activeChannel;
                updateActiveFrequency();
                if (oldChannel != activeChannel) {
                    listener.channelChanged(activeChannel);
                }
            }
            break;
        }
        }
        configurationChanged(address, oldValue, data);
    }

    int readMemory(int address) {
        switch(address) {
        case REG_RXFIFOCNT:
        	return rxFIFO.length();
        }
        return memory[address];
    }

    @Override
    public void dataReceived(USARTSource source, int data) {
        outputSPI = status; /* if nothing replace the outputSPI it will be output */
        if (DEBUG) {
            log("byte received: " + Utils.hex8(data) +
                    " (" + ((data >= ' ' && data <= 'Z') ? (char) data : '.') + ')' +
                    " CS: " + chipSelect + " SPI(" + spiLen + "): " + (command == null ? "<waiting>" : command.name)
                    + " State: " + stateMachine);
        }

        if (!chipSelect) {
            // Chip is not selected
            return;
        }

        if (stateMachine == RadioState.VREG_OFF) {
            /* No VREG but chip select */
            source.byteReceived(0);
            logw(WarningType.EXECUTION, "**** Warning - writing to CC2520 when VREG is off!!!");
            return;
        }

        if (command == null) {
            command = cc2520SPI.getCommand(data);
            if (command == null) {
                logw(WarningType.EMULATION_ERROR, "**** Warning - not implemented command on SPI: " + data);
            } else if (DEBUG) {
                if (!"SNOP".equals(command.name)) {
                    log("SPI command: " + command.name);
                }
            }
        }

        /* command handling */
        spiData[spiLen] = data;
        /* ensure that we do not store too many SPI data items */
        if (spiLen < (spiData.length - 1)) {
            spiLen++;
        }

        if (command != null) {
            command.dataReceived(data);
            if (spiLen == command.commandLen) {
//                System.out.println("CC2520 Executing command: " + command.name);
                command.executeSPICommand();
                command = null;
                spiLen = 0;
            }
        }
        source.byteReceived(outputSPI);
    }

    void rxon() {
        if(stateMachine == RadioState.IDLE) {
            setState(RadioState.RX_CALIBRATE);
            //updateActiveFrequency();
            if (DEBUG) {
                log("Strobe RX-ON!!!");
            }
        } else {
            if (DEBUG) log("WARNING: SRXON when not IDLE");
        }
    }

    void rxtxoff() {
        if (DEBUG) {
            log("Strobe RXTX-OFF!!! at " + cpu.cycles);
            if (stateMachine == RadioState.TX_ACK ||
                    stateMachine == RadioState.TX_FRAME ||
                    stateMachine == RadioState.RX_FRAME) {
                log("WARNING: turning off RXTX during " + stateMachine);
            }
        }
        setState(RadioState.IDLE);
    }

    void stxon() {
        // State transition valid from IDLE state or all RX states
        if( (stateMachine == RadioState.IDLE) ||
                (stateMachine == RadioState.RX_CALIBRATE) ||
                (stateMachine == RadioState.RX_SFD_SEARCH) ||
                (stateMachine == RadioState.RX_FRAME) ||
                (stateMachine == RadioState.RX_OVERFLOW) ||
                (stateMachine == RadioState.RX_WAIT)) {
            status |= STATUS_TX_ACTIVE;
            memory[REG_FSMSTAT1] |= (1 << 1);
            setState(RadioState.TX_CALIBRATE);
            if (sendEvents) {
                sendEvent("STXON", null);
            }
            // Starting up TX subsystem - indicate that we are in TX mode!
            if (DEBUG) log("Strobe STXON - transmit on! at " + cpu.cycles);
        }
    }

    void stxoncca() {
        // Only valid from all RX states,
        // since CCA requires ??(look this up) receive symbol periods to be valid
        if( (stateMachine == RadioState.RX_CALIBRATE) ||
                (stateMachine == RadioState.RX_SFD_SEARCH) ||
                (stateMachine == RadioState.RX_FRAME) ||
                (stateMachine == RadioState.RX_OVERFLOW) ||
                (stateMachine == RadioState.RX_WAIT)) {

            if (sendEvents) {
                sendEvent("STXON_CCA", null);
            }

            if(currentCCA) {
                status |= STATUS_TX_ACTIVE;
                memory[REG_FSMSTAT1] |= (1 << 1);
                setState(RadioState.TX_CALIBRATE);
                if (DEBUG) log("Strobe STXONCCA - transmit on! at " + cpu.cycles);
            }else{
                if (DEBUG) log("STXONCCA Ignored, CCA false");
            }
        }
    }

    void sack(boolean pend) {
        // Set the frame pending flag for all future autoack based on SACK/SACKPEND
        ackFramePending = pend;
        if (stateMachine == RadioState.RX_FRAME) {
            shouldAck = true;
        } else if (crcOk) {
            setState(RadioState.TX_ACK_CALIBRATE);
        }
    }

    private void shrNext() {
        if(shrPos == 5) {
            // Set SFD high
            setSFD(true);

            if (stateMachine == RadioState.TX_PREAMBLE) {
                setState(RadioState.TX_FRAME);
            } else if (stateMachine == RadioState.TX_ACK_PREAMBLE) {
                setState(RadioState.TX_ACK);
            } else {
                log("Can not move to TX_FRAME or TX_ACK after preamble since radio is in wrong mode: " +
                        stateMachine);
            }
        } else {
            if (rfListener != null) {
                if (DEBUG) log("transmitting byte: " + Utils.hex8(SHR[shrPos]));
                rfListener.receivedByte(SHR[shrPos]);
            }
            shrPos++;
            cpu.scheduleTimeEventMillis(shrEvent, SYMBOL_PERIOD * 2);
        }
    }

    private void txNext() {
        if(txfifoPos <= memory[RAM_TXFIFO]) {
            int len = memory[RAM_TXFIFO] & 0xff;
            
            if (autoCRC) {
                if (txfifoPos == len - 1) {
                    txCrc.setCRC(0);
                    for (int i = 1; i < len - 1; i++) {
                        txCrc.addBitrev(memory[RAM_TXFIFO + i] & 0xff);
                    }
                    memory[RAM_TXFIFO + len - 1] = txCrc.getCRCHi();
                    memory[RAM_TXFIFO + len] = txCrc.getCRCLow();
                }
            }
            
            if (txfifoPos > 0x7f) {
                logw(WarningType.EXECUTION, "**** Warning - packet size too large - repeating packet bytes txfifoPos: " + txfifoPos);
            }
            if (rfListener != null) {
                if (DEBUG) log("transmitting byte: " + Utils.hex8(memory[RAM_TXFIFO + (txfifoPos & 0x7f)] & 0xFF));
                rfListener.receivedByte((byte)(memory[RAM_TXFIFO + (txfifoPos & 0x7f)] & 0xFF));
            }
            txfifoPos++;
            // Two symbol periods to send a byte...
            cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);
        } else {
            if (DEBUG) log("Completed Transmission.");
            status &= ~STATUS_TX_ACTIVE;
            memory[REG_FSMSTAT1] &= ~(1 << 1);
            setSFD(false);
            if (overflow) {
                /* TODO: is it going back to overflow here ?=? */
                setState(RadioState.RX_OVERFLOW);
            } else {
                setState(RadioState.RX_CALIBRATE);
            }
            /* Back to RX ON */
            setMode(MODE_RX_ON);
            txfifoFlush = true;
        }
    }

    private void ackNext() {
        if (ackPos < ackBuf.length) {
            if(ackPos == 0) {
                txCrc.setCRC(0);
                if (ackFramePending) {
                    ackBuf[1] |= FRAME_PENDING;
                } else {
                    ackBuf[1] &= ~FRAME_PENDING;
                }
                // set dsn
                ackBuf[3] = dsn;
                int len = 4;
                for (int i = 1; i < len; i++) {
                    txCrc.addBitrev(ackBuf[i] & 0xff);
                }
                ackBuf[4] = txCrc.getCRCHi();
                ackBuf[5] = txCrc.getCRCLow();
            }
            if (rfListener != null) {
                if (DEBUG) log("transmitting byte: " + Utils.hex8(memory[RAM_TXFIFO + (txfifoPos & 0x7f)] & 0xFF));

                rfListener.receivedByte((byte)(ackBuf[ackPos] & 0xFF));
            }
            ackPos++;
            // Two symbol periods to send a byte...
            cpu.scheduleTimeEventMillis(ackEvent, SYMBOL_PERIOD * 2);
        } else {
            if (DEBUG) log("Completed Transmission of ACK.");
            status &= ~STATUS_TX_ACTIVE;
            memory[REG_FSMSTAT1] &= ~(1 << 1);
            setSFD(false);
            setState(RadioState.RX_CALIBRATE);
            /* Back to RX ON */
            setMode(MODE_RX_ON);
        }
    }


    private void setSymbolEvent(int symbols) {
        double period = SYMBOL_PERIOD * symbols;
        cpu.scheduleTimeEventMillis(symbolEvent, period);
        //log("Set Symbol event: " + period);
    }

    void startOscillator() {
        // 1ms crystal startup from datasheet pg12
        cpu.scheduleTimeEventMillis(oscillatorEvent, 1);
    }

    void stopOscillator() {
        status &= ~STATUS_XOSC16M_STABLE;
        setState(RadioState.POWER_DOWN);
        if (DEBUG) log("Oscillator Off.");
        // Reset state
        setFIFOP(false);
    }

    void flushRX() {
        if (DEBUG) {
            log("Flushing RX len = " + rxFIFO.length());
        }
        rxFIFO.reset();
        setSFD(false);
        setFIFOP(false);
        setFIFO(false);
        overflow = false;
        /* goto RX Calibrate */
        if( (stateMachine == RadioState.RX_CALIBRATE) ||
                (stateMachine == RadioState.RX_SFD_SEARCH) ||
                (stateMachine == RadioState.RX_FRAME) ||
                (stateMachine == RadioState.RX_OVERFLOW) ||
                (stateMachine == RadioState.RX_WAIT)) {
            setState(RadioState.RX_SFD_SEARCH);
        }
    }

    void writeTXFIFO(int data) {
        if(txfifoFlush) {
            txCursor = 0;
            txfifoFlush = false;
        }
        if (DEBUG) log("Writing data: " + data + " to tx: " + txCursor);

        if(txCursor == 0) {
            if ((data & 0xff) > 127) {
                logger.logw(this, WarningType.EXECUTION, "CC2520: Warning - packet size too large: " + (data & 0xff));
            }
        } else if (txCursor > 127) {
            logger.logw(this, WarningType.EXECUTION, "CC2520: Warning - TX Cursor wrapped");
            txCursor = 0;
        }
        memory[RAM_TXFIFO + txCursor] = data & 0xff;
        txCursor++;
        if (sendEvents) {
            sendEvent("WRITE_TXFIFO", null);
        }
    }

    // TODO: update any pins here?
    void flushTX() {
        txCursor = 0;
    }

    private void updateCCA() {
         boolean oldCCA = currentCCA;

         currentCCA = (status & STATUS_RSSI_VALID) > 0 && rssi < -95;

         if (currentCCA != oldCCA) {
             ccaGPIO.setActive(currentCCA);
             if (currentCCA) {
                 memory[REG_FSMSTAT1] |= 1 << 4;
             } else {
                 memory[REG_FSMSTAT1] &= ~(1 << 4);
             }
             if (DEBUG) log("Setting CCA to: " + currentCCA);
         }
    }

    private void setSFD(boolean sfd) {
        sfdGPIO.setActive(sfd);
        if (sfd) {
            memory[REG_FSMSTAT1] |= 1 << 5;
        } else {
            memory[REG_FSMSTAT1] &= ~(1 << 5);
        }
        if (DEBUG) log("SFD: " + sfd + "  " + cpu.cycles);
    }

    private void setFIFOP(boolean fifop) {
        currentFIFOP = fifop;
        fifopGPIO.setActive(fifop);
        if (fifop) {
            memory[REG_FSMSTAT1] |= 1 << 6;
        } else {
            memory[REG_FSMSTAT1] &= ~(1 << 6);
        }
        if (DEBUG) log("Setting FIFOP to " + fifop);
    }

    private void setFIFO(boolean fifo) {
        currentFIFO = fifo;
        fifoGPIO.setActive(fifo);
        if (fifo) {
            memory[REG_FSMSTAT1] |= 1 << 7;
        } else {
            memory[REG_FSMSTAT1] &= ~(1 << 7);
        }
        if (DEBUG) log("Setting FIFO to " + fifo);
    }

    private void setRxOverflow() {
        if (DEBUG) log("RXFIFO Overflow! Read Pos: " + rxFIFO.stateToString());
        setFIFOP(true);
        setFIFO(false);
        setSFD(false);
        overflow = true;
        shouldAck = false;
        setState(RadioState.RX_OVERFLOW);
    }


    /*****************************************************************************
     *  External APIs for simulators simulating Radio medium, etc.
     *
     *****************************************************************************/

    @Override
    public boolean isReadyToReceive() {
        return getState() == RadioState.RX_SFD_SEARCH;
    }

    private void updateActiveFrequency() {
        /* INVERTED: f = 5 * (c - 11) + 357 + 0x4000 */
        int freg = memory[REG_FREQCTRL] & 0x7f;
        activeFrequency =  freg + 2394;
        activeChannel = 11 + (freg - 11) / 5;
    }

    @Override
    public int getActiveFrequency() {
        updateActiveFrequency();
        return activeFrequency;
    }

    @Override
    public int getActiveChannel() {
        updateActiveFrequency();
        return activeChannel;
    }

    /**
     * This is actually the "CORR" value.
     * @param lqi The Corr-val
     * @sa CC2520 Datasheet
     */
    @Override
    public void setLQI(int lqi){
        if(lqi < 0) {
            lqi = 0;
        } else if(lqi > 0x7f ) {
            lqi = 0x7f;
        }
        corrval = lqi;
    }

    @Override
    public int getLQI() {
        return corrval;
    }

    @Override
    public void setRSSI(int power) {
        final int minp = -128 + RSSI_OFFSET;
        final int maxp = 128 + RSSI_OFFSET;
        if (power < minp) {
            power = -minp;
        }
        if(power > maxp){
            power = maxp;
        }

        if (DEBUG) log("external setRSSI to: " + power);

        rssi = power;
        memory[REG_RSSI] = power - RSSI_OFFSET;
        updateCCA();
    }

    @Override
    public int getRSSI() {
        return rssi;
    }

    private boolean isDefinedTxPower(int txpower) {
        switch (txpower) {
        case 0xf7:
        case 0xf2:
        case 0xab:
        case 0x88:
        case 0x81:
        case 0x32:
        case 0x2c:
        case 0x13:
        case 0x03:
            return true;
        default:
            return false;
        }
    }

    @Override
    public int getOutputPowerIndicator() {
        // Higher TXPOWER value does not always mean higher transmission power.
        // Instead of using the TXPOWER value, the output power is mapped into 9 classes.
        int txpower = memory[REG_TXPOWER];
        if (txpower >= 0xf7) {
            return 9;
        }
        if (txpower >= 0xf2) {
            return 8;
        }
        if (txpower >= 0xab) {
            return 7;
        }
        if (txpower >= 0x88) {
            return 3;
        }
        if (txpower >= 0x81) {
            return 4;
        }
        if (txpower >= 0x32) {
            return 5;
        }
        if (txpower >= 0x2c) {
            return 2;
        }
        if (txpower >= 0x13) {
            return 6;
        }
        if (txpower >= 0x03) {
            return 1;
        }
        /* Unknown */
        return 0;
//        return memory[REG_TXPOWER];
    }

    @Override
    public int getOutputPowerIndicatorMax() {
        return 9;
//        return 255;
    }

    @Override
    public int getOutputPower() {
        /* From CC2520 datasheet, table 17 */
        int txpower = memory[REG_TXPOWER];
        if (txpower >= 0xf7) {
            return 5;
        }
        if (txpower >= 0xf2) {
            return 3;
        }
        if (txpower >= 0xab) {
            return 2;
        }
        if (txpower >= 0x88) {
            return -4;
        }
        if (txpower >= 0x81) {
            return -2;
        }
        if (txpower >= 0x32) {
            return 0;
        }
        if (txpower >= 0x2c) {
            return -7;
        }
        if (txpower >= 0x13) {
            return 1;
        }
        if (txpower >= 0x03) {
            return -18;
        }
        /* Unknown */
        return -100;
    }

    @Override
    public int getOutputPowerMax() {
        return 5;
    }

    @Override
    public void notifyReset() {
        super.notifyReset();
        setChipSelect(false);
        status &= ~STATUS_TX_ACTIVE;
        memory[REG_FSMSTAT1] &= ~(1 << 1);
        setVRegOn(false);
        reset();
    }

    public void setVRegOn(boolean newOn) {
        if(isRadioOn == newOn) return;

        if(newOn) {
            // 0.6ms maximum vreg startup from datasheet pg 13
            // but Z1 platform does not work with 0.1 so trying with lower...
            cpu.scheduleTimeEventMillis(vregEvent, 0.05);
            if (DEBUG) log("Scheduling vregEvent at: cyc = " + cpu.cycles +
                    " target: " + vregEvent.getTime() + " current: " + cpu.getTime());
        } else {
            isRadioOn = false;
            setState(RadioState.VREG_OFF);
        }
    }

    public void setChipSelect(boolean select) {
        chipSelect = select;
        if (!chipSelect) {
            spiLen = 0;
            if (command != null) {
                command.executeSPICommand();
            }
            command = null;
        }

        if (DEBUG) {
            log("ChipSelect: " + chipSelect);
        }
    }

    public boolean getChipSelect() {
        return chipSelect;
    }

    public void setGPIO(int index, IOPort port, int pin) {
        gpio[index].setConfig(port, pin);
    }


    /*****************************************************************************
     * Chip APIs
     *****************************************************************************/

    @Override
    public int getModeMax() {
        return MODE_MAX;
    }

    private String getLongAddress() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 8; i++) {
            if ((i % 2 == 0) && i > 0) {
                sb.append(':');
            }
            sb.append(Utils.hex8(memory[RAM_IEEEADDR + 7 - i]));
        }
        return sb.toString();
    }

    @Override
    public String info() {
        updateActiveFrequency();
        String commandStr = command == null ? "<waiting>" : command.name;
        return " VREG_ON: " + isRadioOn + "  Chip Select: " + chipSelect +
                "  OSC Stable: " + ((status & STATUS_XOSC16M_STABLE) > 0) +
                "  GPIO Polarity: 0x" + Utils.hex8(memory[REG_GPIOPOLARITY]) +
                "\n RSSI Valid: " + ((status & STATUS_RSSI_VALID) > 0) + "  CCA: " + currentCCA +
                "\n FIFOP: " + currentFIFOP + " threshold: " + fifopThr + "  FIFO: " + currentFIFO + "  SFD: " + sfdGPIO.isActive() +
                "\n " + rxFIFO.stateToString() + " expPacketLen: " + rxlen +
                "\n Radio State: " + stateMachine + "  SPI State: " + commandStr +
                "\n AutoACK: " + autoAck + "  AddrDecode: " + frameFilter + "  AutoCRC: " + autoCRC +
                "\n PanID: 0x" + Utils.hex8(memory[RAM_PANID + 1]) + Utils.hex8(memory[RAM_PANID]) +
                "  ShortAddr: 0x" + Utils.hex8(memory[RAM_SHORTADDR + 1]) + Utils.hex8(memory[RAM_SHORTADDR]) +
                "  LongAddr: 0x" + getLongAddress() +
                "\n Channel: " + activeChannel +
                "  Output Power: " + getOutputPower() + "dB (" + getOutputPowerIndicator() + '/' + getOutputPowerIndicatorMax() +
                ") txpower: 0x" + Utils.hex8(memory[REG_TXPOWER]) +
                "\n";
    }

    @Override
    public void stateChanged(int state) {
    }

    /* return data in register at the correct position */
    @Override
    public int getConfiguration(int parameter) {
        return memory[parameter];
    }


    /* For SPI Commands */
    @Override
    public int getSPIData(int offset) {
        return spiData[offset];
    }

    @Override
    public int getSPIDataLen() {
        return spiLen;
    }

    @Override
    public void outputSPI(int data) {
        outputSPI = data;
    }

    /* reads one byte from RX fifo */
    void readRXFifo() {
        int fifoData = rxFIFO.read();
        if (DEBUG) log("RXFIFO READ: " + rxFIFO.stateToString());
        outputSPI = fifoData;

        /* first check and clear FIFOP - since we now have read a byte! */
        if (currentFIFOP && !overflow) {
            /* FIFOP is lowered when rxFIFO is lower than or equal to fifopThr */
            if(rxFIFO.length() <= fifopThr) {
                if (DEBUG) log("*** FIFOP cleared at: " + rxFIFO.stateToString());
                setFIFOP(false);
            }
        }

        /* initiate read of another packet - update some variables to keep track of packet reading... */
        if (rxfifoReadLeft == 0) {
            rxfifoReadLeft = fifoData;
            if (DEBUG) log("Init read of packet - len: " + rxfifoReadLeft +
                    " fifo: " + rxFIFO.stateToString());
        } else if (--rxfifoReadLeft == 0) {
            /* check if we have another packet in buffer */
            if (rxFIFO.length() > 0) {
                /* check if the packet is complete or longer than fifopThr */
                if (rxFIFO.length() > rxFIFO.peek(0) ||
                        (rxFIFO.length() > fifopThr && !decodeAddress && !frameRejected)) {
                    if (DEBUG) log("More in FIFO - FIFOP = 1! plen: " + rxFIFO.stateToString());
                    if (!overflow) setFIFOP(true);
                }
            }
        }
        // Set the FIFO pin low if there are no more bytes available in the RXFIFO.
        if (rxFIFO.length() == 0) {
            if (DEBUG) log("Setting FIFO to low (buffer empty)");
            setFIFO(false);
        }
    }

} // CC2520
