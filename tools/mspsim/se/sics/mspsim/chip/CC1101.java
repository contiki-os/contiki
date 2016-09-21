/*
 * Copyright (c) 2012, Thingsquare.
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
 */

package se.sics.mspsim.chip;
import java.util.ArrayList;
import java.util.List;

import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOPort.PinState;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.TimeEvent;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;

public class CC1101 extends Radio802154 implements USARTListener {
    protected boolean DEBUG = true;

	/* cc1101-const.h: Configuration registers */
	public static final int CC1101_IOCFG1 = 0x01;
	public static final int CC1101_IOCFG0 = 0x02;
	public static final int CC1101_FIFOTHR = 0x03;
	public static final int CC1101_SYNC1 = 0x04;
	public static final int CC1101_SYNC0 = 0x05;
	public static final int CC1101_PKTLEN = 0x06;
	public static final int CC1101_PKTCTRL1 = 0x07;
	public static final int CC1101_PKTCTRL0 = 0x08;
	public static final int CC1101_ADDR = 0x09;
	public static final int CC1101_CHANNR = 0x0A;
	public static final int CC1101_FSCTRL1 = 0x0B;
	public static final int CC1101_FSCTRL0 = 0x0C;
	public static final int CC1101_FREQ2 = 0x0D;
	public static final int CC1101_FREQ1 = 0x0E;
	public static final int CC1101_FREQ0 = 0x0F;
	public static final int CC1101_MDMCFG4 = 0x10;
	public static final int CC1101_MDMCFG3 = 0x11;
	public static final int CC1101_MDMCFG2 = 0x12;
	public static final int CC1101_MDMCFG1 = 0x13;
	public static final int CC1101_MDMCFG0 = 0x14;
	public static final int CC1101_DEVIATN = 0x15;
	public static final int CC1101_MCSM2 = 0x16;
	public static final int CC1101_MCSM1 = 0x17;
	public static final int CC1101_MCSM0 = 0x18;
	public static final int CC1101_FOCCFG = 0x19;
	public static final int CC1101_BSCFG = 0x1A;
	public static final int CC1101_AGCCTRL2 = 0x1B;
	public static final int CC1101_AGCCTRL1 = 0x1C;
	public static final int CC1101_AGCCTRL0 = 0x1D;
	public static final int CC1101_WOREVT1 = 0x1E;
	public static final int CC1101_WOREVT0 = 0x1F;
	public static final int CC1101_WORCTRL = 0x20;
	public static final int CC1101_FREND1 = 0x21;
	public static final int CC1101_FREND0 = 0x22;
	public static final int CC1101_FSCAL3 = 0x23;
	public static final int CC1101_FSCAL2 = 0x24;
	public static final int CC1101_FSCAL1 = 0x25;
	public static final int CC1101_FSCAL0 = 0x26;
	public static final int CC1101_RCCTRL1 = 0x27;
	public static final int CC1101_RCCTRL0 = 0x28;
	public static final int CC1101_FSTEST = 0x29;
	public static final int CC1101_PTEST = 0x2A;
	public static final int CC1101_AGCTEST = 0x2B;
	public static final int CC1101_TEST2 = 0x2C;
	public static final int CC1101_TEST1 = 0x2D;
	public static final int CC1101_TEST0 = 0x2E;
	public static final int CC1101_PARTNUM = 0x30;
	public static final int CC1101_VERSION = 0x31;
	public static final int CC1101_FREQEST = 0x32;
	public static final int CC1101_LQI = 0x33;
	public static final int CC1101_RSSI = 0x34;
	public static final int CC1101_MARCSTATE = 0x35;
	public static final int CC1101_WORTIME1 = 0x36;
	public static final int CC1101_WORTIME0 = 0x37;
	public static final int CC1101_PKTSTATUS = 0x38;
	public static final int CC1101_VCO_VC_DAC = 0x39;
	public static final int CC1101_TXBYTES = 0x3A;
	public static final int CC1101_RXBYTES = 0x3B;
	public static final int CC1101_RCCTRL1_STATUS = 0x3C;
	public static final int CC1101_RCCTRL0_STATUS = 0x3D;
	public static final int CC1101_PATABLE = 0x3E;
	public static final int CC1101_PA_M30 = 0x03;
	public static final int CC1101_PA_M20 = 0x0E;
	public static final int CC1101_PA_M15 = 0x1E;
	public static final int CC1101_PA_M10 = 0x27;
	public static final int CC1101_PA_M6 = 0x38;
	public static final int CC1101_PA_0 = 0x8E;
	public static final int CC1101_PA_5 = 0x84;
	public static final int CC1101_PA_7 = 0xCC;
	public static final int CC1101_PA_10 = 0xC3;
	public static final int CC1101_PA_11 = 0xC0;

	public final static int CC1101_TXFIFO = 0x3F;
	public final static int CC1101_RXFIFO = 0x3F;

	/* Strobe commands */
	public static final int CC1101_SRES = 0x30;
	public static final int CC1101_SFSTXON = 0x31;
	public static final int CC1101_SXOFF = 0x32;
	public static final int CC1101_SCAL = 0x33;
	public static final int CC1101_SRX = 0x34;
	public static final int CC1101_STX = 0x35;
	public static final int CC1101_SIDLE = 0x36;
	public static final int CC1101_SAFC = 0x37;
	public static final int CC1101_SWOR = 0x38;
	public static final int CC1101_SPWD = 0x39;
	public static final int CC1101_SFRX = 0x3A;
	public static final int CC1101_SFTX = 0x3B;
	public static final int CC1101_SWORRST = 0x3C;
	public static final int CC1101_SNOP = 0x3D;

	public static enum CC1101RadioState {
		CC1101_STATE_SLEEP(0x00)/* 0 */,
		CC1101_STATE_IDLE(0x01),
		CC1101_STATE_XOFF(0x02),
		CC1101_STATE_VCOON_MC(0x03),
		CC1101_STATE_REGON_MC(0x04),
		CC1101_STATE_MANCAL(0x05),
		CC1101_STATE_VCOON(0x06),
		CC1101_STATE_REGON(0x07),
		CC1101_STATE_STARTCAL(0x08),
		CC1101_STATE_BWBOOST(0x09),
		CC1101_STATE_FS_LOCK(0x0A),
		CC1101_STATE_IFADCON(0x0B),
		CC1101_STATE_ENDCAL(0x0C),
		CC1101_STATE_RX(0x0D) /* 13 */,
		CC1101_STATE_RX_END(0x0E),
		CC1101_STATE_RX_RST(0x0F),
		CC1101_STATE_TXRX_SWITCH(0x10),
		CC1101_STATE_RXFIFO_OVERFLOW(0x11),
		CC1101_STATE_FSTXON(0x12),
		CC1101_STATE_TX(0x13),
		CC1101_STATE_TX_END(0x14),
		CC1101_STATE_RXTX_SWITCH(0x15),
		CC1101_STATE_TXFIFO_UNDERFLOW(0x16);

		private final int state;

		CC1101RadioState(int stateNo) {
			state = stateNo;
		}

		public int getStateAsInt() {
			return state | (0b10 << 5); /* TODO 2 pin state not implemented */
		}
	};
	private TimeEvent sendEvent = new TimeEvent(0, "CC1101 Send") {
		public void execute(long t) {
			txNext();
		}
	};

	public final static double SYMBOL_PERIOD = 0.016; /* TODO XXX 16 us */
	public final static double FREQUENCY_CHANNEL_0 = 902; /* MHz */
	public final static double FREQUENCY_CHANNEL_WIDTH = 0.125; /* MHz */

  public final static int CCA_THRESHOLD = -95;

	private StateListener stateListener = null;
	private ReceiverListener receiverListener = null;
	
	private int currentRssiReg = 0;

	private CC1101RadioState state = null;

	protected List<Byte> txfifo = new ArrayList<Byte>();
	protected List<Byte> rxfifo = new ArrayList<Byte>();

	protected int[] registers = new int[64];
	protected int[] memory = new int[512];

	private boolean chipSelect;

	private IOPort gdo0Port = null;
	private int gdo0Pin = -1;
	private IOPort gdo2Port = null;
	private int gdo2Pin = -1;

	private int channel = 0;

	public CC1101(MSP430Core cpu) {
		super("CC1101", "Radio", cpu);
		reset();
	}

	public void log(String str) {
	    if (DEBUG) {
	        System.out.println(str);
	    }
	}
	
	public void strobe(int cmd) {
		switch (cmd) {
		case CC1101_SRES:
			log("CC1101_SRES not implemented");
			reset();
			break;

		case CC1101_SFSTXON:
			log("CC1101_SFSTXON not implemented");
			break;

		case CC1101_SXOFF:
			log("CC1101_SXOFF not implemented");
			break;

		case CC1101_SCAL:
			setState(CC1101RadioState.CC1101_STATE_IDLE);
			break;

		case CC1101_SRX:
			setState(CC1101RadioState.CC1101_STATE_RX);
			break;

		case CC1101_STX:
            int len = (int) (0xff&txfifo.get(0));
            txFooterCountdown = 1 + len + 1/*len*/;
            if (DEBUG) {
                System.out.println("TX started: len = " + len + ", txFooterCountdown = " + txFooterCountdown);
            }
            txNext();
            setState(CC1101RadioState.CC1101_STATE_TX);
			break;

		case CC1101_SIDLE:
			setState(CC1101RadioState.CC1101_STATE_IDLE);
			break;

		case CC1101_SAFC:
			log("CC1101_SAFC not implemented");
			break;

		case CC1101_SWOR:
			log("CC1101_SWOR not implemented");
			break;

		case CC1101_SPWD:
			log("CC1101_SPWD almost implemented");
			/* TODO XXX
			* Wait until CS is de-asserted. (We should at least wait
			* receiving or transmitting.)*/
			setState(CC1101RadioState.CC1101_STATE_IDLE);
			break;

		case CC1101_SFRX:
			rxfifo.clear();
			rxExpectedLen = -1;
			rxGotSynchByte = false;
			setGDO0(false);
			/*printRXFIFO();*/
			break;

		case CC1101_SFTX:
			txfifo.clear();
			txFooterCountdown = -1;
			/*printTXFIFO();*/
			break;

		case CC1101_SWORRST:
			log("CC1101_SWORRST not implemented");
			break;

		case CC1101_SNOP:
			log("CC1101_SNOP not implemented");
			break;

		default:
			System.out.printf("strobe(0x%02x)\n", cmd);
			break;
		}
	}

	public int getLQI() {
		return 0; /* TODO */
	};

	public void setLQI(int lqi) {
	}

	/* SPI */
	public final static int SPI_READ_BIT = 0x80;
	public final static int SPI_BURST_BIT = 0x40;

	private boolean spiBurstMode = false;
	private boolean spiReadMode = false;
	private boolean spiGotAddress = false;
	private int spiAddress;

	public static boolean spiIsBurst(int data) {
		return (data & SPI_BURST_BIT) == SPI_BURST_BIT;
	}

	public static boolean spiIsRead(int data) {
		return (data & SPI_READ_BIT) == SPI_READ_BIT;
	}

	public static boolean spiIsStrobe(int data) {
		data = data & 0x3f;
		return data >= 0x30 && data <= 0x3d;
	}

	private void spiResetState() {
		spiBurstMode = false;
		spiReadMode = false;
		spiGotAddress = false;
		spiAddress = 0xFF;
	}

	public void dataReceived(USARTSource source, int data) {
		if (spiGotAddress) {
			if (!spiBurstMode) {
				/* Single access mode */
				if (spiReadMode) {
					source.byteReceived(getReg(spiAddress));
				} else {
					source.byteReceived(setReg(spiAddress, data));
				}
				spiResetState();
			} else {
				/* Burst mode */
				if (spiReadMode) {
					source.byteReceived(getReg(spiAddress));
				} else {
					source.byteReceived(setReg(spiAddress, data));
				}

				if (spiAddress != CC1101_TXFIFO && spiAddress != CC1101_PATABLE) {
					spiAddress++;
				}
			}
			return;
		}

		/* Read/write. Burst/single. */
		spiBurstMode = spiIsBurst(data);
		spiReadMode = spiIsRead(data);

		/* Is this a strobe command? */
		if (!spiBurstMode && spiIsStrobe(data)) {
			/* Strobe command */
			strobe(data);
			source.byteReceived(0);
			spiResetState();
			return;
		}

		spiAddress = data & 0x3f;
		spiGotAddress = true;

		/* Return MARCSTATE */
		source.byteReceived(getMarcstate());
		return;
	}
	public int setReg(int address, int data) {
		switch (address) {
		case CC1101_TXFIFO:
			txfifo.add((byte) data);
			/*printTXFIFO();*/
			return txfifo.size();
		case CC1101_CHANNR:
			channel = data;
			if (channelListener != null) {
				channelListener.channelChanged(channel);
			}
			return 0;
		}

		log(String.format("setReg(0x%02x) 0x%02x", address, data));
		int oldValue = registers[address];
		registers[address] = data;
		configurationChanged(address, oldValue, data);
		return oldValue;
	}
	public int getReg(int address) {
		/* MSP430Core.profiler.printStackTrace(System.out); */
		switch (address) {
		case CC1101_MARCSTATE:
			return getMarcstate();
		case CC1101_RXBYTES:
		  /*log("getReg(CC1101_RXBYTES) " + rxfifo.size());*/
			return rxfifo.size();
		case CC1101_TXBYTES:
		  /*log("getReg(CC1101_TXBYTES) " + txfifo.size());*/
			return txfifo.size();
		case CC1101_PKTSTATUS:
		  int status;
		  final int CCA_BIT = (1<<4);
		  if(currentRssiReg < CCA_THRESHOLD) {
		    status = CCA_BIT;
		  } else {
		    status = 0;
		  }
		  return status;
		case CC1101_RSSI:
			return currentRssiReg;
		case CC1101_RXFIFO:
			if (rxfifo.size() > 0) {
				int ret = (int) rxfifo.remove(0);
				/*printRXFIFO();*/
				return ret;
			}
			System.err.println("Warning: reading from empty RXFIFO");
			return -1;
		}

        log(String.format("getReg(0x%02x) 0x%02x", address, registers[address]));
		return registers[address];
	}
	
	

	public boolean isReadyToReceive() {
	  /* TODO Implement me */
	  if (getState() == CC1101RadioState.CC1101_STATE_IDLE) {
	    return false;
	  }
	  if (getState() == CC1101RadioState.CC1101_STATE_SLEEP) {
	    return false;
	  }
	  return true;
	}

	/* txFooterCountdown: send CRC footer in these many bytes */
	protected int txFooterCountdown = -1;

	/* TX/RX states */
	public static final int NUM_PREAMBLE = 4;
    public static final int NUM_SYNCH = 4;
    public static final byte SYNCH_BYTE_LAST = (byte) 0x91;
    private boolean txSentSynchByte = false;
    private int txSendSynchByteCnt = 0;
    private boolean txSentFirstCRC = false;
    void txNext() {
        if (txFooterCountdown < 0) {
            System.out.println("Warning: Aborting transmit since txFooterCountdown=" + txFooterCountdown);
            return;
        }
        
        /* Send preamble and synch bytes */
        if (!txSentSynchByte) {
            /* Send NUM_PREAMBLE preamble bytes */
            if (txSendSynchByteCnt < NUM_PREAMBLE) {
                txSendSynchByteCnt++;
                if (rfListener != null) {
                    rfListener.receivedByte((byte) (0xaa));
                }
                cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);
                return;
            }
            /* Send NUM_SYNCH-1 synch bytes */
            if (txSendSynchByteCnt < NUM_PREAMBLE + NUM_SYNCH - 1) {
                txSendSynchByteCnt++;
                if (rfListener != null) {
                    rfListener.receivedByte((byte) (SYNCH_BYTE_LAST + 1));
                }
                cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);
                return;
            }
            /* Send last synch byte */
            if (txSendSynchByteCnt < NUM_PREAMBLE + NUM_SYNCH) {
                txSendSynchByteCnt++;
                if (rfListener != null) {
                    rfListener.receivedByte((byte) (SYNCH_BYTE_LAST));
                }
                cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);

                txSentSynchByte = true;
                return;
            }
            
            txSentFirstCRC = false;
        }

        if (txSentFirstCRC) {
            /* send second CRC byte */
            if (rfListener != null) {
                rfListener.receivedByte((byte) (0xef));
            }
            if (!txfifo.isEmpty()) {
                System.out.println("Warning: TXFIFO not empty after sending CRC bytes");
            }
            setStateRX();
            txSentSynchByte = false;
            txSendSynchByteCnt = 0;
            txSentFirstCRC = false;
            return;
        }

        txFooterCountdown--;

        if (txFooterCountdown == 0) {
            /* countdown is zero, send first CRC byte */
            if (rfListener != null) {
                rfListener.receivedByte((byte) (0xee));
            }
            txSentFirstCRC = true;
            cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);
            return;
        }

        /* Send payload byte (including first FIFO length byte) */
        if (txfifo.isEmpty()) {
            System.out.println("Warning: TXFIFO is empty, sending zero-byte, txFooterCountdown=" + txFooterCountdown);
            if (rfListener != null) {
                rfListener.receivedByte((byte) 0);
            }
            cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);
            setState(CC1101RadioState.CC1101_STATE_TXFIFO_UNDERFLOW);
            return;
        }

        if (rfListener != null) {
            rfListener.receivedByte((byte) (txfifo.get(0).intValue()));
        }
        txfifo.remove(0);
        cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);

        /*printTXFIFO();*/
    }

	public void setGDO0(IOPort port, int pin) {
		gdo0Port = port;
		gdo0Pin = pin;
	}
	public void setGDO0(boolean active) {
		/* TODO Read active low/high configuration */

		if (active) {
		  if (gdo0Port != null) {
	      gdo0Port.setPinState(gdo0Pin, IOPort.PinState.HI);
		  }
			if (gdo0Listener != null) {
			  gdo0Listener.event(IOPort.PinState.HI);
			}
		} else {
      if (gdo0Port != null) {
        gdo0Port.setPinState(gdo0Pin, IOPort.PinState.LOW);
      }
      if (gdo0Listener != null) {
        gdo0Listener.event(IOPort.PinState.LOW);
      }
		}
	}

	public interface GDOListener {
	  public void event(PinState state);
	}
	private GDOListener gdo0Listener = null;
	public void setGDO0Listener(GDOListener l) {
    gdo0Listener = l;
  }
	
	public void setGDO2(IOPort port, int pin) {
		gdo2Port = port;
		gdo2Pin = pin;
	}
	public void setGDO2(boolean active) {
        if (gdo2Port == null) {
            return;
        }
		gdo2Port.setPinState(gdo2Pin, active ? IOPort.PinState.LOW
				: IOPort.PinState.HI);
	}

	private void printRXFIFO() {
	    StringBuilder sb = new StringBuilder();
	    sb.append(String.format("RXFIFO[%03d]: ", rxfifo.size()));
		for (int i = 0; i < rxfifo.size(); i++) {
		    sb.append(String.format("%02x", rxfifo.get(i)));
		}
		log(sb.toString() + "\n");
	}
	private void printTXFIFO() {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("TXFIFO[%03d]: ", txfifo.size()));
		for (int i = 0; i < txfifo.size(); i++) {
		    sb.append(String.format("%02x", txfifo.get(i)));
		}
        log(sb.toString() + "\n");
	}

	public CC1101RadioState getState() {
		return state;
	}
	public interface StateListener {
		public void newState(CC1101RadioState state);
	}
	public void setStateListener(StateListener listener) {
		stateListener = listener;
	}
  public interface ReceiverListener {
    public void newState(boolean on);
  }
  public void setReceiverListener(ReceiverListener listener) {
    receiverListener = listener;
  }

  boolean receiverOn = false;
	boolean setState(CC1101RadioState newState) {
		if (newState != CC1101RadioState.CC1101_STATE_IDLE
				&& newState != CC1101RadioState.CC1101_STATE_RX
				&& newState != CC1101RadioState.CC1101_STATE_TX) {
			log("setState(" + newState + ")");
		}

		state = newState;

		/* Notify state listener */
		if (stateListener != null) {
			stateListener.newState(state);
		}
    if (receiverListener != null && isReadyToReceive() != receiverOn) {
      receiverOn = isReadyToReceive();
      receiverListener.newState(receiverOn);
    }
    return true;
	}

	boolean rxGotSynchByte = false;
	private int rxExpectedLen = -1;
	public void receivedByte(byte data) {
		if (!rxGotSynchByte) {
			/* Await synch byte */
			if (data == SYNCH_BYTE_LAST) {
				rxGotSynchByte = true;
				setGDO0(true);
			}
			return;
		}

		if (rxExpectedLen < 0) {
			rxExpectedLen = 1/*len*/ + (int) data/*payload*/ + 2/*CRC*/;
		}

		rxExpectedLen--;
		if (rxExpectedLen == 0) {
			setGDO0(false);
			rxExpectedLen = -1;
			rxGotSynchByte = false;
		}

		rxfifo.add(data);
		/*printRXFIFO();*/
	}

	public String info() {
		return "CC1101 info: [not implemented]";
	}

	public int getRegister(int register) {
		return registers[register];
	}

	public void setRegister(int register, int data) {
		registers[register] = data;
	}

	public void setRSSI(int power) {
	  currentRssiReg = power;
	}
	public int getRSSI() {
		return currentRssiReg;
	}

	public int getActiveFrequency() {
		return (int) 0; /* Not implemented */
	}

	public int getActiveChannel() {
		return channel;
	}

	public int getOutputPowerIndicator() {
		return 1;
	}

	public double getFrequency() {
		return 0; /* Not implemented */
	}

	public void notifyReset() {
		super.notifyReset();
		setChipSelect(false);
	}

	public int getOutputPower() {
		return 1;
	}

	public int getOutputPowerMax() {
		return 1;
	}

	public int getOutputPowerIndicatorMax() {
		return 1;
	}

	/*****************************************************************************
	 * Chip APIs
	 *****************************************************************************/

	public int getModeMax() {
		return 0;
	}

	/* return data in register at the correct position */
	public int getConfiguration(int parameter) {
		return registers[parameter];
	}

	public boolean getChipSelect() {
		return chipSelect;
	}

	public void setChipSelect(boolean select) {
		chipSelect = select;
		if (!chipSelect) {
			spiResetState();
		}

		if (DEBUG) {
			/*log("Chip select: " + chipSelect);*/
		}
	}

	void setStateRX() {
		setState(CC1101RadioState.CC1101_STATE_RX);
	}

	void reset() {
	  setState(CC1101RadioState.CC1101_STATE_IDLE);

	  registers[CC1101_PARTNUM] = 0;
	  registers[CC1101_VERSION] = 6;
	}

	public int getMarcstate() {
		return getState().getStateAsInt();
	}

	private boolean lastWasRead = false;
  public void setLastInstructionWasRead(boolean wasRead) {
    lastWasRead = wasRead;
  }

  public int getStatusByte() {
    int status = 0;

    /* Bit 7: RF ready */
    if (true) {
      status += 0; /* XXX Always ready! */
    }

    /* Bit 4-6: simplified state */     
    status = status << 3;
    if (state == CC1101RadioState.CC1101_STATE_IDLE) {
      status += 0;
    } else if (state == CC1101RadioState.CC1101_STATE_RX ||
        state == CC1101RadioState.CC1101_STATE_RX_END ||
        state == CC1101RadioState.CC1101_STATE_RX_RST) {
      status += 1;
    } else if (state == CC1101RadioState.CC1101_STATE_TX ||
        state == CC1101RadioState.CC1101_STATE_TX_END) {
      status += 2;
    } else if (state == CC1101RadioState.CC1101_STATE_FSTXON) {
      status += 3;
    } else if (state == CC1101RadioState.CC1101_STATE_ENDCAL ||
        state == CC1101RadioState.CC1101_STATE_MANCAL ||
        state == CC1101RadioState.CC1101_STATE_STARTCAL) {
      status += 4;
    } else if (state == CC1101RadioState.CC1101_STATE_RXTX_SWITCH ||
        state == CC1101RadioState.CC1101_STATE_TXRX_SWITCH) {
      status += 5;
    } else if (state == CC1101RadioState.CC1101_STATE_RXFIFO_OVERFLOW) {
      status += 6;
    } else if (state == CC1101RadioState.CC1101_STATE_TXFIFO_UNDERFLOW) {
      status += 7;
    }

    /* Bit 0-3 */
    status = status << 4;
    if (lastWasRead) {
      /* Return available bytes in RXFIFO */
      int available = rxfifo.size();
      if (available > 15) {
        status += 15;
      } else {
        status += available;
      }
    } else {
      /* Return available bytes in TXFIFO */
      int available = 64 - txfifo.size();
      if (available > 15) {
        status += 15;
      } else {
        status += available;
      }
    }

    return status;
  }

}
