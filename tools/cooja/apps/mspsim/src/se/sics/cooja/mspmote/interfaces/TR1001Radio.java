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
 * $Id: TR1001Radio.java,v 1.15 2009/10/28 15:58:43 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayDeque;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.interfaces.CustomDataRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.mspmote.ESBMote;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.USART;
import se.sics.mspsim.core.USARTListener;

/**
 * TR1001 radio interface on ESB platform. Assumes driver specifics such as
 * preambles, synchbytes, GCR coding, CRC16.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("TR1001 Radio")
public class TR1001Radio extends Radio implements USARTListener,
    CustomDataRadio {
  private static Logger logger = Logger.getLogger(TR1001Radio.class);

  /**
   * Delay used when feeding packet data to radio chip (us). 416us corresponds
   * to 19200 bit/s with encoding.
   */
  public static final long DELAY_BETWEEN_BYTES = 3 * 416;

  private ESBMote mote;

  private boolean radioOn = true;

  private boolean isTransmitting = false;

  private boolean isReceiving = false;

  private boolean isInterfered = false;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private long lastEventTime = 0;

  private USART radioUSART = null;

  private RadioPacket lastIncomingPacket = null;

  private RadioPacket packetFromMote = null;

  /* Outgoing packet data buffer */
  private TR1001RadioByte[] outgoingData = new TR1001RadioByte[128]; /* TODO Adaptive max size */

  private int outgoingDataLength = 0;

  private int millisSinceLastSend = -1;

  /* Outgoing byte data buffer */
  private TR1001RadioByte tr1001ByteFromMote = null;

  private TR1001RadioByte lastIncomingByte = null;

  private long transmissionStartCycles = -1;

  private TR1001RadioPacketConverter tr1001PacketConverter = null;

  private double signalStrength = 0;

  /**
   * Creates an interface to the TR1001 radio at mote.
   * 
   * @param mote
   *          Radio's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public TR1001Radio(Mote mote) {
    this.mote = (ESBMote) mote;

    /* Start listening to CPU's USART */
    IOUnit usart = this.mote.getCPU().getIOUnit("USART 0");
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
    return lastIncomingPacket;
  }

  public void setReceivedPacket(RadioPacket packet) {
    lastIncomingPacket = packet;

    /* Convert to TR1001 packet data */
    TR1001RadioByte[] byteArr = TR1001RadioPacketConverter.fromCoojaToTR1001(packet);
    final ArrayDeque<TR1001RadioByte> byteList = new ArrayDeque<TR1001RadioByte>();
    for (TR1001RadioByte b : byteArr) {
      byteList.addLast(b);
    }

    /* Feed incoming bytes to radio "slowly" via time events */
    TimeEvent receiveCrosslevelDataEvent = new MoteTimeEvent(mote, 0) {
      public void execute(long t) {
        /* Stop receiving data when buffer is empty */
        if (byteList.isEmpty() || isInterfered) {
          byteList.clear();
          return;
        }

        TR1001RadioByte b = byteList.pop();
        radioUSART.byteReceived(b.getByte());

        mote.getSimulation().scheduleEvent(this, t + DELAY_BETWEEN_BYTES);
      }
    };
    receiveCrosslevelDataEvent.execute(mote.getSimulation().getSimulationTime());
  }

  /* Custom data radio support */
  public Object getLastCustomDataTransmitted() {
    return tr1001ByteFromMote;
  }

  public Object getLastCustomDataReceived() {
    return lastIncomingByte;
  }

  public void receiveCustomData(Object data) {
    if (data instanceof TR1001RadioByte) {
      lastIncomingByte = ((TR1001RadioByte) data);

      if (radioUSART.isReceiveFlagCleared()) {
        /*logger.info("----- TR1001 RECEIVED BYTE -----");*/
        radioUSART.byteReceived(lastIncomingByte.getByte());
      } else {
        logger.warn("----- TR1001 RECEIVED BYTE DROPPED -----");
      }
    }
  }

  /* USART listener support */
  public void dataReceived(USART source, int data) {
    if (outgoingDataLength == 0 && !isTransmitting()) {
      /* New transmission discovered */
      /*logger.info("----- NEW TR1001 TRANSMISSION DETECTED -----");*/
      tr1001PacketConverter = new TR1001RadioPacketConverter();

      isTransmitting = true;

      transmissionStartCycles = mote.getCPU().cycles;

      lastEvent = RadioEvent.TRANSMISSION_STARTED;
      lastEventTime = mote.getSimulation().getSimulationTime();
      this.setChanged();
      this.notifyObservers();
    }

    // Remember recent radio activity
    millisSinceLastSend = 0;
    if (!followupTransmissionEvent.isScheduled()) {
      mote.getSimulation().scheduleEvent(
          followupTransmissionEvent,
          mote.getSimulation().getSimulationTime() + Simulation.MILLISECOND);
    }

    if (outgoingDataLength >= outgoingData.length) {
      logger.warn("----- TR1001 DROPPING OUTGOING BYTE (buffer overflow) -----");
      return;
    }

    // Deliver byte to radio medium as custom data
    /* logger.debug("----- TR1001 SENT BYTE -----"); */
    lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
    tr1001ByteFromMote = new TR1001RadioByte((byte) data);
    this.setChanged();
    this.notifyObservers();

    outgoingData[outgoingDataLength++] = tr1001ByteFromMote;

    // Feed to application level immediately
    boolean finished = tr1001PacketConverter
        .fromTR1001ToCoojaAccumulated(tr1001ByteFromMote);
    if (finished) {
      /* Transmission finished - deliver packet immediately */
      if (tr1001PacketConverter.accumulatedConversionIsOk()) {
        packetFromMote = tr1001PacketConverter.getAccumulatedConvertedCoojaPacket();

        /* Notify observers of new prepared packet */
        /* logger.info("----- TR1001 DELIVERED PACKET -----"); */
        lastEvent = RadioEvent.PACKET_TRANSMITTED;
        this.setChanged();
        this.notifyObservers();
      }

      // Reset counters and wait for next packet
      outgoingDataLength = 0;
      millisSinceLastSend = -1;

      // Signal we are done transmitting
      isTransmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      this.setChanged();
      this.notifyObservers();

      /* logger.info("----- TR1001 TRANSMISSION ENDED -----"); */
    }
  }

  public void stateChanged(int state) {
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
    return -1;
  }

  public void signalReceptionStart() {
    lastEvent = RadioEvent.RECEPTION_STARTED;
    isReceiving = true;
    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    isInterfered = false;
    isReceiving = false;
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
      lastIncomingPacket = null;

      lastEventTime = mote.getSimulation().getSimulationTime();
      lastEvent = RadioEvent.RECEPTION_INTERFERED;
      /*logger.info("----- TR1001 RECEPTION INTERFERED -----");*/
      this.setChanged();
      this.notifyObservers();
    }
  }

  public double getCurrentOutputPower() {
    // TODO Implement method
    return 0;
  }

  public int getOutputPowerIndicatorMax() {
    return 100;
  }

  public int getCurrentOutputPowerIndicator() {
    // TODO Implement output power indicator
    return 100;
  }

  public double getCurrentSignalStrength() {
    return signalStrength;
  }

  public void setCurrentSignalStrength(double signalStrength) {
    this.signalStrength = signalStrength;
  }

  public Position getPosition() {
    return mote.getInterfaces().getPosition();
  }

  private TimeEvent followupTransmissionEvent = new MoteTimeEvent(mote, 0) {
    public void execute(long t) {

      if (isTransmitting()) {
        millisSinceLastSend++;

        // Detect transmission end due to inactivity
        if (millisSinceLastSend > 5) {
          /* Dropping packet due to inactivity */
          packetFromMote = null;

          /* Reset counters and wait for next packet */
          outgoingDataLength = 0;
          millisSinceLastSend = -1;

          /* Signal we are done transmitting */
          isTransmitting = false;
          lastEvent = RadioEvent.TRANSMISSION_FINISHED;
          TR1001Radio.this.setChanged();
          TR1001Radio.this.notifyObservers();

          /*logger.debug("----- NULL TRANSMISSION ENDED -----");*/
        }

        /* Reschedule as long as node is transmitting */
        mote.getSimulation().scheduleEvent(this, t + Simulation.MILLISECOND);
      }
    }
  };

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel wrapperPanel = new JPanel(new BorderLayout());
    JPanel panel = new JPanel(new GridLayout(5, 2));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("ALL CHANNELS (-1)");
    final JLabel powerLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update");

    panel.add(new JLabel("STATE:"));
    panel.add(statusLabel);

    panel.add(new JLabel("LAST EVENT:"));
    panel.add(lastEventLabel);

    panel.add(new JLabel("CHANNEL:"));
    panel.add(channelLabel);

    panel.add(new JLabel("OUTPUT POWER:"));
    panel.add(powerLabel);

    panel.add(new JLabel("SIGNAL STRENGTH:"));
    JPanel smallPanel = new JPanel(new GridLayout(1, 2));
    smallPanel.add(ssLabel);
    smallPanel.add(updateButton);
    panel.add(smallPanel);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        powerLabel.setText(getCurrentOutputPower() + " dBm (indicator="
            + getCurrentOutputPowerIndicator() + "/"
            + getOutputPowerIndicatorMax() + ")");
        ssLabel.setText(getCurrentSignalStrength() + " dBm");
      }
    });

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("transmitting");
        } else if (isReceiving()) {
          statusLabel.setText("receiving");
        } else if (radioOn) {
          statusLabel.setText("listening for traffic");
        } else {
          statusLabel.setText("HW off");
        }

        lastEventLabel.setText(lastEvent + " @ time=" + lastEventTime);

        powerLabel.setText(getCurrentOutputPower() + " dBm (indicator="
            + getCurrentOutputPowerIndicator() + "/"
            + getOutputPowerIndicatorMax() + ")");
        ssLabel.setText(getCurrentSignalStrength() + " dBm");
      }
    });

    observer.update(null, null);

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    wrapperPanel.add(BorderLayout.NORTH, panel);
    return wrapperPanel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public double energyConsumption() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public Mote getMote() {
    return mote;
  }

  public boolean isReceiverOn() {
    /* TODO Implement me */
    return true;
  }
}
