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
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.TimeEvent;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;

public class CC1120 extends Radio802154 implements USARTListener {

	/* cc1120-const.h: Configuration registers */
	public final static int CC1120_IOCFG3 = 0x00;
	public final static int CC1120_IOCFG2 = 0x01;
	public final static int CC1120_IOCFG1 = 0x02;
	public final static int CC1120_IOCFG0 = 0x03;
	public final static int CC1120_SYNC3 = 0x04;
	public final static int CC1120_SYNC2 = 0x05;
	public final static int CC1120_SYNC1 = 0x06;
	public final static int CC1120_SYNC0 = 0x07;
	public final static int CC1120_SYNC_CFG1 = 0x08;
	public final static int CC1120_SYNC_CFG0 = 0x09;
	public final static int CC1120_DEVIATION_M = 0x0A;
	public final static int CC1120_MODCFG_DEV_E = 0x0B;
	public final static int CC1120_DCFILT_CFG = 0x0C;
	public final static int CC1120_PREAMBLE_CFG1 = 0x0D;
	public final static int CC1120_PREAMBLE_CFG0 = 0x0E;
	public final static int CC1120_FREQ_IF_CFG = 0x0F;
	public final static int CC1120_IQIC = 0x10;
	public final static int CC1120_CHAN_BW = 0x11;
	public final static int CC1120_MDMCFG1 = 0x12;
	public final static int CC1120_MDMCFG0 = 0x13;
	public final static int CC1120_DRATE2 = 0x14;
	public final static int CC1120_DRATE1 = 0x15;
	public final static int CC1120_DRATE0 = 0x16;
	public final static int CC1120_AGC_REF = 0x17;
	public final static int CC1120_AGC_CS_THR = 0x18;
	public final static int CC1120_AGC_GAIN_ADJUST = 0x19;
	public final static int CC1120_AGC_CFG3 = 0x1A;
	public final static int CC1120_AGC_CFG2 = 0x1B;
	public final static int CC1120_AGC_CFG1 = 0x1C;
	public final static int CC1120_AGC_CFG0 = 0x1D;
	public final static int CC1120_FIFO_CFG = 0x1E;
	public final static int CC1120_DEV_ADDR = 0x1F;
	public final static int CC1120_SETTLING_CFG = 0x20;
	public final static int CC1120_FS_CFG = 0x21;
	public final static int CC1120_WOR_CFG1 = 0x22;
	public final static int CC1120_WOR_CFG0 = 0x23;
	public final static int CC1120_WOR_EVENT0_MSB = 0x24;
	public final static int CC1120_WOR_EVENT0_LSB = 0x25;
	public final static int CC1120_PKT_CFG2 = 0x26;
	public final static int CC1120_PKT_CFG1 = 0x27;
	public final static int CC1120_PKT_CFG0 = 0x28;
	public final static int CC1120_RFEND_CFG1 = 0x29;
	public final static int CC1120_RFEND_CFG0 = 0x2A;
	public final static int CC1120_PA_CFG2 = 0x2B;
	public final static int CC1120_PA_CFG1 = 0x2C;
	public final static int CC1120_PA_CFG0 = 0x2D;
	public final static int CC1120_PKT_LEN = 0x2E;
	public final static int CC1120_EXTENDED_MEMORY_ACCESS = 0x2F;

	/* cc1120-const.h: Extended register space */
	public final static int CC1120_IF_MIX_CFG = 0x00;
	public final static int CC1120_FREQOFF_CFG = 0x01;
	public final static int CC1120_TOC_CFG = 0x02;
	public final static int CC1120_MARC_SPARE = 0x03;
	public final static int CC1120_ECG_CFG = 0x04;
	public final static int CC1120_SOFT_TX_DATA_CFG = 0x05;
	public final static int CC1120_EXT_CTRL = 0x06;
	public final static int CC1120_RCCAL_FINE = 0x07;
	public final static int CC1120_RCCAL_COARSE = 0x08;
	public final static int CC1120_RCCAL_OFFSET = 0x09;
	public final static int CC1120_FREQOFF1 = 0x0A;
	public final static int CC1120_FREQOFF0 = 0x0B;
	public final static int CC1120_FREQ2 = 0x0C;
	public final static int CC1120_FREQ1 = 0x0D;
	public final static int CC1120_FREQ0 = 0x0E;
	public final static int CC1120_IF_ADC2 = 0x0F;
	public final static int CC1120_IF_ADC1 = 0x10;
	public final static int CC1120_IF_ADC0 = 0x11;
	public final static int CC1120_FS_DIG1 = 0x12;
	public final static int CC1120_FS_DIG0 = 0x13;
	public final static int CC1120_FS_CAL3 = 0x14;
	public final static int CC1120_FS_CAL2 = 0x15;
	public final static int CC1120_FS_CAL1 = 0x16;
	public final static int CC1120_FS_CAL0 = 0x17;
	public final static int CC1120_FS_CHP = 0x18;
	public final static int CC1120_FS_DIVTWO = 0x19;
	public final static int CC1120_FS_DSM1 = 0x1A;
	public final static int CC1120_FS_DSM0 = 0x1B;
	public final static int CC1120_FS_DVC1 = 0x1C;
	public final static int CC1120_FS_DVC0 = 0x1D;
	public final static int CC1120_FS_LBI = 0x1E;
	public final static int CC1120_FS_PFD = 0x1F;
	public final static int CC1120_FS_PRE = 0x20;
	public final static int CC1120_FS_REG_DIV_CML = 0x21;
	public final static int CC1120_FS_SPARE = 0x22;
	public final static int CC1120_FS_VCO4 = 0x23;
	public final static int CC1120_FS_VCO3 = 0x24;
	public final static int CC1120_FS_VCO2 = 0x25;
	public final static int CC1120_FS_VCO1 = 0x26;
	public final static int CC1120_FS_VCO0 = 0x27;
	public final static int CC1120_GBIAS6 = 0x28;
	public final static int CC1120_GBIAS5 = 0x29;
	public final static int CC1120_GBIAS4 = 0x2A;
	public final static int CC1120_GBIAS3 = 0x2B;
	public final static int CC1120_GBIAS2 = 0x2C;
	public final static int CC1120_GBIAS1 = 0x2D;
	public final static int CC1120_GBIAS0 = 0x2E;
	public final static int CC1120_IFAMP = 0x2F;
	public final static int CC1120_LNA = 0x30;
	public final static int CC1120_RXMIX = 0x31;
	public final static int CC1120_XOSC5 = 0x32;
	public final static int CC1120_XOSC4 = 0x33;
	public final static int CC1120_XOSC3 = 0x34;
	public final static int CC1120_XOSC2 = 0x35;
	public final static int CC1120_XOSC1 = 0x36;
	public final static int CC1120_XOSC0 = 0x37;
	public final static int CC1120_ANALOG_SPARE = 0x38;
	public final static int CC1120_PA_CFG3 = 0x39;
	public final static int CC1120_WOR_TIME1 = 0x64;
	public final static int CC1120_WOR_TIME0 = 0x65;
	public final static int CC1120_WOR_CAPTURE1 = 0x66;
	public final static int CC1120_WOR_CAPTURE0 = 0x67;
	public final static int CC1120_BIST = 0x68;
	public final static int CC1120_DCFILTOFFSET_I1 = 0x69;
	public final static int CC1120_DCFILTOFFSET_I0 = 0x6A;
	public final static int CC1120_DCFILTOFFSET_Q1 = 0x6B;
	public final static int CC1120_DCFILTOFFSET_Q0 = 0x6C;
	public final static int CC1120_IQIE_I1 = 0x6D;
	public final static int CC1120_IQIE_I0 = 0x6E;
	public final static int CC1120_IQIE_Q1 = 0x6F;
	public final static int CC1120_IQIE_Q0 = 0x70;
	public final static int CC1120_RSSI1 = 0x71;
	public final static int CC1120_RSSI0 = 0x72;
	public final static int CC1120_MARCSTATE = 0x73;
	public final static int CC1120_LQI_VAL = 0x74;
	public final static int CC1120_PQT_SYNC_ERR = 0x75;
	public final static int CC1120_DEM_STATUS = 0x76;
	public final static int CC1120_FREQOFF_EST1 = 0x77;
	public final static int CC1120_FREQOFF_EST0 = 0x78;
	public final static int CC1120_AGC_GAIN3 = 0x79;
	public final static int CC1120_AGC_GAIN2 = 0x7A;
	public final static int CC1120_AGC_GAIN1 = 0x7B;
	public final static int CC1120_AGC_GAIN0 = 0x7C;
	public final static int CC1120_SOFT_RX_DATA_OUT = 0x7D;
	public final static int CC1120_SOFT_TX_DATA_IN = 0x7E;
	public final static int CC1120_ASK_SOFT_RX_DATA = 0x7F;
	public final static int CC1120_RNDGEN = 0x80;
	public final static int CC1120_MAGN2 = 0x81;
	public final static int CC1120_MAGN1 = 0x82;
	public final static int CC1120_MAGN0 = 0x83;
	public final static int CC1120_ANG1 = 0x84;
	public final static int CC1120_ANG0 = 0x85;
	public final static int CC1120_CHFILT_I2 = 0x86;
	public final static int CC1120_CHFILT_I1 = 0x87;
	public final static int CC1120_CHFILT_I0 = 0x88;
	public final static int CC1120_CHFILT_Q2 = 0x89;
	public final static int CC1120_CHFILT_Q1 = 0x8A;
	public final static int CC1120_CHFILT_Q0 = 0x8B;
	public final static int CC1120_GPIO_STATUS = 0x8C;
	public final static int CC1120_FSCAL_CTRL = 0x8D;
	public final static int CC1120_PHASE_ADJUST = 0x8E;
	public final static int CC1120_PARTNUMBER = 0x8F;
	public final static int CC1120_PARTVERSION = 0x90;
	public final static int CC1120_SERIAL_STATUS = 0x91;
	public final static int CC1120_RX_STATUS = 0x92;
	public final static int CC1120_TX_STATUS = 0x93;
	public final static int CC1120_MARC_STATUS1 = 0x94;
	public final static int CC1120_MARC_STATUS0 = 0x95;
	public final static int CC1120_PA_IFAMP_TEST = 0x96;
	public final static int CC1120_FSRF_TEST = 0x97;
	public final static int CC1120_PRE_TEST = 0x98;
	public final static int CC1120_PRE_OVR = 0x99;
	public final static int CC1120_ADC_TEST = 0x9A;
	public final static int CC1120_DVC_TEST = 0x9B;
	public final static int CC1120_ATEST = 0x9C;
	public final static int CC1120_ATEST_LVDS = 0x9D;
	public final static int CC1120_ATEST_MODE = 0x9E;
	public final static int CC1120_XOSC_TEST1 = 0x9F;
	public final static int CC1120_XOSC_TEST0 = 0xA0;
	public final static int CC1120_RXFIRST = 0xD2;
	public final static int CC1120_TXFIRST = 0xD3;
	public final static int CC1120_RXLAST = 0xD4;
	public final static int CC1120_TXLAST = 0xD5;
	public final static int CC1120_NUM_TXBYTES = 0xD6;
	public final static int CC1120_NUM_RXBYTES = 0xD7;
	public final static int CC1120_FIFO_NUM_TXBYTES = 0xD8;
	public final static int CC1120_FIFO_NUM_RXBYTES = 0xD9;

	public static final int CC1120_SRES = 0x30; // Reset chip.;
	public static final int CC1120_SFSTXON = 0x31; // Enable and calibrate
	// frequency synthesizer (if
	// MCSM0.FS_AUTOCAL=1).;
	public static final int CC1120_SXOFF = 0x32; // Turn off crystal
	// oscillator.;
	public static final int CC1120_SCAL = 0x33; // Calibrate frequency
	// synthesizer and turn it off;
	public static final int CC1120_SRX = 0x34; // Enable RX. Perform calibration
	// first if coming from IDLE
	// and;
	public static final int CC1120_STX = 0x35; // In IDLE state: Enable TX.
	// Perform calibration first if;
	public static final int CC1120_SIDLE = 0x36; // Exit RX / TX, turn off
	// frequency synthesizer and
	// exit;
	public static final int CC1120_SAFC = 0x37; // Perform AFC adjustment of the
	// frequency synthesizer;
	public static final int CC1120_SWOR = 0x38; // Start automatic RX polling
	// sequence (Wake-on-Radio);
	public static final int CC1120_SPWD = 0x39; // Enter power down mode when
	// CSn goes high.;
	public static final int CC1120_SFRX = 0x3A; // Flush the RX FIFO buffer.;
	public static final int CC1120_SFTX = 0x3B; // Flush the TX FIFO buffer.;
	public static final int CC1120_SWORRST = 0x3C; // Reset real time clock.;
	public static final int CC1120_SNOP = 0x3D; // No operation. May be used to
	// pad strobe commands to two;

	public final static int CC1120_TXFIFO = 0x3F;
	public final static int CC1120_RXFIFO = 0x3F;

  public final static int CCA_THRESHOLD = -95;

	public static enum CC1120RadioState {
		CC1120_STATE_SLEEP(0b00000)/* 0 */,
		CC1120_STATE_IDLE(0b00001),
		CC1120_STATE_XOFF(0b00010),
		CC1120_STATE_BIAS_SETTLE_MC(0b00011),
		CC1120_STATE_REG_SETTLE_MC(0b00100),
		CC1120_STATE_MANCAL(0b00101),
		CC1120_STATE_BIAS_SETTLE(0b00110),
		CC1120_STATE_REG_SETTLE(0b00111),
		CC1120_STATE_STARTCAL(0b01000),
		CC1120_STATE_BWBOOST(0b01001),
		CC1120_STATE_FS_LOCK(0b01010),
		CC1120_STATE_IFADCON(0b01011),
		CC1120_STATE_ENDCAL(0b01100),
		CC1120_STATE_RX(0b01101) /* 13 */,
		CC1120_STATE_RX_END(0b01110),
		CC1120_STATE_Reserved(0b01111),
		CC1120_STATE_TXRX_SWITCH(0b10000),
		CC1120_STATE_RX_FIFO_ERR(0b10001),
		CC1120_STATE_FSTXON(0b10010),
		CC1120_STATE_TX(0b10011),
		CC1120_STATE_TX_END(0b10100),
		CC1120_STATE_RXTX_SWITCH(0b10101),
		CC1120_STATE_TX_FIFO_ERR(0b10110),
		CC1120_STATE_IFADCON_TXRX(0b10111);

		private final int state;

		CC1120RadioState(int stateNo) {
			state = stateNo;
		}

		public int getStateAsInt() {
			return state | (0b10 << 5); /* TODO 2 pin state not implemented */
		}
	};

	private TimeEvent sendEvent = new TimeEvent(0, "CC1120 Send") {
		public void execute(long t) {
			txNext();
		}
	};

	protected boolean DEBUG = false;

	public final static double SYMBOL_PERIOD = 0.016; /* TODO XXX 16 us */
	public final static double FREQUENCY_CHANNEL_0 = 902; /* MHz */
	public final static double FREQUENCY_CHANNEL_WIDTH = 0.125; /* MHz */

  private StateListener stateListener = null;
  private ReceiverListener receiverListener = null;

  /* RSSI1: RSSI_11_4 */
	private int currentRssiReg1 = 0;
  /* RSSI0: RSSI_3_0 TODO XXX Ignored */
	private int currentRssiReg0 = 0;

	private double frequency = -1;
	private int nextFreq0 = -1, nextFreq1 = -1, nextFreq2 = -1; /* regs */
	private boolean changeFrequencyNextState;

	private CC1120RadioState state = null;

	protected List<Byte> txfifo = new ArrayList<Byte>();
	protected List<Byte> rxfifo = new ArrayList<Byte>();

	protected int[] registers = new int[64];
	protected int[] extendedRegisters = new int[256];
	protected int[] memory = new int[512];

	private boolean chipSelect;

	private IOPort gdo0Port = null;
	private int gdo0Pin = -1;
	private IOPort gdo2Port = null;
	private int gdo2Pin = -1;

	public CC1120(MSP430Core cpu) {
		super("CC1120", "Radio", cpu);
		reset();
	}

	void strobe(int cmd) {
		switch (cmd) {
		case CC1120_SRES:
			System.out.println("CC1120_SRES not implemented");
			reset();
			break;

		case CC1120_SFSTXON:
			System.out.println("CC1120_SFSTXON not implemented");
			break;

		case CC1120_SXOFF:
			System.out.println("CC1120_SXOFF not implemented");
			break;

		case CC1120_SCAL:
			changeFrequencyNextState = true;
			setState(CC1120RadioState.CC1120_STATE_IDLE);
			break;

		case CC1120_SRX:
			setState(CC1120RadioState.CC1120_STATE_RX);
			break;

		case CC1120_STX:
      int len = (int) (0xff&txfifo.get(0));
      txFooterCountdown = 1 + len + 1/*len*/;
      if (DEBUG) {
          System.out.println("TX started: len = " + len + ", txFooterCountdown = " + txFooterCountdown);
      }
      txNext();
			setState(CC1120RadioState.CC1120_STATE_TX);
			break;

		case CC1120_SIDLE:
			setState(CC1120RadioState.CC1120_STATE_IDLE);
			break;

		case CC1120_SAFC:
			System.out.println("CC1120_SAFC not implemented");
			break;

		case CC1120_SWOR:
			System.out.println("CC1120_SWOR not implemented");
			break;

		case CC1120_SPWD:
			System.out.println("CC1120_SPWD almost implemented");
			/* TODO XXX
			 * Wait until CS is de-asserted. (We should at least wait until radio is done
			 * receiving or transmitting.)*/
            setState(CC1120RadioState.CC1120_STATE_SLEEP);
			break;

		case CC1120_SFRX:
			rxfifo.clear();
			rxExpectedLen = -1;
			rxGotSynchByte = false;
			setGDO0(false);
			/* printRXFIFO(); */
			break;

		case CC1120_SFTX:
			txfifo.clear();
			txFooterCountdown = -1;
			/* printTXFIFO(); */
			break;

		case CC1120_SWORRST:
			System.out.println("CC1120_SWORRST not implemented");
			break;

		case CC1120_SNOP:
			System.out.println("CC1120_SNOP not implemented");
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
	private final static int SPI_READ_BIT = 0x80;
	private final static int SPI_BURST_BIT = 0x40;
	private final static int SPI_EXTENDED_ADDRESS = 0x2F;

	private boolean spiAwaitingAddressExtended = false;
	private boolean spiExtendedMode = false;
	private boolean spiBurstMode = false;
	private boolean spiReadMode = false;
	private boolean spiGotAddress = false;
	private int spiAddress;

	private static boolean spiIsBurst(int data) {
		return (data & SPI_BURST_BIT) == SPI_BURST_BIT;
	}

	private static boolean spiIsRead(int data) {
		return (data & SPI_READ_BIT) == SPI_READ_BIT;
	}

	private static boolean spiIsExtended(int data) {
		data &= ~SPI_BURST_BIT;
		data &= ~SPI_READ_BIT;
		return data == SPI_EXTENDED_ADDRESS;
	}

	private static boolean spiIsStrobe(int data) {
		data = data & 0x3f;
		return data >= 0x30 && data <= 0x3e;
	}

	private void spiResetState() {
		spiAwaitingAddressExtended = false;
		spiBurstMode = false;
		spiReadMode = false;
		spiGotAddress = false;
		spiExtendedMode = false;
		spiAddress = 0xFF;
	}

	public void dataReceived(USARTSource source, int data) {
		if (spiGotAddress) {
			if (!spiBurstMode) {
				/* Single access mode */
				if (spiReadMode) {
					source.byteReceived(getReg(spiAddress, spiExtendedMode));
				} else {
					source.byteReceived(setReg(spiAddress, data,
							spiExtendedMode));
				}
				spiResetState();
			} else {
				/* Burst mode */
				if (spiReadMode) {
					source.byteReceived(getReg(spiAddress, spiExtendedMode));
				} else {
					source.byteReceived(setReg(spiAddress, data,
							spiExtendedMode));
				}

				if (spiAddress < CC1120_TXFIFO) {
					spiAddress++;
				}
			}
			return;
		}

		if (spiAwaitingAddressExtended) {
			spiAddress = data;
			spiGotAddress = true;
			spiAwaitingAddressExtended = false;
			spiExtendedMode = true;

			/* Return MARCSTATE */
			source.byteReceived(getMarcstate());
			return;
		}

		/* Read/write. Burst/single. Extended/normal. */
		spiBurstMode = spiIsBurst(data);
		spiReadMode = spiIsRead(data);
		spiExtendedMode = spiIsExtended(data);
		spiAwaitingAddressExtended = spiIsExtended(data);

		/* Is this a strobe command */
		if (!spiBurstMode && !spiAwaitingAddressExtended && spiIsStrobe(data)) {
			/* Strobe command */
			strobe(data);
			source.byteReceived(0);
			spiResetState();
			return;
		}

		if (!spiAwaitingAddressExtended) {
			spiAddress = data & 0x3f;
			spiGotAddress = true;
		}

		/* Return MARCSTATE */
		source.byteReceived(getMarcstate());
		return;
	}
	int setReg(int address, int data, boolean extended) {
		switch (address) {
		case CC1120_TXFIFO:
			txfifo.add((byte) data);
			/* printTXFIFO(); */
			return txfifo.size();
		case CC1120_FREQ2: /* XXX This is probably wrong, extended right? */
			nextFreq2 = data;
			return 0;
		case CC1120_FREQ1:
			nextFreq1 = data;
			return 0;
		case CC1120_FREQ0:
			nextFreq0 = data;
			return 0;
		}

		System.out.println(String.format("setReg(0x%02x, %s) 0x%02x", address,
				extended ? "extended" : "normal", data));
		if (extended) {
			int oldValue = extendedRegisters[address];
			extendedRegisters[address] = data;
			configurationChanged(address, oldValue, data);
			return oldValue;
		}
		int oldValue = registers[address];
		registers[address] = data;
		configurationChanged(address, oldValue, data);
		return oldValue;
	}
	int getReg(int address, boolean extended) {
		/* MSP430Core.profiler.printStackTrace(System.out); */
		if (extended) {
			switch (address) {
			case CC1120_MARCSTATE:
				return getMarcstate();
			case CC1120_NUM_RXBYTES:
				return rxfifo.size();
			case CC1120_NUM_TXBYTES:
				return txfifo.size();
			case CC1120_RSSI1:
				return currentRssiReg1;
			case CC1120_RSSI0:
			  int ret = 0;
        ret += (0b1111 & currentRssiReg0); /* RSSI_3_0 */

        ret = ret << 1;
        if(getRSSI() < CCA_THRESHOLD) {
          ret += 1; /* Carrier detected */
        } else {
          ret += 0; /* No carrier detected */
        }

        ret = ret << 1;
        ret += 1; /* TODO XXX Carrier sense is always valid */

        ret = ret << 1;
        ret += 1; /* TODO XXX RSSI is always valid */
        
        return ret;
			}
		}

		switch (address) {
		case CC1120_RXFIFO:
			if (rxfifo.size() > 0) {
				int ret = (int) rxfifo.remove(0);
				/* printRXFIFO(); */
				return ret;
			}
			System.err.println("Warning: reading from empty RXFIFO");
			return -1;
		}

		if (address != CC1120.CC1120_MARCSTATE) {
			System.out
			.println(String.format("getReg(0x%02x, %s) 0x%02x",
					address, extended ? "extended" : "normal",
							extended ? extendedRegisters[address]
									: registers[address]));
		}

		if (extended) {
			return extendedRegisters[address];
		}
		return registers[address];
	}

	public boolean isReadyToReceive() {
	  /* TODO Implement me */
    if (getState() == CC1120RadioState.CC1120_STATE_IDLE) {
      return false;
    }
    if (getState() == CC1120RadioState.CC1120_STATE_SLEEP) {
      return false;
    }
	  return true;
	}


	/* txFooterCountdown: send CRC footer in these many bytes */
	protected int txFooterCountdown = -1;

	/* TX/RX states */
	public static final int NUM_PREAMBLE = 4;
	public static final int NUM_SYNCH = 4;
	public static final byte SYNCH_BYTE_LAST = (byte) 0xDE;
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
			return;
		}

		if (rfListener != null) {
			rfListener.receivedByte((byte) (txfifo.get(0).intValue()));
		}
		txfifo.remove(0);
		cpu.scheduleTimeEventMillis(sendEvent, SYMBOL_PERIOD * 2);

		/* printTXFIFO(); */
	}

	public void setGDO0(IOPort port, int pin) {
		gdo0Port = port;
		gdo0Pin = pin;
	}
	public void setGDO0(boolean active) {
		/* TODO Read active low/high configuration */

		if (active) {
			gdo0Port.setPinState(gdo0Pin, IOPort.PinState.HI);
		} else {
			gdo0Port.setPinState(gdo0Pin, IOPort.PinState.LOW);
		}
	}

	public void setGDO2(IOPort port, int pin) {
		gdo2Port = port;
		gdo2Pin = pin;
	}
	public void setGDO2(boolean active) {
		gdo2Port.setPinState(gdo2Pin, active ? IOPort.PinState.LOW
				: IOPort.PinState.HI);
	}

	private void printRXFIFO() {
		System.out.print(String.format("RXFIFO[%03d]: ", rxfifo.size()));
		for (int i = 0; i < rxfifo.size(); i++) {
			System.out.print(String.format("%02x", rxfifo.get(i)));
		}
		System.out.println();
	}
	private void printTXFIFO() {
		System.out.print(String.format("TXFIFO[%03d]: ", txfifo.size()));
		for (int i = 0; i < txfifo.size(); i++) {
			System.out.print(String.format("%02x", txfifo.get(i)));
		}
		System.out.println();
	}

	public CC1120RadioState getState() {
		return state;
	}
	public interface StateListener {
		public void newState(CC1120RadioState state);
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
	boolean setState(CC1120RadioState newState) {
		if (newState != CC1120RadioState.CC1120_STATE_IDLE
				&& newState != CC1120RadioState.CC1120_STATE_RX
				&& newState != CC1120RadioState.CC1120_STATE_TX) {
			System.out.println("setState(" + newState + ")");
		}

		if (changeFrequencyNextState
				&& newState == CC1120RadioState.CC1120_STATE_RX
				|| newState == CC1120RadioState.CC1120_STATE_TX) {
			changeFrequencyNextState = false;
			frequency = ((0xff & nextFreq0) << 0) + ((0xff & nextFreq1) << 8)
					+ ((0xff & nextFreq2) << 16);

			frequency *= 32; /* frequency oscillator */
			frequency /= 65536;
			frequency /= 4; /* LO divider */

			/* System.out.println("Activating new frequency: " + frequency + " MHz"); */
			if (channelListener != null) {
				channelListener.channelChanged((int) Math
						.round((frequency - FREQUENCY_CHANNEL_0)
								/ FREQUENCY_CHANNEL_WIDTH));
			}
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
		/* printRXFIFO(); */
	}

	public String info() {
		return "CC1120 info: [not implemented]";
	}

	public int getRegister(int register) {
		return registers[register];
	}

	public void setRegister(int register, int data) {
		registers[register] = data;
	}

  public void setRSSI(int power) {
    currentRssiReg1 = 0xff&power;
  }
  public int getRSSI() {
    return currentRssiReg1;
  }

	public int getActiveFrequency() {
		return (int) (1000 * Math.round(frequency)); /* kHz */
	}

	public int getActiveChannel() {
		return (int) Math.round((frequency - FREQUENCY_CHANNEL_0)
				/ FREQUENCY_CHANNEL_WIDTH);
	}

	public int getOutputPowerIndicator() {
		return 1;
	}

	public double getFrequency() {
		return frequency;
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
			log("setting chipSelect: " + chipSelect);
		}
	}

	void setStateRX() {
		setState(CC1120RadioState.CC1120_STATE_RX);
	}

	void reset() {
		setState(CC1120RadioState.CC1120_STATE_IDLE);
	}

	int getMarcstate() {
		return getState().getStateAsInt();
	}

}
