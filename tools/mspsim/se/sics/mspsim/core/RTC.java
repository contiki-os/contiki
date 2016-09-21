/* Copyright (c) 2013, tado° GmbH. Munich, Germany.
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
 *
 * This file is part of MSPSim.
 * 
 * Author: Víctor Ariño <victor.arino@tado.com>
 * 
 */
package se.sics.mspsim.core;

import java.util.Calendar;
import java.util.GregorianCalendar;

import se.sics.mspsim.core.EmulationLogger.WarningType;


/**
 * RTC module for the MSP430
 * 
 * TODO: alarm mode not implemented
 * 
 * @author Víctor Ariño <victor.arino@tado.com>
 */
public class RTC extends IOUnit {

	/**
	 * Address and size for IO configuration
	 */
	public static final int OFFSET = 0x04A0;
	public static final int SIZE = 0x0001A;

	/**
	 * Interrupt vector
	 */
	public static final int RTC_VECTOR = 0x005C;

	/* Peripheral registers */
	private static final int RTCCTL01 = 0x0000;
	private static final int RTCCTL23 = 0x0002;
	private static final int RTCPS0CTL = 0x0008;
	private static final int RTCPS1CTL = 0x000A;
	private static final int RTCPS = 0x000C;
	private static final int RTCIV = 0x000E;
	private static final int RTCTIM0 = 0x0010;
	private static final int RTCTIM1 = 0x0012;
	private static final int RTCDATE = 0x0014;
	private static final int RTCYEAR = 0x0016;
	private static final int RTCAMINHR = 0x0018;
	private static final int RTCADOWDAY = 0x001A;

	/* RTCCTL01 Control Bits */
	private static final int RTCBCD = 0x8000;
	private static final int RTCHOLD = 0x4000;
	private static final int RTCMODE = 0x2000;
	private static final int RTCRDY = 0x1000;
	private static final int RTCTEVIE = 0x0040;
	private static final int RTCAIE = 0x0020;
	private static final int RTCRDYIE = 0x0010;
	private static final int RTCTEVIFG = 0x0004;
	private static final int RTCAIFG = 0x0002;
	private static final int RTCRDYIFG = 0x0001;

	private static final int RTCSSEL__ACLK = 0x0000;
	private static final int RTCSSEL__SMCLK = 0x0400;
	private static final int RTCSSEL__RT1PS = 0x0800;

	private static final int RTCOFIE = (1 << 7);
	private static final int RTCOFIFG = (1 << 3);
	private static final int RTCEVIFG = (1 << 2);

	/* Possible types of RTC */
	public static enum RtcType {
		TYPE_A, TYPE_D,
	}

	private RtcType type = RtcType.TYPE_A;
	private int rtcIntVector = RTC_VECTOR;

	/**
	 * RTC peripheral for the MSP430
	 * 
	 * @param cpu
	 *           CPU core
	 */
	public RTC(MSP430Core cpu, int offset, RtcType r, int intVector) {
		super("RTC", cpu, cpu.memory, offset);
		type = r;
		rtcIntVector = intVector;
		DEBUG = true;
		cal.setLenient(true);
	}

	/**
	 * Clear everything when reset
	 */
	public void reset(int type) {
		oscFaultInterruptEnable = false;
		eventInterruptEnable = false;
		alarmInterruptEnable = false;
		readyInterruptEnable = false;
		oscFaultInterruptFlag = false;
		eventInterruptFlag = false;
		alarmInterruptFlag = false;
		readyInterruptFlag = false;
	}

	/* RTCCTL0 */
	private boolean oscFaultInterruptEnable = false;
	private boolean eventInterruptEnable = false;
	private boolean alarmInterruptEnable = false;
	private boolean readyInterruptEnable = false;
	private boolean oscFaultInterruptFlag = false;
	private boolean eventInterruptFlag = false;
	private boolean alarmInterruptFlag = false;
	private boolean readyInterruptFlag = false;

	/**
	 * Get the CTLO (byte) register
	 * 
	 * @return
	 */
	private short getCTL0Reg() {
		short ctl0 = 0;
		ctl0 |= (oscFaultInterruptEnable ? 1 : 0) << 7;
		ctl0 |= (eventInterruptEnable ? 1 : 0) << 6;
		ctl0 |= (alarmInterruptEnable ? 1 : 0) << 5;
		ctl0 |= (readyInterruptEnable ? 1 : 0) << 4;
		ctl0 |= (oscFaultInterruptFlag ? 1 : 0) << 3;
		ctl0 |= (eventInterruptFlag ? 1 : 0) << 2;
		ctl0 |= (alarmInterruptFlag ? 1 : 0) << 1;
		ctl0 |= (readyInterruptFlag ? 1 : 0) << 0;
		return ctl0;
	}

	/**
	 * Get the CTL1 (byte) register
	 * 
	 * @return
	 */
	private short getCTL1Reg() {
		short ctl1 = 0;
		ctl1 |= (formatBCD ? 1 : 0) << 7;
		ctl1 |= (rtcHold ? 1 : 0) << 6;
		ctl1 |= (modeCalendar ? 1 : 0) << 5;
		ctl1 |= (rtcReady ? 1 : 0) << 4;
		ctl1 |= (clockSource & 0x3) << 2;
		ctl1 |= (rtcEvent & 0x3);
		return ctl1;
	}

	/**
	 * Get the CTL01 (word) register
	 * 
	 * @return
	 */
	private int getCTL01Reg() {
		return getCTL1Reg() << 8 | getCTL0Reg();
	}

	/* RTCCTL1 */
	private boolean formatBCD = false;
	private boolean rtcHold = true;
	private boolean modeCalendar = true;
	private boolean rtcReady = true;
	private int clockSource = 0;
	private int rtcEvent = 0;

	/**
	 * Handler of the 4 (byte) counter registers, for simplicity they can be
	 * collected in a single 32-bit variable.
	 */
	private long rtcCount = 0;

	/**
	 * Period to increment the counters
	 */
	private double period = osc32KHzMs;
	private static final float osc32KHzMs = 0.031f;

	private int preScaler0Src = 0;
	private int preScaler0Div = 0;
	private boolean preScaler0Hold = false;

	private int preScaler1Src = 0;
	private int preScaler1Div = 0;
	private boolean preScaler1Hold = false;

	private static final int RT0SSEL = 0x4000;
	private static final int RT0PSHOLD = 0x0100;
	private static final int RT0PSIE = 0x0002;
	private static final int RT0PSIFG = 0x0001;
	private static final int RT0IP = 0x001C;

	private static final int RT1SSEL = 0xC000;
	private static final int RT1PSHOLD = 0x0100;
	private static final int RT1PSIE = 0x0002;
	private static final int RT1PSIFG = 0x0001;
	private static final int RT1IP = 0x001C;

	private int getPS0CTL() {
		int ctl0 = 0;
		ctl0 |= (preScaler0Src & 0x01) << 14;
		ctl0 |= (preScaler0Div & 0x03) << 11;
		ctl0 |= (preScaler0Hold ? 1 : 0) << 8;
		return ctl0;
	}

	private int getPS1CTL() {
		int ctl1 = 0;
		ctl1 |= (preScaler1Src & 0x03) << 14;
		ctl1 |= (preScaler1Div & 0x03) << 11;
		ctl1 |= (preScaler1Hold ? 1 : 0) << 8;
		return ctl1;
	}

	/**
	 * In calendar mode, use a gregorian calendar
	 */
	private GregorianCalendar cal = new GregorianCalendar(0, 1, 1, 0, 0, 0);

	/**
	 * Timer to generate the interrupts and handle the calendar
	 */
	private TimeEvent rtcTimer = new TimeEvent(0) {

		public void execute(long t) {
			if (!rtcHold) {
				updateCounters();
				reSchedule();
			}
			if (getIV() > 0) {

				cpu.flagInterrupt(rtcIntVector, RTC.this, true);
			}
		}

		/**
		 * Re-Schedule the timer event
		 */
		private void reSchedule() {
			// cpu.scheduleTimeEventMillis(this, period);
			rtcInit();
		}

		/**
		 * Update the counters or the calendar, depending on the mode
		 */
		private void updateCounters() {
			if (modeCalendar) {
				/* In calendar mode it is updated every second */
				cal.add(Calendar.SECOND, 1);

				/* Track the changes */
				int sec = cal.get(Calendar.SECOND);
				int min = cal.get(Calendar.MINUTE);
				int hour = cal.get(Calendar.HOUR_OF_DAY);

				if (sec == 0) {
					if (rtcEvent == 0) { // minute
						generateInterrupt();
					} else if (rtcEvent == 1 && min == 0) { // hour
						generateInterrupt();
					} else if (rtcEvent == 2 && hour == 0 && min == 0) { // day
						generateInterrupt();
					} else if (rtcEvent == 3 && hour == 12 && min == 0) { // day
						generateInterrupt();
					}
				}

				/*
				 * Generate interrupts when time successfully increased and there's
				 * no other interrupt
				 */
				if (readyInterruptEnable && modeCalendar) {
					readyInterruptFlag = true;
				}

			} else {
				rtcCount += 1;
				long overflow = ((1L << ((rtcEvent + 1) * 8)));
				overflow -= 1;
				if ((rtcCount & overflow) == 0) {
					generateInterrupt();
				}
				if (rtcCount >= (1L << 32)) {
					rtcCount = 0;
				}
			}
		}

		/**
		 * Trigger a microprocessor interrupt
		 */
		private void generateInterrupt() {
			eventInterruptFlag = true;
		}

	};

	private double getPreScalerFreq() {
		double freqSrc = 0;
		if (preScaler1Src > 1) {
			/* From preScaler 0 */
			if (preScaler0Src == 0) {
				freqSrc = cpu.getAclkFrq();
			} else {
				freqSrc = cpu.getSmclkFrq();
			}
			int div = (0x2 << preScaler0Div);
			freqSrc /= div;
		} else if (preScaler1Src == 0) {
			freqSrc = cpu.getAclkFrq();
		} else {
			freqSrc = cpu.getSmclkFrq();
		}
		int div = (0x2 << preScaler1Div);
		freqSrc /= div;
		return freqSrc;
	}

	/**
	 * Initialize the RTC clock. This is called when the hold bit is released
	 */
	private void rtcInit() {
		double freqSrc = 1f;
		if (modeCalendar) {
			/* In this mode we can set the counter to 1 second */
			freqSrc = 1f; // 1Hz = ~1s
		} else {
			if (type == RtcType.TYPE_A) {
				/* For the RTC_A the sources are aclk, smclk, rt1ps */
				if (clockSource == 0) {
					freqSrc = cpu.getAclkFrq();
				} else if (clockSource == 1) {
					freqSrc = cpu.getSmclkFrq();
				} else {
					freqSrc = getPreScalerFreq();
				}
			} else {
				/* For the RTC_D the sources are 32-khz, rt1ps */
				if (clockSource <= 1) {
					freqSrc = 32000;
				} else {
					freqSrc = getPreScalerFreq();
				}
			}
		}
		period = 1000f / freqSrc;
		cpu.scheduleTimeEventMillis(rtcTimer, period);
	}

	/**
	 * Reset the calendar to 01.01.0000
	 */
	private void resetCalendar() {
		cal = new GregorianCalendar(0, 1, 1, 0, 0, 0);
	}

	/**
	 * Get the interrupt vector
	 * 
	 * @return
	 */
	private int getIV() {
		int iv = 0;
		iv |= (readyInterruptFlag ? 1 : 0) << 1;
		iv |= (eventInterruptFlag ? 1 : 0) << 2;
		iv |= (alarmInterruptFlag ? 1 : 0) << 3;
		return iv;
	}

	/**
	 * Given a calendar field, get the BCD or hex representation
	 * 
	 * @param calField
	 *           Calendar.XXXX field
	 * @return the formated field
	 */
	private int formatField(int calField) {
		int f = cal.get(calField);
		/* Day of week is 0-6 in uC and 1-7 in Java */
		if (calField == Calendar.DAY_OF_WEEK) {
			f -= 1;
		}
		int res = 0;
		int base = 0;
		if (formatBCD) {
			base = 10;
		} else {
			base = 16;
		}
		for (int i = 0; f > 0; i++) {
			int v = f % base;
			f = f / base;
			res |= (v << i * 4);
		}
		return res;
	}

	/**
	 * Parses a calendar input
	 * 
	 * This processes the registers written in BCD or hex format
	 * 
	 * @param calField
	 *           the calendar field to parse
	 * @param value
	 *           the input value
	 */
	private void parseCalReg(int calField, int value) {
		int res = 0;
		int factor = 1;
		int base = 0;
		if (formatBCD) {
			base = 10;
		} else {
			base = 16;
		}

		/* Day of week is 0-6 in uC and 1-7 in Java */
		if (calField == Calendar.DAY_OF_WEEK) {
			value += 1;
		}

		while (value > 0) {
			int v = value & 0x0f;
			res += (v * factor);
			value >>= 4;
			factor *= base;
		}
		cal.set(calField, res);
		cal.get(calField); // Until get is not done the fields are not set
	}

	/**
	 * Clear the highest priority interrupt flag
	 */
	private void clearHighestInterrupt() {
		if (readyInterruptFlag) {
			readyInterruptFlag = false;
		} else if (eventInterruptFlag) {
			eventInterruptFlag = false;
		} else if (alarmInterruptFlag) {
			alarmInterruptFlag = false;
		}

		if (getIV() > 0) {
			cpu.flagInterrupt(rtcIntVector, this, true);
		}
	}

	/**
	 * The registers are written
	 */
	public void write(int address, int value, boolean word, long cycles) {
		/*
		 * XXX: this assumes always word access
		 */
		if (!word) {
			logw(WarningType.MISALIGNED_WRITE, "byte access not implemented");
		}

		int lo = (value) & 0xff; // low byte
		int hi = (value >> 8) & 0xff; // high byte

		switch (address - offset) {
		case RTCCTL01:
			oscFaultInterruptEnable = ((value & RTCOFIE) == RTCOFIE);
			eventInterruptEnable = ((value & RTCTEVIE) == RTCTEVIE);
			alarmInterruptEnable = ((value & RTCAIE) == RTCAIE);
			readyInterruptEnable = ((value & RTCRDYIE) == RTCRDYIE);
			oscFaultInterruptFlag = ((value & RTCOFIFG) == RTCOFIFG);
			eventInterruptFlag = ((value & RTCEVIFG) == RTCEVIFG);
			alarmInterruptFlag = ((value & RTCAIFG) == RTCAIFG);
			readyInterruptFlag = ((value & RTCRDYIFG) == RTCRDYIFG);

			rtcHold = ((value & RTCHOLD) == RTCHOLD);
			rtcReady = ((value & RTCRDY) == RTCRDY);
			clockSource = (value & 0x0C00) >> 10;
			rtcEvent = (value & 0x0300) >> 8;

			/*
			 * Either the BCD has changed and is in mode calendar or the mode
			 * calendar has been enabled
			 */
			if ((modeCalendar && formatBCD != ((value & RTCBCD) == RTCBCD))
					|| (modeCalendar == false && (value & RTCMODE) == RTCMODE)) {
				/*
				 * Changing this bit clears seconds, minutes, hours, day of week,
				 * and year to 0 and sets day of month and month to 1. The real-time
				 * clock registers must be set by software afterwards.
				 */
				resetCalendar();
			}
			modeCalendar = ((value & RTCMODE) == RTCMODE);
			formatBCD = ((value & RTCBCD) == RTCBCD);

			/* Initialize the RTC or stop it */
			if (!rtcHold) {
				rtcInit();
			}

			break;

		case RTCPS0CTL:
			preScaler0Src = ((value & 0x4000) >> 14);
			preScaler0Div = ((value & 0x3800) >> 11);
			preScaler0Hold = ((value & RT0PSHOLD) == RT0PSHOLD);

			if (((value & (RT0PSIE | RT0PSIFG | RT0IP)) > 0)) {
				logNotImplemented("prescaling interrupts");
			}
			break;

		case RTCPS1CTL:
			preScaler1Src = ((value & 0xC000) >> 14);
			preScaler1Div = ((value & 0x3800) >> 11);
			preScaler1Hold = ((value & RT1PSHOLD) == RT1PSHOLD);

			if (((value & (RT1PSIE | RT1PSIFG | RT1IP)) > 0)) {
				logNotImplemented("prescaling interrupts");
			}
			break;

		case RTCCTL23:
			logNotImplemented("Calibration");
			break;

		case RTCPS:
			logNotImplemented("prescaling counter");
			break;

		case RTCTIM0: // RTCNT12
			if (modeCalendar) {
				parseCalReg(Calendar.SECOND, lo);
				parseCalReg(Calendar.MINUTE, hi);
			} else {
				rtcCount &= 0xffff0000;
				rtcCount |= value;
			}
			break;

		case RTCTIM1: // RTCNT34
			if (modeCalendar) {
				parseCalReg(Calendar.HOUR_OF_DAY, lo);
				parseCalReg(Calendar.DAY_OF_WEEK, hi);
			} else {
				rtcCount &= 0x0000ffff;
				rtcCount |= (long) (value << 16);
			}
			break;

		case RTCDATE:
			if (modeCalendar) {
				parseCalReg(Calendar.DAY_OF_MONTH, lo);
				parseCalReg(Calendar.MONTH, hi);
			}
			break;

		case RTCYEAR:
			if (modeCalendar) {
				parseCalReg(Calendar.YEAR, value);
			}
			break;

		case RTCIV:
			/*
			 * Any access, read or write, of the RTCIV register automatically
			 * resets the highest-pending interrupt flag. If another interrupt flag
			 * is set, another interrupt is immediately generated after servicing
			 * the initial interrupt. In addition, all flags can be cleared via
			 * software.
			 */
			clearHighestInterrupt();

		default:
			logNotImplemented("register: " + address);
		}
	}

	/**
	 * Registers are read
	 */
	public int read(int address, boolean word, long cycles) {

		if (!word) {
			logw(WarningType.MISALIGNED_READ, "byte access not implemented");
		}

		switch (address - offset) {
		case RTCCTL01:
			if (word) {
				return getCTL01Reg();
			} else {
				return getCTL0Reg();
			}

		case RTCCTL23:
			logNotImplemented("calibration");
			break;

		case RTCPS:
			logNotImplemented("prescaling coutner");
			break;

		case RTCPS0CTL:
			return getPS0CTL();

		case RTCPS1CTL:
			return getPS1CTL();

		case RTCIV:
			/*
			 * Any access, read or write, of the RTCIV register automatically
			 * resets the highest-pending interrupt flag. If another interrupt flag
			 * is set, another interrupt is immediately generated after servicing
			 * the initial interrupt. In addition, all flags can be cleared via
			 * software.
			 */
			int tmp = getIV();
			clearHighestInterrupt();
			return tmp;

		case RTCTIM0: // RTCNT12
			if (modeCalendar) {
				return formatField(Calendar.MINUTE) << 8
						| formatField(Calendar.SECOND);
			} else {
				return (int) (rtcCount & 0xffff);
			}

		case RTCTIM1: // RTCNT34
			if (modeCalendar) {
				return formatField(Calendar.DAY_OF_WEEK) << 8
						| formatField(Calendar.HOUR_OF_DAY);
			} else {
				return (int) ((rtcCount >> 16) & 0xffff);
			}

		case RTCDATE:
			if (modeCalendar) {
				return formatField(Calendar.MONTH) << 8
						| formatField(Calendar.DAY_OF_MONTH);
			}
			break;

		case RTCYEAR:
			if (modeCalendar) {
				return formatField(Calendar.YEAR);
			}
			break;

		default:
			logNotImplemented("register: " + address);
		}
		return 0;
	}

	public void interruptServiced(int vector) {
		/*
		 * NOTE this function is called BEFORE the interrupt is actually
		 * serviced!!!
		 */
		if (vector == rtcIntVector) {
			cpu.flagInterrupt(rtcIntVector, this, false);
		}
	}

	/*
	 * The inherited log function is not working for whatever reason. A quick
	 * redefinition helps a lot while developing the module
	 */
	@Override
	protected void log(String msg) {
		if (DEBUG) {
			System.out.println(msg);
		}
	}

	/**
	 * Log using printf format
	 * 
	 * @param format
	 * @param arguments
	 */
	protected void log(final String format, final Object... arguments) {
		if (DEBUG) {
			System.out.printf(format, arguments);
		}
	}

	private void logNotImplemented(String feature) {
		logw(WarningType.EMULATION_ERROR, feature + " is not implemented");
	}

}
