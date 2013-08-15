/*
 * Copyright (c) 2008-2012, Swedish Institute of Computer Science.
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
import se.sics.mspsim.chip.ChannelListener;
import se.sics.mspsim.chip.RFListener;
import se.sics.mspsim.chip.Radio802154;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.OperatingModeListener;

/**
 * MSPSim 802.15.4 radio to COOJA wrapper.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("IEEE 802.15.4 Radio")
public class Msp802154Radio extends Radio implements CustomDataRadio {
  private static Logger logger = Logger.getLogger(Msp802154Radio.class);

  /**
   * Cross-level:
   * Inter-byte delay for delivering cross-level packet bytes.
   */
  public static final long DELAY_BETWEEN_BYTES =
    (long) (1000.0*Simulation.MILLISECOND/(250000.0/8.0)); /* us. Corresponds to 250kbit/s */

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private final MspMote mote;
  private final Radio802154 radio;

  private boolean isInterfered = false;
  private boolean isTransmitting = false;
  private boolean isReceiving = false;

  private byte lastOutgoingByte;
  private byte lastIncomingByte;

  private RadioPacket lastOutgoingPacket = null;
  private RadioPacket lastIncomingPacket = null;

  public Msp802154Radio(Mote m) {
    this.mote = (MspMote)m;
    this.radio = this.mote.getCPU().getChip(Radio802154.class);
    if (radio == null) {
      throw new IllegalStateException("Mote is not equipped with an IEEE 802.15.4 radio");
    }

    radio.addRFListener(new RFListener() {
      int len = 0;
      int expLen = 0;
      byte[] buffer = new byte[127 + 15];
      public void receivedByte(byte data) {
        if (!isTransmitting()) {
          lastEvent = RadioEvent.TRANSMISSION_STARTED;
          isTransmitting = true;
          len = 0;
          /*logger.debug("----- 802.15.4 TRANSMISSION STARTED -----");*/
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

        buffer[len++] = data;

        if (len == 6) {
//          System.out.println("## CC2420 Packet of length: " + data + " expected...");
          expLen = data + 6;
        }

        if (len == expLen) {
          /*logger.debug("----- 802.15.4 CUSTOM DATA TRANSMITTED -----");*/

          lastOutgoingPacket = CC2420RadioPacketConverter.fromCC2420ToCooja(buffer);
          lastEvent = RadioEvent.PACKET_TRANSMITTED;
          /*logger.debug("----- 802.15.4 PACKET TRANSMITTED -----");*/
          setChanged();
          notifyObservers();

          /*logger.debug("----- 802.15.4 TRANSMISSION FINISHED -----");*/
          isTransmitting = false;
          lastEvent = RadioEvent.TRANSMISSION_FINISHED;
          setChanged();
          notifyObservers();
          len = 0;
        }
      }
    });

    radio.addOperatingModeListener(new OperatingModeListener() {
      public void modeChanged(Chip source, int mode) {
        if (radio.isReadyToReceive()) {
          lastEvent = RadioEvent.HW_ON;
          setChanged();
          notifyObservers();
        } else {
          radioOff();
        }
      }
    });

    radio.addChannelListener(new ChannelListener() {
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
      /*logger.debug("----- 802.15.4 PACKET TRANSMITTED -----");*/
      setChanged();
      notifyObservers();

      /* Register that transmission ended in radio medium */
      /*logger.debug("----- 802.15.4 TRANSMISSION FINISHED -----");*/
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
    /* Note:
     * Only nodes at other abstraction levels deliver full packets.
     * MSPSim motes with 802.15.4 radios would instead directly deliver bytes. */

    lastIncomingPacket = packet;
    /* TODO Check isReceiverOn() instead? */
    if (!radio.isReadyToReceive()) {
      logger.warn("Radio receiver not ready, dropping packet data");
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
          radio.receivedByte(byteToDeliver);
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
        radio.receivedByte(inputByte);
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
    return radio.getActiveChannel();
  }

  public int getFrequency() {
    return radio.getActiveFrequency();
  }

  public void signalReceptionStart() {
    isReceiving = true;

    lastEvent = RadioEvent.RECEPTION_STARTED;
    /*logger.debug("----- 802.15.4 RECEPTION STARTED -----");*/
    setChanged();
    notifyObservers();
  }

  public void signalReceptionEnd() {
    /* Deliver packet data */
    isReceiving = false;
    isInterfered = false;

    lastEvent = RadioEvent.RECEPTION_FINISHED;
    /*logger.debug("----- 802.15.4 RECEPTION FINISHED -----");*/
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
    /*logger.debug("----- 802.15.4 RECEPTION INTERFERED -----");*/
    setChanged();
    notifyObservers();
  }

  public double getCurrentOutputPower() {
    return radio.getOutputPower();
  }

  public int getCurrentOutputPowerIndicator() {
    return radio.getOutputPowerIndicator();
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

          radio.setRSSI((int) avg);

          rssiLastCounter--;
          if (rssiLastCounter > 0) {
            mote.getSimulation().scheduleEvent(this, t+DELAY_BETWEEN_BYTES/2);
          }
        }
      }, mote.getSimulation().getSimulationTime());
    }
    rssiLastCounter = 8;
  }
  
  
  /* This will set the CORR-value of the CC2420
   * 
   * @see se.sics.cooja.interfaces.Radio#setLQI(int)
   */
  public void setLQI(int lqi){
	  radio.setLQI(lqi);
  }

  public int getLQI(){
	  return radio.getLQI();
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
    if (radio.isReadyToReceive()) {
      return true;
    }
    if (radio.getMode() == CC2420.MODE_POWER_OFF) {
      return false;
    }
    if (radio.getMode() == CC2420.MODE_TXRX_OFF) {
      return false;
    }
    return true;
  }
  
  public boolean canReceiveFrom(CustomDataRadio radio) {
    if (radio.getClass().equals(this.getClass())) {
      return true;
    }
    return false;
  }
}
