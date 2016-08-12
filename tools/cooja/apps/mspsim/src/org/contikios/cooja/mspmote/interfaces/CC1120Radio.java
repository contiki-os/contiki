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
 *
 */

package org.contikios.cooja.mspmote.interfaces;

import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.RadioPacket;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.CustomDataRadio;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.mspmote.MspMote;
import org.contikios.cooja.mspmote.MspMoteTimeEvent;
import se.sics.mspsim.chip.CC1120;
import se.sics.mspsim.chip.CC1120.ReceiverListener;
import se.sics.mspsim.chip.ChannelListener;
import se.sics.mspsim.chip.RFListener;
import se.sics.mspsim.chip.Radio802154;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("TI CC1120")
public class CC1120Radio extends Radio implements CustomDataRadio {
	private static Logger logger = Logger.getLogger(CC1120Radio.class);

	/**
	 * Cross-level:
	 * Inter-byte delay for delivering cross-level packet bytes.
	 */
	/* TODO XXX Fix me as well as symbol duration in CC1120.java */
	public static final long DELAY_BETWEEN_BYTES =
			(long) (1000.0*Simulation.MILLISECOND/(200000.0/8.0)); /* us. Corresponds to 200kbit/s */

	private RadioEvent lastEvent = RadioEvent.UNKNOWN;

	private final MspMote mote;
	private final CC1120 cc1120;

	private boolean isInterfered = false;
	private boolean isTransmitting = false;
	private boolean isReceiving = false;

	private byte lastOutgoingByte;
	private byte lastIncomingByte;

	private RadioPacket lastOutgoingPacket = null;
	private RadioPacket lastIncomingPacket = null;

	public CC1120Radio(Mote m) {
		this.mote = (MspMote)m;
		Radio802154 r = this.mote.getCPU().getChip(Radio802154.class);
		if (r == null || !(r instanceof CC1120)) {
			throw new IllegalStateException("Mote is not equipped with an CC1120 radio");
		}
		this.cc1120 = (CC1120) r;

		cc1120.addRFListener(new RFListener() {
			int len = 0;
			int expLen = 0;
			byte[] buffer = new byte[256 + 15];
			private boolean gotSynchbyte = false;
			public void receivedByte(byte data) {
				if (!isTransmitting()) {
					/* Start transmission */
					lastEvent = RadioEvent.TRANSMISSION_STARTED;
					isTransmitting = true;
					len = 0;
					gotSynchbyte = false;
					/*logger.debug("----- CCC1120 TRANSMISSION STARTED -----");*/
					setChanged();
					notifyObservers();
				}
				if (len >= buffer.length) {
					/* Bad size packet, too large */
					logger.debug("Error: bad size: " + len + ", dropping outgoing byte: " + data);
					return;
				}

				/* send this byte to all nodes */
				lastOutgoingByte = data;
				lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
				setChanged();
				notifyObservers();

				/* Await synch byte */
				if (!gotSynchbyte) {
					if (lastOutgoingByte == CC1120.SYNCH_BYTE_LAST) {
						gotSynchbyte = true;
					}
					return;
				}

				final int HEADERLEN = 1; /* 1x Length byte */
				final int FOOTERLEN = 2; /* TODO Fix CRC in Mspsim's CCC1120.java */
				if (len == 0) {
					expLen = (0xff&data) + HEADERLEN + FOOTERLEN;
				}
				buffer[len++] = data;

				if (len == expLen) {
					/*logger.debug("----- CCC1120 CUSTOM DATA TRANSMITTED -----");*/

					final byte[] buf = new byte[expLen];
					System.arraycopy(buffer, 0, buf, 0, expLen);
					lastOutgoingPacket = new RadioPacket() {
						public byte[] getPacketData() {
							return buf;
						}
					};

					lastEvent = RadioEvent.PACKET_TRANSMITTED;
					/*logger.debug("----- CCC1120 PACKET TRANSMITTED -----");*/
					setChanged();
					notifyObservers();

					/*logger.debug("----- CCC1120 TRANSMISSION FINISHED -----");*/
					isTransmitting = false;
					lastEvent = RadioEvent.TRANSMISSION_FINISHED;
					setChanged();
					notifyObservers();
					len = 0;
				}
			}
		});

		cc1120.setReceiverListener(new ReceiverListener() {
		  public void newState(boolean on) {
		    if (cc1120.isReadyToReceive()) {
		      lastEvent = RadioEvent.HW_ON;
		      setChanged();
		      notifyObservers();
		    } else {
		      radioOff();
		    }
		  }
		});

		cc1120.addChannelListener(new ChannelListener() {
			public void channelChanged(int channel) {
				/* XXX Currently assumes zero channel switch time */
				lastEvent = RadioEvent.UNKNOWN;
				setChanged();
				notifyObservers();
			}
		});
	}

	private void radioOff() {
		/* Radio was turned off during transmission.
		 * May for example happen if watchdog triggers */
		if (isTransmitting()) {
			logger.warn("Turning off radio while transmitting, ending packet prematurely");

			/* Simulate end of packet */
			lastOutgoingPacket = new RadioPacket() {
				public byte[] getPacketData() {
					return new byte[0];
				}
			};

			lastEvent = RadioEvent.PACKET_TRANSMITTED;
			/*logger.debug("----- CCC1120 PACKET TRANSMITTED -----");*/
			setChanged();
			notifyObservers();

			/* Register that transmission ended in radio medium */
			/*logger.debug("----- CCC1120 TRANSMISSION FINISHED -----");*/
			isTransmitting = false;
			lastEvent = RadioEvent.TRANSMISSION_FINISHED;
			setChanged();
			notifyObservers();
		}

		lastEvent = RadioEvent.HW_OFF;
		setChanged();
		notifyObservers();
	}

	/* Packet radio support */
	public RadioPacket getLastPacketTransmitted() {
		return lastOutgoingPacket;
	}

	public RadioPacket getLastPacketReceived() {
		return lastIncomingPacket;
	}

	public void setReceivedPacket(RadioPacket packet) {
		lastIncomingPacket = packet;

		/* TODO XXX Need support in CCC1120.java */
		/*if (!radio.isReadyToReceive()) {
			logger.warn("Radio receiver not ready, dropping packet data");
			return;
		}*/

		/* Delivering packet bytes with delays */
		byte[] packetData = packet.getPacketData();
		long deliveryTime = getMote().getSimulation().getSimulationTime();
		for (byte b: packetData) {
			if (isInterfered()) {
				b = (byte) 0xFF;
			}

			final byte byteToDeliver = b;
			getMote().getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
				public void execute(long t) {
					super.execute(t);
					cc1120.receivedByte(byteToDeliver);
					mote.requestImmediateWakeup();
				}
			}, deliveryTime);
			deliveryTime += DELAY_BETWEEN_BYTES;
		}
	}

	/* Custom data radio support */
	public Object getLastCustomDataTransmitted() {
		return lastOutgoingByte;
	}

	public Object getLastCustomDataReceived() {
		return lastIncomingByte;
	}

	public void receiveCustomData(Object data) {
		if (!(data instanceof Byte)) {
			logger.fatal("Bad custom data: " + data);
			return;
		}
		lastIncomingByte = (Byte) data;

		final byte inputByte;
		if (isInterfered()) {
			inputByte = (byte)0xFF;
		} else {
			inputByte = lastIncomingByte;
		}
		mote.getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
			public void execute(long t) {
				super.execute(t);
				cc1120.receivedByte(inputByte);
				mote.requestImmediateWakeup();
			}
		}, mote.getSimulation().getSimulationTime());

	}

	/* General radio support */
	public boolean isTransmitting() {
		return isTransmitting;
	}

	public boolean isReceiving() {
		return isReceiving;
	}

	public boolean isInterfered() {
		return isInterfered;
	}

	public int getChannel() {
		return cc1120.getActiveChannel()+1000;
	}

	public int getFrequency() {
		return cc1120.getActiveFrequency();
	}

	public void signalReceptionStart() {
		isReceiving = true;

		lastEvent = RadioEvent.RECEPTION_STARTED;
		/*logger.debug("----- CCC1120 RECEPTION STARTED -----");*/
		setChanged();
		notifyObservers();
	}

	public void signalReceptionEnd() {
		/* Deliver packet data */
		isReceiving = false;
		isInterfered = false;

		lastEvent = RadioEvent.RECEPTION_FINISHED;
		/*logger.debug("----- CCC1120 RECEPTION FINISHED -----");*/
		setChanged();
		notifyObservers();
	}

	public RadioEvent getLastEvent() {
		return lastEvent;
	}

	public void interfereAnyReception() {
		isInterfered = true;
		isReceiving = false;
		lastIncomingPacket = null;

		lastEvent = RadioEvent.RECEPTION_INTERFERED;
		/*logger.debug("----- CCC1120 RECEPTION INTERFERED -----");*/
		setChanged();
		notifyObservers();
	}

	public double getCurrentOutputPower() {
		/* TODO XXX Need support in CCC1120.java */
		return 1;
	}
	public int getCurrentOutputPowerIndicator() {
		/* TODO XXX Need support in CCC1120.java */
		return 10;
	}
	public int getOutputPowerIndicatorMax() {
		/* TODO XXX Need support in CCC1120.java */
		return 10;
	}


	/**
	 * Last 8 received signal strengths
	 */
	double currentSignalStrength = 0;
	private double[] rssiLast = new double[8];
	private int rssiLastCounter = 0;

	public double getCurrentSignalStrength() {
		return currentSignalStrength;
	}

	public void setCurrentSignalStrength(final double signalStrength) {
		if (signalStrength == currentSignalStrength) {
			return; /* ignored */
		}
		currentSignalStrength = signalStrength;
		if (rssiLastCounter == 0) {
			getMote().getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
				public void execute(long t) {
					super.execute(t);

					/* Update average */
					System.arraycopy(rssiLast, 1, rssiLast, 0, 7);
					rssiLast[7] = currentSignalStrength;
					double avg = 0;
					for (double v: rssiLast) {
						avg += v;
					}
					avg /= rssiLast.length;

					cc1120.setRSSI((int) avg);

					rssiLastCounter--;
					if (rssiLastCounter > 0) {
						mote.getSimulation().scheduleEvent(this, t+DELAY_BETWEEN_BYTES/2);
					}
				}
			}, mote.getSimulation().getSimulationTime());
		}
		rssiLastCounter = 8;
	}

	public Mote getMote() {
		return mote;
	}

	public Position getPosition() {
		return mote.getInterfaces().getPosition();
	}

	public Collection<Element> getConfigXML() {
		return null;
	}

	public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
	}

	public boolean isRadioOn() {
	    return cc1120.isReadyToReceive();
	}

  public boolean canReceiveFrom(CustomDataRadio radio) {
    if (radio.getClass().equals(this.getClass())) {
      return true;
    }
    return false;
  }

}
