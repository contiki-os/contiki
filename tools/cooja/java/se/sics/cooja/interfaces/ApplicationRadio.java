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
 * $Id: ApplicationRadio.java,v 1.9 2009/04/16 14:26:36 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * Application radio. May for example be used by Java-based mote to implement
 * radio functionality. Supports radio channels and output power functionality.
 * The mote should observe the radio for incoming radio packet data.
 *
 * @author Fredrik Osterlind
 */
public class ApplicationRadio extends Radio implements PolledBeforeActiveTicks {
  private Mote myMote;

  private static Logger logger = Logger.getLogger(ApplicationRadio.class);

  private RadioPacket packetFromMote = null;
  private RadioPacket packetToMote = null;

  private boolean isTransmitting = false;
  private boolean isReceiving = false;
  private boolean isInterfered = false;

  private long transmissionEndTime = 0;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;
  private long lastEventTime = 0;

  private boolean outPacketExists = false;
  private RadioPacket outPacket = null;
  private int outPacketDuration = -1;

  private double signalStrength = -100;
  private int radioChannel = 1;
  private double outputPower = 0;
  private int outputPowerIndicator = 100;

  public ApplicationRadio(Mote mote) {
    this.myMote = mote;
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
  }

  /* General radio support */
  public void signalReceptionStart() {
    packetToMote = null;
    if (isInterfered() || isReceiving() || isTransmitting()) {
      interfereAnyReception();
      return;
    }

    isReceiving = true;
    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    if (isInterfered() || packetToMote == null) {
      // Reset interfered flag
      isInterfered = false;

      // Reset data
      packetToMote = null;
      return;
    }

    isReceiving = false;
    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    this.setChanged();
    this.notifyObservers();
  }

  public boolean isTransmitting() {
    return isTransmitting;
  }

  public long getTransmissionEndTime() {
    return transmissionEndTime;
  }

  public boolean isReceiving() {
    return isReceiving;
  }

  public int getChannel() {
    return radioChannel;
  }

  public Position getPosition() {
    return myMote.getInterfaces().getPosition();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    if (!isInterfered()) {
      isInterfered = true;

      lastEvent = RadioEvent.RECEPTION_INTERFERED;
      lastEventTime = myMote.getSimulation().getSimulationTime();
      this.setChanged();
      this.notifyObservers();
    }
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public double getCurrentOutputPower() {
    return outputPower;
  }

  public int getOutputPowerIndicatorMax() {
    return outputPowerIndicator;
  }

  public int getCurrentOutputPowerIndicator() {
    return outputPowerIndicator;
  }

  public double getCurrentSignalStrength() {
    return signalStrength;
  }

  public void setCurrentSignalStrength(double signalStrength) {
    this.signalStrength = signalStrength;
  }

  /* Application radio support */

  /**
   * Start transmitting given packet.
   *
   * @param packet Packet data
   * @param duration Duration to transmit
   */
  public void startTransmittingPacket(RadioPacket packet, int duration) {
    outPacketExists = true;
    outPacketDuration = duration;
    outPacket = packet;
  }

  /**
   * @param i New output power indicator
   */
  public void setOutputPowerIndicator(int i) {
    outputPowerIndicator = i;
  }

  /**
   * @param p New output power
   */
  public void setOutputPower(int p) {
    outputPower = p;
  }

  /**
   * @param channel New radio channel
   */
  public void setChannel(int channel) {
    radioChannel = channel;
  }

  public void doActionsBeforeTick() {
    long currentTime = myMote.getSimulation().getSimulationTime();

    if (outPacketExists) {
      outPacketExists = false;
      isTransmitting = true;
      lastEvent = RadioEvent.TRANSMISSION_STARTED;
      lastEventTime = currentTime;
      transmissionEndTime = currentTime + outPacketDuration;
      this.setChanged();
      this.notifyObservers();

      // Deliver packet right away
      packetFromMote = outPacket;
      lastEvent = RadioEvent.PACKET_TRANSMITTED;
      this.setChanged();
      this.notifyObservers();
    }

    if (isTransmitting && currentTime >= transmissionEndTime) {
      isTransmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      lastEventTime = currentTime;
      this.setChanged();
      this.notifyObservers();
    }
  }

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update SS");

    panel.add(statusLabel);
    panel.add(lastEventLabel);
    panel.add(ssLabel);
    panel.add(updateButton);
    panel.add(Box.createVerticalStrut(3));
    panel.add(channelLabel);
    panel.add(Box.createVerticalGlue());

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ssLabel.setText("Signal strength (not auto-updated): "
            + getCurrentSignalStrength() + " dBm");
      }
    });

    final Observer observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("Transmitting");
        }
        if (isReceiving()) {
          statusLabel.setText("Receiving");
        } else {
          statusLabel.setText("Listening");
        }

        lastEventLabel.setText("Last event (time=" + lastEventTime + "): " + lastEvent);
        ssLabel.setText("Signal strength (not auto-updated): "
            + getCurrentSignalStrength() + " dBm");
      }
    };
    this.addObserver(observer);

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

  public double energyConsumption() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public Mote getMote() {
    return myMote;
  }

  public boolean isReceiverOn() {
    return true;
  }
}
