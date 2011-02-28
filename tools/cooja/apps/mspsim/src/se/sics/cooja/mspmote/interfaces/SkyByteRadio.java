/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: SkyByteRadio.java,v 1.27 2010/09/09 19:52:26 nifi Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.CustomDataRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteTimeEvent;
import se.sics.mspsim.chip.CC2420;
import se.sics.mspsim.chip.RFListener;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.OperatingModeListener;

/**
 * CC2420 to COOJA wrapper.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("CC2420")
public class SkyByteRadio extends Radio implements CustomDataRadio {
  private static Logger logger = Logger.getLogger(SkyByteRadio.class);

  /**
   * Cross-level:
   * Inter-byte delay for delivering cross-level packet bytes.
   */
  public static final long DELAY_BETWEEN_BYTES = 
    (long) (1000.0*Simulation.MILLISECOND/(250000.0/8.0)); /* us. Corresponds to 250kbit/s */

  private long lastEventTime = 0;
  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private final MspMote mote;
  private final CC2420 cc2420;

  private boolean isInterfered = false;
  private boolean isTransmitting = false;
  private boolean isReceiving = false;

  private byte lastOutgoingByte;
  private byte lastIncomingByte;

  private RadioPacket lastOutgoingPacket = null;
  private RadioPacket lastIncomingPacket = null;

  public SkyByteRadio(Mote mote) {
    this.mote = (MspMote)mote;
    this.cc2420 = (CC2420) this.mote.getCPU().getChip(CC2420.class);
    if (cc2420 == null) {
      throw new IllegalStateException("Mote is not equipped with a CC2420");
    }

    cc2420.setRFListener(new RFListener() {
      int len = 0;
      int expLen = 0;
      byte[] buffer = new byte[127 + 15];
      public void receivedByte(byte data) {
        if (!isTransmitting()) {
          lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
          lastEvent = RadioEvent.TRANSMISSION_STARTED;
          isTransmitting = true;
          /*logger.debug("----- SKY TRANSMISSION STARTED -----");*/
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
        lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
        setChanged();
        notifyObservers();

        buffer[len++] = data;

        if (len == 6) {
//          System.out.println("## CC2420 Packet of length: " + data + " expected...");
          expLen = data + 6;
        }

        if (len == expLen) {
          /*logger.debug("----- SKY CUSTOM DATA TRANSMITTED -----");*/

          lastOutgoingPacket = CC2420RadioPacketConverter.fromCC2420ToCooja(buffer);
          lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
          lastEvent = RadioEvent.PACKET_TRANSMITTED;
          /*logger.debug("----- SKY PACKET TRANSMITTED -----");*/
          setChanged();
          notifyObservers();


//          System.out.println("## CC2420 Transmission finished...");

          lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
          /*logger.debug("----- SKY TRANSMISSION FINISHED -----");*/
          isTransmitting = false;
          lastEvent = RadioEvent.TRANSMISSION_FINISHED;
          setChanged();
          notifyObservers();
          len = 0;
        }
      }
    });

    cc2420.addOperatingModeListener(new OperatingModeListener() {
      public void modeChanged(Chip source, int mode) {
        if (isReceiverOn()) {
          lastEvent = RadioEvent.HW_ON;
        } else {
          /* Radio was turned off during transmission.
           * May for example happen if watchdog triggers */
          if (isTransmitting()) {
            logger.fatal("Turning off radio while transmitting");
            lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
            /*logger.debug("----- SKY TRANSMISSION FINISHED -----");*/
            isTransmitting = false;
            lastEvent = RadioEvent.TRANSMISSION_FINISHED;
            setChanged();
            notifyObservers();
          }
          lastEvent = RadioEvent.HW_OFF;
        }
        lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
        setChanged();
        notifyObservers();
      }
    });

    cc2420.setChannelListener(new CC2420.ChannelListener() {
			public void changedChannel(int channel) {
				/* XXX Currently assumes zero channel switch time */
        lastEvent = RadioEvent.UNKNOWN;
				lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
        setChanged();
        notifyObservers();
			}
		});
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return lastOutgoingPacket;
  }

  public RadioPacket getLastPacketReceived() {
    return lastIncomingPacket;
  }

  public void setReceivedPacket(RadioPacket packet) {
    /* Note:
     * Only nodes at other abstraction levels deliver full packets.
     * Sky motes would instead directly deliver bytes. */
    
    lastIncomingPacket = packet;
    /* TODO Check isReceiverOn() instead? */
    if (cc2420.getState() != CC2420.RadioState.RX_SFD_SEARCH) {
      logger.warn("Radio is turned off, dropping packet data");
      return;
    }

    /* Delivering packet bytes with delays */
    byte[] packetData = CC2420RadioPacketConverter.fromCoojaToCC2420(packet);
    long deliveryTime = getMote().getSimulation().getSimulationTime();
    for (byte b: packetData) {
      if (isInterfered()) {
        b = (byte) 0xFF;
      }

      final byte byteToDeliver = b;
      getMote().getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
        public void execute(long t) {
          super.execute(t);
          cc2420.receivedByte(byteToDeliver);
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
        cc2420.receivedByte(inputByte);
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
    cc2420.updateActiveFrequency();
    return cc2420.getActiveChannel();
  }

  public int getFrequency() {
    cc2420.updateActiveFrequency();
    return cc2420.getActiveFrequency();
  }

  public void signalReceptionStart() {
    isReceiving = true;

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    /*logger.debug("----- SKY RECEPTION STARTED -----");*/
    setChanged();
    notifyObservers();
  }

  public void signalReceptionEnd() {
    /* Deliver packet data */
    isReceiving = false;
    isInterfered = false;

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    /*logger.debug("----- SKY RECEPTION FINISHED -----");*/
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

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_INTERFERED;
    /*logger.debug("----- SKY RECEPTION INTERFERED -----");*/
    setChanged();
    notifyObservers();
  }

  public double getCurrentOutputPower() {
    return cc2420.getOutputPower();
  }

  public int getCurrentOutputPowerIndicator() {
    return cc2420.getOutputPowerIndicator();
  }

  public int getOutputPowerIndicatorMax() {
    return 31;
  }

  /**
   * Current received signal strength.
   * May differ from CC2420's internal value which is an average of the last 8 symbols.
   */
  double currentSignalStrength = 0;

  /**
   * Last 8 received signal strengths
   */
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

          cc2420.setRSSI((int) avg);
          
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

  public boolean isReceiverOn() {
    if (cc2420.getMode() == CC2420.MODE_POWER_OFF) {
      return false;
    }
    if (cc2420.getMode() == CC2420.MODE_TXRX_OFF) {
      return false;
    }
    return true;
  }
}
