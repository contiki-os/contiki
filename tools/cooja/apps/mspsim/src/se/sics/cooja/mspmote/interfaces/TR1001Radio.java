/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: TR1001Radio.java,v 1.3 2008/03/18 13:15:41 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.*;
import javax.swing.*;
import java.awt.event.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.mspsim.core.*;
import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.mspmote.ESBMote;

/**
 * TR1001 radio interface on ESB platform. Assumes driver specifics such as
 * preambles, synchbytes, GCR coding, CRC16.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("TR1001 Radio")
public class TR1001Radio extends Radio implements USARTListener, CustomDataRadio {
  private static Logger logger = Logger.getLogger(TR1001Radio.class);

  /**
   * Minimum delay in CPU cycles between each byte fed to USART.
   */
  public static final long CYCLES_BETWEEN_BYTES = 1200; /* ~19.200 bps */

  private ESBMote mspMote;

  private boolean radioOn = true;

  private boolean transmitting = false;

  private boolean isInterfered = false;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private int lastEventTime = 0;

  private USART radioUSART = null;

  private RadioPacket packetToMote = null;

  private RadioPacket packetFromMote = null;

  /* Outgoing packet data buffer */
  private TR1001RadioByte[] outgoingData = new TR1001RadioByte[128]; /* TODO Adaptive max size */

  private int outgoingDataLength = 0;

  private int ticksSinceLastSend = -1;

  /* Incoming byte-to-packet data buffer */
  private Vector<Byte> bufferedBytes = new Vector<Byte>();

  private Vector<Long> bufferedByteDelays = new Vector<Long>();

  /* Outgoing byte data buffer */
  private TR1001RadioByte tr1001ByteFromMote = null;

  private TR1001RadioByte tr1001ByteToMote = null;

  private long transmissionStartCycles = -1;

  /* Incoming byte data buffer */
  private byte lastDeliveredByte = -1;

  private long lastDeliveredByteTimestamp = -1;

  private long lastDeliveredByteDelay = -1;

  private TR1001RadioPacketConverter tr1001PacketConverter = null;

  /**
   * Creates an interface to the TR1001 radio at mote.
   *
   * @param mote
   *          Radio's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public TR1001Radio(ESBMote mote) {
    mspMote = mote;

    /* Start listening to CPU's USART */
    IOUnit usart = mote.getCPU().getIOUnit("USART 0");
    if (usart instanceof USART) {
      radioUSART = (USART) usart;
      radioUSART.setUSARTListener(this);
    }
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return packetFromMote;
  }

  public RadioPacket getLastPacketReceived() {
    return packetToMote;
  }

  public void setReceivedPacket(RadioPacket packet) {
    packetToMote = packet;
    if (packetToMote.getPacketData() == null || packetToMote.getPacketData().length == 0) {
      logger.fatal("Received null packet");
      return;
    }

    if (isInterfered) {
      logger.fatal("Received packet when interfered");
      return;
    }

    /* Convert to TR1001 packet data */
    TR1001RadioByte[] tr1001bytes = TR1001RadioPacketConverter.fromCoojaToTR1001(packetToMote);

    /* Feed to the CPU "slowly" */
    for (TR1001RadioByte b : tr1001bytes) {
      receiveCustomData(b);
    }
  }

  /* Custom data radio support */
  public Object getLastCustomDataTransmitted() {
    return tr1001ByteFromMote;
  }

  public Object getLastCustomDataReceived() {
    return tr1001ByteToMote;
  }

  public void receiveCustomData(Object data) {
    if (data instanceof TR1001RadioByte) {
      tr1001ByteToMote = ((TR1001RadioByte) data);

      bufferedBytes.add(tr1001ByteToMote.getByte());
      bufferedByteDelays.add(tr1001ByteToMote.getDelay());
    }
  }

  /**
   * @return True if undelivered bytes exist.
   */
  public boolean hasPendingBytes() {
    return bufferedBytes.size() > 0;
  }

  /**
   * If non-delivered bytes exist, tries to deliver one byte to the CPU by
   * checking USART receive flag.
   *
   * @param cycles
   *          Current CPU cycles
   */
  public void tryDeliverNextByte(long cycles) {
    // Check that pending bytes exist
    if (!hasPendingBytes()) {
      return;
    }

    // Check if time to deliver byte
    long nextByteDelay = bufferedByteDelays.firstElement();
    if (cycles - lastDeliveredByteDelay < nextByteDelay) {
      return;
    }

    lastDeliveredByte = bufferedBytes.firstElement();

    bufferedBytes.remove(0);
    bufferedByteDelays.remove(0);

    if (radioUSART.isReceiveFlagCleared()) {
      //logger.info(nextByteDelay + " < "
      //    + (cycles - receptionStartedCycles)
      //    + ":\tDelivering 0x" + Utils.hex8(lastDeliveredByte) + " (TODO="
      //    + bufferedBytes.size() + ")");
      radioUSART.byteReceived(lastDeliveredByte);
    } else {
      /*logger.fatal(nextByteDelay + " < "
          + (cycles - receptionStartedCycles)
          + ":\tDROPPING 0x" + Utils.hex8(lastDeliveredByte) + " (TODO="
          + bufferedBytes.size() + ")");*/
    }
    lastDeliveredByteDelay = cycles;

//    /* TODO BUG: Resends last byte, interrupt lost somewhere? */
//    else if (cycles > lastDeliveredByteTimestamp + CYCLES_BETWEEN_BYTES) {
//      logger.warn("0x" + Utils.hex16((int) cycles) + ":\tRedelivering 0x"
//          + Utils.hex8(lastDeliveredByte) + " (TODO=" + bufferedBytes.size()
//          + ")");
//      radioUSART.byteReceived(lastDeliveredByte);
//      lastDeliveredByteTimestamp = cycles;
//    }
  }

  /* USART listener support */
  public void dataReceived(USART source, int data) {
    if (outgoingDataLength == 0 && !isTransmitting()) {
      /* New transmission discovered */
      //logger.debug("----- NEW MSP TRANSMISSION DETECTED -----");
      tr1001PacketConverter = new TR1001RadioPacketConverter();

      transmitting = true;
      lastEventTime = mspMote.getSimulation().getSimulationTime();
      lastEvent = RadioEvent.TRANSMISSION_STARTED;

      transmissionStartCycles = mspMote.getCPU().cycles;
      lastDeliveredByteTimestamp = transmissionStartCycles;

      this.setChanged();
      this.notifyObservers();
    }

    // Remember recent radio activity
    ticksSinceLastSend = 0;

    if (outgoingDataLength >= outgoingData.length) {
      logger.fatal("Ignoring byte due to buffer overflow");
      return;
    }

    // Deliver byte to radio medium as custom data
    lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
    tr1001ByteFromMote = new TR1001RadioByte((byte) data, mspMote.getCPU().cycles - lastDeliveredByteTimestamp);
    outgoingData[outgoingDataLength++] = tr1001ByteFromMote;
    lastDeliveredByteTimestamp = mspMote.getCPU().cycles;
    this.setChanged();
    this.notifyObservers();

    // Feed to application level immediately
    boolean finished = tr1001PacketConverter.fromTR1001ToCoojaAccumulated(tr1001ByteFromMote);
    if (finished) {
        /* Transmission finished - deliver packet immediately */
        if (tr1001PacketConverter.accumulatedConversionIsOk()) {
          packetFromMote = tr1001PacketConverter.getAccumulatedConvertedCoojaPacket();

          /* Notify observers of new prepared packet */
          /*logger.debug("----- MSP DELIVERED PACKET -----");*/
          lastEvent = RadioEvent.PACKET_TRANSMITTED;
          this.setChanged();
          this.notifyObservers();
        }

        // Reset counters and wait for next packet
        outgoingDataLength = 0;
        ticksSinceLastSend = -1;

        // Signal we are done transmitting
        transmitting = false;
        lastEvent = RadioEvent.TRANSMISSION_FINISHED;
        this.setChanged();
        this.notifyObservers();

        /*logger.debug("----- MSP TRANSMISSION ENDED -----");*/
    }
  }

  /* General radio support */
  public boolean isTransmitting() {
    return transmitting;
  }

  public boolean isReceiving() {
    return hasPendingBytes();
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public int getChannel() {
    // TODO Implement support for channels
    return 1;
  }

  public void signalReceptionStart() {
    lastEvent = RadioEvent.RECEPTION_STARTED;
    /*receptionStartedCycles = mspMote.getCPU().cycles;*/
    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    // TODO Should be done according to serial port instead
    // TODO Compare times with OS abstraction level
    if (isInterfered()) {
      isInterfered = false;
      return;
    }
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    this.setChanged();
    this.notifyObservers();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    if (!isInterfered()) {
      isInterfered = true;

      bufferedBytes.clear();
      bufferedByteDelays.clear();

      lastEvent = RadioEvent.RECEPTION_INTERFERED;
      lastEventTime = mspMote.getSimulation().getSimulationTime();
      this.setChanged();
      this.notifyObservers();
    }
  }

  public double getCurrentOutputPower() {
    // TODO Implement method
    return 1.5;
  }

  public int getOutputPowerIndicatorMax() {
    return 100;
  }

  public int getCurrentOutputPowerIndicator() {
    // TODO Implement output power indicator
    return 100;
  }

  public double getCurrentSignalStrength() {
    // TODO Implement signal strength
    return 100;
  }

  public void setCurrentSignalStrength(double signalStrength) {
    // TODO Implement signal strength
    // logger.warn("Not implemented");
  }

  public Position getPosition() {
    return mspMote.getInterfaces().getPosition();
  }

  public void doActionsBeforeTick() {
  }

  public void doActionsAfterTick() {
    // Detect transmission end due to inactivity
    if (isTransmitting() && ticksSinceLastSend > 4) {
      /* Dropping packet due to inactivity */
      packetFromMote = null;

      // Reset counters and wait for next packet
      outgoingDataLength = 0;
      ticksSinceLastSend = -1;

      // Signal we are done transmitting
      transmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      this.setChanged();
      this.notifyObservers();

      //logger.debug("----- NULL TRANSMISSION ENDED -----");
    } else if (isTransmitting() && ticksSinceLastSend >= 0) {
      // Increase counter to detect when transmission ends
      ticksSinceLastSend++;
    }
  }

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update SS");

    panel.add(statusLabel);
    panel.add(lastEventLabel);
    panel.add(ssLabel);
    panel.add(updateButton);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ssLabel.setText("Signal strength (not auto-updated): "
            + getCurrentSignalStrength() + " dBm");
      }
    });

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("Transmitting packet now!");
        } else if (isReceiving()) {
          statusLabel.setText("Receiving packet now!");
        } else if (radioOn) {
          statusLabel.setText("Listening...");
        } else {
          statusLabel.setText("HW turned off");
        }

        lastEventLabel.setText("Last event (time=" + lastEventTime + "): "
            + lastEvent);
        ssLabel.setText("Signal strength (not auto-updated): "
            + getCurrentSignalStrength() + " dBm");
      }
    });

    observer.update(null, null);

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public double energyConsumptionPerTick() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public Mote getMote() {
    return mspMote;
  }
}
